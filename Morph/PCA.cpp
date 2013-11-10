#include "PCA.h"

PCA::PCA() {
}

QString PCA::printNorm() {
	QString ret("Normalized:\n");
	for (int row = 0; row < src.rows; row++) {
		for (int col = 0; col < src.cols; col++) {
			ret += QString("%1").arg(norm.at<double>(row, col), 11, 'f', 8) + "\t";
		}
		ret += "\n";
	}
	ret += "\n";
	ret += "\n";

	return ret;
}

cv::Mat& PCA::getScore() {
	return score;
	/*
	QString ret("Score:\n");

	//cv::Mat score_var(1, src.cols, CV_64FC1);

	for (int i = 0; i < evectors.cols; i++) {
		QString str;
		str.setNum(i + 1);
		ret += "Comp. " + str + "\t";
	}
	ret += "\n";
	for (int i = 0; i < score.rows; i++) {
		for (int j = 0; j < score.cols; j++) {
			ret += QString("%1").arg(score.at<double>(i, j), 11, 'f', 8) + "\t";
		}
		ret += "\n";
	}
	ret += "\n";
	ret += "\n";

	return ret;
	*/
}

QString PCA::showImportance() {
	QString ret("Importance of components:\n");

	cv::Mat score_var(1, src.cols, CV_64FC1);

	// スコアの標準偏差の計算
	var(score, score_var);
	cv::sqrt(score_var, score_var);

    float sum=0.0f;
    for (int i = 0; i < evalues.rows; i++) {
        sum += ((double*)evalues.data)[i];
    }

	// Importance of components
    float proportion = 0.0f;
	ret += "                       ";
	for (int i = 0; i < src.cols; i++) {
		ret += "Comp." + QString("%1").arg(i+1) + "     ";
	}
	ret += "\n";
	ret += "Standard deviation     ";
	for (int i = 0; i < src.cols; i++) {
		ret += QString("%1").arg(score_var.at<double>(0, i), 0, 'f', 8) + " ";
	}
	ret += "\n";
	ret += "Proportion of Variance ";
	for (int i = 0; i < src.cols; i++) {
		ret += QString("%1").arg(((double*)evalues.data)[i] / sum, 0, 'f', 8) + " ";
	}
	ret += "\n";
	ret += "Cumulative Proportion  ";
    for (int i = 0; i < src.cols; i++) {
        proportion += ((double*)evalues.data)[i] / sum;
		ret += QString("%1").arg(proportion, 0, 'f', 8) + " ";
    }
	ret += "\n";

	return ret;
}

QString PCA::showLoadings() {
	QString ret("Loadings:\n");

	ret += "   ";
	for (int i = 0; i < evectors.cols; i++) {
		QString str;
		str.setNum(i + 1);
		ret += "Comp." + str + " ";
	}
	ret += "\n";
	for (int row = 0; row < evectors.rows; row++) {
		ret += "V" + QString("%1").arg(row + 1) + " ";
		for (int col = 0; col < evectors.cols; col++) {
			ret += QString("%1").arg(evectors.at<double>(row, col), 6, 'f', 3, ' ') + " ";
		}
		ret += "\n";
	}
	ret += "\n";

	return ret;
}

/**
 * Compute PCA and store the results.
 * 引数srcは行列で、各列は各変数のデータリスト。つまり、行の数は観測データの数を表し、列の数は、変数の数を表す。
 * normalizeフラグがtrueの場合は、入力データを正規化してPCAを実施する。つまり、不偏相関係数行列から主成分を求める。
 * フラグのfalseの場合は、入力データを正規化しないでPCAを実施する。つまり、不偏分散共分散行列から主成分を求める。
 */
void PCA::pca(cv::Mat src, bool normalize) {
	this->src = src;

	if (normalize) {
		// 各列の平均を計算 (-> mean)
		cv::Mat mean(1, src.cols, CV_64FC1);
		cv::reduce(src, mean, 0, CV_REDUCE_AVG);

		// 各列の分散を計算
		cv::Mat variance(1, src.cols, CV_64FC1);
		var(src, variance);

		// 各列の標準偏差を計算
		cv::sqrt(variance, variance);

		// データを標準化
		norm = cv::Mat(src.rows, src.cols, CV_64FC1);
		for (int row = 0; row < src.rows; row++) {
			for (int col = 0; col < src.cols; col++) {
				norm.at<double>(row, col) = (src.at<double>(row, col) - mean.at<double>(0, col)) / variance.at<double>(0, col);
			}
		}
	} else {
		src.copyTo(norm);
	}
 
	// PCA実施
    cv::PCA pca(norm, cv::Mat(), CV_PCA_DATA_AS_ROW, 0);
	evalues = pca.eigenvalues;
	cv::transpose(pca.eigenvectors, evectors);

	// スコアの計算
	score = cv::Mat(src.rows, src.cols, CV_64FC1);
	score = norm * evectors;

	// 固有値を保存
	evalues = pca.eigenvalues;
}

/**
 * Compute the variance for each column of the source matrix.
 *
 * @param src		the source matrix
 * @param variance	the result
 */
void PCA::var(cv::Mat &src, cv::Mat &variance) {
	cv::Mat mean(1, src.cols, CV_64FC1);
	cv::Mat src2(src.rows, src.cols, CV_64FC1);
	cv::reduce(src, mean, 0, CV_REDUCE_AVG);

	cv::pow(mean, 2.0, mean);
	cv::pow(src, 2.0, src2);
	cv::reduce(src2, variance, 0, CV_REDUCE_AVG);
	cv::subtract(variance, mean, variance);
	variance = variance * src.rows / (src.rows - 1);
}

