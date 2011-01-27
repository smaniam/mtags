#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <vector>
#include <map>

#include "pdftagjson.h"
//#include "poppler-config.h"
#include "poppler/goo/GooString.h"
#include "poppler/PDFDocFactory.h"
#include "poppler/PDFDoc.h"
#include "poppler/Object.h"
#include "poppler/PDFDocEncoding.h"

#include <mhash.h>
#define BUFFERSIZE (PDF_B64_BFR_SZ * 2)
#include <b64/encode.h>


using namespace base64;
using std::vector;
using std::string;
using std::cout;
using std::endl;
using std::map;
using std::cerr;
using std::ifstream;

class VbsData
{
    public:
        string name;
        long   beg;
        long   len;
        long   end;
};

typedef vector<VbsData *> VBS;

//const char *fname;

inline long PdfTagJson::getFileSz()
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

inline void PdfTagJson::delnewline(char *buf, int len) 
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

static JSONNODE * getInfoString(Dict *infoDict, char *key)
{
  Object obj;
  GooString *s1;
  GBool isUnicode;
  Unicode u;
  char buf[8];
  string str;

  int i, n;

  if (infoDict->lookup(key, &obj)->isString()) {
    s1 = obj.getString();
    if ((s1->getChar(0) & 0xff) == 0xfe &&
        (s1->getChar(1) & 0xff) == 0xff) {
      isUnicode = gTrue;
      i = 2;
    } else {
      isUnicode = gFalse;
      i = 0;
    }
    while (i < obj.getString()->getLength()) {
      if (isUnicode) {
        u = ((s1->getChar(i) & 0xff) << 8) |
            (s1->getChar(i+1) & 0xff);
        str += s1->getChar(i++);
        str += s1->getChar(i++);
      } else {
        u = pdfDocEncoding[s1->getChar(i) & 0xff];
        str += s1->getChar(i++);
        //++i;
      }
      //n = uMap->mapUnicode(u, buf, sizeof(buf));
      //fwrite(buf, 1, n, stdout);
    }
    //cout << str << endl;
    JSONNODE *node = json_new_a(key, str.c_str());
    obj.free();
    return node;
  }
  obj.free();
  return NULL;
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

static char * INFOKEY[] = { "Title", "Subject", "Keywords",
    "Author", "Creator", "Producer", "CreationDate", "ModDate", "" };

int PdfTagJson::literal()
{
    PDFDoc *doc;
    GooString *gfname = new GooString(this->fname.c_str());

    doc = PDFDocFactory().createPDFDoc(*gfname, NULL, NULL);

    Object info;

    if (!doc->isOk())
    {
        cerr << "Invalid file: " << this->fname << endl;
        return 1;
    }

    JSONNODE * l = json_new(JSON_NODE);
    std::ostringstream stm;
    stm << doc->getPDFMajorVersion() << "." << doc->getPDFMinorVersion();
    json_push_back(l, json_new_a("Version", stm.str().c_str()));

    json_push_back(l, json_new_i("Pages", doc->getNumPages()));

    doc->getDocInfo(&info);
    if (info.isDict())
    {
        JSONNODE *jin = json_new(JSON_NODE);
        json_set_name(jin, "info");
        //cout << info->getXRef()->getEntry(0)->offset << endl;
        Dict *dict = info.getDict();
        
        for (int i = 0; INFOKEY[i][0] != '\0'; i++)
        {
            JSONNODE *j;
            j = getInfoString(dict, INFOKEY[i]);
            if (j != NULL)
                json_push_back(jin, j);
        }
        json_push_back(l, jin);
    }

    info.free();

    GooString *xmp = doc->readMetadata();
    if (xmp != NULL)
        json_push_back(l, json_new_a("xmp", xmp->getCString()));
    delete doc;
    delete gfname;

    cout << json_write_formatted(l) << endl;
    json_delete(l);

    return 0;
}

int PdfTagJson::checksum()
{
    JSONNODE *c = this->getChkSum();

    if (c == NULL)  return 1;

    JSONNODE * v = json_new(JSON_NODE);
    json_push_back(v, c);
    cout << json_write_formatted(v) << endl;
    json_delete(v);

    return 0;
}

JSONNODE * PdfTagJson::getChkSum()
{
    MHASH           mdd  = NULL;
    MHASH           shd  = NULL;
    unsigned char   *md5res;
    unsigned char   *sha1res;
    unsigned char   bfr[PDF_B64_BFR_SZ];
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

    // WARNING!!! - Placeholder REPLACE
    dbeg = 1;  
    dlen = 100;

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
    long   cnt = dlen / PDF_B64_BFR_SZ;
    long   rmn = dlen % PDF_B64_BFR_SZ;

    while ((f.read((char *)bfr, PDF_B64_BFR_SZ)) && (cnt > 0))
    {
        if (md5) mhash(mdd, bfr, PDF_B64_BFR_SZ);
        if (sha1) mhash(shd, bfr, PDF_B64_BFR_SZ);
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

// Ripped from poppler:PDFDoc.cc:getXRefStart()
static Guint getstartxref(PDFDoc *doc, const char *fname)
{
    Guint pos;
    std::ifstream f;
    char buf[1026];
    char c;
    int n, i;

    f.exceptions(ifstream::failbit | ifstream::badbit );
    f.open(fname, std::ios::binary);
    if (doc->isLinearized())
    {
        for (n = 0; n < 1024; n++)
        {
            if (!f.get(c).good())
                break;
            buf[n] = c;
        }
        buf[n] = '\0';
        
        pos = 0;
        for (i = 0; i < n; i++)
        {
            if (strncmp("endobj", &buf[i], 6) == 0)
            {
                pos = i + 6;
                while ((buf[pos] == '\n') || (buf[pos] == '\r'))
                    pos++;
                break;
            }
        }
    }
    else
    {
        f.seekg(-1024, std::ios::end);
        
        for (n = 0; n < 1024; n++)
        {
            //cout << n << endl;
            if (!f.get(c).good())
                break;
            buf[n] = c;
        }
        buf[n] = '\0';
        
        pos = 0;
        for (i = n - 9; i >= 0; i--)
        {
            if (strncmp(&buf[i], "startxref", 9) == 0)
            {
                break;
            }
        }
        if (i < 0)
            pos = 0;
        else
            pos = strtol(&buf[i+9], NULL, 10);
    }
    f.close();
    
    return pos;
}



static void rdline(std::istream & in, std::string & out) 
{
    char c;
    out.clear();
    while(in.get(c).good()) 
    {
        out.append(1,c);
        if(c == '\r') 
        {
            if(in.good()) 
            {
                c = in.peek();
                if(c == '\n') 
                {
                    out.append(1,'\n');
                    in.ignore();
                }
            }
            break;
        }
        else if (c == '\n')
            break;
    }
    return;
}



static int getsect(const char *fname, VbsData &hdr, VBS &xref, VBS &body)
{
    PDFDoc *doc;
    GooString *gfname = new GooString(fname);

    doc = PDFDocFactory().createPDFDoc(*gfname, NULL, NULL);

    if (!doc->isOk())
    {
        cerr << "Invalid file: " << fname << endl;
        return 1;
    }
    
    
    std::string str;
    std::ifstream f;

    f.exceptions(ifstream::failbit | ifstream::badbit );
    
    
    try
    {
        f.open(fname);
        rdline(f, str);
        if (str.find("%PDF-") == string::npos)
        {
            cerr << "Bad Header\n";
            hdr.beg = 0;
            hdr.len = 0;
            hdr.end = 0;
        }
        else
        {
            hdr.beg = 0;
            hdr.len = str.length();
            hdr.end = hdr.len;
        }
    }
    catch (ifstream::failure e)
    {
        cerr << "Invalid file: " << fname << endl;
        f.close();
        delete doc;
        delete gfname;
        return 1;
    }

    while (true)
    {
        int step = 0;
        int i;
        long pos;
        
        const char *p;
        try
        {
            //cout << "Line: " << endl;
            //std::getline(f, str);
            step = 1;
            rdline(f, str);
            p = str.c_str();
            i = 0;
            while ((p[i] != '\0') && 
                ((p[i] == ' ') || (p[i] == '\t')))
                i++;

            if (strncmp ((const char*)&p[i], "xref", 4) == 0)
            {
                pos = f.tellg();
                pos -= str.length();
                //cout << "XREF: " << pos;
                step = 2;
                while (true)
                {
                    rdline(f, str);
                    p = str.c_str();
                    i = 0;
                    while ((p[i] != '\0') && 
                        ((p[i] == ' ') || (p[i] == '\t')))
                        i++;
                    if (strncmp ((const char*)&p[i], "%%EOF", 4) == 0)
                    {
                        long endpos = f.tellg();
                        //cout << ", " << endpos << endl;
                        VbsData *node = new VbsData();
                        node->beg = pos;
                        node->end = endpos - 1;
                        node->len = endpos - pos + 1;
                        
                        xref.push_back(node);
                        break;
                    }
                }
            }
            //cout << str;
        }
        catch (std::ios_base::failure e)
        {
            if (step == 2)
            {
                // Damaged file or missing newline
                f.clear();
                f.seekg(0, std::ios::end);
                VbsData *node = new VbsData();
                node->beg = pos;
                node->end = f.tellg();
                node->len = node->end - pos + 1;
                xref.push_back(node);
            }
            //std::cerr << str << "Caught exception" << step << std::endl;
            f.close();
            break;
        }
    }
    
    if (xref.size() == 0)
    {
        // ObjStream
        f.open(fname);
        Guint sxref = getstartxref(doc, fname);
        //cout << "Stream XREF: " << sxref << " " << f.tellg() << endl;
        f.seekg(sxref, std::ios::beg);
        //cout << f.tellg() << endl;
        VbsData *node = new VbsData();
        node->beg = sxref;
        try
        {
            while (true)
            {
                rdline(f, str);
                //cout << str << endl;
                if (str.find("endobj") != string::npos)
                {
                    node->end = f.tellg();
                    node->len = node->end - node->beg + 1;
                    xref.push_back(node);
                    break;
                }
            }
        }
        catch (ifstream::failure e)
        {
            cerr << "Damaged file?\n";
            delete doc;
            delete gfname;
            return 3;
        }
        f.close();
    }
    delete doc;
    delete gfname;
    
    // Objects are embedded inbetween
    VbsData *node = new VbsData();
    node->beg = hdr.end + 1;
    for (int i = 0; i < xref.size(); i++)
    {
        node->end = xref[i]->beg - 1;
        node->len = node->end - node->beg + 1;
        // Weird case of two XREFs being adjacent to each other
        if (node->len == 0)
            delete node;
        else
            body.push_back(node);
        
        node = new VbsData();
        node->beg = xref[i]->end + 1;
    }
    delete node;

    return 0;
}


int PdfTagJson::verbose()
{
    /*
    PDFDoc *doc;
    GooString *gfname = new GooString(this->fname.c_str());

    doc = PDFDocFactory().createPDFDoc(*gfname, NULL, NULL);

    Object *obj;

    if (!doc->isOk())
    {
        cerr << "Invalid file: " << this->fname << endl;
        return 1;
    }
    
    Guint sxref = getstartxref(doc, this->fname.c_str());
    cout << "Start XREF: " << sxref << endl;
    
    XRef *xref = doc->getXRef();
    obj = doc->getXRef()->getTrailerDict();
    if (obj->isDict())
    {
        cout << "Trailer Dict\n";
        Dict *d = obj->getDict();
        int len = d->getLength();
        cout << "Len: " << len << endl;
        Object prev;
        if (d->lookup((char *)"Prev", &prev))
        {
            if (prev.isInt())
            {
                cout << "Prev: " << prev.getInt() << endl;
                // int prex = xref->getNumEntry(prev.getInt());
                // cout << "Num Entry: " << prex << "\tType: " <<
                //    xref->getEntry(prex)->obj.getType() <<
                //    endl;
                
            }
        }
        
    }*/
    
    VbsData hdr;
    VBS xref;
    VBS body;
    if (getsect(this->fname.c_str(), hdr, xref, body) != 0)
    {
        cerr << "Error decoding data\n";
        cout << "{}\n";
        return 1;
    }
    
    JSONNODE * v = json_new(JSON_NODE);
    hdr.name = "header";
    json_push_back(v, vNode(&hdr));
    
    if (xref.size() == 1)
    {
        xref[0]->name = "xref";
        json_push_back(v, vNode(xref[0]));
    }
    else if (xref.size() > 0)
    {
        JSONNODE *a = json_new(JSON_ARRAY);
        json_set_name(a, "xref");
        for (int i = 0; i < xref.size(); i ++)
            json_push_back(a, vNode(xref[i]));
        json_push_back(v, a);
    }
    
    if (body.size() == 1)
    {
        body[0]->name = "body";
        json_push_back(v, vNode(body[0]));
    }
    else if (body.size() > 0)
    {
        JSONNODE *a = json_new(JSON_ARRAY);
        json_set_name(a, "body");
        for (int i = 0; i < body.size(); i ++)
            json_push_back(a, vNode(body[i]));
        json_push_back(v, a);
    }
    
    cout << json_write_formatted(v) << endl;
    json_delete(v);
    return 0;
}