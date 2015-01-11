/*
 * test.cpp
 *
 *  Created on: Jan 12, 2010
 *      Author: arnaud
 */
#include "VirtualBody.h"
#include "OrientationFinder.h"

void test_member() {
	VirtualBody* vb = new VirtualBody();
	vb->position = cvPoint3D32f(0, 0, 0);

	Member* m1 = new Member(NULL, vb);
	m1->abscissae_of_fixation = 0;
	m1->length = 1;
	m1->angle = -PI / 4;

	Member* m2 = new Member(m1, vb);
	m2->abscissae_of_fixation = m1->length;
	m2->length = 2;
	m2->angle = PI / 4;

	Member* m3 = new Member(m2, vb);
	m3->abscissae_of_fixation = m2->length;
	m3->length = 2;
	m3->angle = 0;

	display_point("x1", m1->get_x_axis());
	display_point("p1", m1->get_absolute_position());
	display_point("x2", m2->get_x_axis());
	display_point("p2", m2->get_absolute_position());
	display_point("x3", m3->get_x_axis());
	display_point("p3", m3->get_absolute_position());

	delete vb;
	delete m1;
	delete m2;
	delete m3;
}

void test_opengl_body() {
	Human man;
	man.opengl_make_dancing = true;
	start_bodyshower(&man, 800, 600);
}

void test_gingy() {
	Gingy gingy;
	gingy.opengl_make_dancing = true;
	start_bodyshower(&gingy, 800, 600);
}

void test_orientation_finder() {
	//Human* gingy = new Gingy();
	Human* gingy = new Human();
	OrientationFinder* of = new OrientationFinder();
	of->ia->hsv_filter.add_filter(HsvFilter(200, 150, 200, 100, 255, 0, 255)); // blue
	of->ia->hsv_filter.add_filter(HsvFilter(160, 358, 15, 150, 255, 25, 255)); // red
	of->ia->hsv_filter.add_filter(HsvFilter(120, 300, 357, 100, 255, 20, 200)); // pink
	of->ia->hsv_filter.add_filter(HsvFilter(80, 20, 60, 150, 255, 100, 255)); // yellow
	of->ia->hsv_filter.add_filter(HsvFilter(40, 60, 150, 150, 255, 80, 255)); // green
	of->start(gingy, MODE_VIDEO, "inputs/simple_video.mpg");

	delete of;
	delete gingy;
}

void test_orientation_finder2() {
	Human* gingy = new Gingy();
	//	Human* gingy = new Human();
	OrientationFinder* of = new OrientationFinder();
	HsvFilterList* f = &of->ia->hsv_filter;
	f->add_filter(HsvFilter(200, 150, 250, 60, 255, 0, 255)); // blue
	f->add_filter(HsvFilter(160, 350, 5, 150, 255, 50, 255)); // red
	f->add_filter(HsvFilter(120, 300, 357, 50, 255, 40, 200)); // pink
	f->add_filter(HsvFilter(80, 20, 60, 100, 255, 100, 255)); // yellow
	f->add_filter(HsvFilter(40, 60, 150, 100, 255, 80, 255)); // green

	// commercial demo
	//	of->start(gingy, MODE_VIDEO, "inputs/jan/1.MPG");
	//	of->start(gingy, MODE_VIDEO, "inputs/jan/2.MPG");
	//	of->start(gingy, MODE_VIDEO, "inputs/jan/3.MPG");
	//	of->start(gingy, MODE_VIDEO, "inputs/jan/4.MPG");
	of->start(gingy, MODE_VIDEO, "inputs/jan/5.MPG");
	//	of->start(gingy, MODE_CAMERA);

	delete of;
	delete gingy;
}

void test_connected_comps() {
	IplImage* i = cvLoadImage("inputs/comps.png", CV_LOAD_IMAGE_UNCHANGED);

	ImageUtils::reset_timer();
	vector<vector<CvPoint> > points;
	vector<CvRect> boxes;
	ImageUtils::connectedComponents(i, &points, &boxes);

	cout << "time:" << ImageUtils::get_timer() << endl;
	cout << "number of comps:" << points.size() << endl;
	cout << "size of 1:" << points.at(0).size() << endl;
	//	for (int comp = 0; comp < points.size(); ++comp) {
	//		cout << "comp " << comp << endl;
	//		for (vector<CvPoint>::iterator it = points.at(comp).begin(); it
	//				< points.at(comp).end(); ++it)
	//			cout << "(" << it->x << ", " << it->y << ") ";
	//		cout << endl;
	//	}
}

void test_hsv_filters() {
	IplImage* hsv = cvLoadImage("inputs/metro.png", CV_LOAD_IMAGE_COLOR);
	cvCvtColor(hsv, hsv, CV_BGR2HSV);
	IplImage* ans = cvCreateImage(cvSize(hsv->width, hsv->height), 8, 1);

	HsvFilterList filters;
	filters.add_filter(HsvFilter(50, 0, 50, 150, 255, 150, 255)); // red
	filters.add_filter(HsvFilter(150, 100, 150, 150, 255, 150, 255)); // green
	//	filters.add_filter(HsvFilter(20, 150, 200, 100, 255, 0, 255)); // blue
	//	filters.add_filter(HsvFilter(40, 20, 60, 150, 255, 100, 255)); // yellow
	//	filters.add_filter(HsvFilter(100, 300, 357, 100, 255, 20, 200)); // pink

	ImageUtils::reset_timer();
	int nb_times = 100;
	for (int i = 0; i < nb_times; ++i)
		filters.filter_image(hsv, ans);
	cout << "time:" << ImageUtils::get_timer() / nb_times << endl;

	cvNamedWindow("ans", CV_WINDOW_AUTOSIZE);
	cvShowImage("ans", ans);
	cvWaitKey(0);
	cvReleaseImage(&ans);
	cvDestroyWindow("ans");
}

void args_parser(int argc, char **argv) {
	cout << "  ___  __ _ ___ _   _   _ __ ___   ___   ___ __ _ _ __" << endl;
	cout << " / _ \\/ _` / __| | | | | '_ ` _ \\ / _ \\ / __/ _` | '_ \\ "
			<< endl;
	cout << "|  __/ (_| \\__ \\ |_| | | | | | | | (_) | (_| (_| | |_) |"
			<< endl;
	cout << " \\___|\\__,_|___/\\__, | |_| |_| |_|\\___/ \\___\\__,_| .__/ "
			<< endl;
	cout << "                 __/ |                           | | " << endl;
	cout << "                |___/                            |_| " << endl;
	cout << endl;

	bool need_help = false;
	bool video_mode = false;
	bool camera_mode = false;
	bool use_gingy = false;
	bool dancing_man_demo = false;
	string filename;

	/* parse the args */
	if (argc == 1)
		need_help = true;
	else
		for (int i = 0; i < argc; ++i) {
			string word = argv[i];
			if (word == "-h") {
				need_help = true;
				break;
			} // end -h
			if (word == "-g")
				use_gingy = true;
			if (word == "-c")
				camera_mode = true;
			if (word == "-d")
				dancing_man_demo = true;

			if (word == "-v") {
				if (i == argc - 1) {
					need_help = true;
					break;
				}
				video_mode = true;
				filename = argv[i + 1];
			} // end -v
		} // end loop on words

	/* display help if needed */
	if (!video_mode && !camera_mode && !dancing_man_demo)
		need_help = true;
	if (need_help) {
		cout << "** Usage: ./easy_mocap.exe [OPTION]" << endl;
		cout << "Modes :" << endl;
		cout << "  -c                Camera mode" << endl;
		cout << "  -v <filename>     Video mode" << endl;
		cout << "  -d                Dancing man demo" << endl;
		cout << "  -h                This help" << endl;
		cout << "Others :" << endl;
		cout
				<< "  -g                Use gingy instead of the usual cylinder man"
				<< endl;
		cout << "** Moving in the OpenGL world:" << endl;
		cout << "<2/8> : move backwards, forwards" << endl;
		cout << "<4/6> : rotate left, right" << endl;
		cout << "<1/3> : strafe left, right" << endl;
		cout << "<+/-> : go up, down" << endl;
		exit(0);
	}

	Human* human = (use_gingy ? new Gingy() : new Human());

	if (dancing_man_demo) {
		human->opengl_make_dancing = true;
		start_bodyshower(human, 800, 600);
	}

	if (video_mode || camera_mode) {
		OrientationFinder* of = new OrientationFinder();
		HsvFilterList* f = &of->ia->hsv_filter;
		f->add_filter(HsvFilter(200, 150, 250, 60, 255, 40, 255)); // blue
		f->add_filter(HsvFilter(160, 350, 5, 150, 255, 50, 255)); // red
		f->add_filter(HsvFilter(120, 300, 357, 50, 255, 40, 200)); // pink
		f->add_filter(HsvFilter(80, 20, 60, 100, 255, 100, 255)); // yellow
		f->add_filter(HsvFilter(40, 60, 150, 100, 255, 80, 255)); // green

		/* camera mode */
		if (camera_mode) {
			cout << "Starting camera mode." << endl;
			of->start(human, MODE_CAMERA);
		}

		/* video mode */
		if (video_mode) {
			cout << "Starting video mode with video '" << filename << "'."
					<< endl;
			of->start(human, MODE_VIDEO, filename.c_str());
		}

		delete of;
	} // end (video_mode || camera_mode)

	delete human;
}

int main(int argc, char **argv) {
	//	cout << "main" << endl;

	//	test_member();
	//	test_connected_comps();
	//	test_hsv_filters();

	//	test_opengl_body();
	//  test_gingy();
	//	test_orientation_finder();
	//	test_orientation_finder2();

	args_parser(argc, argv);

	//	cout << "end of main" << endl;
}
