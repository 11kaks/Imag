#pragma once

#include <iostream>
#include <fstream>
#include <vector>

/*
Squat analysis class. 

Computes velocities, forces etc. from a given pointlist.
*/
class Squatter
{

public:
	// Weight of the bar and barbells.
	float barbellMass;
	// Height of the squatter.
	float personHeight;
	// How many meters does the weigh move during half repetition.
	float squatDistance;
	// Elapsed time between points. In seconds.
	float timeStep;

	// Position in pixels from the top of the window.
	std::vector<int> pixelPosition;
	// Position in meters relative to the position in the first frame (which pos is 0).
	std::vector<float> listRelativeScaledPos;
	// Velocity
	std::vector<float> listVel;
	// Acceleration
	std::vector<float> listAcc;
	// Force applied to the barbell.
	std::vector<float> listForce;
	// Work done to the barbell.
	std::vector<float> listWork;
	// Power of the work.
	std::vector<float> listPower;
	
	Squatter(std::vector<int> pos)
	:
		barbellMass(0.f),
		personHeight(1.7f),
		timeStep(0.033f),
		pixelPosition(pos)
	{
		// Estimate of the length of the stroke in relation to the squatters height
		squatDistance = personHeight * 0.46f;
		listRelativeScaledPos = std::vector<float>(pixelPosition.size(), 0.f);
		listVel = std::vector<float>(pixelPosition.size(), 0.f);
		listAcc = std::vector<float>(pixelPosition.size(), 0.f);
		listForce = std::vector<float>(pixelPosition.size(), 0.f);
		listWork = std::vector<float>(pixelPosition.size(), 0.f);
		listPower = std::vector<float>(pixelPosition.size(), 0.f);
		listRelativeScaledPos[0] = 0.f;
		listVel[0] = 0.f;
		listAcc[0] = 0.f;
	}
	~Squatter() {}

	/*
	Calculate things for all frames. After this we are ready for printing.
	*/
	void analyze() {
		int highestPoint = -1;
		int lowestPoint = 50000;
		for(size_t i = 0; i < pixelPosition.size(); ++i) {
			int val = pixelPosition[i];
			if(val < lowestPoint) {
				lowestPoint = val;
			}
			if(val > highestPoint) {
				highestPoint = val;
			}
		}
		// Estimate how long distance in real world is one pixel in the video.
		float scale = -squatDistance / (highestPoint - lowestPoint);
		std::cout << "One pixel estimated to be " << scale * -1000 << " mm, And squat distance " << squatDistance << " m." << std::endl;
		for(size_t i = 1; i < pixelPosition.size(); ++i) {
			float deltaY = scale * (pixelPosition[i] - pixelPosition[i - 1]);
			listRelativeScaledPos[i] = (pixelPosition[i] - pixelPosition[0]) * scale;
			listVel[i] = (deltaY / timeStep);
			listAcc[i] = (listVel[i] - listVel[i - 1]) / timeStep;
			float force = listAcc[i] * barbellMass;
			listForce[i] = force;
			float work = listForce[i] * deltaY;
			listWork[i] = work;
			float power = (listWork[i] - listWork[i-1]) / timeStep;
			listPower[i] = power;
		}
	}

	/*
	Print analysis data in csv format to be copy-pasted to Excel.
	*/
	void printCsv() {
		char sep = ',';
		std::cout << "time [s]" << sep << "position [m]" << sep << "velocity [m/s]" << sep << "acceleration [m/s^2]" << sep << "force [N]" << sep << "work [J]" << sep << "power [W]" << std::endl;

		for(size_t i = 0; i < pixelPosition.size(); ++i) {
			std::cout << timeStep * i << sep << listRelativeScaledPos[i] << sep << listVel[i] << sep << listAcc[i] << sep << listForce[i] << sep << listWork[i] << sep << listPower[i] << std::endl;
		}
	}
};

