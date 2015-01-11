/*
 * ImageAnalyzer.cpp
 *
 *  Created on: Jan 13, 2010
 *      Author: arnaud
 */

#include "ImageAnalyzer.h"

/*!
 * constructor
 */
ImageAnalyzer::ImageAnalyzer() {
	/* build the HSV filter */
	// with method 1, the first filter added is the most important
	// with method 2, the filter with the highest index is the most important

	/* reset the filtered image */
	frame_illus = NULL;
	frame_filtered = NULL;
}

/*!
 * destructor
 */
ImageAnalyzer::~ImageAnalyzer() {
	cvReleaseImage(&frame_illus);
	cvReleaseImage(&frame_filtered);
}

/*!
 * convert a rgb color to hsv
 * @param r
 * @param g
 * @param b
 * @param h
 * @param s
 * @param v
 */
inline void ImageAnalyzer::convert_rgb_to_hsv(int& r, int& g, int& b, int& h,
		int& s, int& v) {
	int Min = min(r, min(g, b));
	int Max = max(r, max(g, b));

	/* compute H */
	if (Max == Min)
		h = 0;
	else if (Max == r)
		h = ((60 * (g - b)) / (Max - Min) + 360) % 360;
	else if (Max == g)
		h = ((60 * (b - r)) / (Max - Min) + 120);
	else if (Max == b)
		h = ((60 * (r - g)) / (Max - Min) + 240);

	/* compute S */
	s = (Max == 0 ? 0 : (255 * (Max - Min)) / Max);

	/* compute V */
	v = Max;
}

inline void ImageAnalyzer::convert_rgb_to_hsv2(int& r, int& g, int& b) {
	int Min = min(r, min(g, b));
	int Max = max(r, max(g, b));

	/* compute H */
	if (Max == Min)
		r = 0;
	else if (Max == r)
		r = ((60 * (g - b)) / (Max - Min) + 360) % 360;
	else if (Max == g)
		r = ((60 * (b - r)) / (Max - Min) + 120);
	else if (Max == b)
		r = ((60 * (r - g)) / (Max - Min) + 240);

	/* compute S */
	g = (Max == 0 ? 0 : (255 * (Max - Min)) / Max);

	/* compute V */
	b = Max;
}

void ImageAnalyzer::analyze_frame(IplImage* img, int* number_of_required_comps,
		vector<vector<CvBox2D> >* answer) {
	cout << "ImageAnalyzer::analyze_frame()" << endl;
	ImageUtils::reset_timer();

	/* init the illustration */
	if (!frame_illus)
		frame_illus = cvCreateImage(cvGetSize(img), 8, 3);
	cvZero(frame_illus);

	/* conversion in HSV */
	cvCvtColor(img, img, CV_BGR2HSV);
	cout << "Time after SHV conversion:" << ImageUtils::get_timer() << endl;

	/* create the filtered frame if needed */
	if (!frame_filtered)
		frame_filtered = cvCreateImage(cvSize(img->width, img->height), 8, 1);

	//cout << ImageUtils::infosImage(frame_filtered) << endl;

	/* filter image */
	hsv_filter.filter_image2(img, frame_filtered);
	cout << "Time after filtering:" << ImageUtils::get_timer() << endl;

	/* compute connected components */
	vector<vector<CvPoint> > components;
	vector<CvRect> boxes;
	ImageUtils::connectedComponents(frame_filtered, &components, &boxes);
	cout << "Number of components:" << components.size() << endl;
	cout << "Time after computing connected comps:" << ImageUtils::get_timer()
			<< endl;

	// display
	//	int i = 0;
	//	for (vector<vector<CvPoint> >::iterator it = components.begin(); it
	//			< components.end(); ++it)
	//		ImageUtils::drawListOfPoints(frame_illus, &(*it),
	//				ImageUtils::color(i++));

	/* find the biggest components fitting */
	// create and clear the best indices
	int best_index[nb_filters()];
	int best_second_index[nb_filters()];
	for (unsigned int i = 0; i < nb_filters(); ++i) {
		best_index[i] = -1;
		best_second_index[i] = -1;
	}

	//return;

	/* find the corresponding number of components */
	int current_comp_index = 0;
	for (vector<vector<CvPoint> >::iterator current_comp = components.begin(); current_comp
			< components.end(); ++current_comp) {
		if (current_comp->size() < MIN_COMP_SIZE) {
			++current_comp_index; // incr the index of the iterator
			continue;
		}

			// read the value of the component
			int
					current_comp_value =
							CV_IMAGE_ELEM(frame_filtered, uchar, current_comp->front().y, current_comp->front().x);

		// find the corresponding filter
		int corresponding_filter_index = -1, current_filter_index = 0;
		for (vector<HsvFilter>::iterator it = hsv_filter.filters.begin(); it
				< hsv_filter.filters.end(); ++it) {
			if (current_comp_value == it->draw_value) {
				corresponding_filter_index = current_filter_index;
				break;
			}
			++current_filter_index;
		} // end loop filters

		// is the current component bigger than the previous found
		int* first_idx = &best_index[corresponding_filter_index];
		int* second_idx = &best_second_index[corresponding_filter_index];
		/* new best */
		if (*first_idx == -1 || components[*first_idx].size()
				<= current_comp->size()) {
			*second_idx = *first_idx;
			*first_idx = current_comp_index;
		} // end of new best
		/* new second best */
		else if (*second_idx == -1 || components[*second_idx].size()
				<= current_comp->size()) {
			*second_idx = current_comp_index;
		} // end of second new best

		++current_comp_index; // incr the index of the iterator
	} // end loop components

	/* display */
	vector<HsvFilter>::iterator current_filter = hsv_filter.filters.begin();
	for (unsigned int filter_idx = 0; filter_idx < nb_filters(); ++filter_idx) {
		// draw the best
		if (number_of_required_comps[filter_idx] >= 1 && best_index[filter_idx]
				!= -1) {
			ImageUtils::drawListOfPoints(frame_illus, //
					&components.at(best_index[filter_idx]), //
					current_filter->color());
		}
		// draw the second best
		if (number_of_required_comps[filter_idx] >= 2
				&& best_second_index[filter_idx] != -1)
			ImageUtils::drawListOfPoints(frame_illus, &components.at(
					best_second_index[filter_idx]), current_filter->color());
		++current_filter;
	} // end loop filters

	cout << "Time after finding biggest comps:" << ImageUtils::get_timer()
			<< endl;

	/* find the orientations of the components */
	// reset the boxes
	answer->clear();
	answer->resize(nb_filters());

	current_filter = hsv_filter.filters.begin();
	for (unsigned int filter_idx = 0; filter_idx < nb_filters(); ++filter_idx) {
		for (int nb_comp = 1; nb_comp <= 2; ++nb_comp) {
			if (number_of_required_comps[filter_idx] >= nb_comp) {
				// get the index of the comp
				int comp_index = -1;
				if (nb_comp == 1)
					comp_index = best_index[filter_idx];
				if (nb_comp == 2)
					comp_index = best_second_index[filter_idx];
				// find the box
				if (comp_index >= 0) {
					CvBox2D box = find_orientation(&components.at(comp_index));
					answer ->at(filter_idx).push_back(box);
					// draw the boxes
					ImageUtils::drawBox2D(frame_illus, box,
							current_filter->color());
				}
			} // end if we need n comp
		} // end loop coms
		++current_filter;
	} // end loop filters

	// display the number of comps found
	//	for (unsigned int filter_index = 0; filter_index < nb_filters(); ++filter_index)
	//		cout << "#" << filter_index << ":" << answer->at(filter_index).size()
	//				<< endl;

	cout << "Time after boxes:" << ImageUtils::get_timer() << endl;
}

CvBox2D ImageAnalyzer::find_orientation(vector<CvPoint>* points) {
	CvMat* points_array = cvCreateMat(points->size(), 1, CV_32FC2);
	int i = 0;
	for (vector<CvPoint>::iterator it = points->begin(); it < points->end(); ++it) {
		points_array->data.fl[2 * i] = it->x;
		points_array->data.fl[2 * i + 1] = it->y;
		++i;
	}
	CvBox2D answer = cvMinAreaRect2(points_array);
	cvReleaseMat(&points_array);
	return answer;
}
