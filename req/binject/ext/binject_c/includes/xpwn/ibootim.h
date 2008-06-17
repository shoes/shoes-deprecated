#ifndef IBOOTIM_H
#define IBOOTIM_H

#include <stdint.h>
#include <abstractfile.h>

typedef struct IBootIMHeader {
	char    signature[8];
	uint32_t unknown;
	uint32_t compression_type;
	uint32_t format;
	uint16_t width;
	uint16_t height;
	uint8_t  padding[0x28];
} __attribute__((__packed__)) IBootIMHeader;

#define IBOOTIM_SIG_UINT 0x69426F6F
#define IBOOTIM_SIGNATURE "iBootIm"
#define IBOOTIM_LZSS_TYPE 0x6C7A7373
#define IBOOTIM_ARGB 0x61726762
#define IBOOTIM_GREY 0x67726579

typedef struct InfoIBootIM {
	AbstractFile*		file;
	
	IBootIMHeader   header;
	size_t          length;
	size_t          compLength;
	size_t          offset;
	void*           buffer;

	char            dirty;
} InfoIBootIM;

#ifdef __cplusplus
extern "C" {
#endif
	AbstractFile* createAbstractFileFromIBootIM(AbstractFile* file);
	AbstractFile* duplicateIBootIMFile(AbstractFile* file, AbstractFile* backing);
	void* replaceBootImage(AbstractFile* imageWrapper, AbstractFile* png, size_t *fileSize);
#ifdef __cplusplus
}
#endif

#endif

