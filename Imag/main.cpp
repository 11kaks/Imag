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

static void furier(const std::string imgPath);

/*
Blurring exsamples from:
https://docs.opencv.org/2.4.13.2/doc/tutorials/imgproc/gausian_median_blur_bilateral_filter/gausian_median_blur_bilateral_filter.html
*/

int showFilterAnimation(std::string caption, cv::Mat source, Filter::FILTER_TYPE ft, int minKl, int maxKl);

static void blur(int val, void* data);
static void binarize(int val, void * object);

struct Mats {
	cv::Mat src;
	cv::Mat dst;
};

int main() {
	
	std::string imgFolder	= "..//img//";
	std::string imgName		= "kyykky";
	std::string imgType = ".jpg";
	std::string imgPath = imgFolder + imgName + imgType;

	//showGrayscale(imgPath);

	cv::Mat src = cv::imread(imgPath, cv::IMREAD_COLOR);
	if(!src.data) {
		std::string err = " No image data found in " + imgPath + "! \n ";
		printf(err.c_str());
		cv::waitKey(0);
		exit(1);
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
	//cv::createTrackbar("Kernel size", windowName, &kl, maxKl, blur, &src);
	//blur(kl, &src);

	//furier(imgPath);

	// Morpholgy
	/*int size = 9;
	cv::Mat elem = cv::getStructuringElement(0, cv::Size(size, size));
	cv::morphologyEx(src, dst, cv::MORPH_OPEN, elem);
	cv::imshow(windowName, dst);*/

	int tresh = 0;
	int maxTresh = 255;

	// Binarization
	cv::Mat srcBW;
	cv::cvtColor(src, srcBW, cv::COLOR_RGB2GRAY);
	//cv::threshold(srcBW, dst, 100, 255, 0);
	//cv::imshow(windowName, dst);
	cv::createTrackbar("Kernel size", windowName, &tresh, maxTresh, binarize, &srcBW);
	binarize(tresh, &srcBW);
	// wait for keypress
	cv::waitKey(0);
}

static void binarize(int val, void * object){
	cv::Mat srcBW = *((cv::Mat*)object);

	if(val == 0) {
		cv::imshow(windowName, srcBW);
		return;
	}

	//std::cout << val << std::endl;

	cv::Mat binarized;
	srcBW.copyTo(binarized);
	cv::threshold(srcBW, binarized, val, 255, 1);

	cv::imshow(windowName, binarized);
}


/*
Code from tutorial:
https://docs.opencv.org/2.4/doc/tutorials/core/discrete_fourier_transform/discrete_fourier_transform.html
*/
static void furier(const std::string imgPath) {
	cv::Mat I = cv::imread(imgPath, cv::IMREAD_GRAYSCALE);
	if(!I.data) {
		std::string err = " No image data found in " + imgPath + "! \n ";
		printf(err.c_str());
	}
	/*
	The performance of a DFT is dependent of the image size. It tends to be 
	the fastest for image sizes that are multiple of the numbers two, three and five. 
	*/
	cv::Mat padded;                            //expand input image to optimal size
	int m = cv::getOptimalDFTSize(I.rows);
	int n = cv::getOptimalDFTSize(I.cols); // on the border add zero values
	cv::copyMakeBorder(I, padded, 0, m - I.rows, 0, n - I.cols, cv::BORDER_CONSTANT, cv::Scalar::all(0));

	// Put padded marix and equal-sized zero matrix into one multichanneled
	// matrix complexI
	cv::Mat planes[] = { cv::Mat_<float>(padded), cv::Mat::zeros(padded.size(), CV_32F) };
	cv::Mat complexI;
	cv::merge(planes, 2, complexI);
	// In-place DFT to complexI
	cv::dft(complexI, complexI);

	// => log(1 + sqrt(Re(DFT(I))^2 + Im(DFT(I))^2))
	cv::split(complexI, planes);                   // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))

	//cv::imshow("Real image", planes[0]);
	//cv::imshow("Complex image", planes[1]);

	// Replace real-valued image with magnitude of complex image.
	cv::magnitude(planes[0], planes[1], planes[0]);// planes[0] = magnitude
	// Switch to logarithmic scale too see grayscale instead of
	// black and white pixels.
	cv::Mat magI = planes[0];
	magI += cv::Scalar::all(1);
	cv::log(magI, magI);

	// crop the spectrum, if it has an odd number of rows or columns
	magI = magI(cv::Rect(0, 0, magI.cols & -2, magI.rows & -2));

	// rearrange the quadrants of Fourier image  so that the origin is at the image center
	int cx = magI.cols / 2;
	int cy = magI.rows / 2;

	cv::Mat q0(magI, cv::Rect(0, 0, cx, cy));   // Top-Left - Create a ROI per quadrant
	cv::Mat q1(magI, cv::Rect(cx, 0, cx, cy));  // Top-Right
	cv::Mat q2(magI, cv::Rect(0, cy, cx, cy));  // Bottom-Left
	cv::Mat q3(magI, cv::Rect(cx, cy, cx, cy)); // Bottom-Right

	cv::Mat tmp;                           // swap quadrants (Top-Left with Bottom-Right)
	q0.copyTo(tmp);
	q3.copyTo(q0);
	tmp.copyTo(q3);

	q1.copyTo(tmp);                    // swap quadrant (Top-Right with Bottom-Left)
	q2.copyTo(q1);
	tmp.copyTo(q2);

	cv::normalize(magI, magI, 0, 1, cv::NORM_MINMAX); // Transform the matrix with float values into a
											// viewable image form (float between values 0 and 1).

	cv::imshow("Input Image", I);    // Show the result
	cv::imshow("spectrum magnitude", magI);
	cv::waitKey();
}

/*
Blur trackbar callback function.
*/
static void blur(int val, void* object) {
	cv::Mat src = *((cv::Mat*)object);

	if(val == 0) {
		cv::imshow(windowName, src);
		return;
	}

	//std::cout << val << std::endl;
	
	cv::Mat blurred;
	src.copyTo(blurred);
	int err = Filter::filter(src, blurred, Filter::FILTER_TYPE::BI, val);
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