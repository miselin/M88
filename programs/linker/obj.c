
#include <fcntl.h>
#ifdef _MSC_VER
// for open/read/write/close/etc
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "obj.h"

#pragma pack(push, 1)

struct rechdr {
  uint8_t type;
  uint16_t len;
};

#pragma pack(pop)

struct record {
  struct record *next;
  struct rechdr hdr;
  off_t offset;
  uint8_t *data;
  int recnum;
};

struct lname {
  int recnum;
  uint8_t len;
  char name[256];
  struct lname *next;
};

struct segdef {
  // bytes of alignment needed, 0 = absolute segment
  uint16_t align;
  // 0 = private, 2/4 = public, 5 = stack, 6 = common
  // private should not be merged with another segment
  uint8_t combo;

  char name[256];
  char class_name[256];
  char overlay_name[256];

  struct segdef *next;

  // stores base address of this segment after object concatenation
  uint32_t final_offset;

  // group containing this segment, if any
  struct grpdef *grp;

  // stores offset of this segment within the group, if any
  uint32_t group_offset;

  // total size of this segment
  uint32_t total_size;
};

struct grpdef {
  char name[256];
  struct segdef *segments[32];
  struct grpdef *next;

  // stores base address of this group after object concatenation
  uint32_t final_offset;
};

struct extdef {
  char name[256];
  uint16_t type_index;
  struct extdef *next;
};

struct pubdef {
  char name[256];
  uint16_t offset;
  uint16_t type_index;
  struct segdef *seg;
  struct pubdef *next;
};

struct ledata {
  struct record *rec;
  struct segdef *seg;
  uint16_t offset;
  uint16_t datalen;
  uint8_t *data;
  struct ledata *next;

  // stores offset of this ledata after object cocatenation
  uint32_t final_offset;
};

struct fixup {
  struct ledata *ledata;
  struct fixup *next;

  // 1 = segment-relative, 0 = self-relative
  uint8_t segment_relative;

  uint8_t location;

  uint16_t frame_method;
  uint16_t frame;

  uint16_t target_method;
  uint16_t target;

  struct segdef *target_seg;
  struct grpdef *target_grp;
  struct extdef *target_ext;

  struct segdef *frame_seg;
  struct grpdef *frame_grp;
  struct extdef *frame_ext;

  uint16_t displacement;

  uint32_t data_location;

  int completed;
};

struct obj {
  FILE *fp;
  struct record *recs;
  struct lname *lnames;
  struct segdef *segdefs;
  struct extdef *extdefs;
  struct ledata *ledata;
  struct fixup *fixups;
  struct pubdef *pubdefs;
  struct grpdef *grpdefs;

  // total size of the object in memory, considering alignment
  uint32_t total_size;
};

// if index == 0x80, it's two bytes (byte_a & 0x7F << 8 + byte_b)

static uint16_t index2offset(uint8_t a, uint8_t b) {
  return ((a & 0x7F) << 8) | b;
}

static const char *type2name(struct rechdr *hdr);

static void load_segdefs(struct obj *o);
static void load_grpdefs(struct obj *o);
static void load_extdefs(struct obj *o);
static void load_pubdefs(struct obj *o);
static void load_fixupp(struct obj *o);
static void load_lnames(struct obj *o);
static void load_ledata(struct obj *o);
static void load_modend(struct obj *o);

static int is_code(const char *name, const char *class_name);

static struct segdef *get_segdef(struct obj *o, int index);
static struct grpdef *get_grpdef(struct obj *o, int index);
static struct extdef *get_extdef(struct obj *o, int index);
static int get_lname(struct obj *o, int index, char *buf, int len);

struct obj *obj_open(const char *path) {
  FILE *fp = fopen(path, "rb");
  if (!fp) {
    return NULL;
  }

  struct obj *o = (struct obj *)calloc(1, sizeof(struct obj));
  o->fp = fp;

  struct record *prev = NULL;

  int ok = 1;

  // preload headers for each section
  int recnum = 0;
  while (!feof(fp)) {
    struct record *rec = (struct record *)calloc(1, sizeof(struct record));

    if (fread(&rec->hdr, sizeof(rec->hdr), 1, fp) != 1) {
      free(rec);

      // if we're at EOF, this case is fine
      if (feof(fp)) {
        break;
      }

      ok = 0;
      break;
    }

    rec->offset = ftell(fp);
    rec->recnum = recnum++;

    fseek(fp, rec->hdr.len, SEEK_CUR);

    if (prev) {
      prev->next = rec;
    } else {
      o->recs = rec;
    }

    prev = rec;
  }

  fseek(fp, 0, SEEK_SET);

  if (!ok) {
    obj_close(o);
    return NULL;
  }

  load_lnames(o);
  load_segdefs(o);
  load_grpdefs(o);
  load_extdefs(o);
  load_pubdefs(o);
  load_ledata(o);
  load_fixupp(o);
  load_modend(o);

  // calculate total size we need for the object in memory
  struct ledata *le = o->ledata;
  while (le) {
    if (le->seg->align > 1) {
      if (o->total_size & (le->seg->align - 1)) {
        o->total_size +=
            le->seg->align - (o->total_size & (le->seg->align - 1));
      }
    }

    o->total_size += le->datalen;

    le = le->next;
  }

  return o;
}

uint32_t obj_get_size(struct obj *o) { return o->total_size; }

uint32_t obj_set_location(struct obj *o, int sort_class, uint32_t base) {
  struct ledata *le = o->ledata;
  while (le) {
    int code_seg = is_code(le->seg->name, le->seg->class_name);
    if (sort_class == 0 && !code_seg) {
      le = le->next;
      continue;
    } else if (sort_class == 1 && code_seg) {
      le = le->next;
      continue;
    }

    le->seg->total_size += le->datalen;

    le = le->next;
  }

  // now align all the segments
  struct segdef *seg = o->segdefs;
  while (seg) {
    int code_seg = is_code(seg->name, seg->class_name);
    if (sort_class == 0 && !code_seg) {
      seg = seg->next;
      continue;
    } else if (sort_class == 1 && code_seg) {
      seg = seg->next;
      continue;
    }

    if (seg->align > 1) {
      if (base & (seg->align - 1)) {
        base += seg->align - (base & (seg->align - 1));
      }
    }

    if (base < seg->final_offset) {
      seg->final_offset = base;
    }

    fprintf(stderr, "seg %s at %04X\n", seg->name, base);

    if (seg->grp) {
      if (base < seg->grp->final_offset) {
        seg->grp->final_offset = base;
      }

      seg->group_offset = base - seg->grp->final_offset;
    }

    struct ledata *le = o->ledata;
    while (le) {
      if (le->seg == seg) {
        le->final_offset = base;
        base += le->datalen;
      }

      le = le->next;
    }

    seg = seg->next;
  }

  /*


    if (le->seg->align > 1) {
      if (base & (le->seg->align - 1)) {
        base += le->seg->align - (base & (le->seg->align - 1));
      }
    }

    le->final_offset = base;

    if (base < le->seg->final_offset) {
      le->seg->final_offset = base;
    }

    if (le->seg->grp) {
      if (base < le->seg->grp->final_offset) {
        le->seg->grp->final_offset = base;
      }

      le->seg->group_offset = base - le->seg->grp->final_offset;
    }

    base += le->datalen;

  */

  return base;
}

void obj_load_to(struct obj *o, void *dest) {
  struct ledata *le = o->ledata;
  while (le) {
    memcpy((uint8_t *)dest + le->final_offset, le->data, le->datalen);
    le = le->next;
  }
}

void obj_dump(struct obj *o) {
  struct record *rec = o->recs;
  while (rec) {
    printf("record %d: %s\n", rec->recnum, type2name(&rec->hdr));
    rec = rec->next;
  }

  printf("segdefs:\n");
  struct segdef *seg = o->segdefs;
  while (seg) {
    printf("  %s [%s]\n", seg->name, seg->class_name);
    seg = seg->next;
  }

  printf("pubdefs:\n");
  struct pubdef *pub = o->pubdefs;
  while (pub) {
    printf("  %s @ %04X\n", pub->name, pub->offset);
    pub = pub->next;
  }

  printf("extdefs:\n");
  struct extdef *ext = o->extdefs;
  while (ext) {
    printf("  %s\n", ext->name);
    ext = ext->next;
  }
}

static int do_fixup(struct fixup *fix, struct pubdef *external, void *dest) {
  uint16_t target = 0;
  if (fix->target_seg) {
    target = fix->target_seg->final_offset;
  } else if (fix->target_grp) {
    target = fix->target_grp->final_offset;
  } else if (fix->target_ext) {
    if (!external) {
      return 0;
    }

    target = external->seg->final_offset + external->offset;
    fprintf(stderr, "external fixup %s -> %04X\n", fix->target_ext->name,
            target);
  }

  target += fix->displacement;

  uint16_t loc_offset = fix->ledata->final_offset + fix->data_location;
  uint8_t *loc = (uint8_t *)dest + loc_offset;

  uint16_t current16 = *((uint16_t *)loc);

  switch (fix->location) {
    // 0 = low order byte, 8-bit displacement / low byte of 16-bit offset
    case 0: {
      uint16_t value = target;

      if (fix->segment_relative) {
        value += *loc;
      } else {
        value -= loc_offset + 1;
      }

      *loc = value & 0xFF;
    } break;

    // 1 = 16-bit offset
    case 1:
    case 5: {
      uint16_t value = target;

      if (fix->segment_relative) {
        value += current16;
      } else {
        value -= loc_offset + 2;
      }

      *((uint16_t *)loc) = value;
    } break;

    // 2 = 16-bit base (logical segment base)
    case 2: {
      if (!fix->segment_relative) {
        fprintf(
            stderr,
            "invalid fixup relocation - 16-bit base can't be self-relative\n");
        return 0;
      }

      uint16_t value = target;
      value += current16 << 4;

      // convert relocated address to segment
      value >>= 4;

      *((uint16_t *)loc) = value;
    } break;

    // 3 = 32-bit long pointer (16-bit base : 16-bit offset)
    case 3: {
      if (!fix->segment_relative) {
        fprintf(stderr,
                "invalid fixup relocation - 32-bit long pointer can't be "
                "self-relative\n");
        return 0;
      }

      uint32_t value = target;
      value += *((uint32_t *)loc);

      // convert to segment:offset in 16:16 format
      value = ((value & 0xFFFF0000) << 12) | (value & 0xFFFF);

      *((uint32_t *)loc) = value;
    } break;

    default:
      fprintf(stderr, "unhandled relocation type %d\n", fix->location);
      break;
  }

  return 1;
}

void obj_relocate(struct obj *o, void *dest) {
  struct fixup *fix = o->fixups;
  while (fix) {
    // skip external relocations in this pass
    if (fix->target_ext || fix->frame_ext) {
      fix = fix->next;
      continue;
    }

    fix->completed = do_fixup(fix, NULL, dest);

    fix = fix->next;
  }
}

int obj_relocate_from(struct obj *o, struct obj *other, void *dest) {
  int pending = 0;

  struct fixup *fix = o->fixups;
  while (fix) {
    if (fix->completed) {
      // this symbol is relocated already
      fix = fix->next;
      continue;
    }

    // only external relocations in this pass
    if (!fix->target_ext) {
      fix = fix->next;
      continue;
    }

    // does the other object have a matching published symbol?
    struct pubdef *pub = other->pubdefs;
    while (pub) {
      if (!strcmp(fix->target_ext->name, pub->name)) {
        break;
      }

      pub = pub->next;
    }

    if (!pub) {
      ++pending;
    } else {
      fix->completed = do_fixup(fix, pub, dest);
    }

    fix = fix->next;
  }

  // returns 0 if no more relocations are needed
  return pending;
}

void obj_report_unresolved(struct obj *o) {
  struct fixup *fix = o->fixups;
  while (fix) {
    if (!fix->completed) {
      fprintf(stderr, "unresolved relocation for %s\n", fix->target_ext->name);
    }

    fix = fix->next;
  }
}

void obj_close(struct obj *o) {
  struct lname *ln = o->lnames;
  while (ln) {
    struct lname *next = ln->next;
    free(ln);
    ln = next;
  }

  struct pubdef *pub = o->pubdefs;
  while (pub) {
    struct pubdef *next = pub->next;
    free(pub);
    pub = next;
  }

  struct extdef *ext = o->extdefs;
  while (ext) {
    struct extdef *next = ext->next;
    free(ext);
    ext = next;
  }

  struct segdef *seg = o->segdefs;
  while (seg) {
    struct segdef *next = seg->next;
    free(seg);
    seg = next;
  }

  struct grpdef *grp = o->grpdefs;
  while (grp) {
    struct grpdef *next = grp->next;
    free(grp);
    grp = next;
  }

  struct ledata *le = o->ledata;
  while (le) {
    struct ledata *next = le->next;
    free(le);
    le = next;
  }

  struct fixup *fix = o->fixups;
  while (fix) {
    struct fixup *next = fix->next;
    free(fix);
    fix = next;
  }

  struct record *rec = o->recs;
  while (rec) {
    struct record *next = rec->next;
    if (rec->data) {
      free(rec->data);
    }
    free(rec);
    rec = next;
  }

  fclose(o->fp);
  free(o);
}

static void load_lnames(struct obj *o) {
  int recnum = 0;

  struct record *rec = o->recs;
  while (rec) {
    if (rec->hdr.type == 0x96) {
      if (!rec->data) {
        rec->data = (uint8_t *)calloc(1, rec->hdr.len - 1);
        fseek(o->fp, rec->offset, SEEK_SET);
        if (fread(rec->data, rec->hdr.len - 1, 1, o->fp) != 1) {
          printf("failed to read LNAME data\n");
          break;
        }
      }

      struct lname *prev = NULL;

      for (int i = 0; i < rec->hdr.len - 1;) {
        struct lname *ln = (struct lname *)calloc(1, sizeof(struct lname));
        ln->len = rec->data[i++];
        memcpy(ln->name, rec->data + i, ln->len);
        i += ln->len;

        ln->recnum = recnum;

        // add to end of list, as indices are ordered
        if (prev) {
          prev->next = ln;
        } else {
          o->lnames = ln;
        }

        prev = ln;
      }
    }

    rec = rec->next;
    ++recnum;
  }
}

static void load_segdefs(struct obj *o) {
  struct segdef *prev = NULL;

  struct record *rec = o->recs;
  while (rec) {
    if (rec->hdr.type == 0x98) {
      if (!rec->data) {
        rec->data = (uint8_t *)calloc(1, rec->hdr.len - 1);
        fseek(o->fp, rec->offset, SEEK_SET);
        if (fread(rec->data, rec->hdr.len - 1, 1, o->fp) != 1) {
          printf("failed to read SEGDEF data\n");
          break;
        }
      }

      // first byte is variable length
      uint8_t attrs = rec->data[0];

      struct segdef *seg = (struct segdef *)calloc(1, sizeof(struct segdef));
      uint16_t align = (attrs >> 5) & 7;
      seg->combo = (attrs >> 2) & 7;
      seg->final_offset = ~0U;
      seg->total_size = 0;

      int alignment = 1;
      switch (align) {
        case 1:
          alignment = 1;
          break;
        case 2:
          alignment = 2;
          break;
        case 3:
          alignment = 16;
          break;
        case 4:
          alignment = 256;
          break;
        case 5:
          alignment = 4;
          break;
        default:
          printf("WARN: unhandled SEGDEF alignment %d\n", seg->align);
          alignment = 1;
          break;
      }

      seg->align = alignment;

      int next = 3;
      if (rec->data[next] & 0x80) {
        get_lname(o, index2offset(rec->data[next], rec->data[next + 1]),
                  seg->name, sizeof(seg->name));
        next += 2;
      } else {
        get_lname(o, rec->data[next], seg->name, sizeof(seg->name));
        ++next;
      }

      if (rec->data[next] & 0x80) {
        get_lname(o, index2offset(rec->data[next], rec->data[next + 1]),
                  seg->class_name, sizeof(seg->class_name));
        next += 2;
      } else {
        get_lname(o, rec->data[next], seg->class_name, sizeof(seg->class_name));
        ++next;
      }

      if (rec->data[next] & 0x80) {
        get_lname(o, index2offset(rec->data[next], rec->data[next + 1]),
                  seg->overlay_name, sizeof(seg->overlay_name));
        next += 2;
      } else {
        get_lname(o, rec->data[next], seg->overlay_name,
                  sizeof(seg->overlay_name));
        ++next;
      }

      if (prev) {
        prev->next = seg;
      } else {
        o->segdefs = seg;
      }

      prev = seg;
    }

    rec = rec->next;
  }
}

static void load_grpdefs(struct obj *o) {
  struct grpdef *prev = NULL;

  struct record *rec = o->recs;
  while (rec) {
    if (rec->hdr.type == 0x9A) {
      if (!rec->data) {
        rec->data = (uint8_t *)calloc(1, rec->hdr.len - 1);
        fseek(o->fp, rec->offset, SEEK_SET);
        if (fread(rec->data, rec->hdr.len - 1, 1, o->fp) != 1) {
          printf("failed to read GRPDEF data\n");
          break;
        }
      }

      struct grpdef *grp = (struct grpdef *)calloc(1, sizeof(struct grpdef));
      grp->final_offset = ~0U;

      int offset = 0;

      uint16_t index = rec->data[offset];
      if (index & 0x80) {
        index = index2offset(rec->data[offset], rec->data[offset + 1]);
        offset += 2;
      } else {
        ++offset;
      }

      get_lname(o, index, grp->name, 256);

      int segn = 0;
      while (offset < rec->hdr.len - 1) {
        // skip "index" which is always 0xFF
        ++offset;

        uint16_t seg_index = rec->data[offset];
        if (seg_index & 0x80) {
          seg_index = index2offset(rec->data[offset], rec->data[offset + 1]);
          offset += 2;
        } else {
          ++offset;
        }

        struct segdef *seg = get_segdef(o, seg_index);

        grp->segments[segn++] = seg;

        seg->grp = grp;
      }

      if (prev) {
        prev->next = grp;
      } else {
        o->grpdefs = grp;
      }

      prev = grp;
    }

    rec = rec->next;
  }
}

static void load_extdefs(struct obj *o) {
  struct extdef *prev = NULL;

  struct record *rec = o->recs;
  while (rec) {
    if (rec->hdr.type == 0x8C) {
      if (!rec->data) {
        rec->data = (uint8_t *)calloc(1, rec->hdr.len - 1);
        fseek(o->fp, rec->offset, SEEK_SET);
        if (fread(rec->data, rec->hdr.len - 1, 1, o->fp) != 1) {
          printf("failed to read EXTDEF data\n");
          break;
        }
      }

      uint8_t *data = rec->data;

      int offset = 0;
      while (offset < (rec->hdr.len - 1)) {
        struct extdef *ext = (struct extdef *)calloc(1, sizeof(struct extdef));

        int len = data[offset];
        strncpy(ext->name, (const char *)&data[offset + 1], len);

        offset += len + 1;

        ext->type_index = data[offset];
        if (ext->type_index & 0x80) {
          ext->type_index = index2offset(data[offset], data[offset + 1]);
          offset += 2;
        } else {
          ++offset;
        }

        if (prev) {
          prev->next = ext;
        } else {
          o->extdefs = ext;
        }

        prev = ext;
      }
    }

    rec = rec->next;
  }
}

static void load_pubdefs(struct obj *o) {
  struct pubdef *prev = NULL;

  struct record *rec = o->recs;
  while (rec) {
    if (rec->hdr.type == 0x90) {
      if (!rec->data) {
        rec->data = (uint8_t *)calloc(1, rec->hdr.len - 1);
        fseek(o->fp, rec->offset, SEEK_SET);
        if (fread(rec->data, rec->hdr.len - 1, 1, o->fp) != 1) {
          printf("failed to read PUBDEF data\n");
          break;
        }
      }

      int offset = 0;

      struct segdef *seg = NULL;

      uint16_t group_index = rec->data[offset];
      if (group_index & 0x80) {
        group_index = index2offset(rec->data[offset], rec->data[offset + 1]);
        offset += 2;
      } else {
        ++offset;
      }

      uint16_t seg_index = rec->data[offset];
      if (seg_index & 0x80) {
        seg_index = index2offset(rec->data[offset], rec->data[offset + 1]);
        offset += 2;
      } else {
        ++offset;
      }

      // TODO: this could point at a grpdef

      if (seg_index == 0) {
        // skip "base frame"
        offset += 2;
      } else {
        seg = get_segdef(o, seg_index);
      }

      while (offset < (rec->hdr.len - 1)) {
        struct pubdef *pub = (struct pubdef *)calloc(1, sizeof(struct pubdef));

        pub->seg = seg;

        int len = rec->data[offset];
        strncpy(pub->name, (const char *)&rec->data[offset + 1], len);
        offset += len + 1;

        pub->offset = *((uint16_t *)&rec->data[offset]);
        offset += 2;

        pub->type_index = rec->data[offset];
        if (pub->type_index & 0x80) {
          pub->type_index =
              index2offset(rec->data[offset], rec->data[offset + 1]);
          offset += 2;
        } else {
          ++offset;
        }

        if (prev) {
          prev->next = pub;
        } else {
          o->pubdefs = pub;
        }

        prev = pub;
      }
    }

    rec = rec->next;
  }
}

static void load_fixupp(struct obj *o) {
  struct fixup *prev = NULL;

  struct record *rec = o->recs;
  while (rec) {
    if (rec->hdr.type == 0x9C) {
      if (!rec->data) {
        rec->data = (uint8_t *)calloc(1, rec->hdr.len - 1);
        fseek(o->fp, rec->offset, SEEK_SET);
        if (fread(rec->data, rec->hdr.len - 1, 1, o->fp) != 1) {
          printf("failed to read FIXUPP data\n");
          break;
        }
      }

      int offset = 0;
      while (offset < rec->hdr.len - 1) {
        uint8_t first = rec->data[offset++];

        if ((first & 0x80) == 0) {
          // THREAD
          // TODO
          fprintf(stderr, "unhandled THREAD fixup\n");
          abort();
        } else {
          // FIXUP

          // find the relevant ledata section
          struct ledata *le = o->ledata;
          struct ledata *prev_le = o->ledata;
          while (le) {
            if (le->rec->recnum > rec->recnum) {
              break;
            }

            prev_le = le;
            le = le->next;
          }

          if (!prev_le || (prev_le->rec->recnum > rec->recnum)) {
            printf("no ledata found before fixupp record\n");
            break;
          }

          uint8_t locat_b = rec->data[offset++];

          uint16_t datarec_offset = locat_b | ((first & 0x3) << 8);

          uint8_t location = (first >> 2) & 0xf;
          uint8_t mode = (first >> 6) & 0x1;
          // uint8_t always1 = (first >> 7) & 0x1;

          uint8_t fixdata = rec->data[offset++];
          uint8_t targt = fixdata & 0x3;
          uint8_t p = (fixdata >> 2) & 0x1;
          // uint8_t t = (fixdata >> 3) & 0x1;
          uint8_t frame = (fixdata >> 4) & 0x7;
          // uint8_t f = (fixdata >> 7) & 0x1;

          uint16_t frame_datum = 0;
          if (frame < 3) {
            frame_datum = rec->data[offset];
            if (frame_datum & 0x80) {
              frame_datum =
                  index2offset(rec->data[offset], rec->data[offset + 1]);
              offset += 2;
            } else {
              ++offset;
            }
          }

          uint8_t target_method = (p << 2) | targt;

          uint16_t target_datum = rec->data[offset];
          if (target_datum & 0x80) {
            target_datum =
                index2offset(rec->data[offset], rec->data[offset + 1]);
            offset += 2;
          } else {
            ++offset;
          }

          uint16_t displacement = 0;
          if (p == 0) {
            displacement = *((uint16_t *)&rec->data[offset]);
            offset += 2;
          }

          struct fixup *fix = (struct fixup *)calloc(1, sizeof(struct fixup));
          fix->ledata = prev_le;
          fix->segment_relative = mode;
          fix->location = location;
          fix->frame_method = frame;
          fix->frame = frame_datum;
          fix->target_method = target_method;
          fix->target = target_datum;
          fix->displacement = displacement;
          fix->data_location = datarec_offset;

          if (target_method == 0 || target_method == 4) {
            // T0/T4 - SEGDEF index
            fix->target_seg = get_segdef(o, target_datum);
          } else if (target_method == 1 || target_method == 5) {
            // T1/T5 - GRPDEF index
            fix->target_grp = get_grpdef(o, target_datum);
          } else if (target_method == 2 || target_method == 6) {
            // T2/T6 - EXTDEF index
            fix->target_ext = get_extdef(o, target_datum);
          } else {
            fprintf(stderr, "unhandled target method %d\n", target_method);
            abort();
          }

          // TODO: other frame methods
          if (frame == 0) {
            // F0 - SEGDEF index
            fix->frame_seg = get_segdef(o, frame_datum);
          } else if (frame == 1) {
            // F1 - GRPDEF index
            fix->frame_grp = get_grpdef(o, frame_datum);
          } else if (frame == 2) {
            // F2 - EXTDEF index
            fix->frame_ext = get_extdef(o, frame_datum);
          } else if (frame == 4) {
            // F4 - previous LEDATA/LIDATA segment
            fix->frame_seg = prev_le->seg;
          } else if (frame == 5) {
            // F5 - target segment/group/external index
            fix->frame_seg = NULL;
            fix->frame_grp = NULL;
            fix->frame_ext = NULL;
          } else {
            fprintf(stderr, "unhandled frame method %d\n", frame);
            abort();
          }

          if (prev) {
            prev->next = fix;
          } else {
            o->fixups = fix;
          }

          prev = fix;
        }
      }
    }

    rec = rec->next;
  }
}

static void load_ledata(struct obj *o) {
  struct ledata *prev = NULL;

  struct record *rec = o->recs;
  while (rec) {
    if (rec->hdr.type == 0xA0) {
      if (!rec->data) {
        rec->data = (uint8_t *)calloc(1, rec->hdr.len - 1);
        fseek(o->fp, rec->offset, SEEK_SET);
        if (fread(rec->data, rec->hdr.len - 1, 1, o->fp) != 1) {
          printf("failed to read LEDATA data\n");
          break;
        }
      }

      struct ledata *le = (struct ledata *)calloc(1, sizeof(struct ledata));

      int offset = 0;

      uint16_t index = rec->data[offset];
      if (index & 0x80) {
        index = index2offset(rec->data[offset], rec->data[offset + 1]);
        offset += 2;
      } else {
        ++offset;
      }

      le->seg = get_segdef(o, index);
      le->offset = *((uint16_t *)&rec->data[offset]);
      offset += 2;

      le->datalen = rec->hdr.len - 1 - offset;
      le->data = &rec->data[offset];

      le->rec = rec;

      if (!prev) {
        o->ledata = le;
      } else {
        prev->next = le;
      }

      prev = le;
    }

    rec = rec->next;
  }
}

static void load_modend(struct obj *o) {
  struct record *rec = o->recs;
  while (rec) {
    if (rec->hdr.type == 0x8A) {
      if (!rec->data) {
        rec->data = (uint8_t *)calloc(1, rec->hdr.len - 1);
        fseek(o->fp, rec->offset, SEEK_SET);
        if (fread(rec->data, rec->hdr.len - 1, 1, o->fp) != 1) {
          printf("failed to read MODEND data\n");
          break;
        }
      }

      /*
      uint8_t modtype = rec->data[0];

      uint8_t segbit = (modtype >> 5) & 0x1;
      uint8_t start = (modtype >> 6) & 0x1;
      uint8_t main = (modtype >> 7) & 0x1;
      */

      // TODO: extract start address, use it for the long jump at reset vector
    }

    rec = rec->next;
  }
}

static int is_code(const char *name, const char *class_name) {
  if (!strcmp(class_name, "code") || !strcmp(class_name, "CODE")) {
    return 1;
  }

  if (!strcmp(name, "code") || !strcmp(name, "text") ||
      !strcmp(name, "__NASMDEFSEG") || !strcmp(name, "MAIN_TEXT")) {
    return 1;
  }

  return 0;
}

static struct segdef *get_segdef(struct obj *o, int index) {
  --index;

  struct segdef *seg = o->segdefs;
  while (seg) {
    if (!index--) {
      return seg;
    }

    seg = seg->next;
  }

  return NULL;
}

static struct grpdef *get_grpdef(struct obj *o, int index) {
  --index;

  struct grpdef *grp = o->grpdefs;
  while (grp) {
    if (!index--) {
      return grp;
    }

    grp = grp->next;
  }

  return NULL;
}

static struct extdef *get_extdef(struct obj *o, int index) {
  --index;

  struct extdef *ext = o->extdefs;
  while (ext) {
    if (!index--) {
      return ext;
    }

    ext = ext->next;
  }

  return NULL;
}

static int get_lname(struct obj *o, int index, char *buf, int len) {
  // indices are 1-based, but don't error out on a zero index
  if (!index) {
    *buf = 0;
    return 0;
  }

  struct lname *ln = o->lnames;
  int n = 1;
  while (ln) {
    if (n++ == index) {
      strncpy(buf, ln->name, len);
      return 0;
    }

    ln = ln->next;
  }

  return 1;
}

static const char *type2name(struct rechdr *hdr) {
  switch (hdr->type) {
    case 0x80:
      return "theadr";
    case 0x82:
      return "lheadr";
    case 0x88:
      return "coment";
    case 0x8A:
      return "modend";
    case 0x8B:
      return "modend (32-bit)";
    case 0x8C:
      return "extdef";
    case 0x90:
      return "pubdef";
    case 0x91:
      return "pubdef (32-bit)";
    case 0x94:
      return "linnum";
    case 0x95:
      return "linnum (32-bit)";
    case 0x96:
      return "lnames";
    case 0x98:
      return "segdef";
    case 0x99:
      return "segdef (32-bit)";
    case 0x9A:
      return "grpdef";
    case 0x9C:
      return "fixupp";
    case 0x9D:
      return "fixupp (32-bit)";
    case 0xA0:
      return "ledata";
    case 0xA1:
      return "ledata (32-bit)";
    case 0xA2:
      return "lidata";
    case 0xA3:
      return "lidata (32-bit)";
    case 0xB0:
      return "comdef";
    case 0xB2:
      return "bakpat";
    case 0xB3:
      return "bakpat (32-bit)";
    case 0xB4:
      return "lextdef";
    case 0xB5:
      return "lextdef (32-bit)";
    case 0xB6:
      return "lpubdef";
    case 0xB7:
      return "lpubdef (32-bit)";
    case 0xB8:
      return "lcomdef";
    case 0xBC:
      return "cextdef";
    case 0xC2:
      return "comdat";
    case 0xC3:
      return "comdat (32-bit)";
    case 0xC4:
      return "linsym";
    case 0xC5:
      return "linsym (32-bit)";
    case 0xC6:
      return "alias";
    case 0xC8:
      return "nbakpat";
    case 0xC9:
      return "nbakpat (32-bit)";
    case 0xCA:
      return "llnames";
    case 0xCC:
      return "vernum";
    case 0xCE:
      return "vendext";
    default:
      return "<unknown>";
  }
}
