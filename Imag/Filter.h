#pragma once

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


class Filter
{
public:
	
	static const int MAX_KERNEL_LENGTH = 31;
	
	Filter() {};

	~Filter() {};

	enum FILTER_TYPE {
		BOX,
		MEDIAN,
		GAUSSIAN,
		BI
	};	

	/*
	Do some basic blurs to source image and put the result into
	dst.

	@param src Source image as a cv::Mat matrix.
	@param dst Return value. A cv::Mat matrix where the filtered image
	is placed in.
	@param ft Filtering type.
	@param kl Kernel length. Minimum 1, and maximum 31
	*/
	static int filter(const cv::Mat src, cv::Mat dst, FILTER_TYPE ft, int kl = 3) {
		if(kl < 1) return -1;
		int kern = kl;
		if(kern > MAX_KERNEL_LENGTH) {
			kern = MAX_KERNEL_LENGTH;
		}
		switch(ft) {
		case Filter::BOX:
			return doBoxBlur(src, dst, kern);
			break;
		case Filter::GAUSSIAN:
			return doGaussianBlur(src, dst, kern);
			break;
		case Filter::MEDIAN:
			return doMedianBlur(src, dst, kern);
			break;
		case Filter::BI:
			return doBilateralBlur(src, dst, kern);
			break;
		default:
			return -2;
			break;
		}
	}

	/*
		Simple box blur.
		Kernel anchored at the center.

		@param src Source image as a cv::Mat matrix.
		@param dst Return value. A cv::Mat matrix where the filtered image
					is placed in.
		@param kl Kernel length. Minimum 1, and maximum 31.
	*/
	static int doBoxBlur(const cv::Mat src, cv::Mat dst, int kl = 3) {
		blur(src, dst, cv::Size(kl, kl), cv::Point(-1, -1));
		return 0;
	}

	/*
	Gaussian blur with circle kernel.

	@param src Source image as a cv::Mat matrix.
	@param dst Return value. A cv::Mat matrix where the filtered image
	is placed in.
	@param kl Kernel radius. Minimum 1, and maximum 31.
	*/
	static int doGaussianBlur(const cv::Mat src, cv::Mat dst, int kl = 3) {
		int kern = kl * 2 - 1;
		GaussianBlur(src, dst, cv::Size(kern, kern), 0, 0);
		return 0;
	}

	static int doMedianBlur(const cv::Mat src, cv::Mat dst, int kl = 3) {
		int kern = kl * 2 - 1;
		medianBlur(src, dst, kern);
		return 0;
	}

	static int doBilateralBlur(const cv::Mat src, cv::Mat dst, int kl = 3) {
		bilateralFilter(src, dst, kl, kl * 2, kl / 2);
		return 0;
	}

	private:
};

