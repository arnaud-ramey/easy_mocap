/*
 * VirtualBody.h
 *
 *  Created on: Jan 12, 2010
 *      Author: arnaud
 */

#ifndef VIRTUALBODY_H_
#define VIRTUALBODY_H_

#define SAVE_IMAGES             0 // set to one to save images
#define DEFAULT_OPENGL_WIDTH    800
#define DEFAULT_OPENGL_HEIGHT   600
#define MIN_COMP_SIZE           200 // pixels

/*
 * indices of the body
 */
// the indices start at 0
#define HEAD_COLOR_INDEX            -1
#define TORSO_COLOR_INDEX           0
#define ARM_LEFT_COLOR_INDEX        1
#define FOREARM_LEFT_COLOR_INDEX    2
#define ARM_RIGHT_COLOR_INDEX       3
#define FOREARM_RIGHT_COLOR_INDEX   4
#define SHIN_LEFT_COLOR_INDEX       -1
#define LEG_LEFT_COLOR_INDEX        -1
#define SHIN_RIGHT_COLOR_INDEX      -1
#define LEG_RIGHT_COLOR_INDEX       -1

// some constants for the whole program
#define  PI 	3.14159

#define MODE_VIDEO    1
#define MODE_CAMERA   2

#define SHOW_FRAME_FILTERED   1
#define SHOW_FRAME_ILLUS      2
#define SHOW_HUE_COMPONENT    3

// STL imports
#include <iostream>
#include <vector>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>		// for malloc
#include <math.h>		// for cos and sin
#include <vector>		// for vectors
using namespace std;

// openCV imports
#include <opencv2/opencv.hpp>

#define DRAWN_NO_DRAWN 0
#define DRAWN_CYLINDER 1
#define DRAWN_RTG 2

class Member; // forward def

/*!
 * the representation of a body
 */
class VirtualBody {
public:
  /*! time */
  double t;

  VirtualBody();
  virtual ~VirtualBody();

  CvPoint3D32f position;

  vector<Member*> members;

  bool opengl_make_dancing;
};

/*!
 *  the representation of a body member
 */
class Member {
public:
  Member* father;
  VirtualBody* body;

  double abscissae_of_fixation; // in the absolute ref of the father
  double length; // in cm
  int type_of_drawin; // true if we need to draw a rtg
  string rtg_filename;
  double rtg_scaling;

  Member(Member* f, VirtualBody* b) {
    father = f;
    body = b;
    type_of_drawin = DRAWN_CYLINDER;
    body->members.push_back(this);
  }

  void set_rtg_file(string filename, double scaling_factor) {
    rtg_filename = filename;
    type_of_drawin = DRAWN_RTG;
    rtg_scaling = scaling_factor;
  }
  void set_drawn_by_cylinder() {
    type_of_drawin = DRAWN_CYLINDER;
  }
  void set_not_drawn() {
    type_of_drawin = DRAWN_NO_DRAWN;
  }

  CvPoint3D32f get_absolute_position();
  double get_absolute_rotation_rad();
  CvPoint2D32f get_x_axis();
  CvPoint2D32f get_y_axis();

  void display();

  // changing params
  double angle; // around the direction
};

/*!
 * the representation of a human body
 */
class Human: public VirtualBody {
public:
  Human();
  ~Human();

  Member* torso() {
    return members.at(0);
  }
  Member* head() {
    return members.at(1);
  }
  Member* arm_left() {
    // member 2 is the shoulder left
    return members.at(3);
  }
  Member* forearm_left() {
    return members.at(4);
  }
  Member* arm_right() {
    // member 5 is the shoulder right
    return members.at(6);
  }
  Member* forearm_right() {
    return members.at(7);
  }
  Member* shin_left() {
    // member 8 is the crotch left
    return members.at(9);
  }
  Member* leg_left() {
    return members.at(10);
  }
  Member* shin_right() {
    // member 11 is the crotch left
    return members.at(12);
  }
  Member* leg_right() {
    return members.at(13);
  }
};

class Gingy: public Human {
public:
  Gingy();
};


static inline void normalize(CvPoint2D32f & vec) {
  double norm = sqrt(vec.x * vec.x + vec.y * vec.y);
  vec.x = vec.x / norm;
  vec.y = vec.y / norm;
}

static inline void normalize(CvPoint3D32f & vec) {
  double norm = sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
  vec.x = vec.x / norm;
  vec.y = vec.y / norm;
  vec.z = vec.z / norm;
}

static inline void display_point(string text, CvPoint2D32f p) {
  cout << text << " = (" << p.x << ", " << p.y << ")" << endl;
}

static inline void display_point(string text, CvPoint3D32f p) {
  cout << text << " = (" << p.x << ", " << p.y << ", " << p.z << ")" << endl;
}

void start_bodyshower(Human* vb, int w = DEFAULT_OPENGL_WIDTH, int h =
    DEFAULT_OPENGL_HEIGHT);

#endif /* VIRTUALBODY_H_ */
