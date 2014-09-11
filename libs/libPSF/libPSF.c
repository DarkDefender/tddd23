#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include "PSF.h"
#ifndef NO_ZLIB
	#include <zlib.h>
	#define OPEN(a) gzopen(a,"rb")
	#define READ(a,b) gzread(psf_fd,a,b)
	#define CLOSE() gzclose(psf_fd)
	#define SEEK(a)	gzseek(psf_fd,a,SEEK_SET)
	static gzFile psf_fd;
#else
	#define OPEN(a) fopen(a,"rb")
	#define READ(a,b) fread(a,1,b,psf_fd)
	#define CLOSE() fclose(psf_fd)
	#define SEEK(a)	fseek(psf_fd,a,SEEK_SET)
	static FILE *psf_fd;
#endif

static char file_buffer[512];
static int buffer_pos=0;

static char psf_type;

static unsigned char psf_magic[2];
static char psf_mode;
static char psf_charsize;

static unsigned char psf2_magic[4];
static int	psf2_version;
static int	psf2_headersize;
static int	psf2_flags;
static int	psf2_length;
static int	psf2_charsize;
static int	psf2_width,psf2_height;

#define FONTPATH "/usr/share/consolefonts/"
#define FALLBACKPATH "/usr/share/kbd/consolefonts/"

static char scmp(char *,char *);

int	PSF_FindFont(char *codeset,char *fontface, char *fontsize)
{
	DIR *dfd;
	dfd=opendir(FONTPATH);
	if (dfd==NULL)
		dfd=opendir(FALLBACKPATH);
	if (dfd==NULL)
		return 0;

	struct dirent *pdirent;
	while ((pdirent = readdir(dfd)) != NULL)
	{
		char cs[50],ff[75],fs[20];
		int r = sscanf(pdirent->d_name,"%[^-]-%[^123456789]%[^.].psf%*[.gz]",cs,ff,fs);
		if (r<3)
			continue;
		else if (codeset!=NULL && !scmp(codeset,cs))
			continue;
		else if (fontface!=NULL && !scmp(fontface,ff))
			continue;
		else if (fontsize!=NULL && !scmp(fontsize,fs))
			continue;
		else
		{
			printf("Opening: %s%s\n",FONTPATH,pdirent->d_name);
			char fontfile[PATH_MAX];
			sprintf(fontfile,"%s%s",FONTPATH,pdirent->d_name);
			psf_fd=OPEN(fontfile);
			closedir(dfd);
			return 1;
		}
	}
	closedir(dfd);
	return 0;
}

void PSF_OpenFont(const char *fname)
{
	psf_fd=OPEN(fname);

	return;
}

int PSF_GetGlyphSize()
{
	return (psf_type==PSF_TYPE_1)?psf_charsize:psf2_charsize;
}

int PSF_GetGlyphHeight()
{
	return (psf_type==PSF_TYPE_1)?psf_charsize:psf2_height;
}

int PSF_GetGlyphWidth()
{
	return (psf_type==PSF_TYPE_1)?8:psf2_width;
}

int PSF_GetGlyphTotal()
{
	return (psf_type==PSF_TYPE_1)?((psf_mode&PSF_MODE_512)?512:256):psf2_length;
}

void PSF_ReadHeader()
{
	READ(psf_magic,2);

	//Check if PSF version 1 format
	if (psf_magic[0]==0x36 && psf_magic[1]==0x04)
	{
		psf_type=PSF_TYPE_1;

		READ(&psf_mode,1);
		READ(&psf_charsize,1);
	}
	else
	{
		//Check if PSF version 2 format
		if (psf_magic[0]==0x72 && psf_magic[1]==0xB5)
		{
			psf2_magic[0]=psf_magic[0]; 
			psf2_magic[1]=psf_magic[1];
			READ(&psf2_magic[2],2);
			if (psf2_magic[2]==0x4A && psf2_magic[3]==0x86)
			{
				psf_type=PSF_TYPE_2;

				READ(&psf2_version,4);
				READ(&psf2_headersize,4);
				READ(&psf2_flags,4);
				READ(&psf2_length,4);
				READ(&psf2_charsize,4);
				READ(&psf2_height,4);
				READ(&psf2_width,4);
				SEEK(psf2_headersize);
			}
			else
				psf_type=PSF_TYPE_UNKNOWN;
		}
		else
			psf_type=PSF_TYPE_UNKNOWN;
	}

	return;
}

void PSF_ReadGlyph(void *mem, int size, int fill, int clear)
{
	int tmp;
	char tmpglyph[PSF_GetGlyphSize()];

	READ(tmpglyph,PSF_GetGlyphSize());

	int i,j;
	for (i=0;i<PSF_GetGlyphSize();i++)
	{
		for (j=0;j<8;j++)
		{
			if ((i%(PSF_GetGlyphSize()/PSF_GetGlyphHeight())*8)+j<PSF_GetGlyphWidth())
			{
				if (tmpglyph[i]&(0x80))
					memcpy(mem,&fill,size);
				else
					memcpy(mem,&clear,size);
				mem+=size;
			}

			tmpglyph[i]=tmpglyph[i]<<1;

		}
	}
}

void PSF_CloseFont()
{
	CLOSE();

	return;
}

static char scmp(char *a,char *b)
{
	int i;
	for (i=0;a[i]!='\0';i++)
		if (a[i]!=b[i])
			return 0;
	return 1;
}
