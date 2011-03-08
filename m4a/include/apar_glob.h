#ifndef __APAR_GLOB_H__
#define __APAR_GLOB_H__

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <math.h>
#include <wchar.h>

#include "AtomicParsley.h"
//#include "AP_AtomDefinitions.h"
/*
#include "AP_iconv.h"
#include "AtomicParsley_genres.h"
#include "APar_uuid.h"

#if defined (DARWIN_PLATFORM)
#include "AP_NSImage.h"
#include "AP_NSFile_utils.h"
#endif
*/


////////////////////////////////////////////////////////////////////////////////
//                        Global Variables                                    //
////////////////////////////////////////////////////////////////////////////////

/***
extern bool modified_atoms;
extern bool alter_original;

extern bool svn_build;

extern FILE* source_file;
//extern uint32_t file_size;

extern struct AtomicInfo parsedAtoms[MAX_ATOMS]; //(most I've seen is 144 for an untagged mp4)
extern short atom_number;
extern uint8_t generalAtomicLevel;

extern bool file_opened;
extern bool parsedfile;
extern bool move_moov_atom;
extern bool moov_atom_was_mooved;
extern AtomicInfo* hdlrAtom;
extern AtomicInfo* udtaAtom;
extern bool complete_free_space_erasure;
extern bool initial_optimize_pass;
extern bool psp_brand;
extern bool prevent_update_using_padding;
extern int metadata_style;
extern bool tree_display_only;

extern uint32_t max_buffer;

extern uint32_t bytes_before_mdat;
extern uint32_t bytes_into_mdat;
extern uint64_t mdat_supplemental_offset;
extern uint32_t removed_bytes_tally;
extern uint32_t new_file_size;
extern uint32_t brand;
//extern uint32_t mdatData;

//extern uint32_t gapless_void_padding;

extern struct udta_stats udta_dynamics;
extern struct padding_preferences pad_prefs;

extern bool contains_unsupported_64_bit_atom;

#if defined (WIN32) || defined (__CYGWIN__)
extern short max_display_width;
#else
extern short max_display_width;
#endif
extern char* file_progress_buffer;

extern struct PicPrefs myPicturePrefs;
extern bool parsed_prefs;
extern char* twenty_byte_buffer;

extern EmployedCodecs track_codecs;

extern uint8_t UnicodeOutputStatus;

extern uint8_t forced_suffix_type;

extern void APar_PrintUnicodeAssest(char* unicode_string, int asset_length);
extern void APar_SimplePrintUnicodeAssest(char* unicode_string, int asset_length, bool print_encoding);
extern short APar_FindParentAtom(int order_in_tree, uint8_t this_atom_level);
extern void APar_ExtractDataAtom(int this_atom_number);
extern void APar_fprintf_UTF8_data(char* utf8_encoded_data);
extern uint32_t APar_ProvideTallyForAtom(char* atom_name);
extern void APar_ExtractAAC_Artwork(short this_atom_num, char* pic_output_path, short artwork_count);
extern void json_ExtractDataAtom(int this_atom_number);

extern uint32_t APar_DetermineMediaData_AtomPosition();
extern void APar_SimpleAtomPrintout();
**/

#endif
