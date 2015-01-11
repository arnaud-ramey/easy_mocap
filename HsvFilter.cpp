/*
 * HsvFilter.cpp
 *
 *  Created on: Jan 14, 2010
 *      Author: arnaud
 */

#include "HsvFilter.h"

/*!
 * constructor
 */
HsvFilterList::HsvFilterList() {
	clear();

	h_layer = NULL;
	s_layer = NULL;
	v_layer = NULL;
	h_filter = NULL;
	s_filter = NULL;
	v_filter = NULL;

}

/*!
 * destructor
 */
HsvFilterList::~HsvFilterList() {
	clear();

	cvReleaseImage(&h_layer);
	cvReleaseImage(&s_layer);
	cvReleaseImage(&v_layer);
	cvReleaseImage(&h_filter);
	cvReleaseImage(&s_filter);
	cvReleaseImage(&v_filter);
}

/*!
 * apply the filter to an image
 * @param HSV the hsv image
 * @param out a monochrome image
 */
void HsvFilterList::filter_image(IplImage* HSV, IplImage* out) {
	// clear the answer
	cvZero(out);

	int H, S, V;
	for (int y = 0; y < HSV->height; ++y) {
		uchar* ptr = (uchar*) (HSV->imageData + y * HSV->widthStep);

		for (int x = 0; x < HSV->width; x++) {
			// read HSV
			H = 2 * (int) *ptr++;
			S = (int) *ptr++;
			V = (int) *ptr++;

			//			cout << "HSV:" << H << ", " << S << ", " << V << endl;

			// check the filters
			int current_filter_idx = -1, matching_filter_idx = -1;
			for (vector<HsvFilter>::iterator filter = filters.begin(); filter
					< filters.end(); ++filter) {
				++current_filter_idx;
				// check the current filter
				if (S < filter->Smin || S > filter->Smax)
					continue;
				if (V < filter->Vmin || V > filter->Vmax)
					continue;
				if (filter->Hmin > filter->Hmax) {
					// case of the filter including 0
					if (H < filter->Hmin && H > filter->Hmax)
						continue;
				} else {
					// usual fitler
					if (H < filter->Hmin || H > filter->Hmax)
						continue;
				}
				//cout << "pass:(" << x << ", " << y << ")" << endl;
				// we are OK
				matching_filter_idx = current_filter_idx;
				break;
			} // end loop filter

			// if we found a matching filter, set the pixel to the good color
			if (matching_filter_idx != -1)
				CV_IMAGE_ELEM(out, uchar, y, x) = //
						filters.at(matching_filter_idx).draw_value;

		} // end loop x
	} // end loop y
}

static inline void thresh_image(IplImage* src, IplImage* dst, int min, int max,
		int out_value) {
	if (min < max) {
		//		cvThreshold(src, dst, min - 1, 255, CV_THRESH_TOZERO);
		//		cvThreshold(dst, dst, max + 1, 255, CV_THRESH_TOZERO_INV);
		//		cvThreshold(dst, dst, 0, out_value, CV_THRESH_BINARY);
		cvThreshold(src, dst, max, 255, CV_THRESH_TOZERO_INV);
		cvThreshold(dst, dst, min - 1, out_value, CV_THRESH_BINARY);
	} else { // inverted filter
		// bring everything good to 0
		cvThreshold(src, dst, min - 1, 255, CV_THRESH_TOZERO_INV);
		cvThreshold(dst, dst, max + 1, out_value, CV_THRESH_BINARY_INV);
	}

	//	cout << "min:" << min << ", max:" << max << endl;
	//	cout << "elem in:" << (int) CV_IMAGE_ELEM(src, uchar, 263, 439) << endl;
	//	cout << "elem out:" << (int) CV_IMAGE_ELEM(dst, uchar, 263, 439) << endl;
}

/*!
 * apply the filter to an image
 * @param HSV the hsv image
 * @param out a monochrome image
 */
void HsvFilterList::filter_image2(IplImage* HSV, IplImage* out) {
	// create the images if needed
	if (!h_layer) {
		h_layer = cvCreateImage(cvGetSize(HSV), 8, 1);
		s_layer = cvCreateImage(cvGetSize(HSV), 8, 1);
		v_layer = cvCreateImage(cvGetSize(HSV), 8, 1);
		h_filter = cvCreateImage(cvGetSize(HSV), 8, 1);
		s_filter = cvCreateImage(cvGetSize(HSV), 8, 1);
		v_filter = cvCreateImage(cvGetSize(HSV), 8, 1);
	}

	// clear the answer
	cvZero(out);

	// extract the layers
	cvSplit(HSV, h_layer, s_layer, v_layer, NULL);
	// we have to add a shift of 1 to the h comp to have a value different
	// from 0
	cvAddS(h_layer, cvScalarAll(10), h_layer);

	for (vector<HsvFilter>::iterator it = filters.begin(); it < filters.end(); ++it) {
		// apply the filters on HSV
		thresh_image(h_layer, h_filter, it->Hmin / 2 + 10, it->Hmax / 2 + 10,
				it->draw_value);
		thresh_image(s_layer, s_filter, it->Smin, it->Smax, it->draw_value);
		thresh_image(v_layer, v_filter, it->Vmin, it->Vmax, it->draw_value);

		// combine the filters
		cvMin(h_filter, s_filter, h_filter);
		cvMin(h_filter, v_filter, h_filter);

		// combine with the answer
		cvMax(out, h_filter, out);
	}
}
