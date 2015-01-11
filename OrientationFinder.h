/*
 * OrientationFinder.h
 *
 *  Created on: Jan 15, 2010
 *      Author: arnaud
 */

#ifndef ORIENTATIONFINDER_H_
#define ORIENTATIONFINDER_H_

#include "ImageAnalyzer.h"
#include "VirtualBody.h"

#include <pthread.h>

class OrientationFinder {
public:
	OrientationFinder();
	virtual ~OrientationFinder();

	void start(Human* human, int mode, const char* video_filename = NULL);
	void main_opencv_loop();

	int input_mode;

	// display
	CvCapture* capture;
	IplImage* frame_grabbed;
	string window1_name;
	string window2_name;
	int content_to_display;
#if SAVE_IMAGES
	int frame_counter;
	IplImage* rgb;
#endif

	// structure
	int* number_of_required_comps;

	// the guy
	Human* human;

	// the threads
	pthread_t thread_opengl, thread_opencv;
	int return_value_opengl, return_value_opencv;

	ImageAnalyzer* ia;

	void find_orientations(vector<vector<CvBox2D> >* boxes);

};

#endif /* ORIENTATIONFINDER_H_ */
