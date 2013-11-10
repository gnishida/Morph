#pragma once

#include <qstring.h>
#include <opencv/cv.h>
#include <opencv/highgui.h>

class PCA {
public:
	/** オリジナルの入力データ */
	cv::Mat src;

	/**
	 * 正規化された入力データ
	 *
	 * 不偏相関係数行列から主成分を求めた場合、入力データを正規化したデータが格納される。
	 * しかし、不偏分散共分散行列から主成分を求めた場合は、入力データのコピーが格納されるだけ。
	 */
	cv::Mat norm;

	/**
	 * 固有ベクトル
	 *
	 * PCAを実施後、固有ベクトルが格納される。
	 * 各列が、各固有ベクトルに相当する。一番左の列の固有ベクトルが、主成分である。
	 * 一番右の列の固有ベクトルが、最もマイナーな成分ということになる。
	 */
	cv::Mat evectors;

	/** 固有値 */
	cv::Mat evalues;

	/** PCAスコア */
	cv::Mat score;

public:
	PCA();
	
	void pca(cv::Mat src, bool normalize = true);

	//QString printMean();
	QString printNorm();
	cv::Mat& getScore();
	QString showImportance();
	QString showLoadings();

private:
	void pca();
	void var(cv::Mat &src, cv::Mat &variance);
};

