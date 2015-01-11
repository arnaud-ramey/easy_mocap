/*
 * OrientationFinder.cpp
 *
 *  Created on: Jan 15, 2010
 *      Author: arnaud
 */

#include "OrientationFinder.h"

#include "BodyShower.cpp"

OrientationFinder::OrientationFinder() {
	/* init the analyzer */
	ia = new ImageAnalyzer();

}

OrientationFinder::~OrientationFinder() {
	delete ia;

	cvDestroyWindow(window1_name.c_str());
	cvReleaseCapture(&capture);
	delete[] number_of_required_comps;
	//cvReleaseImage(&frame_grabbed);
#if SAVE_IMAGES
	cvReleaseImage(&rgb);
#endif
}

void *start_opengl(void *ptr) {
	OrientationFinder* finder = (OrientationFinder*) ptr;
	start_bodyshower(finder->human);
	return NULL;
}

void *start_opencv(void *ptr) {
	OrientationFinder* finder = (OrientationFinder*) ptr;
	finder->main_opencv_loop();
	return NULL;
}

void OrientationFinder::start(Human* human, int mode,
		const char* video_filename /*= NULL*/) {
	/* init the windows */
	window1_name = "input";
	cvNamedWindow(window1_name.c_str(), CV_WINDOW_AUTOSIZE);

	window2_name = "output";
	cvNamedWindow(window2_name.c_str(), CV_WINDOW_AUTOSIZE);
	//cvMoveWindow(window2_name.c_str(), 700, 0);

	/* set the mode */
	assert(mode == MODE_CAMERA || mode == MODE_VIDEO);
	this->input_mode = mode;
	content_to_display = SHOW_FRAME_ILLUS;

	/* open the video */
	if (mode == MODE_VIDEO) {
		capture = cvCreateFileCapture(video_filename);
		if (!capture) {
			cout << "Impossible to open the video '" << video_filename
					<< "'. Exiting." << endl;
			exit(-1);
		}
	} else if (mode == MODE_CAMERA) {
		capture = cvCreateCameraCapture(0);
		if (!capture) {
			cout << "Impossible to open the camera. Exiting." << endl;
			exit(-1);
		}
	}
	//system("ls");

	/* set the human */
	this->human = human;

	/* find the number of required components */
	number_of_required_comps = new int[ia->nb_filters()];
	for (unsigned int i = 0; i < ia->nb_filters(); ++i)
		number_of_required_comps[i] = 0;

	if (TORSO_COLOR_INDEX >= 0)
		number_of_required_comps[TORSO_COLOR_INDEX]++;
	if (HEAD_COLOR_INDEX >= 0)
		number_of_required_comps[HEAD_COLOR_INDEX]++;
	if (ARM_LEFT_COLOR_INDEX >= 0)
		number_of_required_comps[ARM_LEFT_COLOR_INDEX]++;
	if (FOREARM_LEFT_COLOR_INDEX >= 0)
		number_of_required_comps[FOREARM_LEFT_COLOR_INDEX]++;
	if (ARM_RIGHT_COLOR_INDEX >= 0)
		number_of_required_comps[ARM_RIGHT_COLOR_INDEX]++;
	if (FOREARM_RIGHT_COLOR_INDEX >= 0)
		number_of_required_comps[FOREARM_RIGHT_COLOR_INDEX]++;
	if (SHIN_LEFT_COLOR_INDEX >= 0)
		number_of_required_comps[SHIN_LEFT_COLOR_INDEX]++;
	if (LEG_LEFT_COLOR_INDEX >= 0)
		number_of_required_comps[LEG_LEFT_COLOR_INDEX]++;
	if (SHIN_RIGHT_COLOR_INDEX >= 0)
		number_of_required_comps[SHIN_RIGHT_COLOR_INDEX]++;
	if (LEG_RIGHT_COLOR_INDEX >= 0)
		number_of_required_comps[LEG_RIGHT_COLOR_INDEX]++;

	/* create the threads */
	return_value_opengl = pthread_create(&thread_opengl, NULL, start_opengl,
			(void*) this);
	//	return_value_opencv = pthread_create(&thread_opencv, NULL, start_opencv,
	//			(void*) this);
	main_opencv_loop();

}

void OrientationFinder::main_opencv_loop() {
	/////////////// main loop
	while (1) {
		double timer = (double) cvGetTickCount();
		double freq = (double) cvGetTickFrequency() * 1000.;

		/* get a frame */
		frame_grabbed = cvQueryFrame(capture);
		//		frame_grabbed = cvLoadImage("inputs/input.png", CV_LOAD_IMAGE_COLOR);
		//		frame_grabbed = cvLoadImage("inputs/metro.png", CV_LOAD_IMAGE_COLOR);
		if (!frame_grabbed) {
			// loop if it is a video
			if (input_mode == MODE_VIDEO) {
				cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, 0);
				continue;
			} else
				break;
		}
		cvShowImage(window1_name.c_str(), frame_grabbed);
		cout << endl << "New frame !" << endl;

#if SAVE_IMAGES
		//		cvSaveImage("export/opencv_in.png", frame_grabbed);
		//
		//		/* save hue to rgb */
		//		if (!rgb)
		//			rgb = cvCloneImage(frame_grabbed);
		//		ImageUtils::extractHue_and_convertRGB(frame_grabbed, rgb);
		//		cvShowImage(window2_name.c_str(), rgb);
		//		cvSaveImage("export/opencv_rgb.png", rgb);
#endif

		/* analyze the image */
		vector<vector<CvBox2D> > boxes;
		ia->analyze_frame(frame_grabbed, number_of_required_comps, &boxes);

		/* compute the orientations */
		find_orientations(&boxes);
		cout << "Time after find_orientation :" << ((double) cvGetTickCount()
				- timer) / freq << endl;

		if (content_to_display == SHOW_FRAME_FILTERED)
			cvShowImage(window2_name.c_str(), ia->frame_filtered);
		else if (content_to_display == SHOW_HUE_COMPONENT)
			cvShowImage(window2_name.c_str(), ia->hsv_filter.h_layer);
		else if (content_to_display == SHOW_FRAME_ILLUS)
			cvShowImage(window2_name.c_str(), ia->frame_illus);
		cout << "Time after image display :" << ((double) cvGetTickCount()
				- timer) / freq << endl;

#if SAVE_IMAGES
		char filtered_name[64];
		sprintf(filtered_name, "export/opencv_filtered_%06d.jpg", frame_counter);
		cvSaveImage(filtered_name, ia->frame_filtered);

		char illus_name[64];
		sprintf(illus_name, "export/opencv_illus_%06d.jpg", frame_counter);
		cvSaveImage(illus_name, ia->frame_illus);
		++frame_counter;
#endif

		/* don't wait for key */
		char keyPressed = cvWaitKey(30) & 255;
		if (keyPressed == 27)
			exit(0);
		if (keyPressed == ' ')
			switch (content_to_display) {
			case SHOW_HUE_COMPONENT:
				content_to_display = SHOW_FRAME_FILTERED;
				break;
			case SHOW_FRAME_FILTERED:
				content_to_display = SHOW_FRAME_ILLUS;
				break;
			default:
				content_to_display = SHOW_HUE_COMPONENT;
				break;
			}

		/* wait for key */
		//		char keyPressed = -1;
		//		while ((int) keyPressed == -1) // wait for a key to be pressed
		//			keyPressed = (cvWaitKey(33) & 255);
		//		if (keyPressed == 27)
		//			break;

		cout << "Time end loop :" << ((double) cvGetTickCount() - timer) / freq
				<< endl;
	}
	/////////////// end of main loop
}

void OrientationFinder::find_orientations(vector<vector<CvBox2D> >* boxes) {
	//	cout << "OrientationFinder::find_orientations()" << endl;
	ImageUtils::reset_timer();

	CvBox2D *torso = NULL, *head = NULL;
	CvBox2D *arm_left = NULL, *forearm_left = NULL, *arm_right = NULL,
			*forearm_right = NULL;
	CvBox2D *shin_left = NULL, *leg_left = NULL, *leg_right = NULL,
			*shin_right = NULL;

	// make the affectations
	vector<vector<CvBox2D> >::iterator current_box_list = boxes->begin();
	for (int filter = 0; filter < (int) ia->nb_filters(); ++filter) {
		vector<CvBox2D>::iterator current_box = current_box_list->begin();
		if (current_box != current_box_list->end() && TORSO_COLOR_INDEX
				== filter)
			torso = &(*current_box++);

		if (current_box != current_box_list->end() && HEAD_COLOR_INDEX
				== filter)
			head = &(*current_box++);

		if (current_box != current_box_list->end() && ARM_LEFT_COLOR_INDEX
				== filter)
			arm_left = &(*current_box++);

		if (current_box != current_box_list->end() && FOREARM_LEFT_COLOR_INDEX
				== filter)
			forearm_left = &(*current_box++);

		if (current_box != current_box_list->end() && ARM_RIGHT_COLOR_INDEX
				== filter)
			arm_right = &(*current_box++);

		if (current_box != current_box_list->end() && FOREARM_RIGHT_COLOR_INDEX
				== filter)
			forearm_right = &(*current_box++);

		if (current_box != current_box_list->end() && SHIN_LEFT_COLOR_INDEX
				== filter)
			shin_left = &(*current_box++);

		if (current_box != current_box_list->end() && LEG_LEFT_COLOR_INDEX
				== filter)
			leg_left = &(*current_box++);

		if (current_box != current_box_list->end() && SHIN_RIGHT_COLOR_INDEX
				== filter)
			shin_right = &(*current_box++);

		if (current_box != current_box_list->end() && LEG_RIGHT_COLOR_INDEX
				== filter)
			leg_right = &(*current_box++);
		++current_box_list;
	} // end loop filter

	// draw stuff
	CvScalar color = cvScalar(255, 255, 255);
	if (torso && arm_left)
		cvLine(ia->frame_illus, cvPointFrom32f(torso->center), cvPointFrom32f(
				arm_left->center), color, 2);
	if (arm_left && forearm_left)
		cvLine(ia->frame_illus, cvPointFrom32f(arm_left->center),
				cvPointFrom32f(forearm_left->center), color, 2);
	if (torso && arm_right)
		cvLine(ia->frame_illus, cvPointFrom32f(torso->center), cvPointFrom32f(
				arm_right->center), color, 2);
	if (arm_right && forearm_right)
		cvLine(ia->frame_illus, cvPointFrom32f(arm_right->center),
				cvPointFrom32f(forearm_right->center), color, 2);

	// find angle for the left arm
	if (torso && arm_left && body) {
		double arm_left_angle = -atan2(arm_left->center.y - torso->center.y,
				arm_left->center.x - torso->center.x);
		body->arm_left()->angle = arm_left_angle;
		// find angle for the forearm
		if (forearm_left) {
			double forearm_left_angle = -atan2(forearm_left->center.y
					- arm_left->center.y, forearm_left->center.x
					- arm_left->center.x);
			body->forearm_left()->angle = forearm_left_angle - arm_left_angle;
		}
	}

	// find angle for the right arm
	if (torso && arm_right && body) {
		double arm_right_angle = PI - atan2(arm_right->center.y
				- torso->center.y, arm_right->center.x - torso->center.x);
		body->arm_right()->angle = arm_right_angle;
		// find angle for the forearm
		if (forearm_right) {
			double forearm_right_angle = PI - atan2(forearm_right->center.y
					- arm_right->center.y, forearm_right->center.x
					- arm_right->center.x);
			body->forearm_right()->angle = forearm_right_angle
					- arm_right_angle;
		}
	}

	//body->forearml()->angle = PI / 8 + PI / 8 * cos(body->t / 7);

	cout << "Time after computing the orientation:" << ImageUtils::get_timer()
			<< endl;
}
