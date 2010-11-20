#ifndef __M4A_JSON_H__
#define __M4A_JSON_H__

#define M4A_FALSE           0
#define M4A_TRUE            1

#define M4A_MODE_INVALID    0
#define M4A_MODE_LITERAL    1
#define M4A_MODE_VERBOSE    2
#define M4A_MODE_BINARY     3
#define M4A_MODE_TESTING    4

#define M4A_CHKSUM_NONE     0
#define M4A_CHKSUM_MD5      1
#define M4A_CHKSUM_SHA1     2

#define M4A_CHKSUM_BFR_SZ   128

extern int m4a_display_json_tree(
    FILE *in,
    FILE *out);

extern int m4a_display_json_tags(
    FILE *in,
    FILE *out,
    unsigned char *md5sum,
    unsigned char *sha1sum,
    char          *covr);

extern int m4a_stream_chksum(
    char *fname, 
    unsigned char *md5sum,
    unsigned char *sha1sum);

extern int m4a_disp_tree();

#endif
