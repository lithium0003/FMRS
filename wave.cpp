#include "wave.hpp"

std::vector<float> WaveData::InterpolateSpline(const std::vector<float> &data){
	const double lambda = 0.5;
	const double mu = 0.5;

	std::vector<double> sig;
	double h = 1.0 / samplerate;
	sig.push_back(0);
	for(size_t i=1; i<data.size(); i++)
	{
		sig.push_back((data[i] - data[i-1])/h);
	}
	double a =2.0;
	double b = lambda;
	double c = mu;
	std::vector<double> d;
	d.push_back(0);
	for(size_t i=1; i<sig.size()-1; i++)
	{
		d.push_back(3*(sig[i+1] - sig[i])/h);
	}
	std::vector<double> hh;
	std::vector<double> kk;
	hh.push_back(d[0]/a);
	hh.push_back(d[1]/a);
	kk.push_back(b/a);
	kk.push_back(b/a);
	for(size_t i=2; i<d.size(); i++)
	{
		hh.push_back( (d[i] - c * hh[i-1])/(a - c*kk[i-1]) );
		kk.push_back( b/(a - c * kk[i-1]) );
	}
	std::vector<double> mm(data.size());
	mm[hh.size()-1] = hh[hh.size()-1];
	for(size_t i=hh.size()-2; i > 0; i--){
		mm[i] = hh[i] - kk[i] * mm[i+1];
	}
	mm[0] = 0;
	mm[data.size()-1] = 0;

	double dh = h / interp;
	std::vector<float> z(interp);
	for(size_t k = 1; k < data.size(); k++){
		int xi = interp * (k-1);
		int xf = interp * k;
		double m0 = mm[k-1] / (6*h);
		double m1 = mm[k] / (6*h);
		double f0 = (data[k-1] - mm[k-1] * h * h / 6) / h;
		double f1 = (data[k] - mm[k] * h * h / 6) / h;
		for(int i = 1; i <= interp; i++){
			double fi = xi * dh;
			double ff = xf * dh;
			double xii = (xi + i) * dh;
			z.push_back(m0 * std::pow((ff - xii), 3)
					+ m1 * std::pow((xii - fi), 3)
					+ f0 * (ff - xii)
					+ f1 * (xii - fi));
		}
	}
	return z;
}

	template<typename T>
void WaveData::LoadData(std::ifstream &fin, int channel)
{
	float b = (std::numeric_limits<T>::min() < 0)? 0: std::numeric_limits<T>::max() / 2.0;
	T ldata = b;
	T rdata = b;
	long count = 0;
	long readcount = header.dataChunk.chunkSize / channel / sizeof(T);

	while(!fin.eof() && count <= readcount){
		count++;
		if(channel == 1){
			left.push_back(ldata - b);

			fin.read((char *)&ldata, sizeof(T));
		}
		else if(channel == 2){
			left.push_back(ldata - b);
			right.push_back(rdata - b);

			fin.read((char *)&ldata, sizeof(T));
			fin.read((char *)&rdata, sizeof(T));
		}
	}

	printf("%ld samples readed\n", count);
	loaded_samples = count-1;
	samplerate = header.fmtChunk.samplesPerSec;

	printf("InterpolateSpline\n");
	if(channel == 1){
		leftp = InterpolateSpline(left);
	}
	else if (channel == 2){
		std::thread t1([this](void) {
				leftp = InterpolateSpline(left);
				});
		std::thread t2([this](void) {
				rightp = InterpolateSpline(right);
				});
		t1.join();
		t2.join();
	}
}

WaveData::WaveData(char *filename)
{
	loaded_samples = -1;
	std::ifstream fin(filename, std::ios::binary);
	if(fin){
		fin.read((char *)&header.riffChunk, sizeof(RIFF_CHUNK));
		fin.read((char *)&header.fmtChunk, sizeof(FMT_CHUNK));
		fin.seekg(header.fmtChunk.chunkSize + 8 - sizeof(FMT_CHUNK),std::ios_base::cur);
		fin.read((char *)&header.dataChunk, sizeof(DATA_CHUNK));
		if(strncmp(header.riffChunk.chunkID,"RIFF", 4) != 0){
			return;
		}
		if(strncmp(header.riffChunk.chunkFormType,"WAVE", 4) != 0){
			return;
		}
		if(strncmp(header.fmtChunk.chunkID,"fmt ", 4) != 0){
			return;
		}
		if(strncmp(header.dataChunk.chunkID,"data", 4) != 0){
			return;
		}
		if(header.fmtChunk.waveFormatType != 1){
			fprintf(stderr, "not PCM WAVE file\n");
			return;
		}
		header_endp = fin.tellg();
		printf("%ld bytes header end\n", header_endp);

		if(header.fmtChunk.bitsPerSample == 8){
			LoadData<uint8_t>(fin, header.fmtChunk.formatChannel);
		}
		else if(header.fmtChunk.bitsPerSample == 16){
			LoadData<int16_t>(fin, header.fmtChunk.formatChannel);
		}
	}
}
