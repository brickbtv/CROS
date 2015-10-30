#ifndef _BOOT_H_
#define _BOOT_H_

/*!
*	Filled by linker on ROM preparing stage.
*/
typedef struct ROMProcessInfo
{
	unsigned int readOnlyAddr; // address
	unsigned int readOnlySize; // size in bytes
	unsigned int readWriteAddr; // address
	unsigned int readWriteSize; // size in bytes
	unsigned int sharedReadWriteAddr;
	unsigned int sharedReadWriteSize;
} ROMProcessInfo;

// LINKER SEARCH THIS NAME "processInfo". Exactly "processInfo". Srsly. "processInfo". Case sensitive.
extern ROMProcessInfo processInfo;

#endif