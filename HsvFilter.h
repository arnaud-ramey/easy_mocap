/*
 * HsvFilter.h
 *
 *  Created on: Jan 14, 2010
 *      Author: arnaud
 */

#ifndef HSVFILTER_H_
#define HSVFILTER_H_

#include "ImageUtils.h"

class HsvFilter {
public:
	int Hmin, Hmax, Smin, Smax, Vmin, Vmax;
	int draw_value;
	HsvFilter(int draw_value, int Hm, int HM, int Sm, int SM, int Vm, int VM) {
		Hmin = Hm;
		Hmax = HM;
		Smin = Sm;
		Smax = SM;
		Vmin = Vm;
		Vmax = VM;
		this->draw_value = draw_value;
	}
	CvScalar color() {
		if (Hmin < Hmax)
			return ImageUtils::hue2rgb((Hmin + Hmax) / 4);
		else
			return ImageUtils::hue2rgb(Hmin / 2);
	}
};

class HsvFilterList {
public:
	HsvFilterList();
	virtual ~HsvFilterList();

	vector<HsvFilter> filters;
	void clear() {
		filters.clear();
	}
	void add_filter(HsvFilter f) {
		filters.push_back(f);
	}

	void filter_image(IplImage* HSV, IplImage* out);

	void filter_image2(IplImage* HSV, IplImage* out);
	IplImage *h_layer, *s_layer, *v_layer;
	IplImage *h_filter, *s_filter, *v_filter;
};

#endif /* HSVFILTER_H_ */
