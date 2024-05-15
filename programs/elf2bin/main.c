/* Simple tool to lay out a static (no relocs) ELF32 program in a binary file.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#pragma pack(1)

struct elfhdr {
  uint8_t ident[4];
  uint8_t bits;
  uint8_t endian;
  uint8_t hdrver;
  uint8_t osabi;
  uint8_t pad[8];
  uint16_t type;
  uint16_t machine;
  uint32_t version;
  uint32_t entry;
  uint32_t phoff;
  uint32_t shoff;
  uint32_t flags;
  uint16_t ehsize;
  uint16_t phentsize;
  uint16_t phnum;
  uint16_t shentsize;
  uint16_t shnum;
  uint16_t shstrndx;
};

struct elf_phdr {
  uint32_t type;
  uint32_t offset;
  uint32_t vaddr;
  uint32_t paddr;
  uint32_t filesz;
  uint32_t memsz;
  uint32_t flags;
  uint32_t align;
};

#pragma pack()

static void usage(char *prog) {
  fprintf(stderr, "Usage: %s [-c] [-h] input output\n", prog);
}

int main(int argc, char *argv[]) {
  int needs_checksum = 0;

  int c = 0;
  while ((c = getopt(argc, argv, "ch")) != -1) {
    switch (c) {
      case 'c':
        // checksum the image and place in the last byte
        // needed for Option ROMs
        needs_checksum = 1;
        break;
      case 'h':
        usage(argv[0]);
        return 1;
      default:
        usage(argv[0]);
        return 1;
    }
  }

  if ((argc - optind) != 2) {
    usage(argv[0]);
    return 1;
  }

  FILE *elf = fopen(argv[optind], "rb");
  if (!elf) {
    perror("fopen");
    return 1;
  }

  struct elfhdr hdr;
  if (fread(&hdr, sizeof(hdr), 1, elf) < 1) {
    perror("fread");
    return 1;
  }

  if (hdr.bits != 1) {
    fprintf(stderr, "Only 32-bit ELF files are supported\n");
    return 1;
  }

  if (hdr.endian != 1) {
    fprintf(stderr, "Only little-endian ELF files are supported\n");
    return 1;
  }

  if (hdr.phentsize != sizeof(struct elf_phdr)) {
    fprintf(stderr, "Unexpected program header size\n");
    return 1;
  }

  fseek(elf, hdr.phoff, SEEK_SET);

  FILE *out = fopen(argv[optind + 1], "wb");

  uint8_t checksum = 0;

  // read all the program headers
  for (uint32_t i = 0; i < hdr.phnum; ++i) {
    struct elf_phdr phdr;
    if (fread(&phdr, sizeof(phdr), 1, elf) < 1) {
      perror("fread");
      return 1;
    }

    off_t pos = ftell(elf);
    fseek(elf, phdr.offset, SEEK_SET);

    fseek(out, phdr.paddr, SEEK_SET);

    uint8_t *buf = malloc(phdr.filesz);

    size_t n = fread(buf, 1, phdr.filesz, elf);
    if (n < phdr.filesz) {
      fprintf(stderr, "%ld < %d\n", n, phdr.filesz);
      perror("fread");
      return 1;
    }

    if (needs_checksum) {
      for (size_t i = 0; i < n; ++i) {
        checksum += buf[i];
      }
    }

    fwrite(buf, 1, n, out);

    free(buf);

    fseek(elf, pos, SEEK_SET);
  }

  if (needs_checksum) {
    fseek(out, -1, SEEK_END);
    fputc(~(checksum) + 1, out);
  }

  fclose(out);
  fclose(elf);

  return 0;
}
