#include <iostream>
#include <fstream>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "Filter.h"

const char windowName[] = "Imag";

// Function headers
int displayCaption(const cv::Mat img, std::string caption, int capTime);
int display(cv::Mat img, int delay);

void showGrayscale(std::string imgPath);

/*
Blurring exsamples from:
https://docs.opencv.org/2.4.13.2/doc/tutorials/imgproc/gausian_median_blur_bilateral_filter/gausian_median_blur_bilateral_filter.html
*/

int showFilterAnimation(std::string caption, cv::Mat source, Filter::FILTER_TYPE ft, int minKl, int maxKl);

static void blur(int val, void* data);

struct Mats {
	cv::Mat src;
	cv::Mat dst;
};

int main() {
	
	std::string imgFolder	= "..//img//";
	std::string imgName		= "cube1";
	std::string imgType = ".png";
	std::string imgPath = imgFolder + imgName + imgType;

	//showGrayscale(imgPath);

	cv::Mat src = cv::imread(imgPath, cv::IMREAD_COLOR);
	if(!src.data) {
		std::string err = " No image data found in " + imgPath + "! \n ";
		printf(err.c_str());
	}
	cv::Mat dst = src.clone();

	int kl = 1;
	int minKl = 1;
	int maxKl = 31;

	/*showFilterAnimation(std::string("Box blur"), src, Filter::FILTER_TYPE::BOX, minKl, maxKl);
	showFilterAnimation(std::string("Median blur"), src, Filter::FILTER_TYPE::MEDIAN, minKl, maxKl);
	showFilterAnimation(std::string("Gaussian blur"), src, Filter::FILTER_TYPE::GAUSSIAN, minKl, maxKl);
	showFilterAnimation(std::string("Bilateral blur"), src, Filter::FILTER_TYPE::BI, minKl, maxKl);*/
	
	/*Mats mats;
	mats.src = src;
	mats.dst = dst;*/

	cv::imshow(windowName, src);

	/// Create Windows
	cv::namedWindow(windowName, 1);

	/*
	The last parameter, &src, is fucking important!! 
	Never, ever remove it!
	Last example of:
	http://answers.opencv.org/question/91462/trackbar-pass-variable/
	*/
	cv::createTrackbar("Kernel size", windowName, &kl, maxKl, blur, &src);
	blur(kl, &src);

	// wait for keypress
	cv::waitKey(0);
}

/*
Blur trackbar callback function.
*/
static void blur(int val, void* object) {
	cv::Mat src = *((cv::Mat*)object);
	//std::cout << val << std::endl;
	
	cv::Mat blurred;
	src.copyTo(blurred);
	int err = Filter::filter(src, blurred, Filter::FILTER_TYPE::BOX, val);
	if(err != 0) {
		std::string errMsg = "Error while filtering.";
		printf(errMsg.c_str());
	}

	cv::imshow(windowName, blurred);
}

/*
	Do filter series.
*/
int showFilterAnimation(std::string caption, cv::Mat source, Filter::FILTER_TYPE ft, int minKl, int maxKl ) {

	// Time caption is shown.
	int DELAY_CAPTION = 1500;
	// Time that single blur state is shown before moving to next one.
	int DELAY_BLUR = 500;

	displayCaption(source, caption, DELAY_CAPTION);

	for(int i = minKl; i < maxKl; i = i + 2) {
		cv::Mat blurred; 
		source.copyTo(blurred);
		int err = Filter::filter(source, blurred, ft, i);
		if(err != 0) {
			std::string errMsg = "Error while filtering.";
			printf(errMsg.c_str());
		}

		if(display(blurred, DELAY_BLUR) != 0) { return 0; }
	}
	return 0;
}

/*
	Shows given caption in black background.
*/
int displayCaption(const cv::Mat img, std::string caption, int capTime) {
	cv::Mat black;
	black = cv::Mat::zeros(img.size(), img.type());
	putText(black, caption,
		cv::Point(black.cols / 4, black.rows / 2),
		cv::FONT_HERSHEY_COMPLEX, 1, cv::Scalar(255, 255, 255));

	imshow(windowName, black);
	int c = cv::waitKey(capTime);
	if(c >= 0) { return -1; }
	return 0;
}

/*
	Shows given image. 
*/
int display(cv::Mat img, int delay) {
	imshow(windowName, img);
	int c = cv::waitKey(delay);
	if(c >= 0) { return -1; }
	return 0;
}

void showGrayscale(std::string imgPath) {

	// Image as a matrix
	cv::Mat img = cv::imread(imgPath, cv::IMREAD_COLOR);
	if(!img.data) {
		std::string err = " No image data found in " + imgPath + "! \n ";
		printf(err.c_str());
	}
	// Grayscale copy of the image
	cv::Mat gs_img;
	cv::cvtColor(img, gs_img, cv::COLOR_BGR2GRAY);
	// Write the new grey scale image to a file
	//cv::imwrite(imgFolder + "gs_" + imgName, gs_img);

	// Show images
	cv::namedWindow("Grayscale", cv::WINDOW_AUTOSIZE);
	cv::imshow("Grayscale", gs_img);
}