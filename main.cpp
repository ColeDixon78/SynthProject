//
//  main.cpp
//  WavetableSynth
//
//  Created by Cole Dixon on 5/30/23.
//

#include <iostream>
#include "WaveFile.hpp"
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

void ConvertFloatToAudioSample(float fFloat, int16 &nOut)
{
	fFloat *= 32767.0f;
	CLAMP(fFloat,-32768.0f,32767.0f);
	nOut = (int16)fFloat;
}

template <typename T>
bool WriteWaveFile(const char *szFileName, float *pRawData, int32 nNumSamples, int16 nNumChannels, int32 nSampleRate)
{
	//open the file if we can
	FILE *File = fopen(szFileName,"w+b");
	if(!File)
	{
		return false;
	}

	//calculate bits per sample and the data size
	int32 nBitsPerSample = sizeof(T) * 8;
	int nDataSize = nNumSamples * sizeof(T);

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

	//write the wave data itself, converting it from float to the type specified
	T *pData = new T[nNumSamples];
	for(int nIndex = 0; nIndex < nNumSamples; ++nIndex)
		ConvertFloatToAudioSample(pRawData[nIndex],pData[nIndex]);
	fwrite(pData,nDataSize,1,File);
	delete[] pData;

	//close the file and return success
	fclose(File);
	return true;
}


float Sine_Oscilator(float &phase, float frequency, float sampleRate){
    phase += 2 * (float)M_PI * frequency/sampleRate;
    if(phase >= 2 * (float)M_PI){
        phase -= 2*(float)M_PI;
    }
    if(phase < 0){
        phase += 2 * (float)M_PI;
    }
    return sin(phase);
}

float Calculate_Frequency(char note, int oct){
    return (float)(440*pow(2.0,((double)((oct-4)*12+(note-'A')))/12.0));
}
int main(int argc, const char * argv[]) {
    // insert code here...
    int sampleRate = 44100;
    int nNumSeconds = 4;
    int nNumChannels = 1;
    int nNumSamples = sampleRate * nNumSeconds * nNumChannels;
    float *pData = new float[nNumSamples];
    float phase = 0.0;
    float frequency;
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex){
        if(nIndex/sampleRate < 2){
            frequency = Calculate_Frequency('A', 4);
        }
        else{
            frequency = Calculate_Frequency('A', 5);
        }
        pData[nIndex] = Sine_Oscilator(phase, frequency, (float)sampleRate);
    }
    std::cout << Calculate_Frequency('A',4) << "\n";
    std::cout << frequency << "\n";
    WriteWaveFile<int16>("outmono.wav",pData,nNumSamples,nNumChannels,sampleRate);
    delete[] pData;
    return 0;
}
