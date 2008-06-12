/*
    DSlash by hyplex@gmail.com
    Homepage: http://code.google.com/p/dslash
    based on ndstrim by recover89@gmail.com
    Trims NDS roms - now even faster and hopefully still reliable.
    ROM size is available in four bytes at 0x80-0x83.
    Wifi data is stored on 136 bytes after ROM data.
    Filesize is checked to be at least 0x200 bytes to make sure it contains a DS cartridge header.
    Filesize is then checked to be at least the ROM size+wifi to avoid errors.
    Source code licensed under GNU GPL version 2 or later.

    Sources:
    http://nocash.emubase.de/gbatek.htm
    http://forums.ds-xtreme.com/showthread.php?t=1964
    http://gbatemp.net/index.php?showtopic=44022
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "dslash.h"


//
// Constants/Macros
//

#define MB              (1024*1024)
#define CPY_BUFFER_LEN  1*MB        // 4 MB buffer size
#define WIFI_LEN        136         // Wifi data after end of rom, 136 bytes long
#define MIN_DS_HEADER   0x200       // Minimum size a NDS Card header will fit in
#define ROM_NAME_OFFSET 0x00
#define ROM_SIZE_OFFSET 0x80        // Offset from beggining of ROM to where rom size is found
#define MSG_BUFFER_LEN  512         // sizes for tmp message buffers

// help message
#define OPTION_STR "hpdvit:"
#define HELP_MSG "Usage: %s [-hpdvi] [-t <icon_file>] ndsfile [trimmed_ndsfile]\n"\
                 "       -h : help\n"\
                 "       -p : print ROM information\n"\
                 "       -i : trim ROM in place (very fast)\n"\
                 "       -d : debug output\n"\
                 "       -v : verbose\n"\
                 "\n"\
                 "       -t <filename>  : extract icon from ROM into filename (bmp format)\n"

//
// structure definitons
//

// options passed in via commandline
static struct {
    unsigned int verbose:1;
    unsigned int debug:1;
    unsigned int print:1;
    unsigned int inplace:1;
} flag;






int get_nds_header(FILE* nds_rom_fp, nds_rom_info_t *rom_info)
{
    uint32_t icon_title_offset;

    dsprintfd("%s called'.\n",__FUNCTION__);

    if ( (rom_info == NULL ) || (nds_rom_fp == NULL))
    {
        return -1;
    }

    // read in header, should only read in one item
    if ((fread(&rom_info->hdr, sizeof(nds_rom_hdr_t), 1, nds_rom_fp)) != 1)
    {
        return -1;
    }

    icon_title_offset=endian_32(rom_info->hdr.icon_title_off);
    if (icon_title_offset != 0 )
    {
        fseek(nds_rom_fp,icon_title_offset,SEEK_SET);
        if ((fread(&rom_info->icon, sizeof(nds_rom_icon_title_t), 1, nds_rom_fp)) != 1)
        {
            return -2;
        }
    }

    return 0;
}





void dsprintf(char *format,...)
{
    va_list ap;
    if (flag.verbose == 1)
    {
        va_start(ap,format);
        vprintf(format,ap);
        va_end(ap);
    }
}
void dsprintfd(char *format,...)
{
    va_list ap;
    if (flag.debug == 1)
    {
        va_start(ap,format);
        vprintf(format,ap);
        va_end(ap);
    }
}




//The ROM size is located in four bytes at 0x80-0x83

int main(int argc, char *argv[]) 
{
    FILE *infileptr=NULL;
    FILE *outfileptr=NULL;
    char *infilename=NULL;
    char *outfilename=NULL;
    int ret=0;
//    char internal_rom_name[MSG_BUFFER_LEN]={0};
    nds_rom_info_t rom_info;


    /* initalize any structures */
    memset(&flag,0,sizeof(flag));
    memset(&rom_info,0,sizeof(rom_info));

    /* get commandline arguments */
    if ((ret=parse_commandline(argc,argv)) != 0 )
    {
        // help messages will exit as ok
        if (ret==1)
        {
            ret=0;  
        }
        goto main_exit;
    }


    // store filenames from argv.
    infilename=argv[optind];
    if ((optind + 1) < argc)
    {
        outfilename=argv[optind+1];
    }

    // in all cases there will be a file/stdio get it ready for use
    dsprintfd("Opening input file: '%s'.\n",infilename);
    // note this file is open in read and write incase we are doing inline triming
    if ((infileptr=fopen(infilename,"r+b")) == NULL)
    {
        perror(infilename);
        ret=-1;
        goto main_exit;
    }

    //Open output
    if (outfilename != NULL )
    {
        dsprintfd("Opening output file: '%s'.\n",outfilename);
        if ((outfileptr=fopen(outfilename,"wb")) == NULL)
        {
            perror(outfilename);
            ret=-1;
            goto main_cleanup;
        }
    }

    // populate ROM info structure
    if (get_nds_header(infileptr, &rom_info) !=0 )
    {
        fprintf(stderr, "Bad ROM header. Could not read ROM information.\n");
        ret=-1;
        goto main_cleanup;
    }


    // if the print flag is on, display ROM info
    if (flag.print)
    {
        print_rom_information(infileptr, &rom_info);
        goto main_cleanup;
    }


    // trim file
    if ( flag.inplace)
    {
        rom_trim_inplace(infileptr, &rom_info);
    }
    else
    {
        rom_trim(infileptr, outfileptr, &rom_info);
    }


// Error handling
main_cleanup:

    //Close input
    dsprintfd("Closing input.\n");
    if (infileptr != NULL )
    {
        if (fclose(infileptr) == EOF) 
        {
            perror(infilename);
        }
    }

    //Close output
    dsprintfd("Closing output.\n");
    if (outfileptr != NULL )
    {
        if (fclose(outfileptr) == EOF) 
        {
            perror(outfilename);
        }
    }
main_exit:

    return ret;
}




int parse_commandline(int argc, char *argv[])
{
    // option string
    // v - verbose
    // h - help
    // d - debug
    // p - print
    int opt; // use by getopt for options


    /* parse command line */
    while ((opt = getopt(argc, argv, OPTION_STR)) != -1) 
    {
        switch (opt) 
        {
            case 'h':
                fprintf(stderr, HELP_MSG, argv[0]);
                return 1;
                break;
            case 'p':
                flag.print=1;
                break;
            case 'd':
                flag.debug=1;
                break;
            case 'v':
                flag.verbose=1;
                break;
            case 'i':
                flag.inplace=1;
                break;
            default: /* '?' */
                fprintf(stderr,"Unknown argument. Use \"%s -h\" for help.\n",
                        argv[0]);
                return 1;
        }
    }

    if (optind >= argc) {
        fprintf(stderr, "Missing nds file!\n");
        return 1;
    }

    dsprintfd("Flags: verbose=%d, debug=%d, print=%d, inplace=%d\n",
                flag.verbose,flag.debug,flag.print,flag.inplace);

    return 0;
}



void print_rom_information(FILE *infileptr, nds_rom_info_t *rom_info)
{

    char str_buff[MSG_BUFFER_LEN]={0};
    char *str_buff_ptr=NULL;
    long filesize=0;
    unsigned int rom_size=0;
    unsigned int total_rom_size=0;
    unsigned int i=0;
    int bytes_saved=0;
    char *tmpstr=NULL;

    // allow prints to work
    flag.verbose=1;


    strncpy(str_buff,(char *)rom_info->hdr.game_title,sizeof(rom_info->hdr.game_title));
    str_buff[sizeof(rom_info->hdr.game_title)]=0;
    dsprintf("Internal ROM name        : %s\n",str_buff);

    // quick hack to conviert unicode to regular ascii
    for (i=0;i<128;i++)
    {
        if (rom_info->icon.title_eng[i] == 0)
        {
            str_buff[i]='\0';
            break;
        }
        str_buff[i]= endian_16(rom_info->icon.title_eng[i]);
    }

    // prepare loop
    i=0;
    str_buff_ptr=str_buff;
    while ((tmpstr=strtok(str_buff_ptr,"\n")) !=NULL)
    {
        if ( i++==0) 
        {
            dsprintf("Internal title info      : %s\n",tmpstr);
            // strtok requires all calls after the first to have NULL
            str_buff_ptr=NULL;
        }
        else
        {
            dsprintf("                         : %s\n",tmpstr);
        }
    }

    //Get input filesize
    if (fseek(infileptr,0,SEEK_END) == 0)
    {
        filesize=ftell(infileptr);
    }
    else
    {
        perror("fseek");
        filesize=-1;
    }

    rom_size=endian_32(rom_info->hdr.rom_size);
    total_rom_size=rom_size+WIFI_LEN;

    strncpy(str_buff,(char *)rom_info->hdr.game_code,sizeof(rom_info->hdr.game_code));
    str_buff[sizeof(rom_info->hdr.game_code)]=0;
    dsprintf("Game code                : %s\n",str_buff);

    strncpy(str_buff,(char *)rom_info->hdr.maker_code,sizeof(rom_info->hdr.maker_code));
    str_buff[sizeof(rom_info->hdr.maker_code)]=0;
    dsprintf("Maker code               : %s\n",str_buff);
    dsprintf("Filesystem ROM size      : % 9u bytes (%.1f MB)\n",filesize,filesize/(float)MB);
    dsprintf("Internal ROM size        : % 9u bytes (%.1f MB)\n",rom_size,rom_size/(float)MB);
    dsprintf("Internal ROM size + wifi : % 9u bytes (%.1f MB)\n",total_rom_size,total_rom_size/(float)MB);

    bytes_saved=filesize-total_rom_size;
    if (filesize<total_rom_size)
    {
        bytes_saved=0;
    }

    dsprintf("Bytes saved by trimming  : % 9u bytes (%.1f MB)\n",bytes_saved,bytes_saved/(float)MB);

    if (filesize<total_rom_size)
    {
        fprintf(stderr,"*Warning*: Filesystem size is less than internal ROM size + wifi.  ROM file may be corrupted!\n");
    }

    return;
}


int rom_trim(FILE *infileptr, FILE *outfileptr, nds_rom_info_t *rom_info)
{

    int ret=0;
    unsigned int filesize=0;
    unsigned int newsize=0;
    unsigned int fpos=0;
    unsigned int tocopy=CPY_BUFFER_LEN;
    char *buffer=NULL;


    if ((infileptr==NULL) || (outfileptr==NULL))
    {
        dsprintfd("infileptr or outfileptr NULL\n");
        ret = -1;
        goto rom_trim_exit;
    }

    //Get input filesize
    if (fseek(infileptr,0,SEEK_END) < 0)
    {
        perror("fseek");
        ret = -1;
        goto rom_trim_exit;
    }
    filesize=ftell(infileptr);
    dsprintfd("Filesize: %d bytes.\n",filesize);

    //Check if file is big enough to contain a DS cartridge header
    if (filesize <= MIN_DS_HEADER)
    {
        fprintf(stderr,"Error: File is too small to contain a proper NDS cartridge header (corrupt ROM file?).\n");
        ret = -1;
        goto rom_trim_exit;
    }

    newsize=endian_32(rom_info->hdr.rom_size)+WIFI_LEN;

    //Check if file is big enough to contain the rom+wifi
    if (filesize <= newsize)
    {
        fprintf(stderr,"Notice: File cannot be trimmed any further.\n");
        ret = -1;
        goto rom_trim_exit;
    }


    if (outfileptr != NULL)
    {

        //Reset input pos
        rewind(infileptr);

        // create a buffer for data copy
        if ((buffer=(char *)malloc(CPY_BUFFER_LEN)) == NULL)
        {
            fprintf(stderr, "Can't malloc\n");
            ret = -1;
            goto rom_trim_exit;
        }

#warning setvbuf
        //Start copying
        dsprintfd("Copying data.\n");
        while (fpos < newsize)
        {
            if (fpos+CPY_BUFFER_LEN > newsize) 
            {
                tocopy=newsize-fpos;
            }
            if (fread(buffer,tocopy,1,infileptr) == 0)
            {
                fprintf(stderr,"Read error.\n"); 
                ret = -1;
                goto rom_trim_cleanup;
            }
            if (fwrite(buffer,tocopy,1,outfileptr) == 0)
            {
                fprintf(stderr,"Write error.\n"); 
                ret = -1;
                goto rom_trim_cleanup;
            }
            fpos+=tocopy;
        }

        //Done
        dsprintf("File trimmed to %d bytes (saved %.1f MB).\n", newsize,(filesize-newsize)/(float)MB);
    }


rom_trim_cleanup:
    free (buffer);
    
rom_trim_exit:

    return ret;
}

int rom_trim_inplace(FILE *infileptr, nds_rom_info_t *rom_info)
{

    off_t newsize=0;
    int infilefd=0;
    struct stat stat_file;

    // Get file descriptor associated with file stream
    if ((infileptr==NULL) || (infilefd=fileno(infileptr))== -1 )
    {
        dsprintfd("infileptr NULL or infileptr not valid stream\n");
        return -1;
    }

    // get file size
    if (fstat(infilefd, &stat_file))
    {
        perror("fstat");
        stat_file.st_size=newsize;
    }

    newsize=(off_t)endian_32(rom_info->hdr.rom_size)+WIFI_LEN;
    if (ftruncate ( infilefd, newsize ))
    {
        perror("ftruncate");
        return -1;
    }


    

    //Done
    dsprintf("ROM trimmed to %d bytes (saved %.1f MB).\n", 
             newsize,(stat_file.st_size-newsize)/(float)MB);

    return 0;
}
