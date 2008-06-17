#ifndef OUTPUTSTATE_H
#define OUTPUTSTATE_H

#include <abstractfile.h>

typedef struct OutputState {
	char* fileName;
	void* buffer;
	size_t bufferSize;
	struct OutputState* next;
	struct OutputState* prev;
} OutputState;

#ifdef __cplusplus
extern "C" {
#endif
	void addToOutput(OutputState** state, char* fileName, void* buffer, size_t bufferSize);
	AbstractFile* getFileFromOutputState(OutputState** state, const char* fileName);
	AbstractFile* getFileFromOutputStateForOverwrite(OutputState** state, const char* fileName);
	void writeOutput(OutputState** state, char* ipsw);
	void releaseOutput(OutputState** state);
	OutputState* loadZip(const char* ipsw);
#ifdef __cplusplus
}
#endif

#endif

