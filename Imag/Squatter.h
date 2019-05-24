#pragma once

#include <iostream>
#include <fstream>
#include <vector>

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

	// Initially position in pixels from the top of the window.
	std::vector<float> listPos;
	// Distance moved in meters during one frame.
	std::vector<float> listPosScaled;
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

	// Index of frames where the direction changes.
	std::vector<int> directionChanges;

	Squatter(std::vector<float> pos)
	:
		barbellMass(0.f),
		personHeight(1.7f),
		timeStep(0.033f),
		listPos(pos)
	{
		// Estimate of the length of the stroke in relation to the squatters height
		squatDistance = personHeight * 0.46f;
		listPosScaled = std::vector<float>(listPos.size(), 0.f);
		listVel = std::vector<float>(listPos.size(), 0.f);
		listAcc = std::vector<float>(listPos.size(), 0.f);
		listForce = std::vector<float>(listPos.size(), 0.f);
		listWork = std::vector<float>(listPos.size(), 0.f);
		listPower = std::vector<float>(listPos.size(), 0.f);
		listPosScaled[0] = 0.f;
		listVel[0] = 0.f;
		listAcc[0] = 0.f;
	}
	~Squatter() {}

	void analyze() {
		int highestPoint = -1;
		int lowestPoint = 50000;
		for(size_t i = 0; i < listPos.size(); ++i) {
			int val = listPos[i];
			if(val < lowestPoint) {
				lowestPoint = val;
			}
			if(val > highestPoint) {
				highestPoint = val;
			}
		}
		// Estimate how long distance in real world is one pixel in the video.
		float scale = -squatDistance / (highestPoint - lowestPoint);
		for(size_t i = 1; i < listPos.size(); ++i) {
			float deltaY = scale * (listPos[i] - listPos[i - 1]);
			listPosScaled[i] = (listPos[i] - listPos[0]) * scale;
			if(i == 1){
				listVel[i] = deltaY / timeStep;
			}
			// Exponential smoothing for velocity
			// https://en.wikipedia.org/wiki/Exponential_smoothing
			float alpha = 0.40f;
			listVel[i] = alpha*(deltaY / timeStep) + (1-alpha)*listVel[i-1];
			float acc = (listVel[i] - listVel[i - 1]) / timeStep;
			listAcc[i] = acc;
			// Force clamped to zero because we want to know about the 
			// upstroke of the squatter
			float force = listAcc[i] * barbellMass;
			if(force > 0.f) {
				listForce[i] = force;
			}
			float work = listForce[i] * listVel[i] * timeStep;
			if(work > 0.f) {
				listWork[i] = work;
			}
			float power = (listWork[i] - listWork[i-1]) / timeStep;
			if(power > 0.f) {
				listPower[i] = power;
			}
		}
	}

	/*
	Print analysis data in csv format to be copy-pasted to Excel.
	*/
	void printCsv() {
		char sep = ',';
		std::cout << "time [s]" << sep << "position [m]" << sep << "velocity [m/s]" << sep << "acceleration [m/s^2]" << sep << "force [N]" << sep << "work [J]" << sep << "power [W]" << std::endl;

		for(size_t i = 0; i < listPos.size(); ++i) {
			std::cout << timeStep * i << sep << listPosScaled[i] << sep << listVel[i] << sep << listAcc[i] << sep << listForce[i] << sep << listWork[i] << sep << listPower[i] << std::endl;
		}
	}
};

