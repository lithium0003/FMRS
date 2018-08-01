#pragma once

#include "fmrs.hpp"

#pragma pack(push, 1)
typedef struct
{
	char chunkID[4];
	uint32_t chunkSize;
	char chunkFormType[4];
} RIFF_CHUNK;
// RIFFチャンクの定義

typedef struct
{
	char chunkID[4];
	uint32_t chunkSize;
	uint16_t waveFormatType;
	uint16_t formatChannel;
	uint32_t samplesPerSec;
	uint32_t bytesPerSec;
	uint16_t blockSize;
	uint16_t bitsPerSample;
} FMT_CHUNK;
// fmtチャンクの定義

typedef struct
{
	char chunkID[4];
	uint32_t chunkSize;
} DATA_CHUNK;
// dataチャンクの定義

typedef struct
{
	RIFF_CHUNK riffChunk;
	FMT_CHUNK fmtChunk;
	DATA_CHUNK dataChunk;
} WAVE_FORMAT_HEAD;
#pragma pack(pop)

class WaveData
{
	private:
		WAVE_FORMAT_HEAD header;
		long header_endp;

		std::vector<float> InterpolateSpline(const std::vector<float> &data);
		template<typename T> void LoadData(std::ifstream &fin, int channel);
	public:
		std::vector<float> left;
		std::vector<float> leftp;
		std::vector<float> right;
		std::vector<float> rightp;
		long loaded_samples;
		uint32_t samplerate;	

		WaveData(char *filename);
};
