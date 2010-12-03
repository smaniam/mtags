#include <iostream>
#include <sstream>

#include <tbytevector.h>
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
    return 0;
}

JSONNODE * Id3TagJson::genLitTree()
{
    MPEG::File f(this->fname.c_str());

    ID3v2::Tag *id3v2tag = f.ID3v2Tag();
    JSONNODE *json = json_new(JSON_NODE);

    JSONNODE *j_id3v2 = NULL;
    if (id3v2tag != NULL)
    {
        j_id3v2 = json_new(JSON_NODE);
        json_set_name(j_id3v2, "ID3v2");

        string s("2.");

        std::stringstream ss;

        ss << id3v2tag->header()->majorVersion();
        s += ss.str() + ".";
        ss.str("");
        ss << id3v2tag->header()->revisionNumber();
        s += ss.str();
        json_push_back(j_id3v2, json_new_a("ver", s.c_str()));

        ID3v2::FrameList::ConstIterator it = id3v2tag->frameList().begin();
        for(; it != id3v2tag->frameList().end(); it++)
        {
            ss.str("");
            ss << (*it)->frameID();
            json_push_back(j_id3v2, json_new_a(ss.str().c_str(),
                ((*it)->toString().to8Bit()).c_str()));
        }
        json_push_back(json, j_id3v2);
    }

    ID3v1::Tag *id3v1tag = f.ID3v1Tag();
    if (id3v1tag != NULL)
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
