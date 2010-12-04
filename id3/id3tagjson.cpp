#include <iostream>
#include <sstream>

//#include <tbytevector.h>
#include <tstringlist.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <id3v1tag.h>

#include "id3tagjson.h"

using namespace TagLib;

int Id3TagJson::verbose()
{
    std::cout << "Verbose: Nothing for now!" << std::endl;
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

        std::stringstream ss;
        TagLib::StringList  idlst;

        ID3v2::FrameList::ConstIterator it = v2tag->frameList().begin();
        for(; it != v2tag->frameList().end(); it++)
        {
            string name;
            string value;
            
            String id = (*it)->frameID();
            if (idlst.contains(id)) continue;
            idlst.append(id);
            //if (id == String("APIC")) cout << "Pic Found!!!\n";

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
                    String fid = (*lit)->frameID();
                    name = fid.to8Bit();
                    value = (*lit)->toString().to8Bit();
                    json_push_back(arr,
                        json_new_a(name.c_str(), value.c_str()));
                }
                json_push_back(j_id3v2, arr);
            }
            else
            {
                value = (*it)->toString().to8Bit();
                json_push_back(j_id3v2, json_new_a(name.c_str(),
                    value.c_str()));
            }
        }

        json_push_back(json, j_id3v2);
    }

V2DONE:
    ID3v1::Tag *id3v1tag = this->mpgfile->ID3v1Tag();
    if (id3v1tag)
    {
        JSONNODE *j_id3v1 = json_new(JSON_NODE);
        json_set_name(j_id3v1, "ID3v1");

        if (!id3v1tag->title().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("title", id3v1tag->title().to8Bit().c_str()));

        if (!id3v1tag->artist().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("artist", id3v1tag->artist().to8Bit().c_str()));

        if (!id3v1tag->album().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("album", id3v1tag->album().to8Bit().c_str()));

        if (!id3v1tag->comment().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("comment", id3v1tag->comment().to8Bit().c_str()));

        if (!id3v1tag->genre().isEmpty())
            json_push_back(j_id3v1, 
                json_new_a("genre", id3v1tag->genre().to8Bit().c_str()));

        if (id3v1tag->year() != 0)
            json_push_back(j_id3v1, 
                json_new_i("year", id3v1tag->year()));

        if (id3v1tag->track() != 0)
            json_push_back(j_id3v1, 
                json_new_i("track", id3v1tag->track()));

        json_push_back(json, j_id3v1);
    }

    return json;
}   
