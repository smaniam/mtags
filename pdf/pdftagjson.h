#ifndef __PDF_TAG_JSON_H__
#define __PDF_TAG_JSON_H__

#include <string>
#include "pdf_jdefs.h"
#include "libjson.h"

using std::string;

class PdfTagJson
{
    private:
        string  fname;
        bool    md5;
        bool    sha1;

    public:
        PdfTagJson(char *fname)
        { 
            this->fname.assign(fname);
            this->md5 = false;
            this->sha1 = false;
        };

        ~PdfTagJson() {};
        int verbose();
        int literal();
        int checksum();
        void setMD5(bool mode) { this->md5 = mode; };
        void setSHA1(bool mode) { this->sha1 = mode; };

    private:
        JSONNODE * getChkSum();
        long       getFileSz();
        void       delnewline(char *buf, int len);
};


#endif
