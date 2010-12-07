#ifndef __ID3_TAG_JSON_H__
#define __ID3_TAG_JSON_H__

#include <string>
#include <mpegfile.h>
#include <id3v2tag.h>
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
            this->art = false;
            this->pixpath.clear();
        };

        ~Id3TagJson() { delete mpgfile; };
        int verbose();
        int literal();
        void setPixPath(const char *path) { this->pixpath.assign(path); };
        void setExtractArt(bool val) { this->art = val; };

    private:
        JSONNODE * genLitTree();
        JSONNODE * getPic(TagLib::ID3v2::Frame *frm);
        JSONNODE * getFrmLitVal(TagLib::ID3v2::Frame *frm);

        void delnewline(char *buf, int len) 
        {
            int i = 0, o = 0;
            
            while (i < len)
            {
                if (buf[i] == '\n') i++;
                else if (o < i) buf[o++] = buf[i++];
                else i++, o++;
            }
            buf[o] = '\n';
        };
};


#endif
