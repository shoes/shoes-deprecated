#include <stdint.h>
#include <openssl/aes.h>
#include <abstractfile.h>

#ifndef INC_8900_H
#define INC_8900_H

typedef struct {
	uint32_t        magic;              /* string "8900" */
	unsigned char  version[3];            /* string "1.0" */
	uint8_t         format;                /* plaintext format is 0x4, encrypted format is 0x3 */
	uint32_t        unknown1;
	uint32_t        sizeOfData;            /* size of data (ie, filesize - header(0x800) - footer signature(0x80) - footer certificate(0xC0A)) */
	uint32_t        footerSignatureOffset; /* offset to footer signature */
	uint32_t        footerCertOffset;      /* offset to footer certificate, from end of header (0x800) */
	uint32_t        footerCertLen;
	unsigned char  salt[0x20];            /* a seemingly random salt (an awfully big one though... needs more attention) */
	uint16_t        unknown2;
	uint16_t        epoch;                 /* the security epoch of the file */
	unsigned char  headerSignature[0x10]; /* encrypt(sha1(header[0:0x40])[0:0x10], key_0x837, zero_iv) */
	unsigned char  padding[0x7B0];
} __attribute__((__packed__)) Apple8900Header;

#define SIGNATURE_8900 0x38393030

#define key_0x837 ((uint8_t[]){0x18, 0x84, 0x58, 0xA6, 0xD1, 0x50, 0x34, 0xDF, 0xE3, 0x86, 0xF2, 0x3B, 0x61, 0xD4, 0x37, 0x74})

typedef struct Info8900 {
	AbstractFile*		file;
	
	Apple8900Header header;
	size_t          offset;
	void*           buffer;

	unsigned char	footerSignature[0x80];
	unsigned char*	footerCertificate;
	
	AES_KEY         encryptKey;
	AES_KEY         decryptKey;
	
	char            dirty;
} Info8900;

AbstractFile* createAbstractFileFrom8900(AbstractFile* file);
AbstractFile* duplicate8900File(AbstractFile* file, AbstractFile* backing);
void replaceCertificate8900(AbstractFile* file, AbstractFile* certificate);

#endif
