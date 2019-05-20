#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>


class FrameUtils
{
public:
	FrameUtils();
	~FrameUtils();

	/*
	From
	https://stackoverflow.com/questions/16265673/rotate-image-by-90-180-or-270-degrees
	*/
	static void rotate_90n(cv::Mat const &src, cv::Mat &dst, int angle) {
		CV_Assert(angle % 90 == 0 && angle <= 360 && angle >= -360);
		if(angle == 270 || angle == -90) {
			// Rotate clockwise 270 degrees
			cv::transpose(src, dst);
			cv::flip(dst, dst, 0);
		} else if(angle == 180 || angle == -180) {
			// Rotate clockwise 180 degrees
			cv::flip(src, dst, -1);
		} else if(angle == 90 || angle == -270) {
			// Rotate clockwise 90 degrees
			cv::transpose(src, dst);
			cv::flip(dst, dst, 1);
		} else if(angle == 360 || angle == 0 || angle == -360) {
			if(src.data != dst.data) {
				src.copyTo(dst);
			}
		}
	}

};

