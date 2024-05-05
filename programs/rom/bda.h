#ifndef _BDA_H
#define _BDA_H

#include <stdint.h>

#pragma pack(1)

struct optionrom_hdr {
  unsigned short signature;
  unsigned char length;  // in 512-byte blocks
};

struct ivt_entry {
  unsigned short offset;
  unsigned short segment;
};

struct bda_t {
  struct ivt_entry ivt[256];
  uint8_t poststack[256];
  uint16_t comport[4];
  uint16_t lptport[3];
  uint16_t ebda;  // segment for EBDA
  union {
    struct {
      uint16_t ipl : 1;
      uint16_t fpu : 1;
      uint16_t mouse : 1;
      uint16_t rsvd : 1;
      uint16_t initialvid : 2;
      uint16_t diskcount : 2;
      uint16_t nodma : 1;
      uint16_t numserials : 3;
      uint16_t gameport : 1;
      uint16_t rsvd2 : 1;
      uint16_t numprinters : 2;
    } equipflags;
    uint16_t equipflags_raw;
  };
  uint8_t rsvd1;
  uint16_t memsize;  // in Kbytes
  uint8_t rsvd2;
  uint8_t ctlflags;
  union {
    struct {
      uint8_t rshift : 1;
      uint8_t lshift : 1;
      uint8_t ctrl : 1;
      uint8_t alt : 1;
      uint8_t scrolllock : 1;
      uint8_t numlock : 1;
      uint8_t capslock : 1;
      uint8_t insert : 1;
    } kbflags0;
    uint8_t kbflags0_raw;
  };
  union {
    struct {
      uint8_t lctrl : 1;
      uint8_t lalt : 1;
      uint8_t syskey : 1;
      uint8_t suspkey : 1;
      uint8_t scrollkey : 1;
      uint8_t numkey : 1;
      uint8_t capskey : 1;
      uint8_t inskey : 1;
    } kbflags1;
    uint8_t kbflags1_raw;
  };
  uint8_t altkeypad;
  uint16_t kbhead;  // buffer head in kbbuf
  uint16_t kbtail;  // buffer tail in kbbuf
  uint8_t kbbuf[32];
  union {
    struct {
      uint8_t recal0 : 1;
      uint8_t recal1 : 1;
      uint8_t recal2 : 1;
      uint8_t recal3 : 1;
      uint8_t unused : 1;
      uint8_t intflag : 1;
    } recal;
    uint8_t recal_raw;
  };
  union {
    struct {
      uint8_t motor0 : 1;
      uint8_t motor1 : 1;
      uint8_t motor2 : 1;
      uint8_t motor3 : 1;
      uint8_t unused : 1;
      uint8_t writeop : 1;
    } motor;
    uint8_t motor_raw;
  };
  uint8_t motorshutoff;
  union {
    struct {
      uint8_t invalid : 1;
      uint8_t addrnotfound : 1;
      uint8_t sectornotfound : 1;
      uint8_t dmaerr : 1;
      uint8_t badcrc : 1;
      uint8_t ctlfail : 1;
      uint8_t seekfail : 1;
      uint8_t timeout : 1;
    } diskstatus;
    uint8_t diskstatus_raw;
  };
  uint8_t necstatus[7];
  uint8_t videomode;
  uint16_t screencols;
  uint16_t regensize;
  uint16_t videopage_offset;
  struct {
    uint8_t col;
    uint8_t row;
  } cursorpos[8];  // one cursor position per page
  uint8_t cursorbot;
  uint8_t cursortop;
  uint8_t videopage;
  uint16_t crtaddr;  // 3B4 = mono, 3D4 = color
  uint8_t crtmode;
  uint8_t palettemask;
  uint32_t daycounter;
  uint32_t timer;         // 0 at midnight
  uint8_t clockrollover;  // 1 if timer >24hrs
  uint8_t biosbreak;      // 1 if Ctrl-Break hit
  uint16_t softreset;
  uint8_t hdopstatus;
  uint8_t hdcount;
  uint8_t xthdctl;
  uint8_t fixeddisk_port;
  uint32_t lpttimeout;
  uint32_t comtimeout;
  uint16_t kbbufstart;  // seg=40h
  uint16_t kbbufend;    // seg=40h
  uint8_t screenrows;
  uint16_t charheight;
  union {
    struct {
      uint8_t alphacursor : 1;
      uint8_t mono : 1;
      uint8_t rsvd1 : 1;
      uint8_t inactive : 1;
      uint8_t rsvd2 : 1;
      uint8_t ram : 2;
      uint8_t mode : 1;
    } videomodeopts;
    uint8_t videomodeopts_raw;
  };
  uint8_t egafeatures;
  union {
    struct {
      uint8_t vga : 1;
      uint8_t gray : 1;
      uint8_t mono : 1;
      uint8_t nodefaultpal : 1;
      uint8_t mode0 : 1;  // bit4 for scan lines mode
      uint8_t rsvd : 1;
      uint8_t switchen : 1;
      uint8_t mode1 : 1;  // bit7 for scan lines mode
    } videodata;
    uint8_t videodata_raw;
  };
  uint8_t dccindex;
  uint8_t diskrate;
  uint8_t hdstatus;
  uint8_t hderror;
  uint8_t hdintctl;
  uint8_t hdcombo;
  struct {
    union {
      uint8_t state : 3;
      uint8_t rsvd : 1;
      uint8_t media : 1;
      uint8_t doublestep : 1;
      uint8_t rate : 2;
    } mediastate;
    uint8_t mediastate_raw;
  } mediastate[4];
  uint8_t disk0_track;
  uint8_t disk1_track;
  union {
    struct {
      uint8_t laste1 : 1;
      uint8_t laste0 : 1;
      uint8_t rctrl : 1;
      uint8_t ralt : 1;
      uint8_t enhanced : 1;
      uint8_t forcenumlock : 1;
      uint8_t lastid : 1;
      uint8_t idread : 1;
    } kbmode;
    uint8_t kbmode_raw;
  };
  union {
    struct {
      uint8_t scrolllock : 1;
      uint8_t numlock : 1;
      uint8_t capslock : 1;
      uint8_t circus : 1;
      uint8_t ack : 1;
      uint8_t resend : 1;
      uint8_t modeind : 1;
      uint8_t transmiterr : 1;
    } kbleds;
    uint8_t kbleds_raw;
  };
  uint32_t userwait_complete;
  uint32_t userwaitvalue;
  uint8_t rtcflag;
  uint8_t lanadmaflag;
  uint8_t lana0_status;
  uint8_t lana1_status;
  struct ivt_entry hdintvec;
  uint32_t videosotaddr;
  uint8_t rsvd_40ac[8];
  uint8_t kbnmi;
  uint32_t kbbreakpending;
  uint8_t port60queue;
  uint8_t scancode;
  uint8_t nmi_head;
  uint8_t nmi_tail;
  uint8_t nmi_buf[16];
  uint16_t daycounter2;
  uint8_t intraapp[16];
};

extern struct bda_t __far *bda;

#pragma pack()

#endif
