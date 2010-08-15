#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <sys/types.h>
#include "common.h"
#include <hfs/hfsplus.h>
#include "abstractfile.h"
#include <sys/stat.h>

#define HFS_BUFSIZE 32768

void writeToFile(HFSPlusCatalogFile* file, AbstractFile* output, Volume* volume) {
	unsigned char buffer[HFS_BUFSIZE];
	io_func* io;
	off_t curPosition;
	size_t bytesLeft;
	
	io = openRawFile(file->fileID, &file->dataFork, (HFSPlusCatalogRecord*)file, volume);
	if(io == NULL) {
		panic("error opening file");
		return;
	}
	
	curPosition = 0;
	bytesLeft = file->dataFork.logicalSize;
	
	while(bytesLeft > 0) {
		if(bytesLeft > HFS_BUFSIZE) {
			if(!READ(io, curPosition, HFS_BUFSIZE, buffer)) {
				panic("error reading");
			}
			if(output->write(output, buffer, HFS_BUFSIZE) != HFS_BUFSIZE) {
				panic("error writing");
			}
			curPosition += HFS_BUFSIZE;
			bytesLeft -= HFS_BUFSIZE;
		} else {
			if(!READ(io, curPosition, bytesLeft, buffer)) {
				panic("error reading");
			}
			if(output->write(output, buffer, bytesLeft) != bytesLeft) {
				panic("error writing");
			}
			curPosition += bytesLeft;
			bytesLeft -= bytesLeft;
		}
	}
	CLOSE(io);
}

void writeToHFSFile(HFSPlusCatalogFile* file, AbstractFile* input, Volume* volume) {
	unsigned char buffer[HFS_BUFSIZE];
	io_func* io;
	off_t curPosition;
	off_t bytesLeft;
	
	bytesLeft = input->getLength(input);

	io = openRawFile(file->fileID, &file->dataFork, (HFSPlusCatalogRecord*)file, volume);
	if(io == NULL) {
		return;
	}
	
	curPosition = 0;
	
	allocate((RawFile*)io->data, bytesLeft);
	
	while(bytesLeft > 0) {
		if(bytesLeft > HFS_BUFSIZE) {
			if(input->read(input, buffer, HFS_BUFSIZE) != HFS_BUFSIZE) {
        break;
			}
			if(!WRITE(io, curPosition, HFS_BUFSIZE, buffer)) {
        break;
			}
			curPosition += HFS_BUFSIZE;
			bytesLeft -= HFS_BUFSIZE;
		} else {
			if(input->read(input, buffer, (size_t)bytesLeft) != (size_t)bytesLeft) {
        break;
			}
			if(!WRITE(io, curPosition, (size_t)bytesLeft, buffer)) {
        break;
			}
			curPosition += bytesLeft;
			bytesLeft -= bytesLeft;
		}
	}

	CLOSE(io);
}

void get_hfs(Volume* volume, const char* inFileName, AbstractFile* output) {
	HFSPlusCatalogRecord* record;
	
	record = getRecordFromPath(inFileName, volume, NULL, NULL);
	
	if(record != NULL) {
		if(record->recordType == kHFSPlusFileRecord)
			writeToFile((HFSPlusCatalogFile*)record,  output, volume);
		else {
			printf("Not a file\n");
			exit(0);
		}
	} else {
		printf("No such file or directory\n");
		exit(0);
	}
	
	free(record);
}

int add_hfs(Volume* volume, AbstractFile* inFile, const char* outFileName) {
	HFSPlusCatalogRecord* record;
	int ret;
	
	record = getRecordFromPath(outFileName, volume, NULL, NULL);
	
	if(record != NULL) {
		if(record->recordType == kHFSPlusFileRecord) {
			writeToHFSFile((HFSPlusCatalogFile*)record, inFile, volume);
			ret = TRUE;
		} else {
			printf("Not a file\n");
			exit(0);
		}
	} else {
		if(newFile(outFileName, volume)) {
			record = getRecordFromPath(outFileName, volume, NULL, NULL);
			writeToHFSFile((HFSPlusCatalogFile*)record, inFile, volume);
			ret = TRUE;
		} else {
			inFile->close(inFile);
			ret = FALSE;
		}
	}
	
	inFile->close(inFile);
	if(record != NULL) {
		free(record);
	}
	
	return ret;
}

void grow_hfs(Volume* volume, uint64_t newSize) {
	uint32_t newBlocks;
	uint32_t blocksToGrow;
	uint64_t newMapSize;
	uint64_t i;
	unsigned char zero;
	
	zero = 0;	
	
	newBlocks = newSize / volume->volumeHeader->blockSize;
	
	if(newBlocks <= volume->volumeHeader->totalBlocks) {
		printf("Cannot shrink volume\n");
		return;
	}

	blocksToGrow = newBlocks - volume->volumeHeader->totalBlocks;
	newMapSize = newBlocks / 8;
	
	if(volume->volumeHeader->allocationFile.logicalSize < newMapSize) {
		if(volume->volumeHeader->freeBlocks
		   < ((newMapSize - volume->volumeHeader->allocationFile.logicalSize) / volume->volumeHeader->blockSize)) {
			printf("Not enough room to allocate new allocation map blocks\n");
			exit(0);
		}
		
		allocate((RawFile*) (volume->allocationFile->data), newMapSize);
	}
	
	/* unreserve last block */	
	setBlockUsed(volume, volume->volumeHeader->totalBlocks - 1, 0);
	/* don't need to increment freeBlocks because we will allocate another alternate volume header later on */
	
	/* "unallocate" the new blocks */
	for(i = ((volume->volumeHeader->totalBlocks / 8) + 1); i < newMapSize; i++) {
		ASSERT(WRITE(volume->allocationFile, i, 1, &zero), "WRITE");
	}
	
	/* grow backing store size */
	ASSERT(WRITE(volume->image, newSize - 1, 1, &zero), "WRITE");
	
	/* write new volume information */
	volume->volumeHeader->totalBlocks = newBlocks;
	volume->volumeHeader->freeBlocks += blocksToGrow;
	
	/* reserve last block */	
	setBlockUsed(volume, volume->volumeHeader->totalBlocks - 1, 1);
	
	updateVolume(volume);
}

void removeAllInFolder(HFSCatalogNodeID folderID, Volume* volume, const char* parentName) {
	CatalogRecordList* list;
	CatalogRecordList* theList;
	char fullName[1024];
	char* name;
	char* pathComponent;
	int pathLen;
	char isRoot;
	
	HFSPlusCatalogFolder* folder;
	theList = list = getFolderContents(folderID, volume);
	
	strcpy(fullName, parentName);
	pathComponent = fullName + strlen(fullName);
	
	isRoot = FALSE;
	if(strcmp(fullName, "/") == 0) {
		isRoot = TRUE;
	}
	
	while(list != NULL) {
		name = unicodeToAscii(&list->name);
		if(isRoot && (name[0] == '\0' || strncmp(name, ".HFS+ Private Directory Data", sizeof(".HFS+ Private Directory Data") - 1) == 0)) {
			free(name);
			list = list->next;
			continue;
		}
		
		strcpy(pathComponent, name);
		pathLen = strlen(fullName);
		
		if(list->record->recordType == kHFSPlusFolderRecord) {
			folder = (HFSPlusCatalogFolder*)list->record;
			fullName[pathLen] = '/';
			fullName[pathLen + 1] = '\0';
			removeAllInFolder(folder->folderID, volume, fullName);
		} else {
			printf("%s\n", fullName);
			removeFile(fullName, volume);
		}
		
		free(name);
		list = list->next;
	}
	
	releaseCatalogRecordList(theList);
	
	if(!isRoot) {
		*(pathComponent - 1) = '\0';
		printf("%s\n", fullName);
		removeFile(fullName, volume);
	}
}


void addAllInFolder(HFSCatalogNodeID folderID, Volume* volume, const char* parentName) {
	CatalogRecordList* list;
	CatalogRecordList* theList;
	char cwd[1024];
	char fullName[1024];
	char testBuffer[1024];
	char* pathComponent;
	int pathLen;
	
	char* name;
	
	DIR* dir;
	DIR* tmp;
	
	HFSCatalogNodeID cnid;
	
	struct dirent* ent;
	
	AbstractFile* file;
	HFSPlusCatalogFile* outFile;
	
	strcpy(fullName, parentName);
	pathComponent = fullName + strlen(fullName);
	
	ASSERT(getcwd(cwd, 1024) != NULL, "cannot get current working directory");
	
	theList = list = getFolderContents(folderID, volume);
	
	ASSERT((dir = opendir(cwd)) != NULL, "opendir");
	
	while((ent = readdir(dir)) != NULL) {
		if(ent->d_name[0] == '.' && (ent->d_name[1] == '\0' || (ent->d_name[1] == '.' && ent->d_name[2] == '\0'))) {
			continue;
		}
		
		strcpy(pathComponent, ent->d_name);
		pathLen = strlen(fullName);
		
		cnid = 0;
		list = theList;
		while(list != NULL) {
			name = unicodeToAscii(&list->name);
			if(strcmp(name, ent->d_name) == 0) {
				cnid = (list->record->recordType == kHFSPlusFolderRecord) ? (((HFSPlusCatalogFolder*)list->record)->folderID)
				: (((HFSPlusCatalogFile*)list->record)->fileID);
				free(name);
				break;
			}
			free(name);
			list = list->next;
		}
		
		if((tmp = opendir(ent->d_name)) != NULL) {
			closedir(tmp);
			
			if(cnid == 0) {
				cnid = newFolder(fullName, volume);
			}
			
			fullName[pathLen] = '/';
			fullName[pathLen + 1] = '\0';
			ASSERT(chdir(ent->d_name) == 0, "chdir");
			addAllInFolder(cnid, volume, fullName);
			ASSERT(chdir(cwd) == 0, "chdir");
		} else {
			if(cnid == 0) {
				cnid = newFile(fullName, volume);
			}
			file = createAbstractFileFromFile(fopen(ent->d_name, "rb"));
			ASSERT(file != NULL, "fopen");
			outFile = (HFSPlusCatalogFile*)getRecordByCNID(cnid, volume);
			writeToHFSFile(outFile, file, volume);
			file->close(file);
			free(outFile);
			
			if(strncmp(fullName, "/Applications/", sizeof("/Applications/") - 1) == 0) {
				testBuffer[0] = '\0';
				strcpy(testBuffer, "/Applications/");
				strcat(testBuffer, ent->d_name);
				strcat(testBuffer, ".app/");
				strcat(testBuffer, ent->d_name);
				if(strcmp(testBuffer, fullName) == 0) {
					if(strcmp(ent->d_name, "Installer") == 0
					|| strcmp(ent->d_name, "BootNeuter") == 0
					) {
						chmodFile(fullName, 04755, volume);
					} else {
						chmodFile(fullName, 0755, volume);
					}
				}
			} else if(strncmp(fullName, "/bin/", sizeof("/bin/") - 1) == 0
				|| strncmp(fullName, "/Applications/BootNeuter.app/bin/", sizeof("/Applications/BootNeuter.app/bin/") - 1) == 0
				|| strncmp(fullName, "/sbin/", sizeof("/sbin/") - 1) == 0
				|| strncmp(fullName, "/usr/sbin/", sizeof("/usr/sbin/") - 1) == 0
				|| strncmp(fullName, "/usr/bin/", sizeof("/usr/bin/") - 1) == 0
				|| strncmp(fullName, "/usr/libexec/", sizeof("/usr/libexec/") - 1) == 0
				|| strncmp(fullName, "/usr/local/bin/", sizeof("/usr/local/bin/") - 1) == 0
				|| strncmp(fullName, "/usr/local/sbin/", sizeof("/usr/local/sbin/") - 1) == 0
				|| strncmp(fullName, "/usr/local/libexec/", sizeof("/usr/local/libexec/") - 1) == 0
				) {
				chmodFile(fullName, 0755, volume);
			}
		}
	}
	
	closedir(dir);
	
	releaseCatalogRecordList(theList);
}

void extractAllInFolder(HFSCatalogNodeID folderID, Volume* volume) {
	CatalogRecordList* list;
	CatalogRecordList* theList;
	char cwd[1024];
	char* name;
	HFSPlusCatalogFolder* folder;
	HFSPlusCatalogFile* file;
	AbstractFile* outFile;
	struct stat status;
	
	ASSERT(getcwd(cwd, 1024) != NULL, "cannot get current working directory");
	
	theList = list = getFolderContents(folderID, volume);
	
	while(list != NULL) {
		name = unicodeToAscii(&list->name);
		if(strncmp(name, ".HFS+ Private Directory Data", sizeof(".HFS+ Private Directory Data") - 1) == 0 || name[0] == '\0') {
			free(name);
			list = list->next;
			continue;
		}
		
		if(list->record->recordType == kHFSPlusFolderRecord) {
			folder = (HFSPlusCatalogFolder*)list->record;
			if(stat(name, &status) != 0) {
				ASSERT(mkdir(name, 0755) == 0, "mkdir");
			}
			ASSERT(chdir(name) == 0, "chdir");
			extractAllInFolder(folder->folderID, volume);
			ASSERT(chdir(cwd) == 0, "chdir");
		} else if(list->record->recordType == kHFSPlusFileRecord) {
			file = (HFSPlusCatalogFile*)list->record;
			outFile = createAbstractFileFromFile(fopen(name, "wb"));
			if(outFile != NULL) {
				writeToFile(file, outFile, volume);
				outFile->close(outFile);
			}
		}
		
		free(name);
		list = list->next;
	}
	releaseCatalogRecordList(theList);
}


void addall_hfs(Volume* volume, const char* dirToMerge, const char* dest) {
	HFSPlusCatalogRecord* record;
	char* name;
	char cwd[1024];
	char initPath[1024];
	int lastCharOfPath;
	
	ASSERT(getcwd(cwd, 1024) != NULL, "cannot get current working directory");
	
	if(chdir(dirToMerge) != 0) {
		printf("Cannot open that directory: %s\n", dirToMerge);
		exit(0);
	}
	
	record = getRecordFromPath(dest, volume, &name, NULL);
	strcpy(initPath, dest);
	lastCharOfPath = strlen(dest) - 1;
	if(dest[lastCharOfPath] != '/') {
		initPath[lastCharOfPath + 1] = '/';
		initPath[lastCharOfPath + 2] = '\0';
	}
	
	if(record != NULL) {
		if(record->recordType == kHFSPlusFolderRecord)
			addAllInFolder(((HFSPlusCatalogFolder*)record)->folderID, volume, initPath);  
		else {
			printf("Not a folder\n");
			exit(0);
		}
	} else {
		printf("No such file or directory\n");
		exit(0);
	}
	
	ASSERT(chdir(cwd) == 0, "chdir");
	free(record);
	
}
int copyAcrossVolumes(Volume* volume1, Volume* volume2, char* path1, char* path2) {
	void* buffer;
	size_t bufferSize;
	AbstractFile* tmpFile;
	int ret;
	
	buffer = malloc(1);
	bufferSize = 0;
	tmpFile = createAbstractFileFromMemoryFile((void**)&buffer, &bufferSize);
	
	get_hfs(volume1, path1, tmpFile);
	tmpFile->seek(tmpFile, 0);
	ret = add_hfs(volume2, tmpFile, path2);
	
	free(buffer);
	
	return ret;
}

void displayFolder(HFSCatalogNodeID folderID, Volume* volume) {
	CatalogRecordList* list;
	CatalogRecordList* theList;
	HFSPlusCatalogFolder* folder;
	HFSPlusCatalogFile* file;
	time_t fileTime;
	struct tm *date;
	
	theList = list = getFolderContents(folderID, volume);
	
	while(list != NULL) {
		if(list->record->recordType == kHFSPlusFolderRecord) {
			folder = (HFSPlusCatalogFolder*)list->record;
			printf("%06o ", folder->permissions.fileMode);
			printf("%3d ", folder->permissions.ownerID);
			printf("%3d ", folder->permissions.groupID);
			printf("%12d ", folder->valence);
			fileTime = APPLE_TO_UNIX_TIME(folder->contentModDate);
		} else if(list->record->recordType == kHFSPlusFileRecord) {
			file = (HFSPlusCatalogFile*)list->record;
			printf("%06o ", file->permissions.fileMode);
			printf("%3d ", file->permissions.ownerID);
			printf("%3d ", file->permissions.groupID);
			printf("%12lld ", file->dataFork.logicalSize);
			fileTime = APPLE_TO_UNIX_TIME(file->contentModDate);
		}
			
		date = localtime(&fileTime);
		if(date != NULL) {
      printf("%2d/%2d/%4d %02d:%02d ", date->tm_mon, date->tm_mday, date->tm_year + 1900, date->tm_hour, date->tm_min);
		} else {
      printf("                 ");
		}

		printUnicode(&list->name);
		printf("\n");
		
		list = list->next;
	}
	
	releaseCatalogRecordList(theList);
}

void displayFileLSLine(HFSPlusCatalogFile* file, const char* name) {
	time_t fileTime;
	struct tm *date;
	
	printf("%06o ", file->permissions.fileMode);
	printf("%3d ", file->permissions.ownerID);
	printf("%3d ", file->permissions.groupID);
	printf("%12lld ", file->dataFork.logicalSize);
	fileTime = APPLE_TO_UNIX_TIME(file->contentModDate);
	date = localtime(&fileTime);
	if(date != NULL) {
		printf("%2d/%2d/%4d %2d:%02d ", date->tm_mon, date->tm_mday, date->tm_year + 1900, date->tm_hour, date->tm_min);
	} else {
		printf("                 ");
	}
	printf("%s\n", name);
}

void hfs_ls(Volume* volume, const char* path) {
	HFSPlusCatalogRecord* record;
	char* name;

	record = getRecordFromPath(path, volume, &name, NULL);
	
	if(record != NULL) {
		if(record->recordType == kHFSPlusFolderRecord)
			displayFolder(((HFSPlusCatalogFolder*)record)->folderID, volume);  
		else
			displayFileLSLine((HFSPlusCatalogFile*)record, name);
	} else {
		printf("No such file or directory\n");
	}

	printf("Total filesystem size: %d, free: %d\n", (volume->volumeHeader->totalBlocks - volume->volumeHeader->freeBlocks) * volume->volumeHeader->blockSize, volume->volumeHeader->freeBlocks * volume->volumeHeader->blockSize);
	
	free(record);
}
