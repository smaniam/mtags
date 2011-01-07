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


using namespace base64;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::map;

class VbsData
{
    public:
        string name;
        long   beg;
        long   len;
        long   end;
};

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
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(fname);
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

    cout << json_write_formatted(l) << endl;
    json_delete(l);
    
    return 0;
}

typedef map<string, JSONNODE *> JMAP;

JSONNODE *ImgTagJson::genLitExif(const Exiv2::Image::AutoPtr & image)
//JSONNODE *ImgTagJson::genLitExif(const Exiv2::ExifData &data)
{
    const Exiv2::ExifData &data = image->exifData();

    if (data.empty()) return NULL;

    Exiv2::ExifData::const_iterator end = data.end();

    JMAP *grpmap = new JMAP();
    JSONNODE *tree = json_new(JSON_NODE);
    json_set_name(tree, "Exif");
    for (Exiv2::ExifData::const_iterator i = data.begin(); i != end; ++i)
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

        json_push_back(grp, json_new_a((i->tagName()).c_str(),
            (i->print()).c_str()));
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
    const Exiv2::IptcData &data = image->iptcData();

    if (data.empty()) return NULL;

    Exiv2::IptcData::const_iterator end = data.end();

    JMAP *grpmap = new JMAP();
    JSONNODE *tree = json_new(JSON_NODE);
    for (Exiv2::IptcData::const_iterator i = data.begin(); i != end; ++i)
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

        json_push_back(grp, json_new_a((i->tagName()).c_str(),
            (i->print()).c_str()));
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
    json_set_name(tree, "Iptc");
    return tree;
}

JSONNODE *ImgTagJson::genLitXmp(const Exiv2::Image::AutoPtr & image)
{
    const Exiv2::XmpData &data = image->xmpData();

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

        json_push_back(grp, json_new_a((i->tagName()).c_str(),
            (i->print()).c_str()));
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
    json_set_name(tree, "Xmp");
    return tree;
}


