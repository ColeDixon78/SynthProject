//
//  WaveFile.hpp
//  WavetableSynth
//
//  Created by Cole Dixon on 5/30/23.
//

#ifndef WaveFile_hpp
#define WaveFile_hpp

#define _USE_MATH_DEFINES
//type defs
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

#include <stdio.h>
bool WriteWaveFile(const char *szFileName, void *pData, int32 nDataSize, int16 nNumChannels, int32 nSampleRate, int32_t nBitsPerSample);
#endif /* WaveFile_hpp */