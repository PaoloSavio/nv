/*
 *  BufferUtils.c
 *  Notation
 *
 *  Created by Zachary Schneirov on 1/15/06.
 */

/*Copyright (c) 2010, Zachary Schneirov. All rights reserved.
  Redistribution and use in source and binary forms, with or without modification, are permitted 
  provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright notice, this list of conditions 
     and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice, this list of 
	 conditions and the following disclaimer in the documentation and/or other materials provided with
     the distribution.
   - Neither the name of Notational Velocity nor the names of its contributors may be used to endorse 
     or promote products derived from this software without specific prior written permission. */


#include "BufferUtils.h"
#include <string.h>

static const unsigned char gsToLowerMap[256] = {
'\0', 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, '\t',
'\n', 0x0b, 0x0c, '\r', 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
0x1e, 0x1f,  ' ',  '!',  '"',  '#',  '$',  '%',  '&', '\'',
'(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',  '0',  '1',
'2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',
'<',  '=',  '>',  '?',  '@',  'a',  'b',  'c',  'd',  'e',
'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',
'z',  '[', '\\',  ']',  '^',  '_',  '`',  'a',  'b',  'c',
'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',
'n',  'o',  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
'x',  'y',  'z',  '{',  '|',  '}',  '~', 0x7f, 0x80, 0x81,
0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8a, 0x8b,
0x8c, 0x8d, 0x8e, 0x8f, 0x90, 0x91, 0x92, 0x93, 0x94, 0x95,
0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 0xb0, 0xb1, 0xb2, 0xb3,
0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd,
0xbe, 0xbf, 0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf, 0xd0, 0xd1,
0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xdb,
0xdc, 0xdd, 0xde, 0xdf, 0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5,
0xe6, 0xe7, 0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };

#define MIN(a, b)  (((a)<(b))?(a):(b))

char *replaceString(char *oldString, const char *newString) {
    size_t newLen = strlen(newString) + 1;

    //realloc is smart enough to do better memory management than we can do right here
    char *resizedString = (char*)realloc(oldString, newLen);
    memmove(resizedString, newString, newLen);
    
    return resizedString;
}

void ResizeBuffer(void ***buffer, unsigned int objCount, unsigned int *bufSize) {
	assert(buffer && bufSize);
	
	if (*bufSize < objCount || !*buffer) {
		*buffer = (void **)realloc(*buffer, sizeof(void*) * objCount);
		*bufSize = objCount;
	}
	
}

int IsZeros(const void *s1, size_t n) {
	if (n != 0) {
		const unsigned char *p1 = s1;
		
		do {
			if (*p1++ != 0)
				return (0);
		} while (--n != 0);
	}
	return (1);
}

void modp_tolower_copy(char* dest, const char* str, int len) {
	int i;
	NSUInteger eax, ebx;
	const uint8_t* ustr = (const uint8_t*) str;
	const int leftover = len % sizeof(NSUInteger);
	const int imax = len / sizeof(NSUInteger);
	const NSUInteger* s = (const NSUInteger*) str;
	NSUInteger* d = (NSUInteger*) dest;
	for (i = 0; i != imax; ++i) {
		eax = s[i];
		/*
		 * This is based on the algorithm by Paul Hsieh
		 * http://www.azillionmonkeys.com/qed/asmexample.html
		 */
#if __LP64__ || NS_BUILD_32_LIKE_64
		ebx = (0x7f7f7f7f7f7f7f7fllu & eax) + 0x2525252525252525llu;
		ebx = (0x7f7f7f7f7f7f7f7fllu & ebx) + 0x1a1a1a1a1a1a1a1allu;
		ebx = ((ebx & ~eax) >> 2)  & 0x2020202020202020llu;
#else
		ebx = (0x7f7f7f7fu & eax) + 0x25252525u;
		ebx = (0x7f7f7f7fu & ebx) + 0x1a1a1a1au;
		ebx = ((ebx & ~eax) >> 2)  & 0x20202020u;
#endif
		*d++ = eax + ebx;
	}
	
	i = imax * sizeof(NSUInteger);
	dest = (char*) d;
	switch (leftover) {
#if __LP64__ || NS_BUILD_32_LIKE_64
		case 7: *dest++ = (char) gsToLowerMap[ustr[i++]];
		case 6: *dest++ = (char) gsToLowerMap[ustr[i++]];
		case 5: *dest++ = (char) gsToLowerMap[ustr[i++]];
		case 4: *dest++ = (char) gsToLowerMap[ustr[i++]];			
#endif
		case 3: *dest++ = (char) gsToLowerMap[ustr[i++]];
		case 2: *dest++ = (char) gsToLowerMap[ustr[i++]];
		case 1: *dest++ = (char) gsToLowerMap[ustr[i]];
		case 0: *dest = '\0';
	}
}

void replace_breaks(char *str, size_t up_to_len) {
	
	//traverses string to up_to_len chars or NULL, whichever comes first
	//replaces any occurance of \n, \r, or \t with a space
	
	if (!str) return;
	
	size_t i = 0;
	int c;
	char *s = str;
	do {
		c = *s;
		if ('\n' == c || '\t' == c || '\r' == c) {
			*s = ' ';
		}
	} while (++i < up_to_len && *(s++) != 0);
}

int ContainsUInteger(const NSUInteger *uintArray, size_t count, NSUInteger auint) {
	size_t i;
	for (i=0; i<count; i++) {
		if (uintArray[i] == auint) return 1;
	}
	return 0;
}


int ContainsHighAscii(const void *s1, size_t n) {
	
	register NSUInteger *intBuffer = (NSUInteger*)s1;
	register NSUInteger i, integerCount = n/sizeof(NSUInteger);	
	register NSUInteger pattern = 
#if __LP64__ || NS_BUILD_32_LIKE_64
	0x8080808080808080;
#else
	0x80808080;
#endif
	
	for (i=0; i<integerCount; i++ ) {
		if (pattern & intBuffer[i]) {
			return 1;
		}
	}
	
	unsigned char *charBuffer = (unsigned char*)s1;
	NSUInteger leftOverCharCount = n % sizeof(NSUInteger);
	
	for (i = n - leftOverCharCount; i<n; i++) {
		if (charBuffer[i] > 127) {
			return 1;
		}
	}
	
	return 0;
}

CFStringRef CFStringFromBase10Integer(int quantity) {
	char *buffer = NULL;
	if (asprintf(&buffer, "%d", quantity) < 0 || !buffer)
		return nil;
	
	//try to get on the fast path of __CFStringCreateImmutableFunnel3; most overhead will be in cfruntime allocation, anyway, though, so whatever.
	CFStringEncoding encoding = CFStringGetSystemEncoding() == kCFStringEncodingMacRoman ? kCFStringEncodingMacRoman : kCFStringEncodingASCII;
	return CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, buffer, encoding, kCFAllocatorDefault);	
}

unsigned DumbWordCount(const void *s1, size_t len) {

	unsigned count = len > 0;
	//we could do a lot more here, but we don't.
	const void *ptr = s1;
	while ((ptr = memchr(ptr + 1, 0x20, len))) {
		count++;
	}
//	printf("bacon: %u\n", count);

	return count;
}

NSInteger genericSortContextFirst(int (*context) (void*, void*), void* one, void* two) {
	
	return context(one, two);
}

NSInteger genericSortContextLast(void* one, void* two, int (*context) (void*, void*)) {
	
	return context(&one, &two);
}

void QuickSortBuffer(void **buffer, unsigned int objCount, int (*compar)(const void *, const void *)) {
	qsort_r((void *)buffer, (size_t)objCount, sizeof(void*), compar, (int (*)(void *, const void *, const void *))genericSortContextFirst);
}

/*
CFStringRef CopyReasonFromFSErr(OSStatus err) {
    
    size_t codeCount = sizeof(errorCodes) / sizeof(OSStatus);
    size_t stringCount = sizeof(errorStrings) / sizeof(char*);
    assert(stringCount == codeCount);
    
    unsigned int i;
    
    if (err < 0) {
	
	for (i=0; i<codeCount; i++) {
	    if (errorCodes[i] == err)
		return CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, errorStrings[i], kCFStringEncodingUTF8, kCFAllocatorNull);
	}
	return CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("an error of type %d occurred"), err);
    }
    
    return CFStringCreateWithCStringNoCopy(kCFAllocatorDefault, strerror((int)err), kCFStringEncodingUTF8, kCFAllocatorNull);
}*/

#if 0
//this does not use the user's defined date styles
const double dayInSeconds = 86400.0;
enum {ThisDay = 0, NextDay, PriorDay};
CFStringRef GetRelativeDateStringFromTimeAndLocaleInfo(CFAbsoluteTime time, CFStringRef *designations, char **months) {
    static CFAbsoluteTime currentDay = 0.0;
    if (currentDay == 0.0)
	currentDay = ceil(CFAbsoluteTimeGetCurrent() / dayInSeconds) * dayInSeconds;

    CFGregorianDate unitsDate = CFAbsoluteTimeGetGregorianDate(time, NULL);
    
    CFAbsoluteTime timeDay = ceil(time / dayInSeconds) * dayInSeconds;
    if (timeDay == currentDay) {
	return designations[ThisDay];
    } else if (timeDay == currentDay + dayInSeconds) {
	return designations[NextDay];
    } else if (timeDay == currentDay - dayInSeconds) {
	return designations[PriorDay];
    }
    
    return CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR("%s %u, %u  %u:%u %s"), 
				    months[unitsDate.month], unitsDate.day, unitsDate.year, unitsDate.hour, unitsDate.minute, amppmStr);
}
#endif

CFStringRef CreateRandomizedFileName() {
    static int sequence = 0;
    
    sequence++;
    
    ProcessSerialNumber psn;
    OSStatus err = noErr;
    if ((err = GetCurrentProcess(&psn)) != noErr) {
	printf("error getting process serial number: %d\n", (int)err);
	
	//just use the location of our memory
	psn.lowLongOfPSN = (unsigned long)&psn;
    }
    
    CFStringRef name = CFStringCreateWithFormat(kCFAllocatorDefault, NULL, CFSTR(".%lu%lu-%d-%d"), 
						psn.highLongOfPSN, psn.lowLongOfPSN, (int)CFAbsoluteTimeGetCurrent(), sequence);
    
    return name;
}

OSStatus FSCreateFileIfNotPresentInDirectory(FSRef *directoryRef, FSRef *childRef, CFStringRef filename, Boolean *created) {
	UniChar chars[256];
    OSStatus result = noErr;
	
    if (created) *created = false;
    
    if ((result = FSRefMakeInDirectoryWithString(directoryRef, childRef, filename, chars))) {
		if (result == fnfErr) {
			if (created) *created = true;
			
			result = FSCreateFileUnicode(directoryRef, CFStringGetLength(filename), chars, kFSCatInfoNone, NULL, childRef, NULL);
		}
		return result;
    }
    
    return noErr;	
}

OSStatus FSRefMakeInDirectoryWithString(FSRef *directoryRef, FSRef *childRef, CFStringRef filename, UniChar* charsBuffer) {
    CFRange range;
    range.location = 0;
    range.length = CFStringGetLength(filename);
	
	if (range.length > 255)	return errFSNameTooLong;
	
    CFStringGetCharacters(filename, range, charsBuffer);

    return FSMakeFSRefUnicode(directoryRef, range.length, charsBuffer, kTextEncodingDefaultFormat, childRef);
}

//use BlockSizeForNotation((NotationController *)delegate) for maximum read size
//use noCacheMask for options if not expecting to read again
OSStatus FSRefReadData(FSRef *fsRef, size_t maximumReadSize, UInt64 *bufferSize, void** newBuffer, UInt16 modeOptions) {
    OSStatus err = noErr;
	HFSUniStr255 dfName; //this is just NULL / 0, anyway
    FSIORefNum refNum;
    SInt64 forkSize;
    ByteCount readActualCount = 0, totalReadBytes = 0;
	
	if (!bufferSize || !newBuffer || !fsRef) {
		printf("FSRefReadData: NULL buffers or fsRef\n");
		return paramErr;
	}
    
    if ((err = FSGetDataForkName(&dfName)) != noErr) {
		printf("FSGetDataForkName: error %d\n", (int)err);
		return err;
    }
    
	//FSOpenFork
    //get vrefnum or whatever
    //get fork size
	//read data
    if ((err = FSOpenFork(fsRef, dfName.length, dfName.unicode, fsRdPerm, &refNum)) != noErr) {
		printf("FSOpenFork: error %d\n", (int)err);
		return err;
    }
    if ((forkSize = *bufferSize) < 1) {
		if ((err = FSGetForkSize(refNum, &forkSize)) != noErr) {
			printf("FSGetForkSize: error %d\n", (int)err);
			return err;
		}
    }
    
	size_t copyBufferSize = MIN(maximumReadSize, (size_t)forkSize);
    void *fullSizeBuffer = (void*)valloc(forkSize);
    
    while (noErr == err && totalReadBytes < (ByteCount)forkSize) {
		err = FSReadFork(refNum, fsAtMark + modeOptions, 0, copyBufferSize, fullSizeBuffer + totalReadBytes, &readActualCount);
		totalReadBytes += readActualCount;
    }
    OSErr lastReadErr = err;
	
	if ((err = FSCloseFork(refNum)) != noErr)
		printf("FSCloseFork: error %d\n", (int)err);
    
    *newBuffer = fullSizeBuffer;
	//in case we read less than the expected size or the size was not initially known
	*bufferSize = totalReadBytes;
    
    return (eofErr == lastReadErr ? noErr : lastReadErr);
}

OSStatus FSRefWriteData(FSRef *fsRef, size_t maximumWriteSize, UInt64 bufferSize, const void* buffer, UInt16 modeOptions, Boolean truncateFile) {
	OSStatus err = noErr;
	HFSUniStr255 dfName; //this is just NULL / 0, anyway
    FSIORefNum refNum;
    ByteCount writeActualCount = 0, totalWrittenBytes = 0;
	
	if (!buffer || !fsRef) {
		printf("FSRefWriteData: NULL buffers or fsRef\n");
		return paramErr;
	}
    
    if ((err = FSGetDataForkName(&dfName)) != noErr) {
		printf("FSGetDataForkName: error %d\n", (int)err);
		return err;
    }
    
	//FSOpenFork
    //get vrefnum or whatever
    if ((err = FSOpenFork(fsRef, dfName.length, dfName.unicode, fsWrPerm, &refNum)) != noErr) {
		printf("FSOpenFork: error %d\n", (int)err);
		return err;
    }
    
	ByteCount writeBufferSize = MIN(maximumWriteSize, bufferSize);
    
    while (noErr == err && totalWrittenBytes < bufferSize) {

	err = FSWriteFork(refNum, fsAtMark + modeOptions, 0, 
			  MIN(writeBufferSize, bufferSize - totalWrittenBytes),
			  buffer + totalWrittenBytes, &writeActualCount);
	totalWrittenBytes += writeActualCount;
    }
    OSErr writeError = err;
	
	if (truncateFile && (err = FSSetForkSize(refNum, fsFromStart, bufferSize))) {
		printf("FSOpenFork: FSSetForkSize %d\n", (int)err);
		return err;
	}
    
	if ((err = FSCloseFork(refNum)) != noErr)
		printf("FSCloseFork: error %d\n", (int)err);
	
    return writeError;
}
