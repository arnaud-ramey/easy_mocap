/*
 * ImageUtils.h
 *
 *  Created on: Jan 14, 2010
 *      Author: arnaud
 */

#ifndef IMAGEUTILS_H_
#define IMAGEUTILS_H_

// STL imports
#include <iostream>
#include <vector>
using namespace std;

// openCV imports
#include <cv.h>
#include <cvaux.h>
#include <highgui.h>

#include "DisjointSets.h"

class ImageUtils {
public:
	ImageUtils();
	virtual ~ImageUtils();

	static void reset_timer();
	static double get_timer();
	static string infosImage(const IplImage* i);

	static inline DisjointSets disjointSets(const IplImage* img,
			struct DisjointSets::Node* node_best = new DisjointSets::Node(1));

	static void connectedComponents(const IplImage* img,
			vector<vector<CvPoint> >* componentsPoints,
			vector<CvRect>* boundingBoxes);

	static CvScalar color(int i);
	static CvScalar hue2rgb(float hue);
	static void convert_hue_to_RGB(const IplImage* src, IplImage* dest);
	static void extractHue_and_convertRGB(const IplImage* src, IplImage* dest);

	static void drawListOfPoints(CvArr* img, vector<CvPoint>* pts,
			CvScalar color);
	static void drawPoint(CvArr* img, CvPoint p, CvScalar color, int thickness =
			1);
	static void drawBox2D(CvArr* img, CvBox2D &box, CvScalar color,
			int thickness = 1);

};

static const CvScalar predefined_colors[21] = { CV_RGB(255,0,0),
		CV_RGB(0,0,255), CV_RGB(255,128,0), CV_RGB(255,255,0),
		CV_RGB(128,255,0), CV_RGB(0,255,0), CV_RGB(0,255,128),
		CV_RGB(0,255,255), CV_RGB(0,128,255), CV_RGB(128,0,255),
		CV_RGB(255,0,255), CV_RGB(255,0,128), CV_RGB(204,0,102),
		CV_RGB(153,0,0), CV_RGB(153,77,0), CV_RGB(153,153,0),
		CV_RGB(77,153,0), CV_RGB(153,153,0), CV_RGB(0,77,153),
		CV_RGB(77,0,153), CV_RGB(153,0,153) };

#endif /* IMAGEUTILS_H_ */
