#include <stdio.h>
#include <vector>
#include <fstream>
#include <string>
#include <functional>
#include <regex>
#include <sstream>

#include "fmrs.hpp"

std::string stripExt(const std::string& s) 
{
	std::string::size_type i = s.rfind('.', s.length());

	if (i != std::string::npos) {
		return s.substr(0, i);
	}
	return s;
}

struct options
{
	bool spec;
	bool aasc;
	double start;
	double end;
} arg_option = { true, true, std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::quiet_NaN()};

bool GetOption(int argc, char *argv[])
{
	double *opt_target = NULL;
	for(int i = 2; i < argc; i++)
	{
		if(opt_target != NULL){
			std::string t(argv[i]);
			std::regex re(R"(((\d+)(h|hour))?((\d+)(m|min))?((\d+\.?\d*)(s|sec))?)");
			std::smatch match;

			if(regex_match(t, match, re)){
				double hour, min, sec;
				hour = min = sec = 0;

				std::stringstream(match[2]) >> hour;
				std::stringstream(match[5]) >> min;
				std::stringstream(match[8]) >> sec;

				*opt_target = hour * 3600 + min * 60 + sec;
				printf("%fsec\n", *opt_target);
			}
			opt_target = NULL;
			continue;
		}

		if(strcmp(argv[i],"--nospec") == 0){
			arg_option.spec = false;
			printf("no spec output\n");
			continue;
		}
		if(strcmp(argv[i],"--noaasc") == 0){
			arg_option.aasc = false;
			printf("no aasc output\n");
			continue;
		}
		if(strcmp(argv[i],"--start") == 0 || strcmp(argv[i],"-s") == 0){
			opt_target = &arg_option.start;
			printf("start time: ");
			continue;
		}
		if(strcmp(argv[i],"--end") == 0 || strcmp(argv[i],"-e") == 0){
			opt_target = &arg_option.end;
			printf("end time: ");
			continue;
		}
		printf("unknown option %s\n", argv[i]);
		return false;
	}
	if(opt_target != NULL){
		printf("missing argument\n");
		return false;
	}
	return true;
}

void PrintUsage(char *my)
{
	printf("usage: %s input.wav [--nospec] [--noaasc] [--start 30s] [--end 40s]\n", my);
}

int main(int argc, char *argv[])
{
	if(argc < 2){
		PrintUsage(argv[0]);
		return 0;
	}

	if(!GetOption(argc, argv)){
		PrintUsage(argv[0]);
		return 0;
	}

	auto wav = WaveData(argv[1]);
	if(wav.left.empty()){
		printf("wav file open error\n");
		return 1;
	}
	auto detector = DetectionUnits(wav.samplerate, 20, 20000);

	std::string basename(argv[1]);
	basename = stripExt(basename);
	std::ofstream specf;
	if(arg_option.spec)
		specf.open(basename+"_spec.bin", std::ios::binary);
	std::ofstream aascf;
	if(arg_option.aasc)
		aascf.open(basename+"_aasc.bin", std::ios::binary);

	detector.FixData(wav.left, wav.leftp);

	int count = 0;
	int s = (isnan(arg_option.start))? -1: wav.samplerate * arg_option.start;
	int e = (isnan(arg_option.end))? -1: wav.samplerate * arg_option.end;
	detector.AnalyzeData(
			wav.left, 
			wav.leftp,
			[&specf, &aascf, &count, s, e](std::vector<float> spec, std::vector<float> aasc)
			{
				count++;
				if(count < s)
					return true;

				if(specf){
					for(auto sp: spec){
						specf.write((char*)&sp, sizeof(sp));
					}
				}
				if(aascf){
					for(auto aa: aasc){
						aascf.write((char*)&aa, sizeof(aa));
					}
				}

				// break convert if return false
				if(e >= 0 && count >= e)
					return false;
				return true;
			});

	return 0;
}
