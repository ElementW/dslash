/* ndstrim by recover89@gmail.com
   Trims NDS roms fast and reliable.
   ROM size is available in four bytes at 0x80-0x83.
   Wifi data is stored on 136 bytes after ROM data.
   Filesize is checked to be at least 0x200 bytes to make sure it contains a DS cartridge header.
   Filesize is then checked to be at least the rom size+wifi to avoid errors.
   Source code licensed under GNU GPL version 2 or later.
   
   Sources:
   http://nocash.emubase.de/gbatek.htm
   http://forums.ds-xtreme.com/showthread.php?t=1964
   http://gbatemp.net/index.php?showtopic=44022
*/

#ifndef NDSTRIM_H
#define NDSTRIM_H

#include <endian.h>


// determine byte order.  nds rom files are little endian
#if __BYTE_ORDER == __LITTLE_ENDIAN
#  define endian_16(a) a
#  define endian_32(a) a
#else // __BYTE_ORDER == __BIG_ENDIAN
#  define endian_16(a) ((((uint16_t)(a) & 0xff00) >> 8) | \
                        (((uint16_t)(a) & 0x00ff) << 8))
#  define endian_32(a) ((((uint32_t)(a) & 0xff000000) >> 24) | \
                        (((uint32_t)(a) & 0x00ff0000) >> 8)  | \
                        (((uint32_t)(a) & 0x0000ff00) << 8)  | \
                        (((uint32_t)(a) & 0x000000ff) << 24))
#endif




//
// Constants/Macros
//


//
// structure declarations
//


typedef struct __attribute__ ((packed)) {
    uint8_t  game_title[12];// 000h    12     Game Title  (Uppercase ASCII, padded with 00h)
    uint8_t  game_code[4];  // 00Ch    4      Gamecode    (Uppercase ASCII, NTR-<code>)        (0=homebrew)
    uint8_t  maker_code[2]; // 010h    2      Makercode   (Uppercase ASCII, eg. "01"=Nintendo) (0=homebrew)
    uint8_t  unit_code;     // 012h    1      Unitcode    (00h=Nintendo DS)
    uint8_t  enc_seed_sel;  // 013h    1      Encryption Seed Select (00..07h, usually 00h)
    uint8_t  dev_capacity;  // 014h    1      Devicecapacity         (Chipsize = 128KB SHL nn) (eg. 7 = 16MB)
    uint8_t  reserve1[9];   // 015h    9      Reserved           (zero filled)
    uint8_t  rom_ver;       // 01Eh    1      ROM Version        (usually 00h)
    uint8_t  auto_start;    // 01Fh    1      Autostart (Bit2: Skip "Press Button" after Health and Safety)
                            // (Also skips bootmenu, even in Manual mode & even Start pressed)
    uint32_t rom_offset9;   // 020h    4      ARM9 rom_offset    (4000h and up, align 1000h)
    uint32_t ent_addr9;     // 024h    4      ARM9 entry_address (2000000h..23BFE00h)
    uint32_t ram_addr9;     // 028h    4      ARM9 ram_address   (2000000h..23BFE00h)
    uint32_t size9;         // 02Ch    4      ARM9 size          (max 3BFE00h) (3839.5KB)
    uint32_t rom_offset4;   // 030h    4      ARM7 rom_offset    (8000h and up)
    uint32_t ent_addr7;     // 034h    4      ARM7 entry_address (2000000h..23BFE00h, or 37F8000h..3807E00h)
    uint32_t ram_addr7;     // 038h    4      ARM7 ram_address   (2000000h..23BFE00h, or 37F8000h..3807E00h)
    uint32_t size_7;        // 03Ch    4      ARM7 size          (max 3BFE00h, or FE00h) (3839.5KB, 63.5KB)
    uint32_t fnt_off;       // 040h    4      File Name Table (FNT) offset
    uint32_t fnt_size;      // 044h    4      File Name Table (FNT) size
    uint32_t fat_off;       // 048h    4      File Allocation Table (FAT) offset
    uint32_t fat_size;      // 04Ch    4      File Allocation Table (FAT) size
    uint32_t overlay_off9;  // 050h    4      File ARM9 overlay_offset
    uint32_t overlay_size9; // 054h    4      File ARM9 overlay_size
    uint32_t overlay_off7;  // 058h    4      File ARM7 overlay_offset
    uint32_t overlay_size7; // 05Ch    4      File ARM7 overlay_size
    uint32_t normal_cmd;    // 060h    4      Port 40001A4h setting for normal commands (usually 00586000h)
    uint32_t key_cmd;       // 064h    4      Port 40001A4h setting for KEY1 commands   (usually 001808F8h)
    uint32_t icon_title_off;// 068h    4      Icon_title_offset (0=None) (8000h and up)
    uint16_t sec_sum;       // 06Ch    2      Secure Area Checksum, CRC-16 of [ [20h]..7FFFh]
    uint16_t sec_timeout;   // 06Eh    2      Secure Area Loading Timeout (usually 051Eh)
    uint32_t load_list9;    // 070h    4      ARM9 Auto Load List RAM Address (?)
    uint32_t load_list7;    // 074h    4      ARM7 Auto Load List RAM Address (?)
    uint8_t  sec_dis[8];    // 078h    8      Secure Area Disable (by encrypted "NmMdOnly") (usually zero)
    uint32_t rom_size;      // 080h    4      Total Used ROM size (remaining/unused bytes usually FFh-padded)
    uint32_t rom_hdr_size;  // 084h    4      ROM Header Size (4000h)
    uint8_t  reserve2[0x38];// 088h    38h    Reserved (zero filled)
    uint8_t  reserve3[0x9c];// 0C0h    9Ch    Nintendo Logo (compressed bitmap, same as in GBA Headers)
    uint16_t logo_sum;      // 15Ch    2      Nintendo Logo Checksum, CRC-16 of [0C0h-15Bh], fixed CF56h
    uint16_t hdr_sum;       // 15Eh    2      Header Checksum, CRC-16 of [000h-15Dh]
    uint32_t dbg_rom_off;   // 160h    4      Debug rom_offset   (0=none) (8000h and up)       ;only if debug
    uint32_t dbg_size;      // 164h    4      Debug size         (0=none) (max 3BFE00h)        ;version with
    uint32_t dbg_ram_addr;  // 168h    4      Debug ram_address  (0=none) (2400000h..27BFE00h) ;SIO and 8MB
} nds_rom_hdr_t ;

typedef struct __attribute__ ((packed)) {
    uint16_t version;       // 000h  2    Version  (0001h)
    uint16_t crc16;         // 002h  2    CRC16 across entries 020h..83Fh
    uint8_t  reserve1[0x1c];// 004h  1Ch  Reserved (zero-filled)
    uint8_t  icn_bmp[0x200];// 020h  200h Icon Bitmap  (32x32 pix) (4x4 tiles, each 4x8 bytes, 4bit depth)
    uint8_t  icn_pal[0x20]; // 220h  20h  Icon Palette (16 colors, 16bit, range 0000h-7FFFh)
                            //            (Color 0 is transparent, so the 1st palette entry is ignored)
    uint16_t title_jpn[0x80];// 240h  100h Title 0 Japanese (128 characters, 16bit Unicode)
    uint16_t title_eng[0x80];// 340h  100h Title 1 English  ("")
    uint16_t title_frn[0x80];// 440h  100h Title 2 French   ("")
    uint16_t title_grm[0x80];// 540h  100h Title 3 German   ("")
    uint16_t title_itl[0x80];// 640h  100h Title 4 Italian  ("")
    uint16_t title_spn[0x80];// 740h  100h Title 5 Spanish  ("")
//  uint8_t unknown[0x840]; //  840h  ?    (Maybe newer/chinese firmware do also support chinese title?)
//  uint8_t unknown[0x840]; //  840h  -    End of Icon/Title structure (next 1C0h bytes usually FFh-filled)
} nds_rom_icon_title_t ;

typedef struct {
    nds_rom_hdr_t        hdr;  
    nds_rom_icon_title_t icon;
} nds_rom_info_t ;



//
// function declarations
//
int parse_commandline(int argc, char *argv[]);
void print_rom_information(FILE *infileptr, nds_rom_info_t *rom_info);
int get_nds_header(FILE* nds_rom_fp, nds_rom_info_t *rom_info);
int rom_trim(FILE *infileptr, FILE *outfileptr, nds_rom_info_t *rom_info);
int rom_trim_inplace(FILE *infileptr, nds_rom_info_t *rom_info);
void dsprintf(char *format,...);
void dsprintfd(char *format,...);

#endif /* NDSTRIM_H */
