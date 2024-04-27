#ifndef _OBJ_H
#define _OBJ_H

struct obj;

#define OBJ_SEG_UNKNOWN 0
#define OBJ_SEG_TEXT 1
#define OBJ_SEG_DATA 2
#define OBJ_SEG_BSS 3

#define OBJ_GRP_UNKNOWN 0
#define OBJ_GRP_TEXT 1
#define OBJ_GRP_DATA 2

#define OBJ_SYM_TEXT 4
#define OBJ_SYM_DATA 6
#define OBJ_SYM_BSS 8

// Open the given obj file
struct obj *obj_open(const char *path);

// Get the number of public symbols in this file
uint32_t obj_get_num_symbols(struct obj *o);

// Get the total size of all loaded segments
uint32_t obj_get_size(struct obj *o);

// Set the location of the object file in memory
uint32_t obj_set_location(struct obj *o, int sort_class, uint32_t base);

// Remove all locations/offsets to allow re-relocation
void obj_clear_location(struct obj *o);

// Write the object's data to the given memory location
void obj_load_to(struct obj *o, void *dest);

// Perform initial relocations on the object file
void obj_relocate(struct obj *o, void *dest);

// Perform relocations that require symbols from another object file
int obj_relocate_from(struct obj *o, struct obj *other, void *dest);

// Dump the object file to stdout
void obj_dump(struct obj *o);

// Report any unresolved symbols in the object file
void obj_report_unresolved(struct obj *o);

// Close the object file
void obj_close(struct obj *obj);

// Retrieve a group iterator for the object file
struct grpiter *obj_get_grpiter(struct obj *o);

const char *obj_grp_get_name(struct grpiter *gi);
uint32_t obj_grp_get_type(struct grpiter *gi);
uint32_t obj_grp_get_size(struct grpiter *gi);
uint32_t obj_grp_get_base(struct grpiter *gi);
void obj_grp_set_base(struct grpiter *gi, uint32_t base);

// Returns 0 when the iterator is at the end of the group list
// The iterator will be freed when this function returns 0
int obj_grp_next(struct grpiter *gi);

// Retrieve a segment iterator for the object file
// This will skip over any segments that are part of a group
struct segiter *obj_get_segiter(struct obj *o);

const char *obj_seg_get_name(struct segiter *si);
uint32_t obj_seg_get_type(struct segiter *si);
uint32_t obj_seg_get_size(struct segiter *si);
uint32_t obj_seg_get_base(struct segiter *si);
void obj_seg_set_base(struct segiter *si, uint32_t base);

// Returns 0 when the iterator is at the end of the group list
// The iterator will be freed when this function returns 0
int obj_seg_next(struct segiter *si);

struct symiter *obj_get_symiter(struct obj *o);

const char *obj_sym_get_name(struct symiter *si);
uint32_t obj_sym_get_offset(struct symiter *si);
uint32_t obj_sym_get_type(struct symiter *si);

int obj_sym_next(struct symiter *si);

#endif
