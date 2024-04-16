#ifndef _OBJ_H
#define _OBJ_H

struct obj;

struct obj *obj_open(const char *path);
uint32_t obj_get_size(struct obj *o);
uint32_t obj_set_location(struct obj *o, int sort_class, uint32_t base);
void obj_load_to(struct obj *o, void *dest);
void obj_relocate(struct obj *o, void *dest);
int obj_relocate_from(struct obj *o, struct obj *other, void *dest);
void obj_dump(struct obj *o);
void obj_report_unresolved(struct obj *o);
void obj_close(struct obj *obj);

#endif
