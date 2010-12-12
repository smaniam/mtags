#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>

#include <tbytevector.h>
#include <tstringlist.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v1tag.h>
#include <id3v2extendedheader.h>
#include <frames/attachedpictureframe.h>

#include "id3tagjson.h"
#include "id3stringdefs.h"
#include <mhash.h>
#define BUFFERSIZE (ID3_B64_BFR_SZ * 2)
#include <b64/encode.h>


using namespace TagLib;
using namespace base64;
using std::vector;

class VbsData
{
    public:
        String name;
        long   beg;
        long   len;
        long   end;
};

inline long Id3TagJson::locateID3v1()
{
    std::ifstream f(fname.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    if (f.is_open())
    {
        std::ifstream::pos_type size = f.tellg();
        if (size < 128) 
            return -1;
        f.seekg(-128, std::ios::end);
        char tagid[4];
        tagid[3] = '\0';
        f.read(tagid, 3);
        f.close();
        if (string(tagid) == string("TAG"))
            return ((long)size - 128);
    }
    return -1;
}

inline long Id3TagJson::locateAPE(bool id3v1)
{
    std::ifstream f(fname.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    if (f.is_open())
    {
        long minsz;

        minsz = (id3v1 ? 160 : 32);
        std::ifstream::pos_type size = f.tellg();
        if (size < minsz) 
            return -1;

        long seekloc = 0 - minsz;
        f.seekg(seekloc, std::ios::end);
        char tagid[9];
        tagid[8] = '\0';
        f.read(tagid, 8);
        f.close();
        if (string(tagid) == string("APETAGEX"))
            return ((long)size - minsz);
    }
    return -1;
}

inline long Id3TagJson::getFileSz()
{
    std::ifstream f(fname.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    if (f.is_open())
    {
        std::ifstream::pos_type size = f.tellg();
        f.close();
        return (size);
    }
    return -1;
}

inline void Id3TagJson::delnewline(char *buf, int len) 
{
    int i = 0, o = 0;
    
    while (i < len)
    {
        if (buf[i] == '\n') i++;
        else if (o < i) buf[o++] = buf[i++];
        else i++, o++;
    }
    buf[o] = '\n';
}

inline JSONNODE * vNode(VbsData *data)
{
    JSONNODE *node = json_new(JSON_NODE);
    json_set_name(node, data->name.to8Bit().c_str());

    json_push_back(node, json_new_i("start", data->beg));
    json_push_back(node, json_new_i("length", data->len));
    json_push_back(node, json_new_i("end", data->end));

    return node;
}

void Id3TagJson::getLocation(long &v2beg, long & v2len, long & dbeg, 
    long & dlen, long & v1beg, long & v1len, long & apebeg, long & apelen)
{
    v2beg = -1;
    v2len = 0;
    ID3v2::Tag *v2tag = this->mpgfile->ID3v2Tag();
    if ((v2tag) && (v2tag->frameList().size() != 0))
    {
        v2beg = this->mpgfile->firstFrameOffset() -
            v2tag->header()->completeTagSize();
        v2len = v2tag->header()->completeTagSize();
    }


    long filesz = getFileSz();

    dbeg = this->mpgfile->firstFrameOffset();
    dlen = filesz - dbeg -1;

    v1beg  = locateID3v1();
    if (v1beg != -1)
    {
       v1len = 128;
       dlen  = v1beg - dbeg;
    }
    else
       v1len = 0;

    apebeg = locateAPE((v1beg != -1));
    if (apebeg != -1)
    {
       apelen = 32;
       dlen   = apebeg - dbeg;
    }
    else
       apelen = 0;
}

int Id3TagJson::verbose()
{
    long   v2beg, v2len, dbeg, dlen, v1beg, v1len, apebeg, apelen;

    getLocation(v2beg, v2len, dbeg, dlen, v1beg, v1len, apebeg, apelen);
    
    JSONNODE *json = json_new(JSON_NODE);

    ID3v2::Tag *v2tag = this->mpgfile->ID3v2Tag();
    if (v2tag)
    {
        if (v2tag->frameList().size() == 0) goto NOTAGS;

        long hdrlen;

        vector<VbsData *> vbs;

        hdrlen = 10;  // Fixed IDV2 header length
        if (v2tag->header()->extendedHeader())
        {
            hdrlen += v2tag->extendedHeader()->size();
        }

        JSONNODE *id3 = json_new(JSON_NODE);
        json_set_name(id3, "ID3v2");
        

        json_push_back(id3, json_new_i("start", v2beg));
        json_push_back(id3, json_new_i( "length", v2len));
        json_push_back(id3, json_new_i("end", (v2beg + v2len - 1)));

        long frmbeg = v2beg + hdrlen ;

        //cout << "Tag Size: " << v2tag->header()->completeTagSize() << "\n";

        ID3v2::FrameList::ConstIterator it = v2tag->frameList().begin();
        for(; it != v2tag->frameList().end(); it++)
        {
            VbsData *data = new VbsData();

            data->name = (*it)->frameID();
            data->beg  = frmbeg;
            data->len  = (*it)->size() +
                ((v2tag->header()->majorVersion() > 2) ? 10:6);
            data->end  = data->beg + data->len - 1;

            //std::cout << data->name << " " << data->beg << " " <<
            //    data->len << " " << data->end << "\n";

            frmbeg += data->len;

            vbs.push_back(data);
        }


        JSONNODE *val = json_new(JSON_NODE);
        json_set_name(val, "value");

        TagLib::StringList  idlst;

        it = v2tag->frameList().begin();
        for(; it != v2tag->frameList().end(); it++)
        {
            string name;
            string value;
            
            String id = (*it)->frameID();
            if (idlst.contains(id)) continue;
            

            idlst.append(id);
            name = id.to8Bit();

            // Search for the first occurance of the string in VBS vector
            unsigned int i = 0;
            while (!(id == vbs[i]->name) && i < vbs.size()) i++;
            
            ID3v2::FrameList l = 
                v2tag->frameListMap()[name.c_str()];
            if (l.size() > 1)
            {
                JSONNODE *arr = json_new(JSON_ARRAY);
                json_set_name(arr, id.to8Bit().c_str());
                
                ID3v2::FrameList::ConstIterator lit = l.begin();
                for (;lit != l.end(); lit++)
                {
                    String nid = (*lit)->frameID();;
                    JSONNODE *node = vNode(vbs[i]);
                    json_push_back(arr, node);
                    i++;
                    while ((i < vbs.size()) && !(nid == vbs[i]->name)) i++;
                }
                json_push_back(val, arr);
            }
            else
            {
                JSONNODE *node = vNode(vbs[i]);
                json_push_back(val, node);
            }
        }

        // Cleanup memory
        while (!vbs.empty())
        {
            delete vbs.back();
            vbs.pop_back();
        }

        json_push_back(id3, val);
        json_push_back(json, id3);
    }
NOTAGS:

    VbsData vbs;

    if (v1beg != -1)
    {
        vbs.name = String("ID3v1");
        vbs.beg  = v1beg;
        vbs.len  = v1len;
        vbs.end  = v1beg + v1len - 1;
        JSONNODE  *v1 = vNode(&vbs);

        JSONNODE *val = json_new(JSON_NODE);
        json_set_name(val, "value");

        vbs.name  = String("Title");
        vbs.beg  += 3;
        vbs.len   = 30;
        vbs.end   = vbs.beg + vbs.len - 1;
        JSONNODE  *node = vNode(&vbs);
        json_push_back(val, node);

        vbs.name  = String("Artist");
        vbs.beg  += vbs.len;
        vbs.len   = 30;
        vbs.end  += vbs.len;
        node      = vNode(&vbs);
        json_push_back(val, node);

        vbs.name  = String("Album");
        vbs.beg  += vbs.len;
        vbs.len   = 30;
        vbs.end  += vbs.len;
        node      = vNode(&vbs);
        json_push_back(val, node);

        vbs.name  = String("Year");
        vbs.beg  += vbs.len;
        vbs.len   = 4;
        vbs.end  += vbs.len;
        node      = vNode(&vbs);
        json_push_back(val, node);

        vbs.name  = String("Comment");
        vbs.beg  += vbs.len;
        vbs.len   = 30;
        vbs.end  += vbs.len;
        node      = vNode(&vbs);
        json_push_back(val, node);

        vbs.name  = String("Genre");
        vbs.beg  += vbs.len;
        vbs.len   = 1;
        vbs.end  += vbs.len;
        node      = vNode(&vbs);
        json_push_back(val, node);

        json_push_back(v1, val);
        json_push_back(json, v1);

    }

    vbs.name = String("DATA");
    vbs.beg  = dbeg;
    vbs.len  = dlen;
    vbs.end  = dbeg + dlen - 1;
    JSONNODE *dnode = vNode(&vbs);
    json_push_back(json, dnode);

    cout << json_write_formatted(json) << endl;
    json_delete(json);
    return 0;
}




int Id3TagJson::literal()
{
    JSONNODE *l = this->genLitTree();

    cout << json_write_formatted(l) << endl;
    json_delete(l);
    
    return 0;
}






int Id3TagJson::checksum()
{
    JSONNODE *c = this->getChkSum();

    if (c == NULL)  return 1;

    JSONNODE * v = json_new(JSON_NODE);
    json_push_back(v, c);
    cout << json_write_formatted(v) << endl;
    json_delete(v);

    return 0;
}







JSONNODE * Id3TagJson::genLitTree()
{
    ID3v2::Tag *v2tag = this->mpgfile->ID3v2Tag();
    JSONNODE *json = json_new(JSON_NODE);

    if (v2tag)
    {

        if (v2tag->frameList().size() == 0) goto V2DONE;
        
        JSONNODE *j_id3v2 = json_new(JSON_NODE);
        json_set_name(j_id3v2, "ID3v2");

        TagLib::StringList  idlst;

        ID3v2::FrameList::ConstIterator it = v2tag->frameList().begin();
        for(; it != v2tag->frameList().end(); it++)
        {
            string name;
            string value;
            
            String id = (*it)->frameID();
            if (idlst.contains(id)) continue;
            
            idlst.append(id);
            name = id.to8Bit();
            
            ID3v2::FrameList l = 
                v2tag->frameListMap()[name.c_str()];
            if (l.size() > 1)
            {
                JSONNODE *arr = json_new(JSON_ARRAY);
                json_set_name(arr, id.to8Bit().c_str());
                
                ID3v2::FrameList::ConstIterator lit = l.begin();
                for (;lit != l.end(); lit++)
                {
                    JSONNODE *node = getFrmLitVal((*lit));
                    json_push_back(arr, node);
                }
                json_push_back(j_id3v2, arr);
            }
            else
            {
                JSONNODE *node = getFrmLitVal((*it));
                json_push_back(j_id3v2, node);
            }
        }

        json_push_back(json, j_id3v2);
    }

V2DONE:
    
    ID3v1::Tag *v1tag = this->mpgfile->ID3v1Tag();
    if ((v1tag) && (locateID3v1() != -1))
    {
        JSONNODE *j_id3v1 = json_new(JSON_NODE);
        json_set_name(j_id3v1, "ID3v1");

        json_push_back(j_id3v1, 
            json_new_a("Title", v1tag->title().to8Bit().c_str()));

        json_push_back(j_id3v1, 
            json_new_a("Artist",v1tag->artist().to8Bit().c_str()));

        json_push_back(j_id3v1, 
            json_new_a("Album", v1tag->album().to8Bit().c_str()));

        json_push_back(j_id3v1, 
            json_new_a("Comment", v1tag->comment().to8Bit().c_str()));

        json_push_back(j_id3v1, 
            json_new_a("Genre", v1tag->genre().to8Bit().c_str()));

        json_push_back(j_id3v1, 
            json_new_i("Year", v1tag->year()));

        json_push_back(j_id3v1, 
            json_new_i("Track", v1tag->track()));

        json_push_back(json, j_id3v1);
    }

    JSONNODE *chksum = getChkSum();
    if (chksum != NULL)
        json_push_back(json, chksum);

    return json;
}   




JSONNODE * Id3TagJson::getFrmLitVal(TagLib::ID3v2::Frame *frm)
{
    String id = frm->frameID();
    JSONNODE *json;

    if (id == String("APIC"))
    {
        json = getPic(frm);
        return json;
    }
    string name = id.to8Bit();
    string value = frm->toString().to8Bit();
    
    json = json_new(JSON_STRING);
    json_set_name(json, name.c_str());
    json_set_a(json, value.c_str());
    return json;
}





JSONNODE * Id3TagJson::getPic(TagLib::ID3v2::Frame *frm)
{
        JSONNODE *json = json_new(JSON_NODE);
        json_set_name(json, "APIC");
        ID3v2::AttachedPictureFrame *picfrm = 
            (ID3v2::AttachedPictureFrame *)frm;
        
        json_push_back(json, json_new_a("type", pictype[picfrm->type()]));
        json_push_back(json,
            json_new_a("mime", picfrm->mimeType().to8Bit().c_str()));
        if (picfrm->description().size() != 0)
            json_push_back(json, json_new_a("description",
                picfrm->description().to8Bit().c_str()));
        
        if (this->art)
        {
            ByteVector b = picfrm->picture();
            if (!pixpath.empty())
            {
                char tmp[512];
                char *bname;
                
                strcpy (tmp, fname.c_str());
                bname = basename(tmp);

                string img = pixpath + "/" + bname + "." +
                    pictype[picfrm->type()] + ".jpg";

                std::ofstream f(img.c_str(),
                    std::ios::out |std::ios::binary|std::ios::trunc);
                if (f.is_open())
                {
                    f.write(b.data(), b.size());
                    f.close();
                    json_push_back(json, json_new_a("data",
                        img.c_str()));
                }
                else
                {
                    std::cerr << "Unable to open file: " << img << "\n";
                }
            }
            else
            {
                base64_encodestate  inst;
                char obuf[ID3_B64_BFR_SZ*2];
                string b64out;
                int clen;
                
                const char *inp = b.data();
                b64out.clear();
                
                base64_init_encodestate(&inst);
                
                int blks = b.size() / ID3_B64_BFR_SZ;
                for (int i = 0; i < blks; i++)
                {
                    clen = base64_encode_block(
                        &inp[i*ID3_B64_BFR_SZ],
                        ID3_B64_BFR_SZ, obuf, &inst);
                    delnewline(obuf, clen);
                    b64out.append(obuf);
                }
                clen = base64_encode_block(&inp[blks*ID3_B64_BFR_SZ],
                    b.size() % ID3_B64_BFR_SZ,
                    obuf, &inst);
                delnewline(obuf, clen);
                b64out.append(obuf);
                clen = base64_encode_blockend(obuf, &inst);
                delnewline(obuf, clen);
                b64out.append(obuf);

                json_push_back(json, json_new_a("data",
                        b64out.c_str()));
            }
        }
        return json;
}

int Id3TagJson::albumart()
{
    ID3v2::Tag *v2tag = this->mpgfile->ID3v2Tag();
    JSONNODE *json;

    if (v2tag)
    {
        if (v2tag->frameList().size() == 0) goto NOPICS;
        
        ID3v2::FrameList::ConstIterator it = v2tag->frameList().begin();
        for(; it != v2tag->frameList().end(); it++)
        {
            string name;
            string value;
            
            String id = (*it)->frameID();

            if (!(id == String("APIC"))) continue;

            json = json_new(JSON_NODE);
            name = id.to8Bit();
            
            ID3v2::FrameList l = 
                v2tag->frameListMap()[name.c_str()];
            if (l.size() > 1)
            {
                JSONNODE *arr = json_new(JSON_ARRAY);
                json_set_name(arr, id.to8Bit().c_str());
                
                ID3v2::FrameList::ConstIterator lit = l.begin();
                for (;lit != l.end(); lit++)
                {
                    JSONNODE *node = getFrmLitVal((*lit));
                    json_push_back(arr, node);
                }
                json_push_back(json, arr);
            }
            else
            {
                JSONNODE *node = getFrmLitVal((*it));
                json_push_back(json, node);
            }

            cout << json_write_formatted(json) << endl;
            json_delete(json);
        }
    }

NOPICS:
    return 0;
}   






JSONNODE * Id3TagJson::getChkSum()
{
    MHASH           mdd  = NULL;
    MHASH           shd  = NULL;
    unsigned char   *md5res;
    unsigned char   *sha1res;
    unsigned char   bfr[ID3_CHKSUM_BFR_SZ];
    unsigned char   md5sum[64];
    unsigned char   sha1sum[64];

    long v2beg, v2len, dbeg, dlen, v1beg, v1len, apebeg, apelen;

    if (!(md5 || sha1)) return NULL;

    std::ifstream f(fname.c_str(), std::ios::in|std::ios::binary);

    if (!f.is_open()) return NULL;

    if (md5)
    {
        mdd = mhash_init(MHASH_MD5);
        if (mdd == MHASH_FAILED) return NULL;
    }

    if (sha1)
    {
        shd = mhash_init(MHASH_SHA1);
        if (shd == MHASH_FAILED) return NULL;
    }

    getLocation(v2beg, v2len, dbeg, dlen, v1beg, v1len, apebeg, apelen);
    if (dlen == 0) return NULL;

    f.seekg(dbeg, std::ios::beg);
    long   cnt = dlen / ID3_CHKSUM_BFR_SZ;
    long   rmn = dlen % ID3_CHKSUM_BFR_SZ;

    while ((f.read((char *)bfr, ID3_CHKSUM_BFR_SZ)) && (cnt > 0))
    {
        if (md5) mhash(mdd, bfr, ID3_CHKSUM_BFR_SZ);
        if (sha1) mhash(shd, bfr, ID3_CHKSUM_BFR_SZ);
        cnt--;
    }

    if (f.read((char *)bfr, rmn))
    {
        if (md5) mhash(mdd, bfr, rmn);
        if (sha1) mhash(shd, bfr, rmn);
    }
    f.close();

    JSONNODE  *node = json_new(JSON_NODE);
    json_set_name(node, "stream");

    if (md5)
    {
        md5res = (unsigned char *) mhash_end(mdd);
        for (int i = 0; i < (int)mhash_get_block_size(MHASH_MD5); i++) 
        {
            // printf("%.2x", md5res[i]);
            sprintf((char *)&md5sum[i<<1], "%.2x", md5res[i]);
        }

        json_push_back(node, json_new_a("md5sum", (const char*)md5sum));
    }

    if (sha1)
    {
        sha1res = (unsigned char *) mhash_end(shd);
        for (int i = 0; i < (int)mhash_get_block_size(MHASH_SHA1); i++) 
        {
            // printf("%.2x", sha1res[i]);
            sprintf((char *)&sha1sum[i<<1], "%.2x", sha1res[i]);
        }

        json_push_back(node, json_new_a("sha1sum", (const char*)sha1sum));
    }

    if (json_empty(node))
    {
        json_delete(node);
        node = NULL;
    }

    return node;
}
