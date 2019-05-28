# Imag project

In this project I have implemented a C++ software that analyzes basic kinematics 
from a video file of a person doing a set of squats. Imag follows barbell plate as it moves 
between frames and records its position. Based on position and time changes, the following 
values are calculated: velocity, acceleration, force, work and power. The results are 
provided in CSV format for further analysis using e.g. Excel.

## Preprocessing

Trimming and rotating the video. Selection of region of interest.

## Processing

Object tracking with OpenCV's meanshift() method.

## Analyzing (Squatter class)

Squatter's height and barbell's mass are hard-coded into the program. The squat length, that is, 
the difference of barbell's highest and lowest position, is estimated based on squatter' height. 

In order to get any meaningful results, position difference of one pixel is converted into meters 
based on squat length. For the video used in testing, one pixel was approximately 5.6 mm.
The zero-level of position is set at the starting position of the barbell. 

Due to tracking errors and rather low granularity, the velocity calculated from position changes was very noisy, and
oscillated around zero too rapidly for any meaningful analysis (i.e. that causes huge accelerations). 
To counter that I smoothed the velocity with exponential smoothing.
As all other values are calculated from the velocity, the smoothing provided cleaner results throughout the process.
Calculating acceleration is a trivial task once the velocities are known.