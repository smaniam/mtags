#ifndef __ID3_TAG_JSON_H__
#define __ID3_TAG_JSON_H__

#include <string>
#include "img_jdefs.h"
#include "exiv2.hpp"
#include "libjson.h"

using std::string;


class ImgTagJson
{
    private:
        string  fname;
        string  pixpath;
        bool    md5;
        bool    sha1;
        bool    exif;
        bool    xmp;
        bool    iptc;

    public:
        ImgTagJson(char *fname)
        { 
            this->fname.assign(fname);
            this->pixpath.clear();
            this->md5 = false;
            this->sha1 = false;
            this->xmp = true;
            this->exif = true;
            this->iptc= true;
        };

        ~ImgTagJson() {};
        int verbose(){ return 0;};
        int literal();
        int checksum(){ return 0;};
        void setPixPath(const char *path) { this->pixpath.assign(path); };
        void setXmp(bool mode) { this->xmp = mode; };
        void setExif(bool mode) { this->exif = mode; };
        void setIptc(bool mode) { this->iptc = mode; };

    private:
        //JSONNODE * genLitExif(const Exiv2::ExifData &exif);
        JSONNODE * genLitExif(const Exiv2::Image::AutoPtr & image);
        JSONNODE * genLitIptc(const Exiv2::Image::AutoPtr & image);
        JSONNODE * genLitXmp(const Exiv2::Image::AutoPtr & image);
        long       getFileSz();
        void       delnewline(char *buf, int len);
};


#endif
