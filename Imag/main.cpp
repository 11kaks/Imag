#include <iostream>
#include <fstream>
#include <stdlib.h>     /* abs */

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "Filter.h"
#include "FrameUtils.h"

const char windowName[] = "Imag";
const char windowCutStart[] = "Cut";
const char windowCutEnd[] = "Cut End";

/*
Blurring exsamples from:
https://docs.opencv.org/2.4.13.2/doc/tutorials/imgproc/gausian_median_blur_bilateral_filter/gausian_median_blur_bilateral_filter.html
*/


static void blurCallback(int val, void* data);
static void binarizeCallback(int val, void * object);

static void detectCircles(cv::Mat &scr_gray, cv::Mat &scr_display);
static void detectLines(cv::Mat &scr_gray, cv::Mat &src_display);

static void loadVideo(std::string &vidPath);

/* Console input y/n. */
static bool askYesNo(const char* promt);
/* Console input for an integer. */
static int askInt(const char* promt);

std::string imgFolder = "..//img//";
std::string imgName = "kyykky";
std::string imgType = ".jpg";

std::string vidFolder = "..//vid//";
std::string vidName = "mak2";
std::string vidType = ".mp4";

std::string saveFolder = "saved//";

/* How long time a single frame takes. */
double timePerFrameMs = 33.;
int frameWidth = 120;
int frameHeight = 120;
int initialFrameCount = 0;
cv::Point2d frameCenter(1, 1);

int startFrame = 0;
int endFrame = 100;
int currFrame = 0;
int cutStartFrame = 0;
int cutEndFrame = 100;

/* Binarization treshold. */
int binVal = 1;
/* Blur*/
int blurVal = 3;

std::vector<cv::Mat> timeLine;


/* Default rotation 90 works with smartphone portrait videos. */
int rotation = 0;

int main() {


	/// Create Windows
	cv::namedWindow(windowName, 1);



	/// Reduce the noise so we avoid false circle detection
	//cv::GaussianBlur(srcBW, srcBW, cv::Size(9, 9), 2, 2);


	/*
	TODO:

	rotate - ok
	detect framerate - ok
	detect width, height and center point - ok
	scratch the beginning and the end times - ok
	go to first frame and:
		detect circles
		run search for the rest of the frames and follow the circle
		save circle's center points
	analyze stuff from center points§

	*/

	std::string vidPath = vidFolder + vidName + vidType;

	loadVideo(vidPath);

	cv::Mat frame = timeLine[cutStartFrame];
	cv::Mat frameGray;
	cv::cvtColor(frame, frameGray, cv::COLOR_RGB2GRAY);

	int kl = 3;
	int maxKl = 25;

	/*
	The last parameter, &src, is fucking important!!
	Never, ever remove it!
	Last example of:
	http://answers.opencv.org/question/91462/trackbar-pass-variable/
	*/
	//cv::createTrackbar("Kernel size", windowName, &kl, maxKl, blur, &src);
	//blur(kl, &frameGray);
	//cv::waitKey(0);

	int tresh = 1;
	int maxTresh = 255;

	std::cout << "Drag the slider until the barbell is clearly visible." << std::endl;

	// Binarization
	cv::createTrackbar("Kernel size", windowName, &tresh, maxTresh, binarizeCallback, &frameGray);
	binarizeCallback(tresh, &frameGray);

	// wait for keypress
	cv::waitKey(0);

	std::cout << "Binarization kernel size set to " << binVal << "." << std::endl;

	cv::waitKey(0);
}


static void trimStartCallbeck(int val, void* object) {

	if(val < timeLine.size() && val >= 0) {
		currFrame = val;
		cutStartFrame = val;
		cv::imshow(windowCutStart, timeLine[currFrame]);
	} else {
		std::cout << "timeLine not ok"<< std::endl;
	}
}
static void trimEndCallbeck(int val, void* object) {

	if(val < timeLine.size() && val >= 0) {
		currFrame = val;
		cutStartFrame = val;
		cv::imshow(windowCutStart, timeLine[currFrame]);
	} else {
		std::cout << "timeLine not ok" << std::endl;
	}
}


static void detectLines(cv::Mat &src_gray, cv::Mat &src_display) {

	cv::Mat display = src_display.clone();
	cv::Mat src = src_gray.clone();
	//cv::threshold(src, src, 155, 255, 1);
	Canny(src, src, 50, 200, 3);

	std::vector<cv::Vec4i> lines;

	HoughLinesP(src, lines, 1, CV_PI / 180, 50, 50, 10);

	if(lines.size() == 0) {
		std::cout << "No lines detected." << std::endl;
	} else {
		std::cout << "Detected " << lines.size() << " lines. " << std::endl;
		/// Draw the detected lines
		for(size_t i = 0; i < lines.size(); i++) {
			cv::Vec4i l = lines[i];
			cv::Point p1(l[0], l[1]);
			cv::Point p2(l[2], l[3]);
			if(abs(p1.x - p2.x) < 15) {
				std::cout << "Detected a vertical line. " << std::endl;
				cv::line(display, p1, p2, cv::Scalar(0, 0, 255), 3, cv::LINE_AA);
			}
		}
	}
	cv::namedWindow("Detected lines", cv::WINDOW_AUTOSIZE);
	cv::imshow("Detected lines", display);
}

static void detectCircles(cv::Mat &src_gray, cv::Mat &src_display) {

	std::vector<cv::Vec3f> circles;

	/// Apply the Hough Transform to find the circles
	cv::HoughCircles(src_gray, circles, cv::HOUGH_GRADIENT, 1, src_gray.rows / 8, 5, 30, 0, 100);

	cv::Mat display = src_display.clone();

	if(circles.size() == 0) {
		std::cout << "No circles detected." << std::endl;
	} else {
		std::cout << "Detected " << circles.size() << " circles. " << std::endl;
		/// Draw the circles detected
		for(size_t i = 0; i < circles.size(); i++) {
			cv::Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
			int radius = cvRound(circles[i][2]);
			// circle center
			circle(display, center, 3, cv::Scalar(0, 255, 0), -1, 8, 0);
			// circle outline
			circle(display, center, radius, cv::Scalar(0, 0, 255), 2, 8, 0);
		}
	}
	// Save
	//cv::imwrite(imgFolder + saveFolder + imgName + "_circles" + imgType, display);

	/// Show your results
	cv::namedWindow("Circles", cv::WINDOW_AUTOSIZE);
	cv::imshow("Circles", display);
}

static void binarizeCallback(int val, void * object) {
	
	cv::Mat srcGray = *((cv::Mat*)object);

	if(val == 0) {
		cv::imshow(windowName, srcGray);
		return;
	}

	binVal = val;
	/*
	cv::Mat blurred;
	srcGray.copyTo(blurred);
	int err = Filter::filter(srcGray, blurred, Filter::FILTER_TYPE::BI, binVal);
	*/
	cv::Mat binarized;
	srcGray.copyTo(binarized);
	cv::threshold(srcGray, binarized, val, 255, 0);
	
	cv::imshow(windowName, binarized);
}

/*
Blur trackbar callback function.
*/
static void blurCallback(int val, void* object) {
	cv::Mat src = *((cv::Mat*)object);

	if(val == 0) {
		cv::imshow(windowName, src);
		return;
	}

	blurVal = val;

	cv::Mat blurred;
	src.copyTo(blurred);
	int err = Filter::filter(src, blurred, Filter::FILTER_TYPE::BI, val);
	if(err != 0) {
		std::string errMsg = "Error while filtering.";
		printf(errMsg.c_str());
	}

	cv::imshow(windowName, blurred);
}

static bool askYesNo(const char* promt) {
	std::cout << promt << " (y/n)" << std::endl;
	std::string ansS;
	std::cin >> ansS;
	char ans = ansS[0];
	ans = std::toupper(ans);
	//std::cout << ans << std::endl;
	if(ans == 'Y') {
		return true;
	} else if(ans == 'N') {
		return false;
	} else {
		std::cout << "Answer not recogniced. Please answer yes or no." << std::endl;
		return askYesNo(promt);
	}
}

static int askInt(const char* promt) {
	std::cout << promt << " (y/n)" << std::endl;
	std::string ansS;
	std::cin >> ansS;
	try {
		int ans = std::stoi(ansS);
		return ans;
	} catch(std::invalid_argument e) {
		std::cout << "Invalid argument (" << ansS << "). Give an integer value with or without minus sign. " << std::endl;
		return askInt(promt);
	} catch(std::out_of_range e) {
		std::cout << "Value (" << ansS << ") out of integer range (" << INT_MIN << "," << INT_MAX << ")." << std::endl;
		return askInt(promt);
	}
}

/*
Load a video to the timeLine and ask for video cut.
*/
static void loadVideo(std::string &vidPath) {

	// TODO: siirrä mainiin tää sotku, jos vaikka binarisointi toimis

	cv::VideoCapture cap(vidPath);
	cv::Mat frame;

	if(!cap.isOpened()) {
		std::cerr << "Opening video file from " << vidPath << " failed." << std::endl;
	}

	std::cout << "Loading video... " << std::endl;

	// take the first frame for getting some basic info about the video
	cap.read(frame);

	timePerFrameMs = cap.get(cv::CAP_PROP_POS_MSEC);
	frameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	frameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	initialFrameCount = cap.get(cv::CAP_PROP_FRAME_COUNT);
	cutEndFrame = initialFrameCount;

	cv::namedWindow(windowCutStart, 1);
	//cv::namedWindow(windowCutEnd, 1);

	timeLine = std::vector<cv::Mat>(initialFrameCount);

	timeLine[0] = frame.clone();
	FrameUtils::rotate_90n(timeLine[0], timeLine[0], rotation);
	cv::imshow(windowCutStart, timeLine[0]);
	//cv::waitKey(0);
	for(size_t i = 1; i < timeLine.size(); ++i) {
		cap.read(frame);
		timeLine[i] = frame.clone();
		FrameUtils::rotate_90n(timeLine[i], timeLine[i], rotation);
	}
	cap.release();
	std::cout << "done" << std::endl;

	int startFrame = 0;
	int lastFrame = initialFrameCount - 1;

	/*
	cv::createTrackbar("Frame", windowCutStart, &startFrame, lastFrame, cropStart);
	cropStart(startFrame, 0);

	cv::imshow(windowName, timeLine[0]);
	while(cv::waitKey(1) < 0);

	startFrame = cutStartFrame;
	std::cout << "Cut start set to frame " << cutStartFrame << std::endl;

	cv::createTrackbar("Frame", windowCutEnd, &startFrame, lastFrame, cropEnd);
	cropEnd(startFrame, 0);

	cv::imshow(windowName, timeLine[cutStartFrame]);
	while(cv::waitKey(1) < 0);

	std::cout << "Cut end set to frame " << cutEndFrame << std::endl;
	*/

	// Debugging values. remove
	cutStartFrame = 0;
	cutEndFrame = initialFrameCount - 1;

	cv::destroyWindow(windowCutStart);
	//cv::destroyWindow(windowCutEnd);
}