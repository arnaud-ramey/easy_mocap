/*
 * ImageAnalyzer.h
 *
 *  Created on: Jan 13, 2010
 *      Author: arnaud
 */

#ifndef IMAGEANALYZER_H_
#define IMAGEANALYZER_H_

#include "ImageUtils.h"
#include "HsvFilter.h"
#include "VirtualBody.h"

class ImageAnalyzer {
public:
	ImageAnalyzer();
	virtual ~ImageAnalyzer();

	inline unsigned int nb_filters() {return hsv_filter.filters.size();}
	// display
	IplImage* frame_illus;

	// filter
	HsvFilterList hsv_filter;
	IplImage* frame_filtered;

	void analyze_frame(IplImage* img, int* number_of_required_comps, vector<vector<CvBox2D> >* answer);

private:
	static inline void convert_rgb_to_hsv(int& r, int& g, int& b, int& h, int& s,
			int& v);
	static inline void convert_rgb_to_hsv2(int& r, int& g, int& b);


	CvBox2D find_orientation(vector<CvPoint>* points);

	// analyze
	vector<CvBox2D> boxes;
};

#endif /* IMAGEANALYZER_H_ */
