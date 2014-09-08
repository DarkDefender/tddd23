#ifndef PSF
#define PSF

#ifdef __cplusplus
extern "C" {
#endif

#define	PSF_MODE_512		1
#define	PSF_MODE_HAS_TAB	2
#define	PSF_MODE_HAS_EQ		4
#define	PSF_MODE_MAX		5

#define PSF2_HAS_UNICODE_TABLE 0x01

#define	PSF_TYPE_1			1
#define	PSF_TYPE_2			2
#define	PSF_TYPE_UNKNOWN	4

int	PSF_FindFont(char *codeset,char *fontface, char *fontsize);
void PSF_OpenFont(char *fname);
void PSF_ReadHeader();
int PSF_GetGlyphSize();
int PSF_GetGlyphHeight();
int PSF_GetGlyphWidth();
int PSF_GetGlyphTotal();
void PSF_ReadGlyph(void *mem, int size, int fill, int clear);
void PSF_CloseFont();

#ifdef __cplusplus
}
#endif

#endif
