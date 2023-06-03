//
//  main.cpp
//  WavetableSynth
//
//  Created by Cole Dixon on 5/30/23.
//

#include <cmath>
#include <map>
#include <list>
#include <string>
#include <iostream>
#include "WaveFile.hpp"
#define _USE_MATH_DEFINES
#define MAX_BAND 12
//type defs
typedef int int32;
typedef unsigned int uint32;
typedef short int16;
typedef unsigned short uint16;
typedef signed char int8;
typedef unsigned char uint8;
#define CLAMP(value,min,max) {if(value < min) { value = min; } else if(value > max) { value = max; }}

float Calculate_Frequency(std::string note, int oct);
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

float Saw_Oscilator(float &phase, float frequency, float sampleRate){
    phase += 2 * (float)M_PI * frequency/sampleRate;
    if(phase >= 2 * (float)M_PI){
        phase -= 2*(float)M_PI;
    }
    if(phase < 0){
        phase += 2 * (float)M_PI;
    }
    return (phase/M_PI) - 1;
}

float Saw_Oscilator_Band_Limited(float &phase, float frequency, float sampleRate){
    phase += 2 * (float)M_PI * frequency/sampleRate;
    if(phase >= 2 * (float)M_PI){
        phase -= 2*(float)M_PI;
    }
    if(phase < 0){
        phase += 2 * (float)M_PI;
    }
    float res = 0;
    for(int i = 1; i < MAX_BAND; i++){
        res += sin(phase * i);
    }
    return res;
}

float Square_Oscilator(float &phase, float frequency, float sampleRate){
    phase += 2 * (float)M_PI * frequency/sampleRate;
    if(phase >= 2 * (float)M_PI){
        phase -= 2*(float)M_PI;
    }
    if(phase < 0){
        phase += 2 * (float)M_PI;
    }
    return phase < M_PI ? 1.0 : -1.0;
}

float Square_Oscilator_Band_Limited(float &phase, float frequency, float sampleRate){
    phase += 2 * (float)M_PI * frequency/sampleRate;
    if(phase >= 2 * (float)M_PI){
        phase -= 2*(float)M_PI;
    }
    if(phase < 0){
        phase += 2 * (float)M_PI;
    }
    float res = 0;
    for(int i = 1; i < MAX_BAND; i++){
        res += sin(phase * (2*i-1));
    }
    return res;
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

//Note class to store pitch and volume
class Note{
    public:
        float freq;
        float amp;
        void setNote(std::string c){
            note = c;
            freq = Calculate_Frequency(note,oct);
            std::cout << freq << "\n";
        }
        void setOct(int i){
            oct = i;
            freq = Calculate_Frequency(note,oct);
        }
        Note();
        Note(std::string a);
        Note(std::string a, int x);
        Note(std::string a, int x, float i);
    private:
        std::string note;
        int oct;
};
Note::Note():note("C"),oct(4),amp(1.0f){freq = Calculate_Frequency(note,oct);}
Note::Note(std::string a):note(a),oct(4),amp(1.0f){freq = Calculate_Frequency(note,oct);}
Note::Note(std::string a, int x):note(a),oct(x),amp(1.0f){freq = Calculate_Frequency(note,oct);}
Note::Note(std::string a, int x, float i):note(a),oct(x),amp(i){freq = Calculate_Frequency(note,oct);}

float Calculate_Frequency(std::string note, int oct){
    std::map<std::string, int> noteMap;
    noteMap["A"] = 0;
    noteMap["A#"] = 1;
    noteMap["B"] = 2;
    noteMap["C"] = 3;
    noteMap["C#"] = 4;
    noteMap["D"] = 5;
    noteMap["D#"] = 6;
    noteMap["E"] = 7;
    noteMap["F"] = 8;
    noteMap["F#"] = 9;
    noteMap["G"] = 10;
    noteMap["G#"] = 11;
    //noteMap = {{"A",0},{"A#",1},{"B",2},{"C",3},{"C#",4},{"D",5},{"D#",6},{"E",7},{"F",8},{"F#",9},{"G",10},{"G#",11}};
    return (float)(440*pow(2.0,((double)((oct-5)*12+(noteMap[note])))/12.0));
}

class Wave{
    public:
        float Oscilator(float &phase, float frequency, float sampleRate){return 0;};
        float Oscilator_Band_Limited(float &phase, float frequency, float sampleRate){return 0;};
};
class SinWav:public Wave{
    public:
        float Oscilator(float &phase, float frequency, float sampleRate){
            return sin(phase);
        }
        float Oscilator_Band_Limited(float &phase, float frequency, float sampleRate){
            return sin(phase);
        }
};
class SawWav:public Wave{
    public:
        float Oscilator(float &phase, float frequency, float sampleRate){
            return (phase/M_PI) - 1;
        }
        float Oscilator_Band_Limited(float &phase, float frequency, float sampleRate){
            float res = 0;
            for(int i = 1; i < MAX_BAND; i++){
                res += sin(phase * i);
            }
            return res;
        }
};
class SqrWav:public Wave{
    public:
        float Oscilator(float &phase, float frequency, float sampleRate){
            return phase < M_PI ? 1.0 : -1.0;
        }
        float Oscilator_Band_Limited(float &phase, float frequency, float sampleRate){
            float res = 0;
            for(int i = 1; i < MAX_BAND; i++){
                res += sin(phase * (2*i-1));
            }
            return res;
        }
};
class Noise:public Wave{
    public:
};

class WaveTable{
    public:
        void setProbability(int row, float prob[]){
            
        }
        WaveTable makeWaveTable(){
            if(w==NULL){
                w = WaveTable();
                return w;
            }
            else{
                return w;
            }
        }
        void addWave(std::string s){
            if(size == 4){return;}
            if(s.compare("sine") == 0){
                waveTable.push_back(SinWav());
                size++;
                probMatrix[size][size] = 1.0f;
            }
            else if(s.compare("square")==0){
                waveTable.push_back(SqrWav());
                size++;
                probMatrix[size][size] = 1.0f;
            }
            else if(s.compare("saw")==0){
                waveTable.push_back(SawWav());
                size++;
                probMatrix[size][size] = 1.0f;
            }
            else if(s.compare("noise")==0){
                waveTable.push_back(Noise());
                size++;
                probMatrix[size][size] = 1.0f;
            }
        };
    private:
        static int size;
        WaveTable w;
        float probMatrix[4][4];
        std::list<Wave> waveTable;
        WaveTable(){};
};

int main(int argc, const char * argv[]) {
    // insert code here...
    int sampleRate = 44100;
    int nNumSeconds = 4;
    int nNumChannels = 1;
    int nNumSamples = sampleRate * nNumSeconds * nNumChannels;
    float *pData = new float[nNumSamples];
    float phase = 0.0;
    float frequency;

    Note seq[4];
    seq[0].setNote("C");
    seq[1].setNote("E");
    seq[2].setNote("G");
    seq[3].setNote("B");
    seq[3].setOct(5);

    int step;
    int sub = 4;
    for(int nIndex = 0; nIndex < nNumSamples; ++nIndex){
        step = (int) nIndex / sampleRate;
        pData[nIndex] = Square_Oscilator_Band_Limited(phase, seq[step].freq, (float)sampleRate) * seq[step].amp;
    }
    WriteWaveFile<int16>("outmono.wav",pData,nNumSamples,nNumChannels,sampleRate);
    delete[] pData;
    return 0;
}
