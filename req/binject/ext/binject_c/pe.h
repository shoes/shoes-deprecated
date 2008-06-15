
/*

anal_pe - Copyright 2005-2007 by Michael Kohn

Webpage: http://www.mikekohn.net/
Email: mike@mikekohn.net

This code falls under the LGPL license.

*/

struct funct_t
{
  char funct_name[1024];
  unsigned int funct_retvalue;
  long funct_ptr;
  long file_offset;
};

struct dos_header_t
{
  unsigned short int e_magic;         // Magic number
  unsigned short int e_cblp;          // Bytes on last page of file
  unsigned short int e_cp;            // Pages in file
  unsigned short int e_crlc;          // Relocations
  unsigned short int e_cparhdr;       // Size of header in paragraphs
  unsigned short int e_minalloc;      // Minimum extra paragraphs needed
  unsigned short int e_maxalloc;      // Maximum extra paragraphs needed
  unsigned short int e_ss;            // Initial (relative) SS value
  unsigned short int e_sp;            // Initial SP value
  unsigned short int e_csum;          // Checksum
  unsigned short int e_ip;            // Initial IP value
  unsigned short int e_cs;            // Initial (relative) CS value
  unsigned short int e_lfarlc;        // File address of relocation table
  unsigned short int e_ovno;          // Overlay number
  unsigned char e_res[8];             // Reserved words
  unsigned short int e_oemid;         // OEM identifier (for e_oeminfo)
  unsigned short int e_oeminfo;       // OEM information; e_oemid specific
  unsigned char e_res2[20];           // Reserved words
  int e_lfanew;                       // File address of new exe header
};

struct image_file_header_t
{
  unsigned short int Machine;
  unsigned short int NumberOfSections;
  unsigned int TimeDateStamp;
  unsigned int PointerToSymbolTable;
  unsigned int NumberOfSymbols;
  unsigned short int SizeOfOptionalHeader;
  unsigned short int Characteristics;
};

struct image_data_directory_t
{
  unsigned int VirtualAddress;
  unsigned int Size;
};

struct image_optional_header_t
{
  unsigned short int Magic;
  unsigned char MajorLinkerVersion;
  unsigned char MinorLinkerVersion;
  unsigned int SizeOfCode;
  unsigned int SizeOfInitializedData;
  unsigned int SizeOfUninitializedData;
  unsigned int AddressOfEntryPoint;
  unsigned int BaseOfCode;
  unsigned int BaseOfData;
  unsigned int ImageBase;
  unsigned int SectionAlignment;
  unsigned int FileAlignment;
  unsigned short int MajorOperatingSystemVersion;
  unsigned short int MinorOperatingSystemVersion;
  unsigned short int MajorImageVersion;
  unsigned short int MinorImageVersion;
  unsigned short int MajorSubsystemVersion;
  unsigned short int MinorSubsystemVersion;
  unsigned int Reserved1;
  unsigned int SizeOfImage;
  unsigned int SizeOfHeaders;
  unsigned int CheckSum;
  unsigned short int Subsystem;
  unsigned short int DllCharacteristics;
  unsigned int SizeOfStackReserve;
  unsigned int SizeOfStackCommit;
  unsigned int SizeOfHeapReserve;
  unsigned int SizeOfHeapCommit;
  unsigned int LoaderFlags;
  unsigned int NumberOfRvaAndSizes;
  int DataDirectoryCount;
  int image_data_dir[32];
};

struct section_header_t
{
  unsigned char Name[8];
  union
  {
    unsigned int PhysicalAddress;
    unsigned int VirtualSize;
  } Misc;
  unsigned int VirtualAddress;
  unsigned int SizeOfRawData;
  unsigned int PointerToRawData;
  unsigned int PointerToRelocations;
  unsigned int PointerToLinenumbers;
  unsigned short int NumberOfRelocations;
  unsigned short int NumberOfLinenumbers;
  unsigned int Characteristics;
};

struct resource_dir_t
{
  unsigned int Characteristics;
  unsigned int TimeDateStamp;
  unsigned short int MajorVersion;
  unsigned short int MinorVersion;
  unsigned short int NumberOfNamedEntries;
  unsigned short int NumberOfIdEntries;
};

struct resource_dir_entry_t
{
  unsigned int Name;
  unsigned int OffsetToData;
};

struct resource_data_t
{
  unsigned int OffsetToData;
  unsigned int Size;
  unsigned int CodePage;
  unsigned int Reserved;
};

struct import_dir_t
{
  unsigned int FunctionNameList;
  unsigned int TimeDateStamp;
  unsigned int ForwardChain;
  unsigned int ModuleName;
  unsigned int FunctionAddressList;
};

struct export_dir_t
{
  unsigned int Characteristics;
  unsigned int TimeDateStamp;
  unsigned short int MajorVersion;
  unsigned short int MinorVersion;
  unsigned int Name;
  unsigned int Base;
  unsigned int NumberOfFunctions;
  unsigned int NumberOfNames;
  unsigned int AddressOfFunctions;
  unsigned int AddressOfNames;
  unsigned int AddressOfNameOrdinals;
};

struct vb_header_t
{
  char szVbMagic[5];
  unsigned short int wRuntimeBuild;
  char szLangDll[15];
  char szSecLangDll[15];
  unsigned short int wRuntimeRevision;
  unsigned int dwLCID;
  unsigned int dwSecLCID;
  unsigned int lpSubMain;
  unsigned int lpProjectData;
  unsigned int fMdlIntCtls;
  unsigned int fMdlIntCtls2;
  unsigned int dwThreadFlags;
  unsigned int dwThreadCount;
  unsigned short int wFormCount;
  unsigned short int wExternalCount;
  unsigned int dwThunkCount;
  unsigned int lpGuiTable;
  unsigned int lpExternalTable;
  unsigned int lpComRegisterData;
  unsigned int bSzProjectDescription;
  unsigned int bSzProjectExeName;
  unsigned int bSzProjectHelpFile;
  unsigned int bSzProjectName;
};

struct com_reg_data_t
{
  unsigned int bRegInfo;
  unsigned int bSZProjectName;
  unsigned int bSZHelpDirectory;
  unsigned int bSZProjectDescription;
  unsigned char uuidProjectClsId[16];
  unsigned int dwTlbLcid;
  unsigned short int wUnknown;
  unsigned short int wTlbVerMajor;
  unsigned short int wTlbVerMinor;
};

struct reg_info_t
{
  unsigned int bNextObject;
  unsigned int bObjectName;
  unsigned int bObjectDescription;
  unsigned int dwInstancing;
  unsigned int dwObjectId;
  unsigned char uuidObject[16];
  unsigned int fIsInterface;
  unsigned int bUidObjectIFace;
  unsigned int bUidEventsIFace;
  unsigned int fHasEvents;
  unsigned int dwMiscStatus;
  unsigned int fClassType;
  unsigned short int fObjectType;
  unsigned short int wToolboxBitmap32;
  unsigned short int wDefaultIcon;
  unsigned short int fIsDesigner;
  unsigned int bDesignerData;
  unsigned char szObjectName[1024];
};

struct debug_directory_t
{
  unsigned int Characteristics;
  unsigned int TimeDateStamp;
  unsigned short int MajorVersion;
  unsigned short int MinorVersion;
  unsigned int Type;
  unsigned int SizeOfData;
  unsigned int AddressOfRawData;
  unsigned int PointerToRawData;
};

int read_dos_header(FILE *in, struct dos_header_t *dos_header);
int print_dos_header(struct dos_header_t *dos_header);

int read_image_file_header(FILE *in, struct image_file_header_t *image_file_header);
int print_image_file_header(struct image_file_header_t *image_file_header);

int read_image_optional_header(FILE *in, struct image_optional_header_t *image_optional_header, int header_size);
int print_image_optional_header(struct image_optional_header_t *image_optional_header);

int read_section_header(FILE *in, struct section_header_t *section_header);
int print_section_header(struct section_header_t *section_header,int count);

int print_imports(FILE *in, int addr, int size, struct section_header_t *section_header);
int print_exports(FILE *in, int addr, int size, struct section_header_t *section_header, struct funct_t *funct);
int print_debug_section(FILE *in, int addr, int size, struct section_header_t *section_header);

int print_vb_info(FILE *in, struct image_optional_header_t *image_optional_header, struct section_header_t *section_header);

int rip_binary(FILE *in, char *filename, int address, int size);
int read_unicode(FILE *in, int addr, char *s, int max_chars);

int parse_resource_dir(FILE *in, struct section_header_t *section_header, int offset, int level, int res_type);
int read_resource_data(FILE *in, int addr, struct resource_data_t *resource_data, int offset);
int read_resource_dir(FILE *in, int addr, struct resource_dir_t *resource_dir);
int read_resource_dir_entry(FILE *in, int addr, struct resource_dir_entry_t *resource_dir_entry, int offset);
int print_resource_data(struct resource_data_t *resource_data, int level);
int print_resource_type(int id, int level);
int print_resource_dir(struct resource_dir_t *resource_dir, int level);
int print_resource_dir_entry(struct resource_dir_entry_t *resource_dir_entry, int level);


