/*
********************************************************************************
**
**   ID3TAGS.CPP: Extracts ID3 style tags from MP3 media file
**                and displays it in JSON format
**   AUTHOR:      smaniam@ymail.com
**   LICENSE:     GNU GPL version 3
**
**
********************************************************************************
*/
#include <stdio.h>
#include <iostream>
#include <string>
#include <getopt.h>

#include "id3_jdefs.h"
#include "id3tagjson.h"


using std::string;
using std::cout;
using std::endl;

/*
** SOME USEFUL STUFF FOR THE PROGRAM TO FUNCTION
*/


//#define USAGE "Usage:\n\tid3tags [--literal [ --with-md5sum ] [ --with-sha1sum ] [ --extract-art | --extract-art-to=<path> ] [ --verbose ] [[ --extract-art | --extract-art=<path> ] | [ --with-md5sum ] [ --with-sha1sum ]] <id3-media-file>\n\n"
#define USAGE "Usage:\n\tid3tags --literal [ --extract-art | --extract-art-to=<path> ] <id3-media-file>\n\n"

class Id3TagsCfg 
{
public:
    int            mode;
    bool           md5sum;
    bool           sha1sum;
    unsigned char  bfr[2][64];
    bool           art;
    string         pixpath;
    Id3TagsCfg() {
        mode           = ID3_MODE_INVALID;
        md5sum         = false;
        sha1sum        = false;
        art            = false;
        pixpath.clear();
    }
    ~Id3TagsCfg() {}
    int getMode() { return mode; }
    bool getArt() { return art; }
    string getPixPath() { return pixpath; }
};


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
    char *id3file =      NULL;
    Id3TagsCfg           cfg;


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
                cfg.mode = ID3_MODE_LITERAL;
                break;

            case 'v':
                cfg.mode = ID3_MODE_VERBOSE;
                cout << "Verbose mode not yet supported\n";
                break;

            case 't':
                cfg.mode = ID3_MODE_TESTING;
                break;

            case 'm':
                cfg.md5sum  = true;
                break;

            case 's':
                cfg.sha1sum = true;
                break;

            case 'e':
                cfg.art = true;
                break;

            case 'p':
                cfg.art = true;
                cfg.pixpath = optarg;
                cout << cfg.pixpath << endl;
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
        id3file = argv[optind];
    }
    else
    {
        fprintf(stderr, "No Files specified\n%s", USAGE);
        return 1;
    }

    Id3TagJson tags(id3file);
    tags.setExtractArt(cfg.getArt());
    if (!cfg.getPixPath().empty()) 
        tags.setPixPath(cfg.getPixPath().c_str());

    if (cfg.getMode() == ID3_MODE_LITERAL) tags.literal();
}
