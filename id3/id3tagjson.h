#ifndef __ID3_TAG_JSON_H__
#define __ID3_TAG_JSON_H__

#include <string>
#include "id3_jdefs.h"
#include "libjson.h"

using std::string;


class Id3TagJson
{
    private:
        string fname;
        bool   art;
    public:
        Id3TagJson() {};
        ~Id3TagJson() {};
        int verbose();
        int literal();
        void setFname(char *fname) { this->fname.assign(fname); }

    private:
        JSONNODE * genLitTree();
};


#endif
