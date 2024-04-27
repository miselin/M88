// Handcrafted 16-bit OMF linker to prepare a ROM image.
// It's not general-purpose, it assumes everything exists in one segment,
// and it doesn't support all OMF features. It emits exactly 64K bytes.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "obj.h"

struct linker_object {
  struct obj *o;
  uint32_t base;
  const char *name;
  struct linker_object *next;
};

static void emit_pad(FILE *fp, uint32_t count) {
  for (uint32_t i = 0; i < count; ++i) {
    fputc(0x90, fp);
  }
}

int main(int argc, const char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s inputs... output\n", argv[0]);
    return 1;
  }

  struct linker_object *objects = NULL;
  struct linker_object *prev = NULL;

  for (int i = 1; i < argc - 1; ++i) {
    struct obj *o = obj_open(argv[i]);
    if (!o) {
      fprintf(stderr, "Failed to open %s\n", argv[i]);
      return 1;
    }

    // printf("Object %s:\n", argv[i]);
    // obj_dump(o);

    fflush(stdout);

    struct linker_object *lo = malloc(sizeof(struct linker_object));
    lo->o = o;
    lo->base = 0;
    lo->name = argv[i];
    lo->next = NULL;

    if (!prev) {
      objects = lo;
    } else {
      prev->next = lo;
    }

    prev = lo;
  }

  // first pass, just put everything in memory
  // code first, data second
  uint32_t base = 0;
  struct linker_object *lo = objects;
  while (lo) {
    lo->base = base;
    base = obj_set_location(lo->o, 0, base);
    lo = lo->next;
  }

  lo = objects;
  while (lo) {
    lo->base = base;
    base = obj_set_location(lo->o, 1, base);
    lo = lo->next;
  }

  // create the binary image, and fill it with nops
  uint8_t *mem = (uint8_t *)malloc(base);
  memset(mem, 0x90, base);

  lo = objects;
  while (lo) {
    obj_load_to(lo->o, mem);
    lo = lo->next;
  }

  // second pass, fix up relocations within each object
  lo = objects;
  while (lo) {
    obj_relocate(lo->o, mem);
    lo = lo->next;
  }

  // third pass, fix up relocations that require a symbol from another object
  int ok = 0;
  for (int attempt = 0; attempt < 3; ++attempt) {
    int relocated = 0;

    lo = objects;
    while (lo) {
      struct linker_object *lo2 = objects;
      while (lo2) {
        if (lo2 != lo) {
          relocated += obj_relocate_from(lo->o, lo2->o, mem);
        }

        lo2 = lo2->next;
      }

      lo = lo->next;
    }

    if (relocated == 0) {
      ok = 1;
      break;
    }
  }

  if (!ok) {
    fprintf(stderr, "Failed to resolve all relocations\n");

    lo = objects;
    while (lo) {
      obj_report_unresolved(lo->o);
      lo = lo->next;
    }

    return 1;
  }

  // finally, write the output file
  FILE *fp = fopen(argv[argc - 1], "wb");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for writing\n", argv[argc - 1]);
    return 1;
  }

  if (fwrite(mem, 1, base, fp) < base) {
    fprintf(stderr, "Failed to write output file - short write\n");
    return 1;
  }

  free(mem);

  // pad the ROM image to add the reset vector at 0xFFF0
  uint32_t pad = 0xFFF0 - base;
  emit_pad(fp, pad);

  uint8_t reset_jump[5] = {0xEA, 0x00, 0x00, 0x00, 0xF0};
  fwrite(reset_jump, 1, 5, fp);

  emit_pad(fp, 0x10 - 5);

  fclose(fp);

  // all done, clean up
  lo = objects;
  while (lo) {
    struct linker_object *next = lo->next;
    obj_close(lo->o);
    free(lo);
    lo = next;
  }

  return 0;
}
