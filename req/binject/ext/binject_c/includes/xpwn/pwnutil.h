#ifndef PWNUTIL_H
#define PWNUTIL_H

#include <xpwn/plist.h>
#include <xpwn/outputstate.h>
#include <hfs/hfsplus.h>

#ifdef __cplusplus
extern "C" {
#endif
	int patch(AbstractFile* in, AbstractFile* out, AbstractFile* patch);
	Dictionary* parseIPSW(const char* inputIPSW, const char* bundleRoot, char** bundlePath, OutputState** state);
	int doPatch(StringValue* patchValue, StringValue* fileValue, const char* bundlePath, OutputState** state);
	void doPatchInPlace(Volume* volume, const char* filePath, const char* patchPath);
	void fixupBootNeuterArgs(Volume* volume, char unlockBaseband, char selfDestruct, char use39, char use46);
#ifdef __cplusplus
}
#endif

#endif
