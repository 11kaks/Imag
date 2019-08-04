#include <iostream>
#include <fstream>
#include <stdlib.h>     /* abs */

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

#include "FrameUtils.h"
#include "Squatter.h"

const char windowName[] = "Imag";

/*
Load a video to the timeLine and ask for video cut.
Sets basic values like framerate etc.
*/
static void loadAndPreprocess(std::string &vidPath);
/* Console input y/n. */
static bool askYesNo(const char* promt);
/* Console input for an integer. */
static int askInt(const char* promt);

static void trimStartCallbeck(int val, void* object);
static void trimEndCallbeck(int val, void* object);
static void timeLineDragCallback(int val, void* object);

std::string vidFolder = "..//vid//";
std::string vidName = "etu_40kg";
std::string vidType = ".mp4";
float w = 40;


/* How long time a single frame takes. */
int frameRate = 30;
float frameTimeS = 33.f;
int frameWidth = 120;
int frameHeight = 120;
int initialFrameCount = 0;
int currFrameIdx = 0;
int trimStartFrameIdx = 0;
int trimEndFrameIdx = 100;
int cutLength = 0;
/* Region of interest. */
cv::Rect roi;
cv::Scalar circleColor(150, 10, 10);
/* All the frames in the video. */
std::vector<cv::Mat> timeLine;
/* Found center points for visualization. */
std::vector<cv::Point> centerPoints;
/* Center point y-koordinate for analysis. */
std::vector<int> listPosY;
/* Default rotation 90 works with smartphone portrait videos. */
int rotation = 90;

int main() {

	std::string vidPath = vidFolder + vidName + vidType;

	loadAndPreprocess(vidPath);

	// Adapted from tutorial https://docs.opencv.org/3.4/d7/d00/tutorial_meanshift.html
	cv::Mat frame, roiFrame, hsvRoi, mask;
	// take first frame of the video
	frame = timeLine[trimStartFrameIdx];
	// setup initial location of window
	roi = cv::selectROI(windowName, frame);

	std::cout << "Processing the video. Don't press any key, please..." << std::endl;

	// set up the ROI for tracking
	roiFrame = frame(roi);
	// Convert to HSV colorspace
	cv::cvtColor(roiFrame, hsvRoi, cv::COLOR_BGR2HSV);
	/*
	inRange() is basically bandpass thresholding function.
	Hue: any
	Saturation: from 60 to 255 to exclude too light pixels
	Value: from 32 to 255 to exclude some of the darkest pixels
	*/
	cv::inRange(hsvRoi, cv::Scalar(0, 60, 32), cv::Scalar(180, 255, 255), mask);
	// Hue range from 0 to 180
	float range_[] = { 0, 180 };
	const float* range[] = { range_ };
	cv::Mat roiHist;
	// Separate histogram bin for every hue value.
	int histSize[] = { 180 };
	int channels[] = { 0 };
	cv::calcHist(&hsvRoi, 1, channels, mask, roiHist, 1, histSize, range);
	// Normalize from 0 to 255
	cv::normalize(roiHist, roiHist, 0, 255, cv::NORM_MINMAX);

	// Setup the termination criteria, either 10 iteration or move by atleast 1 pt
	cv::TermCriteria term_crit(cv::TermCriteria::EPS | cv::TermCriteria::COUNT, 10, 1);

	for(int i = 0; i < cutLength; ++i) {
		cv::Mat hsv, dst;
		frame = timeLine[i + trimStartFrameIdx];
		if(frame.empty())
			break;
		cv::cvtColor(frame, hsv, cv::COLOR_BGR2HSV);
		cv::calcBackProject(&hsv, 1, channels, roiHist, dst, range);
		// apply meanshift to get the new location
		cv::meanShift(dst, roi, term_crit);
		// Draw it on image
		cv::rectangle(frame, roi, 255, 2);

		cv::Point center(cvRound(roi.x + (roi.width / 2)), cvRound(roi.y + (roi.height / 2)));
		centerPoints[i] = center;
		listPosY[i] = center.y;
		circle(frame, center, 3, circleColor, -1, 8, 0);
	}

	std::cout << "done" << std::endl;

	// Allow user to drag the timeline of the analyzed video
	// This is shown to the user while analysis is running.
	cv::namedWindow(windowName, 1);
	cv::createTrackbar("Frame", windowName, 0, cutLength - 1, timeLineDragCallback);
	timeLineDragCallback(0, 0);

	std::cout << "Analyzing..." << std::endl;

	Squatter s(listPosY);
	s.barbellMass = w;
	s.timeStep = frameTimeS;
	s.analyze();
	s.printCsv();

	std::cout << "done" << std::endl;

	cv::waitKey(0);
}

static void loadAndPreprocess(std::string &vidPath) {

	cv::VideoCapture cap(vidPath);
	cv::Mat frame;

	if(!cap.isOpened()) {
		std::cerr << "Opening video file from " << vidPath << " failed." << std::endl;
		return;
	}

	frameRate = cap.get(cv::CAP_PROP_FPS);
	frameTimeS = 1.f / (float)frameRate;
	frameWidth = cap.get(cv::CAP_PROP_FRAME_WIDTH);
	frameHeight = cap.get(cv::CAP_PROP_FRAME_HEIGHT);
	initialFrameCount = cap.get(cv::CAP_PROP_FRAME_COUNT);
	trimEndFrameIdx = initialFrameCount - 1;
	roi.width = frameWidth;
	roi.height = frameHeight;
	timeLine = std::vector<cv::Mat>(initialFrameCount);

	std::cout << "Loading video... " << std::endl;
	for(size_t i = 0; i < timeLine.size(); ++i) {
		cap.read(frame);
		timeLine[i] = frame.clone();
		FrameUtils::rotate_90n(timeLine[i], timeLine[i], rotation);
	}
	cap.release();
	std::cout << "done" << std::endl;

	std::cout << "Video frame rate: " << frameRate << "(" << frameTimeS << "/s)" << std::endl;
	std::cout << "Video frame height: " << frameHeight << " px" << std::endl;

	cv::destroyWindow(windowName);
	std::cout << "Drag the timeline just before the beginning of a squat. Press any key to confirm." << std::endl;
	cv::namedWindow(windowName, 1);
	cv::createTrackbar("Trim start", windowName, 0, trimEndFrameIdx, trimStartCallbeck);
	trimStartCallbeck(0, 0);
	cv::waitKey(0);
	cv::destroyWindow(windowName);
	std::cout << "Cut start set to frame " << trimStartFrameIdx << std::endl;

	std::cout << "Drag the timeline just after the end of a squat. Press any key to confirm." << std::endl;
	cv::namedWindow(windowName, 1);
	cv::createTrackbar("Trim end", windowName, 0, trimEndFrameIdx-trimStartFrameIdx, trimEndCallbeck);
	trimEndCallbeck(0, 0);
	cv::waitKey(0);
	cv::destroyWindow(windowName);
	std::cout << "Cut end set to frame " << trimEndFrameIdx << std::endl;

	// Debugging values to hard-code the above.
	/*cutStartFrame = 0;
	cutEndFrame = initialFrameCount - 1;*/

	cutLength = trimEndFrameIdx - trimStartFrameIdx;
	std::cout << "Cut length " << cutLength << " frames" << std::endl;

	centerPoints = std::vector<cv::Point>(cutLength);
	listPosY = std::vector<int>(cutLength);
}

static void timeLineDragCallback(int val, void* object) {
	// Trackbar is zero-based
	if(val < cutLength) {
		currFrameIdx = val + trimStartFrameIdx;
		cv::imshow(windowName, timeLine[currFrameIdx]);
	} else {
		std::cout << "timeLine not ok" << std::endl;
	}
}

static void trimStartCallbeck(int val, void* object) {
	if(val < timeLine.size()) {
		currFrameIdx = val;
		trimStartFrameIdx = val;
		cv::imshow(windowName, timeLine[currFrameIdx]);
	} else {
		std::cout << "timeLine not ok" << std::endl;
	}
}

static void trimEndCallbeck(int val, void* object) {
	if(val + trimStartFrameIdx < timeLine.size()) {
		currFrameIdx = val + trimStartFrameIdx;
		trimEndFrameIdx = currFrameIdx;
		cv::imshow(windowName, timeLine[currFrameIdx]);
	} else {
		std::cout << "timeLine not ok" << std::endl;
	}
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