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

inline JSONNODE * vNode(VbsData *data)
{
    JSONNODE *node = json_new(JSON_NODE);
    json_set_name(node, data->name.to8Bit().c_str());

    json_push_back(node, json_new_i("start", data->beg));
    json_push_back(node, json_new_i("length", data->len));
    json_push_back(node, json_new_i("end", data->end));

    return node;
}

int Id3TagJson::verbose()
{
    JSONNODE *json = json_new(JSON_NODE);

    ID3v2::Tag *v2tag = this->mpgfile->ID3v2Tag();
    if (v2tag)
    {
        if (v2tag->frameList().size() == 0) goto NOTAGS;

        long tagbeg;
        long hdrlen;

        vector<VbsData *> vbs;

        tagbeg = this->mpgfile->firstFrameOffset() -
            v2tag->header()->completeTagSize();

        hdrlen = 10;  // Fixed IDV2 header length
        if (v2tag->header()->extendedHeader())
        {
            hdrlen += v2tag->extendedHeader()->size();
        }

        JSONNODE *id3 = json_new(JSON_NODE);
        json_set_name(id3, "ID3v2");
        

        json_push_back(id3, json_new_i("start", tagbeg));
        json_push_back(id3, json_new_i( "length", 
            v2tag->header()->completeTagSize()));
        json_push_back(id3, json_new_i("end", 
            (tagbeg + v2tag->header()->completeTagSize() - 1)));

        long frmbeg = tagbeg + hdrlen ;

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

            // Search for the first occurance of the string VBS Node
            int i = 0;
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
                    if (lit == l.end()) break;
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
        json_push_back(id3, val);
        json_push_back(json, id3);
    }
NOTAGS:
    cout << json_write_formatted(json) << endl;
    return 0;
}




int Id3TagJson::literal()
{
    JSONNODE *l = this->genLitTree();

    cout << json_write_formatted(l) << endl;
    json_delete(l);
    
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
    if (v1tag)
    {
        JSONNODE *j_id3v1 = json_new(JSON_NODE);
        json_set_name(j_id3v1, "ID3v1");

        if (!v1tag->title().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("title", v1tag->title().to8Bit().c_str()));

        if (!v1tag->artist().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("artist",v1tag->artist().to8Bit().c_str()));

        if (!v1tag->album().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("album", v1tag->album().to8Bit().c_str()));

        if (!v1tag->comment().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("comment", v1tag->comment().to8Bit().c_str()));

        if (!v1tag->genre().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("genre", v1tag->genre().to8Bit().c_str()));

        if (v1tag->year() != 0)
            json_push_back(j_id3v1, 
                json_new_i("year", v1tag->year()));

        if (v1tag->track() != 0)
            json_push_back(j_id3v1, 
                json_new_i("track", v1tag->track()));

        if (json_empty(j_id3v1))
            json_delete(j_id3v1);
        else
            json_push_back(json, j_id3v1);
    }

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
