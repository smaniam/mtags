#ifndef __ID3_TAG_JSON_H__
#define __ID3_TAG_JSON_H__

#include <string>
#include <mpegfile.h>
#include "id3_jdefs.h"
#include "libjson.h"

using std::string;


class Id3TagJson
{
    private:
        string  fname;
        bool    art;
        string  pixpath;
        TagLib::MPEG::File  *mpgfile;

    public:
        Id3TagJson(char *fname)
        { 
            this->fname.assign(fname);
            this->mpgfile = new TagLib::MPEG::File(fname);
        };
        ~Id3TagJson() { delete mpgfile; };
        int verbose();
        int literal();
        void setPixPath(char *path) { this->pixpath.assign(path); }

    private:
        JSONNODE * genLitTree();
        //JSONNODE * getPic();
};


#endif
