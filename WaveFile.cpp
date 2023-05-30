//
//  WaveFile.cpp
//  WavetableSynth
//
//  Created by Cole Dixon on 5/30/23.
//

#include "WaveFile.hpp"
#include <iostream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#define _USE_MATH_DEFINES
//type defs
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;

#define CLAMP(value,min,max) {if(value < min) { value = min; } else if(value > max) { value = max; }}
//wave file header
struct SMinimalWaveFileHeader
{
    //the main chunk
    unsigned char m_szChunkID[4];
    uint32          m_nChunkSize;
    unsigned char m_szFormat[4];

    //sub chunk 1 "fmt "
    unsigned char m_szSubChunk1ID[4];
    uint32          m_nSubChunk1Size;
    uint16          m_nAudioFormat;
    uint16          m_nNumChannels;
    uint32          m_nSampleRate;
    uint32          m_nByteRate;
    uint16          m_nBlockAlign;
    uint16          m_nBitsPerSample;

    //sub chunk 2 "data"
    unsigned char m_szSubChunk2ID[4];
    uint32          m_nSubChunk2Size;

    //then comes the data!
};

bool WriteWaveFile(const char *szFileName, void *pData, int32 nDataSize, int16 nNumChannels, int32 nSampleRate, int32_t nBitsPerSample){
    //open the file if we can
    FILE *File = fopen(szFileName,"w+b");
    if(!File)
    {
        return false;
    }

    SMinimalWaveFileHeader waveHeader;

    //fill out the main chunk
    memcpy(waveHeader.m_szChunkID,"RIFF",4);
    waveHeader.m_nChunkSize = nDataSize + 36;
    memcpy(waveHeader.m_szFormat,"WAVE",4);

    //fill out sub chunk 1 "fmt "
    memcpy(waveHeader.m_szSubChunk1ID,"fmt ",4);
    waveHeader.m_nSubChunk1Size = 16;
    waveHeader.m_nAudioFormat = 1;
    waveHeader.m_nNumChannels = nNumChannels;
    waveHeader.m_nSampleRate = nSampleRate;
    waveHeader.m_nByteRate = nSampleRate * nNumChannels * nBitsPerSample / 8;
    waveHeader.m_nBlockAlign = nNumChannels * nBitsPerSample / 8;
    waveHeader.m_nBitsPerSample = nBitsPerSample;

    //fill out sub chunk 2 "data"
    memcpy(waveHeader.m_szSubChunk2ID,"data",4);
    waveHeader.m_nSubChunk2Size = nDataSize;

    //write the header
    fwrite(&waveHeader,sizeof(SMinimalWaveFileHeader),1,File);

    //write the wave data itself
    fwrite(pData,nDataSize,1,File);

    //close the file and return success
    fclose(File);
    return true;
};
