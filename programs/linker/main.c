// Handcrafted 16-bit OMF linker to prepare a ROM image.
// It's not general-purpose, it assumes everything exists in one segment,
// and it doesn't support all OMF features. It emits exactly 64K bytes.

#include <arpa/inet.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "font.h"
#include "obj.h"

struct linker_object {
  struct obj *o;
  uint32_t base;
  const char *name;
  struct linker_object *next;
};

#pragma pack(1)

struct aout_hdr {
  uint32_t magic;
  uint32_t textsize;
  uint32_t datasize;
  uint32_t bsssize;
  uint32_t symsize;
  uint32_t entry;
  uint32_t textreloc_size;
  uint32_t datareloc_size;
};

struct aout_sym {
  uint32_t name;
  unsigned char type;
  char other;
  uint16_t desc;
  uint32_t value;
};

#pragma pack()

static int emit_binary(const char *filename, struct linker_object *objects);
static int emit_aout(const char *filename, struct linker_object *objects);

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

  emit_binary(argv[argc - 1], objects);

  struct linker_object *lo = objects;
  while (lo) {
    obj_clear_location(lo->o);
    lo = lo->next;
  }

  emit_aout("rom.aout", objects);

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

int find_symbol(const char *name, struct linker_object *objects,
                uint16_t *offset) {
  struct linker_object *lo = objects;
  while (lo) {
    struct symiter *si = obj_get_symiter(lo->o);
    if (si) {
      do {
        if (strcmp(obj_sym_get_name(si), name) == 0) {
          *offset = obj_sym_get_offset(si);
          while (obj_sym_next(si))
            ;
          return 0;
        }
      } while (obj_sym_next(si));
    }

    lo = lo->next;
  }

  return -1;
}

static int emit_binary(const char *filename, struct linker_object *objects) {
  // first pass, just put everything in memory
  // code first, data second
  uint32_t base = 0;
  struct linker_object *lo = objects;
  while (lo) {
    lo->base = base;
    base = obj_set_location(lo->o, 0, base);
    lo = lo->next;
  }

  // align code to 512 bytes
  if (base & 0x1ff) {
    base = (base + 0x200) & ~0x1ff;
  }

  lo = objects;
  while (lo) {
    lo->base = base;
    base = obj_set_location(lo->o, 1, base);
    lo = lo->next;
  }

  // align data to 512 bytes
  if (base & 0x1ff) {
    base = (base + 0x200) & ~0x1ff;
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
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for writing\n", filename);
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

  // fill in remaining fixed address content in the ROM image

  // POST entry point
  fseek(fp, 0xE05B, SEEK_SET);
  uint16_t postentry[2] = {0x0000, 0xF000};
  fwrite(postentry, sizeof(uint16_t), 2, fp);

  // INT 10h entry point
  fseek(fp, 0xF065, SEEK_SET);
  uint8_t int10h = 0xCF;  // IRET
  fwrite(&int10h, 1, 1, fp);

  // Low 128 characters of graphic video font
  fseek(fp, 0xFA6E, SEEK_SET);
  fwrite(vgafont8, 1, 128 * 8, fp);

  // dummy interrupt handler
  fseek(fp, 0xFF53, SEEK_SET);
  uint8_t dummyint = 0xCF;  // IRET
  fwrite(&dummyint, 1, 1, fp);

  // ROM date
  time_t now = time(NULL);
  struct tm *tm_info = localtime(&now);  // Convert to local time

  fseek(fp, 0xFFF5, SEEK_SET);
  char romdate[9];
  strftime(romdate, 9, "%m/%d/%y", tm_info);
  fwrite(romdate, 1, 8, fp);

  // System model
  fseek(fp, 0xFFFE, SEEK_SET);
  char model = 0xFE;
  fwrite(&model, 1, 1, fp);

  fclose(fp);

  return 0;
}

/*
; F000:E05B POST Entry Point
; F000:E2C3 NMI Entry Point
; F000:E6F2 INT 19 Entry Point
; F000:E6F5 Configuration Data Table
; F000:E729 Baut Rate Generator Table
; F000:E739 INT 14 Entry Point
; F000:E82E INT 16 Entry Point
; F000:E987 INT 09 Entry Point
; F000:EC59 INT 13 (Floppy) Entry Point
; F000:EF57 INT 0E Entry Point
; F000:EFC7 Floppy Disk Controller Parameter Table
; F000:EFD2 INT 17
; F000:F065 INT 10 (Video) Entry Point
; F000:F0A4 INT 1D MDA and CGA Video Parameter Table
; F000:F841 INT 12 Entry Point
; F000:F84D INT 11 Entry Point
; F000:F859 INT 15 Entry Point
; F000:FA6E Low 128 Characters of Graphic Video Font
; F000:FE6E INT 1A Entry Point
; F000:FEA5 INT 08 Entry Point
; F000:FF53 Dummy Interrupt Handler (IRET)
; F000:FF54 INT 05 (Print Screen) Entry Point
; F000:FFF0 Power-On Entry Point
; F000:FFF5 ROM Date in ASCII "MM/DD/YY" Format (8 Characters)
; F000:FFFE System Model (0xFC - AT, 0xFE - XT)
*/

static int emit_aout(const char *filename, struct linker_object *objects) {
  uint32_t base = 0;
  uint32_t num_syms = 0;

  // first pass, just put everything in memory
  // code first, data second
  struct linker_object *lo = objects;
  while (lo) {
    lo->base = base;
    base = obj_set_location(lo->o, 0, base);
    num_syms += obj_get_num_symbols(lo->o);
    lo = lo->next;
  }

  // align to 512 bytes
  if (base & 0x1ff) {
    base = (base + 0x200) & ~0x1ff;
  }

  uint32_t textsize = base;

  lo = objects;
  while (lo) {
    lo->base = base;
    base = obj_set_location(lo->o, 1, base);
    lo = lo->next;
  }

  // align to 512 bytes
  if (base & 0x1ff) {
    base = (base + 0x200) & ~0x1ff;
  }

  uint32_t datasize = base - textsize;

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

  struct aout_hdr hdr;
  memset(&hdr, 0, sizeof(struct aout_hdr));

  hdr.magic = 0407;
  hdr.textsize = textsize;
  hdr.datasize = datasize;
  hdr.symsize = num_syms * sizeof(struct aout_sym);

  // finally, write the output file
  FILE *fp = fopen(filename, "wb");
  if (!fp) {
    fprintf(stderr, "Failed to open %s for writing\n", filename);
    return 1;
  }

  if (fwrite(&hdr, sizeof(hdr), 1, fp) != 1) {
    fprintf(stderr, "Failed to write output file - short write\n");
    return 1;
  }

  if (fwrite(mem, 1, base, fp) < base) {
    fprintf(stderr, "Failed to write output file - short write\n");
    return 1;
  }

  uint32_t string_table_curr = 4;
  uint32_t string_table_start = string_table_curr;

  // write the symbol table
  lo = objects;
  int i = 0;
  while (lo) {
    struct symiter *si = obj_get_symiter(lo->o);
    if (si) {
      do {
        struct aout_sym sym;
        memset(&sym, 0, sizeof(struct aout_sym));

        const char *name = obj_sym_get_name(si);
        if (name) {
          sym.name = string_table_curr;
          string_table_curr += strlen(name) + 1;
        }

        sym.type = obj_sym_get_type(si);
        sym.value = obj_sym_get_offset(si);

        fwrite(&sym, 1, sizeof(struct aout_sym), fp);
        ++i;
      } while (obj_sym_next(si));
    }

    lo = lo->next;
  }

  uint32_t string_table_length = (string_table_curr - string_table_start) + 4;

  // write the string table
  fwrite(&string_table_length, 1, sizeof(string_table_length), fp);
  lo = objects;
  while (lo) {
    struct symiter *si = obj_get_symiter(lo->o);
    if (si) {
      do {
        const char *name = obj_sym_get_name(si);
        if (name) {
          fwrite(name, 1, strlen(name) + 1, fp);
        }
      } while (obj_sym_next(si));
    }

    lo = lo->next;
  }
  fputc(0x00, fp);

  free(mem);

  fclose(fp);

  return 0;
}
