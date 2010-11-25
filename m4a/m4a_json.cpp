#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "apar_glob.h"
#include "m4a_json.h"
#include "mhash.h"
extern "C"
{
#include "b64/cencode.h"
}

const char *TABSPACE = "    ";

int m4a_display_json_tree(
    FILE *in,
    FILE *out)
{
    char     *line = NULL;
    size_t   len = 0;
    ssize_t  lnsz;
    int      prvlvl = 0;

    char pfx[512];
    int i, j;
    int     inst = 0;

    fprintf(out, "{\n");
    while ((lnsz = getline(&line, &len, in)) != -1) 
    {
        size_t  span;
        int     curlvl;
        char    *tok;
        char    *ptree[10];
        char    lvlspc[256];
        char    extname[32];

        line[lnsz-1] = '\0';
        //printf("%s", line);

        tok = strtok(line, " ");
        if ((tok != NULL) && (strcmp(tok, "Atom") != 0)) continue;

        i = 0;
        while (tok != NULL)
        {
            //printf("===> %s\n", tok);
            ptree[i] = tok;
            tok = strtok(NULL, " ");
            i++;
        }
        //for (j = 0; j < i; j++) printf ("===> %s\n", ptree[j]);

        if (strcmp(ptree[1], "----") == 0)
        {
            int idx;
            inst++;
            idx = m4a_get_atomidx("----", inst, 0);
            if ((idx =  m4a_get_atomidx("name", 1, idx)) != -1)
            {
                strcpy(extname, "----[");
                strcat(extname, parsedAtoms[idx].ReverseDNSname);
                strcat(extname, "]");

                ptree[1] = extname;
            }
        }

        pfx[0] = '\0';
        span = strspn(line, " ");
        curlvl = (span >> 2) + 1;
        curlvl = (curlvl << 1) - 1;
        //printf ("Initial number of spaces: %d [%d]\n", span, curlvl);
        if (prvlvl != 0) 
        { 
            if (curlvl > prvlvl)
            {
                strcat(pfx, ",\n");
                for (j = 0; j < prvlvl + 1; j++) strcat(pfx, TABSPACE);
                strcat(pfx, "\"value\": {\n");
            }
            else if (curlvl == prvlvl)
            {
                pfx[0] = '\n'; pfx[1] = '\0'; 
                for (j = 0; j < prvlvl; j++) strcat(pfx, TABSPACE);
                strcat(pfx, "},\n");
            }
            else
            {
                strcat(pfx, "\n");
                for (i = prvlvl + 1; i > curlvl+1; i--) 
                {
                    for (j = 1; j < i; j++) strcat(pfx, TABSPACE);
                    strcat(pfx, "}\n");
                }
                for (j = 1; j < i; j++) strcat(pfx, TABSPACE);
                strcat(pfx, "},\n");
            }
        }
        lvlspc[0] = '\0';
        for (j = 0; j < curlvl; j++) strcat(lvlspc, TABSPACE);
        strcat(pfx, lvlspc);
        strcat(lvlspc, TABSPACE);

        ptree[6][strlen(ptree[6])-1] = '\0'; // Delete the trailing comma
        fprintf(
            out, 
           "%s\"%s\": {\n%s\"start\": \"%s\",\n%s\"length\": \"%s\",\n%s\"end\": \"%s\"",
            pfx, ptree[1], 
            lvlspc, ptree[3], 
            lvlspc, ptree[6], 
            lvlspc, ptree[9]);

        prvlvl = curlvl;
    }

    /*
    **  Update brackets on completion
    */
    pfx[0] =  '\n'; pfx[1] = '\0';
    for (i = prvlvl + 1; i > 0; i--) 
    {
        for (j = 1; j < i; j++) strcat(pfx, TABSPACE);
        strcat(pfx, "}\n");
    }
    fprintf(out, "%s\n", pfx);

    if (line != NULL) free(line);

    return 0;
}

void m4a_stuff_backslash(char *inp, char *out)
{
    int i = 0;
    int j = 0;

    char prev = 'a';

    while (i <= (int)strlen(inp))
    {
        if (inp[i] == ' ')
        {
            if (prev != ' ') out[j++] = ' ';
            prev = ' ';
            i++;
        }
        else if (inp[i] == '"')
        {
            out[j++] = '\\';
            out[j++] = '"';
            i++;
            prev = 'a';
        }
        else if (inp[i] == '\t')
        {
            out[j++] = '\\';
            out[j++] = 't';
            i++;
            prev = 'a';
        }
        else
        {
            out[j++] = inp[i++];
            prev = 'a';
        }
    }
}

int m4a_display_json_tags(
    FILE *in,
    FILE *out,
    unsigned char *md5sum,
    unsigned char *sha1sum,
    M4A_ART       *art,
    int           cnt,
    char          *path)
{
    char     *line = NULL;
    size_t   len = 0;
    ssize_t  lnsz;
    int      fst = 0;

    int      nonstr = M4A_FALSE;

    fprintf(out, "\{\n");
    while ((lnsz = getline(&line, &len, in)) != -1) 
    {
        char *tok;
        char *ptree[64];
        char pfx[512];
        int i, j;
        char atom[128];
        char value[256];
        char sanitised[256];

        line[lnsz-1] = '\0';

        tok = strtok(line, " ");

        // Empty Line
        if (tok == NULL) continue;

        // Overflowing Value has \n in it
        if ((tok != NULL) && (strcmp(tok, "Atom") != 0)) 
        {
            line[strlen(line)] = ' ';
            m4a_stuff_backslash(line, sanitised);
            fprintf(out, "\\n %s", sanitised);
            continue;
        }

        // Gather all tokens
        i = 0;
        while (tok != NULL)
        {
            ptree[i] = tok;
            tok = strtok(NULL, " ");
            i++;
        }
        //for (j = 0; j < i; j++) printf ("===> %s\n", ptree[j]);


        // Aggregate Token for Atom name
        atom[0] = '\0';
        j = 1;
        ptree[j][strlen(ptree[j]) - 1] = '\0';
        while (strcmp(ptree[j], "contains:") != 0)
        {
            strcat(atom, ptree[j]);
            j++;
        }
        strcat(atom, "\"");

        // Aggregate non-covr tags
        if (strcmp(atom, "\"covr\"") != 0)
        {
            j++;
            value[0] = '\0';
            strcpy(value, ptree[j++]);
            for (; j < i; j++) sprintf(value, "%s %s", value, ptree[j]);
            nonstr = M4A_FALSE;
        }
        else
            strcpy(value, "true");


        // Generate appropriate Prefix
        pfx[0] = '\0';
        if (!nonstr) strcpy(pfx, "\"");
        if (fst) strcat(pfx, ",\n");
        strcat(pfx, TABSPACE);

        m4a_stuff_backslash(value, sanitised);
        fprintf(
            out, 
           "%s%s: \"%s",
            pfx, atom, sanitised);


        // Extract Cover art
        if (strcmp(atom, "\"covr\"") == 0)
        {
            if (art != NULL)
            {
                const char *extn = NULL;
                FILE  *fp;

                fprintf(out, "\",\n%s\"@img\": [ ", TABSPACE);

                if (path != NULL)
                {
                    char fname[512];
                    int err = M4A_FALSE;

                    for (i = 0; i < cnt; i++)
                    {
                        if ( art[i].type == M4A_PNG) 
                        {
                            extn = "png";
                        }
                        else if ( art[i].type == M4A_JPG) 
                        {
                            extn = "jpg";
                        }
                        sprintf(fname, "%s.%d.%s", path, i+1, extn);

                        if ((fp = fopen(fname, "wb")) != NULL)
                        {
                            if (fwrite(art[i].data, art[i].size, 1, fp) !=
                                art[i].size)
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

                        if (i != 0) fputs(", ", out);
                        if (err == M4A_TRUE)
                            fputs("null", out);
                        else
                            fprintf(out, "\"%s\"", fname);
                    }
                }
                else
                {
                    base64_encodestate  inst;
                    char                bfr[M4A_B64_BFR_SZ*2];
                    int                 clen;
                    int                 blks;


                    base64_init_encodestate(&inst);
                    for (i = 0; i < cnt; i++)
                    {
                        blks = art[i].size/1024;
                        fputs ("\"", out);
                        for (j = 0; j < blks; j++)
                        {
                            clen = base64_encode_block(
                                (const char*) &art[i].data[j * M4A_B64_BFR_SZ],
                                M4A_B64_BFR_SZ,
                                bfr,
                                &inst);
                            fwrite((void *)bfr, clen, 1, out);
                        }

                        clen = base64_encode_block(
                            (const char*) &art[i].data[j * M4A_B64_BFR_SZ],
                            art[i].size % M4A_B64_BFR_SZ,
                            bfr,
                            &inst);
                        fwrite((void *)bfr, clen, 1, out);

                        clen = base64_encode_blockend(bfr, &inst);
                        fwrite((void *)bfr, clen, 1, out);
                        if (i != 0) fputs(", ", out);
                        fputs ("\"", out);
                    }
                }
                fprintf (out, " ]");
                nonstr = M4A_TRUE;
            }
        }

        fst = 1;
    }

    if ((md5sum != NULL) || (sha1sum != NULL))
    {
        char pfx[3];

        pfx[0] = '\0';
        if (!nonstr) strcpy(pfx, "\"");
        if (fst) strcat(pfx, ",");
        fprintf( out, "%s\n%s\"stream\": {", pfx, TABSPACE); 
        pfx[0] = '\0';
        if (md5sum != NULL)
        {
            fprintf(
                out, " \"md5sum\": \"%s\"", md5sum);
            pfx[0] = ','; pfx[1] = '\0';
        }

        if (sha1sum != NULL)
        {
            fprintf(
                out, "%s \"sha1sum\": \"%s\"", pfx, sha1sum);
        }
        fprintf( out, " }\n}\n"); 
    }
    else
        fprintf(out, "\"\n}\n");
    if (line != NULL) free(line);

    return 0;
}



int m4a_stream_chksum(
    char *fname, 
    unsigned char *md5sum,
    unsigned char *sha1sum)
{
    int             i;
    int             len = 0;
    AtomicInfo*     atom = NULL;
    MHASH           mdd  = NULL;
    MHASH           shd  = NULL;
    unsigned char   *md5res;
    unsigned char   *sha1res;
    unsigned char   bfr[M4A_CHKSUM_BFR_SZ];
    FILE            *fp;
    int             cnt;

    for (i=0; i < atom_number; i++) 
    { 
        atom = &parsedAtoms[i]; 
        if (memcmp(atom->AtomicName, "mdat", 4) == 0) break;
    }

    if (i == atom_number) return 3;

    if (atom->AtomicLength > 100)
    {
        len = atom->AtomicLength;
    } else if (atom->AtomicLength == 0) 
    { 
        //mdat.length = 0 = ends at EOF
        len = (uint32_t)file_size - atom->AtomicStart;
    } else if (atom->AtomicLengthExtended != 0 ) 
    {
        // Adding a (limited) uint64_t into a uint32_t
        len = atom->AtomicLengthExtended; 
    }

    if (md5sum != NULL)
    {
        mdd = mhash_init(MHASH_MD5);
        if (mdd == MHASH_FAILED) return 1;
    }
    if (sha1sum != NULL)
    {
        shd = mhash_init(MHASH_SHA1);
        if (shd == MHASH_FAILED) return 1;
    }

    if ((fp = fopen(fname, "r")) == NULL)
    {
        perror("fopen");
        return 1;
    }

    if (fseek(fp, (atom->AtomicStart - 1), SEEK_SET) != 0)
    {
        perror("fseek");
        return 2;
    }

    while ((cnt = fread(&bfr, 1, M4A_CHKSUM_BFR_SZ, fp)) > 0) 
    {
        if (md5sum != NULL) mhash(mdd, bfr, cnt);
        if (sha1sum != NULL) mhash(shd, bfr, cnt);
    }

    if (md5sum != NULL)
    {
        md5res = (unsigned char *) mhash_end(mdd);
        for (i = 0; i < (int)mhash_get_block_size(MHASH_MD5); i++) 
        {
            // printf("%.2x", md5res[i]);
            sprintf((char *)&md5sum[i<<1], "%.2x", md5res[i]);
        }
        // printf (" %s\n", md5sum);
    }

    if (sha1sum != NULL)
    {
        sha1res = (unsigned char *) mhash_end(shd);
        for (i = 0; i < (int)mhash_get_block_size(MHASH_SHA1); i++) 
        {
            // printf("%.2x", sha1res[i]);
            sprintf((char *)&sha1sum[i<<1], "%.2x", sha1res[i]);
        }
        // printf (" %s\n", sha1sum);
    }

    return 0;
}

int m4a_disp_tree()
{
    int i;
    for (i=0; i < atom_number; i++) { 
        AtomicInfo* atom = &parsedAtoms[i]; 
               
        fprintf(stdout, "%i  -  Atom \"%s\" (level %u) has next atom at #%i\n",
            i, atom->AtomicName, atom->AtomicLevel, 
            atom->NextAtomNumber);
        }

    return 0;
}

int m4a_get_atomidx(const char *name, int inst, int from)
{
    int idx    = from;
    int cnt    = 0;
    AtomicInfo*  atom;

    
    if ((atom_number == 0) || (name == NULL) ) return -1;
    while(1)
    {
        atom = &parsedAtoms[idx];

        if (memcmp(atom->AtomicName, name, 4) == 0)
        {
            /*
            fprintf(stdout, 
                "%i  -  Atom \"%s\" (level %u) has next atom at #%i\n",
                idx, atom->AtomicName, atom->AtomicLevel, 
                atom->NextAtomNumber);
            */

            if (++cnt == inst) return idx;
        }

        if (parsedAtoms[idx].NextAtomNumber == 0)
            break;
        idx = parsedAtoms[idx].NextAtomNumber;
    }

    // Not Found
    return -2;
}


int m4a_extract_art(int atmidx, M4A_ART *img)
{
    char* art = (char*)malloc( 
        sizeof(char) * (parsedAtoms[atmidx].AtomicLength-16) +1 );

    memset(art, 0, (parsedAtoms[atmidx].AtomicLength-16) +1);

    fseeko(source_file,parsedAtoms[atmidx].AtomicStart+16, SEEK_SET);
    fread(art, 1,
        parsedAtoms[atmidx].AtomicLength-16, source_file);
        
        
    if (memcmp(art, "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8) == 0) 
    {
        img->type = M4A_PNG;
    }
    else if (memcmp(art, "\xFF\xD8\xFF\xE0", 4) == 0 || 
        memcmp(art, "\xFF\xD8\xFF\xE1", 4) == 0) 
    {
        img->type = M4A_JPG;
    }
    else
        return 1;
        
    img->size = parsedAtoms[atmidx].AtomicLength-16;
    img->data = art;
    return 0;
}
