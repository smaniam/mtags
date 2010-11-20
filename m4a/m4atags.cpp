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


void GetBasePath(const char *filepath, char* &basepath) {
    //with a myriad of m4a, m4p, mp4, whatever else comes up... it might just be easiest to strip off the end.
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


#define USAGE "Usage:\n\tm4atags <[--literal | --verbose]> [--pix-path=<path>] [--with-md5sum | --with-sha1sum ] <m4a-media-file>\n\n"

typedef struct 
{
    int mode;
    int md5sum;
    int sha1sum;
    char pixpath[128];
    int hckout[2];
    int stdout_sav;
    int stdout_new;
    FILE *str;
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
    cfg.pixpath[0]     = '\0';

    while (1)
    {
        static struct option long_options[] =
        {
            {"literal",        no_argument,       0, 'l'},
            {"verbose",        no_argument,       0, 'v'},
            {"with-md5sum",    no_argument,       0, 'm'},
            {"with-sha1sum",   no_argument,       0, 's'},
            {"pix-path",       required_argument, 0, 'p'},
            {"output",         required_argument, 0, 'o'},
            {"help",           no_argument,       0, 'h'},
            {"test",           no_argument,       0, 't'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index. */
        int option_index = 0;

        c = getopt_long (argc, argv, "p:o:lvhtmsc",
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

            case 'p':
                printf ("option -p : ");
                printf ("Not Yet Supported.....\n");
                strcpy(cfg.pixpath, optarg);
                return 20;

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
                printf ("Invalid Option\n");
                abort ();
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
        int getcksum = M4A_FALSE;
        unsigned char  bfr[2][64];
        unsigned char  *md5sum  = NULL;
        unsigned char  *sha1sum = NULL;

        openSomeFile(m4afile, true);
        if (cfg.md5sum == M4A_TRUE) 
        {
            md5sum   = bfr[0];
            getcksum = M4A_TRUE;
        }

        if (cfg.sha1sum == M4A_TRUE)
        {
            sha1sum  = bfr[1];
            getcksum = M4A_TRUE;
        }

        if (getcksum == M4A_TRUE)
        {
             if (m4a_stream_chksum(m4afile, md5sum, sha1sum) != 0)
             {
                 fprintf(stderr, "Checksum failure exiting\n");
                 return 10;
             }
        }

        redirect_io(&cfg);
        if (metadata_style >= THIRD_GEN_PARTNER) 
        {
            APar_PrintUserDataAssests();
        } 
        else if (metadata_style == ITUNES_STYLE) 
        {
            // don't try to extractPix
            APar_PrintDataAtoms(m4afile, NULL, 0, PRINT_DATA);
        }
        reset_io(&cfg);
        m4a_display_json_tags(cfg.str, stdout, md5sum, sha1sum, NULL);
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

        mda = APar_DetermineMediaData_AtomPosition();
        printf ("Location of mdat: %d\n", mda);

        //APar_SimpleAtomPrintout();
        m4a_stream_chksum(m4afile, bfr[0], bfr[1]);
    }
    exit (0);
}

