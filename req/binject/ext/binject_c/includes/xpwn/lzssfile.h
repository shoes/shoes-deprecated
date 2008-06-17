#include <stdint.h>
#include <abstractfile.h>

#define COMP_SIGNATURE 0x636F6D70
#define LZSS_SIGNATURE 0x6C7A7373

typedef struct CompHeader {
	uint32_t signature;
	uint32_t compression_type;
	uint32_t checksum;
	uint32_t length_uncompressed;
	uint32_t length_compressed;
	uint8_t  padding[0x16C];
} __attribute__((__packed__)) CompHeader;

typedef struct InfoComp {
	AbstractFile*		file;
	
	CompHeader      header;
	size_t          offset;
	void*           buffer;

	char            dirty;
} InfoComp;

AbstractFile* createAbstractFileFromComp(AbstractFile* file);
AbstractFile* duplicateCompFile(AbstractFile* file, AbstractFile* backing);
