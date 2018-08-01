#pragma once

#include "fmrs.hpp"

class DetectionUnits
{
	private:
		int nz;  // サンプリング周波数Fs

		const int t=16;          // 周期数T
		const double f = 1.01;  //設定周波数増加率σ
		//データスプライン補間開始閾値
		//切り出しのサンプル数がこの長さ以下になれば
		//スプライン補間を開始する
		const int start_interp=2000;
		double omh_base;    //最低設定周波数
		double omh_max;     //最高周波数の目安

		int nnz1;           //t周期数のデータ数
		int nnz2;           //補間後のt周期数のデータ数
		double h;           //サンプリング間隔Ts
		double dt1;         //低設定周波数の切り出し波のサンプリング間隔
		double dt2;         //高設定周波数の切り出し波のサンプリング間隔
		int fm;             //信号検出ユニットの数
		int gm;             //データのスプライン補間開始する信号検出ユニット番号
		std::vector<double> omh;   //設定周波数
		std::vector<double> om;
		std::vector<int> cutlen;  //設定周波数の切り出し波の長さ(データ数)

		std::vector<std::vector<double> > ws; //切り出し波ws
		std::vector<std::vector<double> > wc; //切り出し波wc

		// 信号検出ユニットの設定周波数計算
		void init_detection_unit();
		//切り出し波の設定
		void init_cutwaves();
	public:
		DetectionUnits(int SampleFq, double base_fq, double max_fq);
		void FixData(std::vector<float> &s, std::vector<float> &z);
		void AnalyzeData(
				const std::vector<float> &s,
				const std::vector<float> &z,
				const std::function<bool (const std::vector<float>&, const std::vector<float>&)> callback = nullptr);
};
