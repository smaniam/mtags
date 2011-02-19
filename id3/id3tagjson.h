#ifndef __ID3_TAG_JSON_H__
#define __ID3_TAG_JSON_H__

#include <string>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include "id3_jdefs.h"
#include "libjson/libjson.h"

using std::string;


class Id3TagJson
{
    private:
        string  fname;
        bool    art;
        string  pixpath;
        bool    md5;
        bool    sha1;
        TagLib::MPEG::File  *mpgfile;

    public:
        Id3TagJson(char *fname)
        { 
            this->fname.assign(fname);
            this->mpgfile = new TagLib::MPEG::File(fname);
            this->art = false;
            this->pixpath.clear();
            this->md5 = false;
            this->sha1 = false;
        };

        ~Id3TagJson() { delete mpgfile; };
        int verbose();
        int literal();
        int albumart();
        int checksum();
        void setPixPath(const char *path) { this->pixpath.assign(path); };
        void setExtractArt(bool val) { this->art = val; };
        void setChkSum(int val) { if (val & 1) md5 = true; else if (val & 2) sha1 = true; else val = 0; };

    private:
        JSONNODE * genLitTree();
        JSONNODE * getPic(TagLib::ID3v2::Frame *frm);
        JSONNODE * getFrmLitVal(TagLib::ID3v2::Frame *frm);
        JSONNODE * getChkSum();
        void       getLocation(long &v2beg, long & v2len, long & dbeg, 
                       long & dlen, long & v1beg, long & v1len, long & apebeg,
                       long & apelen);
        long       locateID3v1();
        long       locateAPE(bool id3v1 = false);
        long       getFileSz();

        void       delnewline(char *buf, int len) ;
};


#endif
