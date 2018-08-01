#include "analyze.hpp"

//コンストラクタ 初期設定
DetectionUnits::DetectionUnits(int SampleFq, double base_fq, double max_fq)
	:nz(SampleFq), omh_base(base_fq), omh_max(max_fq)
{
	printf("SampleFreq=%d\n", SampleFq);
	printf("minFreq=%f\n", base_fq);
	printf("maxFreq=%f\n", max_fq);

	nnz1=t*nz;
	h=1.0/nz;
	nnz2=interp*nnz1;
	dt1=1.0/nz;
	dt2=1.0/(nz*interp);
	gm=-1;

	omh.push_back(omh_base);
	om.push_back(omh[0]*2*M_PI);
	cutlen.push_back(nnz1/omh[0]);

	init_detection_unit();
	init_cutwaves();
}

// 信号検出ユニットの設定周波数計算
void DetectionUnits::init_detection_unit()
{
	int i=0;
	printf("min on unit %d=%fHz\n", i, omh[i]);
	while(omh[i++] < omh_max)
	{
		if(gm < 0){
			cutlen.push_back(cutlen[i-1]/f);
			omh.push_back((double)nnz1/cutlen[i]);
			om.push_back(omh[i]*2*M_PI);
			if(cutlen[i] < start_interp){
				gm = i+1;
				printf("switch to spline on unit %d=%fHz\n", i, omh[i]);
			}
		}
		else{
			if(gm == i){
				cutlen.push_back(cutlen[i-1]*interp/f);
			}
			else{
				cutlen.push_back(cutlen[i-1]/f);
			}
			omh.push_back((double)nnz2/cutlen[i]);
			om.push_back(omh[i]*2*M_PI);
		}
	}
	i--;
	printf("max on unit %d=%fHz\n", i, omh[i]);
}

//切り出し波の設定
void DetectionUnits::init_cutwaves()
{
	int j=0;
	for(auto cl: cutlen){
		ws.push_back(std::vector<double>(cl+1));
		wc.push_back(std::vector<double>(cl+1));
		if(j < gm){
			for(int i=1; i<=cl; i++){
				ws[j][i] = sin(om[j] * dt1 * i);
				wc[j][i] = cos(om[j] * dt1 * i);
			}
		}
		else {
			for(int i=1; i<=cl; i++){
				ws[j][i] = sin(om[j] * dt2 * i);
				wc[j][i] = cos(om[j] * dt2 * i);
			}
		}
		j++;
	}
}

//入力データの両境界のパティングを行う
void DetectionUnits::FixData(std::vector<float> &s, std::vector<float> &z)
{
	std::vector<float> pad1(cutlen[0]);
	s.insert(s.begin(), pad1.begin(), pad1.end());
	s.insert(s.end(), pad1.begin(), pad1.end());

	std::vector<float> pad_p(interp-1);
	z.insert(z.begin(), pad_p.begin(), pad_p.end());

	std::vector<float> pad2(cutlen[gm]);
	z.insert(z.begin(), pad2.begin(), pad2.end());
	z.insert(z.end(), pad2.begin(), pad2.end());
}

//解析を実施する
//各サンプル点毎にcallbackでspec, aascを返す
void DetectionUnits::AnalyzeData(
		const std::vector<float> &s,
		const std::vector<float> &z,
		const std::function<bool (const std::vector<float>&, const std::vector<float>&)> callback)
{
	const double mu = -0.006;
	const double nu = 0.44;

	std::vector<int> ih(cutlen);
	std::vector<double> sdds(cutlen.size());
	std::vector<double> sddc(cutlen.size());
	std::vector<double> sads(cutlen.size());
	std::vector<double> sadc(cutlen.size());
	std::vector<float> spec(cutlen.size());
	std::vector<double> aads(cutlen.size());
	std::vector<double> aadc(cutlen.size());
	std::vector<float> aasc(cutlen.size());

	int ml = cutlen[0];
	int mh = cutlen[gm];
	double totalsec = (double)z.size() / (nz * interp);
	for(size_t ic=1; ic<z.size(); ic++){
		double nowsec = (double)ic / (nz * interp);
		printf("%ld(%.2f%%) %.2fsec/%.2fsec\r", ic, (double)ic/z.size()*100, nowsec, totalsec);
		bool out_flag = (ic % interp == 1);
		if(out_flag){
			int ii = ic/interp+ml;
			for(int i = 0; i < gm; i++){
				int ik = ii - cutlen[i];
				if(ih[i] <= 0)
					ih[i] = cutlen[i];
				double sds = sdds[i] + ws[i][ih[i]]*s[ii] - ws[i][ih[i]]*s[ik];
				double sdc = sddc[i] + wc[i][ih[i]]*s[ii] - wc[i][ih[i]]*s[ik];
				ih[i]--;
				sdds[i] = sds;
				sddc[i] = sdc;
				sads[i] = 2.0 * sds / cutlen[i];
				sadc[i] = 2.0 * sdc / cutlen[i];
				double adss = sads[i] * ws[i][ih[i] + 1];
				double adsc = sads[i] * wc[i][ih[i] + 1];
				double adcc = sadc[i] * wc[i][ih[i] + 1];
				double adcs = sadc[i] * ws[i][ih[i] + 1];
				aadc[i] = adss + adcc;
				aads[i] = adsc - adcs;
				spec[i] = sqrt(aads[i] * aads[i] + aadc[i] * aadc[i]);
			}
		}
		int ii = ic+mh;
		for(size_t i = gm; i < cutlen.size(); i++){
			int ik = ii - cutlen[i];
			if(ih[i] <= 0)
				ih[i] = cutlen[i];
			double sds = sdds[i] + ws[i][ih[i]]*z[ii] - ws[i][ih[i]]*z[ik];
			double sdc = sddc[i] + wc[i][ih[i]]*z[ii] - wc[i][ih[i]]*z[ik];
			ih[i]--;
			sdds[i] = sds;
			sddc[i] = sdc;
			if(out_flag){
				sads[i] = 2.0 * sds / cutlen[i];
				sadc[i] = 2.0 * sdc / cutlen[i];
				double adss = sads[i] * ws[i][ih[i] + 1];
				double adsc = sads[i] * wc[i][ih[i] + 1];
				double adcc = sadc[i] * wc[i][ih[i] + 1];
				double adcs = sadc[i] * ws[i][ih[i] + 1];
				aadc[i] = adss + adcc;
				aads[i] = adsc - adcs;
				spec[i] = sqrt(aads[i] * aads[i] + aadc[i] * aadc[i]);
			}
		}
		if(out_flag){
			for(size_t j = 0; j < cutlen.size(); j++){
				aasc[j] = mu * aads[j] + nu * aadc[j];
			}

			if(callback){
				if(!callback(spec, aasc))
					break;
			}
		}
	}
	printf("\n");
}
