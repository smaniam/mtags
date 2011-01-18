#include <iostream>
#include <fstream>
#include <string.h>
#include <vector>
#include <map>

#include "imgtagjson.h"

#include <mhash.h>
#define BUFFERSIZE (IMG_B64_BFR_SZ * 2)
#include <b64/encode.h>
#include "exiv2.hpp"
extern "C" {
#include "jhead.h"
}


using namespace base64;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::map;
using std::cerr;

class VbsData
{
    public:
        string name;
        long   beg;
        long   len;
        long   end;
};

static const char *TTSTR[] = { "exif", "xmp", "iptc", "jfif", "stream" };


inline long ImgTagJson::getFileSz()
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

inline void ImgTagJson::delnewline(char *buf, int len) 
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
    json_set_name(node, data->name.c_str());

    json_push_back(node, json_new_i("start", data->beg));
    json_push_back(node, json_new_i("length", data->len));
    json_push_back(node, json_new_i("end", data->end));

    return node;
}

int ImgTagJson::literal()
{
    Exiv2::Image::AutoPtr image;
    try
    {
        image = Exiv2::ImageFactory::open(fname);
    }
    catch (Exiv2::Error& e)
    {
        cerr << "Invalid file: " << fname << endl;
        return 6;
    }
    if (image.get() == 0) return 1;

    image->readMetadata();

    JSONNODE *l = json_new(JSON_NODE);

    if (exif)
    {
        JSONNODE *e = this->genLitExif(image);
        if (e != NULL) json_push_back(l, e);
    }

    if (iptc)
    {
        JSONNODE *i = this->genLitIptc(image);
        if (i != NULL) json_push_back(l, i);
    }

    if (xmp)
    {
        JSONNODE *x = this->genLitXmp(image);
        if (x != NULL) json_push_back(l, x);
    }

    JSONNODE *chksum = getChkSum();
    if (chksum != NULL)
        json_push_back(l, chksum);

    cout << json_write_formatted(l) << endl;
    json_delete(l);
    
    return 0;
}

typedef map<string, JSONNODE *> JMAP;

JSONNODE *ImgTagJson::genLitExif(const Exiv2::Image::AutoPtr & image)
{
    //const Exiv2::ExifData &data = image->exifData();
    Exiv2::ExifData &data = image->exifData();
    data.sortByKey();

    if (data.empty()) return NULL;

    Exiv2::ExifData::const_iterator end = data.end();

    JMAP *grpmap = new JMAP();

    JSONNODE *tree = json_new(JSON_NODE);
    json_set_name(tree, "exif");
    for (Exiv2::ExifData::const_iterator i = data.begin(); i != end; i++)
    {
        JSONNODE *grp;

        if (grpmap->find(i->groupName()) == grpmap->end())
        {
            grp = json_new(JSON_NODE);
            json_set_name(grp, i->groupName().c_str());
            grpmap->insert(JMAP::value_type(i->groupName(), grp)); 
        }
        else
           grp = (grpmap->find(i->groupName()))->second;

        Exiv2::ExifData::const_iterator nxt = i;
        nxt++;
        if ((nxt != data.end()) && (i->key() == nxt->key()))
        {
            //cout << "Array Elem! " << i->key() << endl;
            JSONNODE *arr = json_new(JSON_ARRAY);
            json_set_name(arr, i->tagName().c_str());
            json_push_back(arr, json_new_a((i->tagName()).c_str(),
                (i->print()).c_str()));

            while ((nxt != data.end()) && (nxt->key() == i->key()))
            {
                json_push_back(arr, json_new_a((nxt->tagName()).c_str(),
                    (nxt->print()).c_str()));
                nxt++;
            }
            json_push_back(grp, arr);
            if (nxt == data.end()) break;
            nxt--;
            i = nxt;
        }
        else
        {
            json_push_back(grp, json_new_a((i->tagName()).c_str(),
                (i->print()).c_str()));
        }
    }
    JMAP::iterator it;
    for(it = grpmap->begin(); it != grpmap->end(); it++)
    {
        json_push_back(tree, it->second);
        grpmap->erase(it);
    }
        //cout << it->first << endl;
    delete grpmap;
    //cout << json_write_formatted(tree) << endl;
    return tree;
}

JSONNODE *ImgTagJson::genLitIptc(const Exiv2::Image::AutoPtr & image)
{
    Exiv2::IptcData &data = image->iptcData();
    data.sortByKey();

    if (data.empty()) return NULL;

    Exiv2::IptcData::const_iterator end = data.end();

    JMAP *grpmap = new JMAP();
    JSONNODE *tree = json_new(JSON_NODE);
    for (Exiv2::IptcData::const_iterator i = data.begin(); i != end; i++)
    {
        JSONNODE *grp;
        if (grpmap->find(i->groupName()) == grpmap->end())
        {
            grp = json_new(JSON_NODE);
            json_set_name(grp, i->groupName().c_str());
            grpmap->insert(JMAP::value_type(i->groupName(), grp)); 
        }
        else
           grp = (grpmap->find(i->groupName()))->second;

        Exiv2::IptcData::const_iterator nxt = i + 1;
        if ((nxt != data.end()) && (i->key() == nxt->key()))
        {
            //cout << "Array Elem! " << i->key() << endl;
            JSONNODE *arr = json_new(JSON_ARRAY);
            json_set_name(arr, i->tagName().c_str());
            json_push_back(arr, json_new_a((i->tagName()).c_str(),
                (i->print()).c_str()));

            while ((nxt != data.end()) && (nxt->key() == i->key()))
            {
                json_push_back(arr, json_new_a((nxt->tagName()).c_str(),
                    (nxt->print()).c_str()));
                nxt++;
            }
            json_push_back(grp, arr);
            if (nxt == data.end()) break;
            i = nxt - 1;
        }
        else
        {
            json_push_back(grp, json_new_a((i->tagName()).c_str(),
                (i->print()).c_str()));
        }
    }
    JMAP::iterator it;
    for(it = grpmap->begin(); it != grpmap->end(); it++)
    {
        json_push_back(tree, it->second);
        grpmap->erase(it);
    }
        //cout << it->first << endl;
    delete grpmap;
    //cout << json_write_formatted(tree) << endl;
    json_set_name(tree, "iptc");
    return tree;
}

JSONNODE *ImgTagJson::genLitXmp(const Exiv2::Image::AutoPtr & image)
{
    Exiv2::XmpData &data = image->xmpData();
    data.sortByKey();

    if (data.empty()) return NULL;

    Exiv2::XmpData::const_iterator end = data.end();

    JMAP *grpmap = new JMAP();
    JSONNODE *tree = json_new(JSON_NODE);
    for (Exiv2::XmpData::const_iterator i = data.begin(); i != end; ++i)
    {
        JSONNODE *grp;
        if (grpmap->find(i->groupName()) == grpmap->end())
        {
            grp = json_new(JSON_NODE);
            json_set_name(grp, i->groupName().c_str());
            grpmap->insert(JMAP::value_type(i->groupName(), grp)); 
        }
        else
           grp = (grpmap->find(i->groupName()))->second;

        Exiv2::XmpData::const_iterator nxt = i + 1;
        if ((nxt != data.end()) && (i->key() == nxt->key()))
        {
            //cout << "Array Elem! " << i->key() << endl;
            JSONNODE *arr = json_new(JSON_ARRAY);
            json_set_name(arr, i->tagName().c_str());
            json_push_back(arr, json_new_a((i->tagName()).c_str(),
                (i->print()).c_str()));

            while ((nxt != data.end()) && (nxt->key() == i->key()))
            {
                json_push_back(arr, json_new_a((nxt->tagName()).c_str(),
                    (nxt->print()).c_str()));
                nxt++;
            }
            json_push_back(grp, arr);
            if (nxt == data.end()) break;
            i = nxt - 1;
        }
        else
        {
            json_push_back(grp, json_new_a((i->tagName()).c_str(),
                (i->print()).c_str()));
        }
        //cout << json_write_formatted(grp) << endl;
    }
    JMAP::iterator it;
    for(it = grpmap->begin(); it != grpmap->end(); it++)
    {
        json_push_back(tree, it->second);
        grpmap->erase(it);
    }
        //cout << it->first << endl;
    delete grpmap;
    //cout << json_write_formatted(tree) << endl;
    json_set_name(tree, "xmp");
    return tree;
}

/*
** A few static variables (dont intend to pollute Class space for now)
*/
typedef struct {
    long     pos;
    int      Type;
    long     Size;
} JPG_SECTION;

static JPG_SECTION *s_sec = NULL;
static int        s_seccnt;
static int        s_secrd;
#define PSEUDO_IMAGE_MARKER 0x123

int ImgTagJson::getTagPos()
{
    FILE *f;

    f = fopen(this->fname.c_str(), "rb");
    if (f == NULL) return 1;

    // A straight forward ripoff from jhead:jpgfile.c:ReadJpegSections()
    unsigned char c;

    c = fgetc(f);
    if (c != 0xff || fgetc(f) != M_SOI){
        fclose(f);
        return 2;
    }

    //s_sec = (JPG_SECTION *) operator new (sizeof(Section_t) * 10);
    s_sec = new JPG_SECTION[20];
    s_secrd = 0;
    s_seccnt = 20;
    while (true)
    {
        int i;
        int itemlen;
        int prev = 0;
        int marker = 0;
        int ll,lh, got;
        long nxt;

        for (i = 0;;i++)
        {
            marker = fgetc(f);
            if (marker != 0xff && prev == 0xff) break;
            prev = marker;
        }

        if (i > 10)
            cerr << "Extraneous " << (i - 1) << " padding bytes before section "
                << marker << endl;

        s_sec[s_secrd].Type = marker;
        s_sec[s_secrd].pos = ftell(f) - 2;
        // Read the length of the section.
        lh = fgetc(f);
        ll = fgetc(f);
        itemlen = (lh << 8) | ll;
        if (itemlen < 2)
        {
            cerr << "Invalid Marker" << endl;
            delete[] s_sec;
            fclose(f);
            return 2;
        }
        s_sec[s_secrd].Size = itemlen;

        got = fseek(f, itemlen - 2, SEEK_CUR);
        if (got == -1)
        {
            cerr << "Premature end of file" << endl;
            delete[] s_sec;
            fclose(f);
            return 3;
        }
        s_secrd++;

        switch(marker)
        {
            case M_SOS:  // End of Tags, Begin of Compressed data
                // cout << "M_SOS: " << s_sec[s_secrd-1].pos << "\t Len: " <<
                //    s_sec[s_secrd-1].Size << endl;
                s_sec[s_secrd].pos = ftell(f);
                fseek(f, 0, SEEK_END);
                s_sec[s_secrd].Size = ftell(f) - s_sec[s_secrd].pos;
                s_sec[s_secrd].Type = PSEUDO_IMAGE_MARKER;
                //fseek(f, s_sec[s_secrd].pos, SEEK_SET);
                s_secrd++;
                goto END_JPG;
                break;
            case M_EOI:   // in case it's a tables-only JPEG stream
                cerr << "No Image in file" << endl;
                delete[] s_sec;
                return 4;
            case M_COM: // Comment section
                break;
            case M_JFIF:
                 // JFIF Header - Ignore
                 break;
            case M_EXIF:
                 nxt = ftell(f);
                 fseek(f, s_sec[s_secrd - 1].pos + 4, SEEK_SET);
                 unsigned char data[6];
                 data[4] = data[5] = '\0';
                 fread(data, 1, 4, f);
                 if (memcmp(data, "Exif", 4) == 0)
                 {
                     //cout << "M_EXIF: " << data << endl;
                     fseek(f, nxt, SEEK_SET);
                     break;
                 }
                 else
                 {
                     data[4] = (char) (fgetc(f) & 0xff);
                     if (memcmp(data, "http:", 5) == 0)
                     {
                         //cout << "M_XMP: " << data << endl;
                         s_sec[s_secrd-1].Type = M_XMP;
                         fseek(f, nxt, SEEK_SET);
                         break;
                     }
                 }
                 fseek(f, nxt, SEEK_SET);
                 // Not EXIF or XMP -- Ignore this section
                 s_secrd--;
                 break;
            case M_IPTC:
                 //cout << "IPTC: " << marker << endl;
                 break;
            case M_SOF0: 
            case M_SOF1: 
            case M_SOF2: 
            case M_SOF3: 
            case M_SOF5: 
            case M_SOF6: 
            case M_SOF7: 
            case M_SOF9: 
            case M_SOF10:
            case M_SOF11:
            case M_SOF13:
            case M_SOF14:
            case M_SOF15:
                 // WARNING: This is to simply further processing
                 s_sec[s_secrd-1].Type = M_SOF0;
                break;
            default:
                 //cerr << "Other JPEG Section marker " << marker << endl;
                 s_secrd--; // Ignoring for now
                break;
        }
    }
END_JPG:

    fclose(f);
    for (int i = 0; i < s_secrd; i++)
    {
        TTYPE marker;
        if (s_sec[i].Type == M_EXIF) marker = EXIF;
        else if (s_sec[i].Type == M_XMP) marker = XMP;
        else if (s_sec[i].Type == M_IPTC) marker = IPTC;
        else if (s_sec[i].Type == M_JFIF) marker = JFIF;
        else if (s_sec[i].Type == PSEUDO_IMAGE_MARKER) marker = DATA;
        else continue;

        this->tloc[marker].valid = true;
        this->tloc[marker].pos = s_sec[i].pos;
        this->tloc[marker].len = s_sec[i].Size;
        // cout << "Tag: " << marker << "\tPos: " << this->tloc[marker].pos <<
        //     "\tLen: " << this->tloc[marker].len << endl;
    }

    delete[] s_sec;
    s_sec = NULL;
    s_secrd = 0;
    s_seccnt = 0;

    return 0;
}

int ImgTagJson::verbose()
{
    if (this->getTagPos() != 0)
    {
        cerr << "Invalid File - Only Jpg files with valid data supported\n" << 
            endl;
        cout << "{}" << endl;
        return 1;
    }

    JSONNODE *tree = json_new(JSON_NODE);
    JSONNODE *hdr  = json_new(JSON_NODE);
    json_set_name(hdr, "Hdr");
    json_push_back(hdr, json_new_i("start", 3));
    json_push_back(hdr, json_new_i("length", this->tloc[DATA].pos - 3));
    json_push_back(hdr, json_new_i("end", this->tloc[DATA].pos));
    for (int i = 0; i < TTCNT-1; i++)
    {
        if (this->tloc[i].valid)
        {
            JSONNODE *n  = json_new(JSON_NODE);
            json_set_name(n, TTSTR[i]);
            json_push_back(n, json_new_i("start", this->tloc[i].pos + 1));
            json_push_back(n, json_new_i("length", this->tloc[i].len + 2));
            json_push_back(n, json_new_i("end", 
                (this->tloc[i].pos + this->tloc[i].len + 2)));

            json_push_back(hdr, n);
        }
    }
    json_push_back(tree, hdr);

    // Validation not necessary
    JSONNODE *data = json_new(JSON_NODE);
    json_set_name(data, TTSTR[DATA]);
    json_push_back(data, json_new_i("start", this->tloc[DATA].pos + 1));
    json_push_back(data, json_new_i("length", this->tloc[DATA].len));
    json_push_back(data, json_new_i("end", 
        (this->tloc[DATA].pos + this->tloc[DATA].len + 1)));
    json_push_back(tree, data);

    cout << json_write_formatted(tree) << endl;
    json_delete(tree);

    return 0;
}


int ImgTagJson::checksum()
{
    JSONNODE *c = this->getChkSum();

    if (c == NULL)  return 1;

    JSONNODE * v = json_new(JSON_NODE);
    json_push_back(v, c);
    cout << json_write_formatted(v) << endl;
    json_delete(v);

    return 0;
}

JSONNODE * ImgTagJson::getChkSum()
{
    MHASH           mdd  = NULL;
    MHASH           shd  = NULL;
    unsigned char   *md5res;
    unsigned char   *sha1res;
    unsigned char   bfr[IMG_B64_BFR_SZ];
    unsigned char   md5sum[64];
    unsigned char   sha1sum[64];

    long dbeg, dlen;

    if (!(md5 || sha1)) return NULL;

    if (this->getTagPos() != 0)
    {
        cerr << "Invalid File - Only Jpg files with valid data supported\n" << 
            endl;
        cout << "{}" << endl;
        return NULL;
    }

    dbeg = this->tloc[DATA].pos;
    dlen = this->tloc[DATA].len;

    if (dlen == 0) return NULL;

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

    std::ifstream f(fname.c_str(), std::ios::in|std::ios::binary);
    if (!f.is_open()) return NULL;

    f.seekg(dbeg, std::ios::beg);
    long   cnt = dlen / IMG_B64_BFR_SZ;
    long   rmn = dlen % IMG_B64_BFR_SZ;

    while ((f.read((char *)bfr, IMG_B64_BFR_SZ)) && (cnt > 0))
    {
        if (md5) mhash(mdd, bfr, IMG_B64_BFR_SZ);
        if (sha1) mhash(shd, bfr, IMG_B64_BFR_SZ);
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

