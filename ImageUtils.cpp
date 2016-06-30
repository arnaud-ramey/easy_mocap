/*
 * ImageUtils.cpp
 *
 *  Created on: Jan 14, 2010
 *      Author: arnaud
 */

#include "ImageUtils.h"

/*!
 * @brief   computes the disjoint set of the image
 *
 * @param   img a monochrome image
 * @param   node_best a pointer for the biggest connected component found
 * @return  the computed disjoint set
 */
inline DisjointSets ImageUtils::disjointSets(const IplImage* img,
		struct DisjointSets::Node* node_best /* = new DisjointSets::Node(1) */) {

	//	double timer = (double)cvGetTickCount();
	//	double freq = (double) cvGetTickFrequency() * 1000.;

	// get the image size
	CvRect boundaries = cvGetImageROI(img);
	// the value of the current pixel and its neighbours
  bool is_curr_non_null, is_left_OK = true, is_up_OK = true;
	uchar curr_value;
	//	cout << "Time 010:" << ((double)cvGetTickCount() - timer) / freq << endl;

	// the future answer
	DisjointSets s(boundaries.width * boundaries.height);
	//! the biggest component
	DisjointSets::Node* best_ptr = new DisjointSets::Node(1);
	//	cout << "Time 015:" << ((double)cvGetTickCount() - timer) / freq << endl;


	vector<DisjointSets::Node*>::iterator node_curr_it = s.m_nodes.begin();
	--node_curr_it;
	vector<DisjointSets::Node*>::iterator node_left_it = node_curr_it;
	--node_left_it;
	vector<DisjointSets::Node*>::iterator node_up_it = node_curr_it;

	for (int y = 0; y < boundaries.height; y++) {
		if (y == 1) {
			node_up_it = s.m_nodes.begin();
			node_up_it--;
		}

		for (int x = 0; x < boundaries.width; x++) {
			++node_curr_it;
			++node_left_it;
			++node_up_it;

			// if pixel is white => skip
			curr_value = CV_IMAGE_ELEM(img, uchar, boundaries.y + y,
					boundaries.x + x);
			is_curr_non_null = (curr_value != 0);
			if (!is_curr_non_null)
				continue;

			// initiate the node
			*node_curr_it = new DisjointSets::Node(y * boundaries.width + x);
			DisjointSets::Node * node_curr_father;

			if (x > 0) {
				//is_left_OK = (*node_left_it);
				// changing => have to have the same color
				is_left_OK = (*node_left_it && curr_value
						== CV_IMAGE_ELEM(img, uchar, boundaries.y + y,
								boundaries.x + x-1));
			}
			if (y > 0) {
				//is_up_OK = (*node_up_it);
				// changing => have to have the same color
				is_up_OK = (*node_up_it && curr_value
						== CV_IMAGE_ELEM(img, uchar, boundaries.y + y-1,
								boundaries.x + x));
			}

			/* fusion with left and up - if not the first column or line */
			/* priority to the up parent, then left : */
			if (x > 0 && y > 0 && is_left_OK && is_up_OK) {
				node_curr_father = s.Union(s.FindSet(*node_up_it),
						*node_curr_it);
				node_curr_father = s.Union(s.FindSet(*node_left_it),
						node_curr_father);
			}

			/* fusion with left - if not the first column */
			else if (x > 0 && is_left_OK) {
				node_curr_father = s.Union(s.FindSet(*node_left_it),
						*node_curr_it);
			}

			/*fusion with up - if not the first line */
			else if (y > 0 && is_up_OK) {
				node_curr_father = s.Union(s.FindSet(*node_up_it),
						*node_curr_it);
			}

			/* only case left : first white pixel of the image */
			else {
				node_curr_father = *node_curr_it;
			}

			/* display */
			//			DisjointSets::Node* curr = s.elementAt( y * boundaries.width + x );
			//			cout << "Position (" << x << "," << y << ")";
			//			cout << ", curr:" << curr->index;
			//			cout << ", father:" << ( curr->parent ? curr->parent->index : -1 );
			//			cout << ", root:" << s.FindSet( curr )->index;
			//			cout << endl;

			/* save the best node */
			//cout << "size of curr node:" <<node_curr->size << endl;
			if (node_curr_father->size > best_ptr->size) {
				//				cout << "New best:" << node_curr_father->toString() << endl;
				best_ptr = node_curr_father;
			}

		} // end loop x
	} // end loop y

	/* copy the best answer */
	*node_best = *best_ptr;

	//	cout << "Time 03:" << ((double)cvGetTickCount() - timer) / freq << endl;
	return s;
}

double timer, freq;

/*!
 * reset the timer
 */
void ImageUtils::reset_timer() {
	timer = (double) cvGetTickCount();
	freq = (double) cvGetTickFrequency() * 1000.;
}

/*!
 * @return the timer
 */
double ImageUtils::get_timer() {
	return ((double) cvGetTickCount() - timer) / freq;
}

/*!
 * \brief   returns a string with information about the image
 * (size, depth...)
 */
string ImageUtils::infosImage(const IplImage* i) {
	std::ostringstream concl;
	concl << "size:(" << i->width << ", " << i->height << ")";
	concl << " |ROI:(" << cvGetImageROI(i).x << ", " << cvGetImageROI(i).y
			<< ")";
	concl << "+(" << cvGetImageROI(i).width << ", " << cvGetImageROI(i).height
			<< ")";
	concl << " |nChannels:" << i->nChannels;
	concl << " |depth:" << i->depth;
	return concl.str();
}

/*!
 * @brief   returns all the list of points making all the connected components
 * of the image
 *
 * @param   img the monochrome image
 * @param   componentsPoints the vector of vector of points which will
 * contain the results
 * @param   boundingBoxes the bounding boxes of the points
 */
void ImageUtils::connectedComponents(const IplImage* img, vector<
		vector<CvPoint> >* componentsPoints, vector<CvRect>* boundingBoxes) {
	//	double timer = (double)cvGetTickCount();
	//	double freq = (double) cvGetTickFrequency() * 1000.;

	DisjointSets s = disjointSets(img);
	//	cout << "Time 1:" << ((double)cvGetTickCount() - timer) / freq << endl;

	CvRect boundaries = cvGetImageROI(img);
	CvRect* bb_father;
	int w = boundaries.width, h = boundaries.height;
	int xR, yR;

	// this table will contain the position of each father in the vector
	vector<short> father_id(w * h, -1);
	short* father_idx_in_vec;

	componentsPoints->clear();
	boundingBoxes->clear();
	//	cout << "Time 2:" << ((double)cvGetTickCount() - timer) / freq << endl;

	for (int y = 0; y < h; y++) {
		for (int x = 0; x < w; x++) {
			xR = x + boundaries.x;
			yR = y + boundaries.y;

			/* if black pixel => skip */
			if (CV_IMAGE_ELEM( img, uchar, yR, xR ) == 0)
				continue;

			father_idx_in_vec
					= &father_id[s.FindSet(s.elementAt(y * w + x))->index];

			/* new father => push his index and create a new vector */
			if (*father_idx_in_vec == -1) {
				*father_idx_in_vec = componentsPoints->size();
				componentsPoints->push_back(
						vector<CvPoint> (1, cvPoint(xR, yR)));
				boundingBoxes->push_back(cvRect(xR, yR, 1, 1));
			}
			/* old father => only update */
			else {
				(*componentsPoints)[*father_idx_in_vec].push_back(cvPoint(xR,
						yR));
				bb_father = &(*boundingBoxes)[*father_idx_in_vec];
				bb_father->x = min(xR, bb_father->x);
				bb_father->y = min(yR, bb_father->y);
				bb_father->width = max(bb_father->width, xR + 1 - bb_father->x);
				bb_father->height = max(bb_father->height, yR + 1
						- bb_father->y);
			} // end if new father

		} // end loop x
	} // end loop y

	//	cout << "Time 3:" << ((double)cvGetTickCount() - timer) / freq << endl;
}

/*!
 * \brief   color conversion function
 */
CvScalar ImageUtils::hue2rgb(float hue) {
	int rgb[3], p, sector;
	const int sector_data[][3] = { { 0, 2, 1 }, { 1, 2, 0 }, { 1, 0, 2 }, { 2,
			0, 1 }, { 2, 1, 0 }, { 0, 1, 2 } };
	hue *= 0.033333333333333333333333333333333f;
	sector = cvFloor(hue);
	p = cvRound(255 * (hue - sector));
	p ^= sector & 1 ? 255 : 0;

	rgb[sector_data[sector][0]] = 255;
	rgb[sector_data[sector][1]] = 0;
	rgb[sector_data[sector][2]] = p;

	return cvScalar(rgb[2], rgb[1], rgb[0], 0);
}

/*!
 * \brief   create the rgb image which illustrate the layer__Hue
 * \param   src a rgb image
 * \param   dest a rgb image
 */
void ImageUtils::extractHue_and_convertRGB(const IplImage* src, IplImage* dest) {
	IplImage* hue = cvCreateImage(cvGetSize(src), 8, 1);

	/* get the layer__Hue component */
	cvCvtColor(src, dest, CV_BGR2HSV); // conversion in HSV
	//	cvCvtColor(src, dest, CV_RGB2HSV); // conversion in HSV
	cvSplit(dest, hue, 0, 0, 0); // split the image

	convert_hue_to_RGB(hue, dest);
	cvReleaseImage(&hue);
}

/*!
 * \brief   create the rgb image which illustrate the layer__Hue
 * \param   src a rgb image
 * \param   dest a rgb image
 */
inline void ImageUtils::convert_hue_to_RGB(const IplImage* src, IplImage* dest) {
	CvScalar rgb;

	for (int y = 0; y < src->height; y++) {
		uchar* ptr = (uchar*) (src ->imageData + y * src ->widthStep);
		uchar* ptr_dest = (uchar*) (dest->imageData + y * dest->widthStep);

		for (int x = 0; x < src->width; x++) {
			rgb = hue2rgb(min((int) ptr[x], 179));
			if (x % 50 == 0 && y % 50 == 0) {
				cout << "x:" << x << ", y:" << y << " - Hue:" << (float) ptr[x]
						<< " \t";
				cout << "RGB:" << rgb.val[0] << "-" << rgb.val[1] << "-"
						<< rgb.val[2] << " \t" << endl;
			}
			ptr_dest[3 * x + 0] = (int) rgb.val[0];
			ptr_dest[3 * x + 1] = (int) rgb.val[1];
			ptr_dest[3 * x + 2] = (int) rgb.val[2];
		}
	}

	/*std::ostringstream m;
	 m << "extractHue_and_convertRGB():dest:" << dest->width << "x" << dest->height << "|" << dest->nChannels << " channels";
	 debugInfo(m.str());*/
}

/*!
 * @brief   return the predefined color number i
 */
CvScalar ImageUtils::color(int i) {
	return predefined_colors[i % 21];
}

/*!
 * \brief   draw a list of points
 */
void ImageUtils::drawListOfPoints(CvArr* img, vector<CvPoint>* pts,
		CvScalar color) {
	for (vector<CvPoint>::iterator it = pts->begin(); it < pts->end(); it++) {
		drawPoint(img, *it, color, 1);
	}
}

/*!
 * \brief   to draw a point in an image
 */
inline void ImageUtils::drawPoint(CvArr* img, CvPoint p, CvScalar color,
		int thickness /*= 1*/) {
	cvLine(img, p, p, color, thickness);
}

void ImageUtils::drawBox2D(CvArr* img, CvBox2D &box, CvScalar color,
		int thickness /*= 1*/) {
	CvPoint2D32f points[4];
	cvBoxPoints(box, points);
	for (int i = 0; i < 4; ++i)
		cvLine(img, cvPointFrom32f(points[i]), cvPointFrom32f(points[(i + 1)
				% 4]), color, thickness);
}
