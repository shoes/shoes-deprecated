#include <stdint.h>
#include <abstractfile.h>

#define IMG2_SIGNATURE 0x496D6732

typedef struct Img2Header {
	uint32_t signature;        /* 0x0 */
	uint32_t imageType;        /* 0x4 */
	uint16_t unknown1;         /* 0x8 */
	uint16_t security_epoch;   /* 0xa */
	uint32_t flags1;           /* 0xc */
	uint32_t dataLenPadded;    /* 0x10 */
	uint32_t dataLen;          /* 0x14 */
	uint32_t unknown3;         /* 0x18 */
	uint32_t flags2;           /* 0x1c */ /* 0x01000000 has to be unset */
	uint8_t  reserved[0x40];   /* 0x20 */
	uint32_t unknown4;         /* 0x60 */ /* some sort of length field? */
	uint32_t header_checksum;  /* 0x64 */ /* standard crc32 on first 0x64 bytes */
	uint32_t checksum2;        /* 0x68 */
	uint8_t  unknown5[0x394]; /* 0x68 */
} __attribute__((__packed__)) Img2Header;

typedef struct InfoImg2 {
	AbstractFile*		file;
	
	Img2Header      header;
	size_t          offset;
	void*           buffer;

	char            dirty;
} InfoImg2;

AbstractFile* createAbstractFileFromImg2(AbstractFile* file);
AbstractFile* duplicateImg2File(AbstractFile* file, AbstractFile* backing);
