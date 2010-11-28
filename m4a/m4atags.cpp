/*
********************************************************************************
**
**   M4ATAGS.CPP: Extracts M4A (iTunes) style tags from a media file
**                and displays it in JSON format
**   AUTHOR:      smaniam@ymail.com
**   LICENSE:     GNU version 3
**
**
********************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <wchar.h>
extern "C"
{
#include "b64/cencode.h"
}


#include "AP_commons.h"
#include "AtomicParsley.h"
#include "AP_AtomExtracts.h"
#include "AP_iconv.h"          
#include "AtomicParsley_genres.h"  
#include "APar_uuid.h"

#include "m4a_json.h"

extern uint32_t APar_DetermineMediaData_AtomPosition();
extern void APar_SimpleAtomPrintout();

/*
** Atomic Parsley Function: Relook for possible deletion
*/

void ExtractPaddingPrefs(char* env_padding_prefs) {
    pad_prefs.default_padding_size = DEFAULT_PADDING_LENGTH;
    pad_prefs.minimum_required_padding_size = MINIMUM_REQUIRED_PADDING_LENGTH;
    pad_prefs.maximum_present_padding_size = MAXIMUM_REQUIRED_PADDING_LENGTH;
    
    if (env_padding_prefs != NULL) {
        if (env_padding_prefs[0] == 0x22 || env_padding_prefs[0] == 0x27) env_padding_prefs++;
    }
    char* env_pad_prefs_ptr = env_padding_prefs;
    
    while (env_pad_prefs_ptr != NULL) {
        env_pad_prefs_ptr = strsep(&env_padding_prefs,":");
        
        if (env_pad_prefs_ptr == NULL) break;
        
        if (memcmp(env_pad_prefs_ptr, "DEFAULT_PAD=", 12) == 0) {
            strsep(&env_pad_prefs_ptr,"=");
            sscanf(env_pad_prefs_ptr, "%u", &pad_prefs.default_padding_size);
        }
        if (memcmp(env_pad_prefs_ptr, "MIN_PAD=", 8) == 0) {
            strsep(&env_pad_prefs_ptr,"=");
            sscanf(env_pad_prefs_ptr, "%u", &pad_prefs.minimum_required_padding_size);
        }
        if (memcmp(env_pad_prefs_ptr, "MAX_PAD=", 8) == 0) {
            strsep(&env_pad_prefs_ptr,"=");
            sscanf(env_pad_prefs_ptr, "%u", &pad_prefs.maximum_present_padding_size);
        }
    }
    //fprintf(stdout, "Def %u; Min %u; Max %u\n", pad_prefs.default_padding_size, pad_prefs.minimum_required_padding_size, pad_prefs.maximum_present_padding_size);
    return;
}


void strip_extn(const char *filepath, char* &basepath) {
    int split_here = 0;
    for (int i=strlen(filepath); i >= 0; i--) {
        const char* this_char=&filepath[i];
        if ( strncmp(this_char, ".", 1) == 0 ) {
            split_here = i;
            break;
        }
    }
    memcpy(basepath, filepath, (size_t)split_here);
    
    return;
}

/*
** SOME USEFUL STUFF FOR THE PROGRAM TO FUNCTION
*/


#define USAGE "Usage:\n\tm4atags [--literal [ --with-md5sum ] [ --with-sha1sum ] [ --extract-art | --extract-art-to=<path> ] [ --verbose ] [[ --extract-art | --extract-art=<path> ] | [ --with-md5sum ] [ --with-sha1sum ]] <m4a-media-file>\n\n"

typedef struct 
{
    int            mode;
    int            md5sum;
    int            sha1sum;
    unsigned char  bfr[2][64];
    int            art;
    char           pixpath[128];
    int            hckout[2];
    int            stdout_sav;
    int            stdout_new;
    FILE           *str;
} M4A_TAG_CFG;


/*
**  INLINE FUNCTIONS FOR MANIPULATION OF IO
*/

inline int redirect_io(M4A_TAG_CFG *cfg)
{
    if (pipe(cfg->hckout) != 0)
    {
        perror("Pipe");
        return 1;
    }

    if ((cfg->stdout_sav = dup(fileno(stdout))) == -1)
    {
        perror("Dup stdout");
        return 2;
    }

    if ((cfg->stdout_new = dup2(cfg->hckout[1], 1)) == -1)
    {
        perror("Dup pipe");
        return 3;
    }
    
    stdout = fdopen(1, "w");
    cfg->str = fdopen(cfg->hckout[0], "r");
    return 0;
}

inline void reset_io(M4A_TAG_CFG *cfg)
{
    fflush(stdout);
    fclose(stdout);
    close(cfg->hckout[1]);
    
    close (cfg->stdout_new);
    dup (cfg->stdout_sav);
    stdout = fdopen(1, "w");
    return;
}

inline void cleanup_io(M4A_TAG_CFG *cfg)
{
    close(cfg->hckout[0]);
}


inline int get_chksum(M4A_TAG_CFG *cfg, char *m4afile,
    unsigned char **md5sum, unsigned char **sha1sum)
{
    int getcksum = M4A_FALSE;

    *md5sum  = NULL;
    *sha1sum = NULL;

    if (cfg->md5sum == M4A_TRUE) 
    {
        *md5sum   = cfg->bfr[0];
        getcksum = M4A_TRUE;
    }

    if (cfg->sha1sum == M4A_TRUE)
    {
        *sha1sum  = cfg->bfr[1];
        getcksum = M4A_TRUE;
    }

    if (getcksum == M4A_TRUE)
    {
         if (m4a_stream_chksum(m4afile, *md5sum, *sha1sum) != 0)
         {
             fprintf(stderr, "Checksum failure exiting\n");
             return 10;
         }
         return 0;
    }
    return 1;
}
/*
********************************************************************************
**
**
**     Main Function:
**          Parses Command line Arguments and Displays requested Tag Information
**
**
********************************************************************************
*/

int
main (int argc, char **argv)
{
    int                  c;
    char *m4afile =      NULL;
    M4A_TAG_CFG          cfg;

    cfg.mode           = M4A_MODE_INVALID;
    cfg.md5sum         = M4A_FALSE;
    cfg.sha1sum        = M4A_FALSE;
    cfg.art            = M4A_FALSE;
    cfg.pixpath[0]     = '\0';

    while (1)
    {
        static struct option long_options[] =
        {
            {"literal",        no_argument,       0, 'l'},
            {"verbose",        no_argument,       0, 'v'},
            {"with-md5sum",    no_argument,       0, 'm'},
            {"with-sha1sum",   no_argument,       0, 's'},
            {"extract-art",    no_argument,       0, 'e'},
            {"extract-art-to", required_argument, 0, 'p'},
            {"output",         required_argument, 0, 'o'},
            {"help",           no_argument,       0, 'h'},
            {"test",           no_argument,       0, 't'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index. */
        int option_index = 0;

        c = getopt_long (argc, argv, "p:o:lvhtmse",
               long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                /* Is mode set ? */
                if (long_options[option_index].flag != 0)
                    break;
                printf ("option %s", long_options[option_index].name);
                if (optarg)
                    printf (" with arg %s", optarg);
                printf ("\n");
                break;

            case 'l':
                cfg.mode = M4A_MODE_LITERAL;
                break;

            case 'v':
                cfg.mode = M4A_MODE_VERBOSE;
                break;

            case 't':
                cfg.mode = M4A_MODE_TESTING;
                break;

            case 'm':
                cfg.md5sum  = M4A_TRUE;
                break;

            case 's':
                cfg.sha1sum = M4A_TRUE;
                break;

            case 'e':
                cfg.art = M4A_TRUE;
                break;

            case 'p':
                cfg.art = M4A_TRUE;
                strcpy(cfg.pixpath, optarg);
                break;

            case 'o':
                printf ("option -o with value `%s'\n", optarg);
                printf ("Not Yet Supported.....\n");
                return 20;

            case 'h':
                printf ("\n%s", USAGE);
                return 0;

            case '?':
                /* getopt_long already printed an error message. */
                return 20;

            default:
                fprintf (stderr, "Invalid Option\n%s\n", USAGE);
                return 30;
        }
    }

    /* Grab File names*/
    if (optind < argc)
    {
        /*  For Later Use only one for now
        while (optind < argc)
        {
            printf ("%s ", argv[optind++]);
        }
        */
        m4afile = argv[optind];
    }
    else
    {
        fprintf(stderr, "No Files specified\n%s", USAGE);
        return 1;
    }

    TestFileExistence(m4afile, true);
    xmlInitEndianDetection();
    
    ExtractPaddingPrefs(NULL);
    
    tree_display_only=true;
    APar_ScanAtoms(m4afile);
    
    if (cfg.mode == M4A_MODE_LITERAL)
    {
        unsigned char *md5sum   =  NULL;
        unsigned char *sha1sum  =  NULL;
        int cnt                 =  0;
        M4A_ART  *art           =  NULL;
        M4A_ART                    bfr[M4A_MAX_ART];
        char     *bfname        =  NULL;
        char                       path[512];


        openSomeFile(m4afile, true);
        get_chksum(&cfg, m4afile, &md5sum, &sha1sum);

        if (cfg.art == M4A_TRUE)
        {
            int   cvr;
            int   idx;
            int   ret;

            cvr = m4a_get_atomidx((const char *) "covr", 1, 0);
            idx = parsedAtoms[cvr].NextAtomNumber;
            while (parsedAtoms[idx].AtomicLevel > parsedAtoms[cvr].AtomicLevel)
            {
                ret = m4a_extract_art(idx, &bfr[cnt]);
                if (ret != 0) break;
                cnt++;
                idx = parsedAtoms[idx].NextAtomNumber;
            }

            if (cnt != 0) 
            {
                char tmp[512];

                strcpy(tmp, m4afile);
                if (cfg.pixpath[0] != '\0')
                {
                    char *bname;
                    strcpy(path, cfg.pixpath);
                    strcat(path, "/");
                    bname = basename(tmp);
                    strcat(path, bname);

                    // printf ("Fname: %s\n", path);
                    bfname = path;
                }
                art = bfr;
            }
            
        }

        redirect_io(&cfg);
        if (metadata_style >= THIRD_GEN_PARTNER) 
        {
            APar_PrintUserDataAssests();
        } 
        else if (metadata_style == ITUNES_STYLE) 
        {
            APar_PrintDataAtoms(m4afile, NULL, 0, PRINT_DATA);
        }
        reset_io(&cfg);
        m4a_display_json_tags(
            cfg.str, stdout, md5sum, sha1sum, art, cnt, bfname);
        openSomeFile(m4afile, false);
    }
    else if (cfg.mode == M4A_MODE_VERBOSE)
    {
        redirect_io(&cfg);
        APar_PrintAtomicTree();
        reset_io(&cfg);
        m4a_display_json_tree(cfg.str, stdout);
    }
    else if (cfg.mode == M4A_MODE_TESTING)
    {
        int mda;
        unsigned char  bfr[2][64];
        int cvr;

        mda = APar_DetermineMediaData_AtomPosition();
        printf ("Location of mdat: %d\n", mda);

        //APar_SimpleAtomPrintout();
        m4a_stream_chksum(m4afile, bfr[0], bfr[1]);

        cvr = m4a_get_atomidx((const char *) "covr", 1, 0);
        printf("\n");
        
    }
    else
    {
        unsigned char *md5sum = NULL, *sha1sum = NULL;

        if (get_chksum(&cfg, m4afile, &md5sum, &sha1sum) == 0)
        {
            if ((md5sum != NULL) || (sha1sum != NULL))
            {
                char pfx[2];

                pfx[0] = '\0';
                printf("{\n    \"stream\": {"); 
                if (md5sum != NULL)
                {
                    printf(" \"md5sum\": \"%s\"", md5sum);
                    pfx[0] = ','; pfx[1] = '\0';
                }

                if (sha1sum != NULL)
                {
                    printf("%s \"sha1sum\": \"%s\"", pfx, sha1sum);
                }
                printf(" }\n}\n"); 
            }
        }
        else if (cfg.art == M4A_TRUE)
        {
            M4A_ART   art[M4A_MAX_ART];
            int       cvr;
            int       idx;
            int       ret;
            int       cnt = 0;
            char      path[512];
            FILE      *fp;
            FILE      *out = stdout;

            if (cfg.pixpath[0] != '\0')
            {
                char tmp[256];
                char *bname = NULL;

                strcpy(tmp, m4afile);
                strcpy(path, cfg.pixpath);
                strcat(path, "/");
                bname = basename(tmp);
                strcat(path, bname);
            }

            fputs ("{\n    \"@img\": [ ", out);
            openSomeFile(m4afile, true);
            cvr = m4a_get_atomidx((const char *) "covr", 1, 0);
            idx = parsedAtoms[cvr].NextAtomNumber;
            while (parsedAtoms[idx].AtomicLevel > parsedAtoms[cvr].AtomicLevel)
            {
                int err = M4A_FALSE;
                const char *extn  = NULL;

                ret = m4a_extract_art(idx, &art[cnt]);
                if (ret != 0) break;

                if ( art[cnt].type == M4A_PNG) 
                {
                    extn = "png";
                }
                else if ( art[cnt].type == M4A_JPG) 
                {
                    extn = "jpg";
                }

                if (cfg.pixpath[0] != '\0')
                {
                    char       fname[512];

                    sprintf(fname, "%s.%d.%s", path, cnt+1, extn);
                    if ((fp = fopen(fname, "wb")) != NULL)
                    {
                        if (fwrite(art[cnt].data, 1, art[cnt].size, fp) !=
                            art[cnt].size)
                        {
                            perror("img write:");
                            err = M4A_TRUE;
                        }
                        fclose(fp);
                    }
                    else
                    {
                        perror("img create:");
                        err = M4A_TRUE; 
                    }

                    if (cnt != 0) fputs(", ", out);
                    if (err == M4A_TRUE)
                        fputs("null", out);
                    else
                        fprintf(out, "\"%s\"", fname);

                }
                else
                {
                    base64_encodestate  inst;
                    char                bfr[M4A_B64_BFR_SZ*2];
                    int                 clen;
                    int                 blks;
                    int                 j;

                    base64_init_encodestate(&inst);

                    blks = art[cnt].size/1024;
                    if (cnt != 0) fputs(", ", out);
                    fprintf (out, "{\"type\": \"%s\", \"data\": \"", extn);
                    for (j = 0; j < blks; j++)
                    {
                        clen = base64_encode_block(
                            (const char*) &art[cnt].data[j * M4A_B64_BFR_SZ],
                            M4A_B64_BFR_SZ,
                            bfr,
                            &inst);
                        //fwrite((void *)bfr, clen, 1, out);
                        m4a_print_without_newlines(out, bfr, clen);
                    }

                    clen = base64_encode_block(
                        (const char*) &art[cnt].data[j * M4A_B64_BFR_SZ],
                        art[cnt].size % M4A_B64_BFR_SZ,
                        bfr,
                        &inst);
                    m4a_print_without_newlines(out, bfr, clen);

                    clen = base64_encode_blockend(bfr, &inst);

                    m4a_print_without_newlines(out, bfr, clen);
                    if (cnt != 0) fputs(", ", out);
                    fputs ("\"}", out);
                }
                cnt++;
                idx = parsedAtoms[idx].NextAtomNumber;
            }
            openSomeFile(m4afile, false);
            fputs (" ]\n}\n", out);
        }
    }
    exit (0);
}

