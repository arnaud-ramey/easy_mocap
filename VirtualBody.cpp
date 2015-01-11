/*
 * VirtualBody.cpp
 *
 *  Created on: Jan 12, 2010
 *      Author: arnaud
 */

#include "VirtualBody.h"

void Member::display() {
	ostringstream m;
	m << "Member:rotation=" << get_absolute_rotation_rad() * 180 / PI
			<< ", pos=";
	display_point(m.str(), get_absolute_position());
}

CvPoint2D32f Member::get_x_axis() {
	if (father == NULL)
		return cvPoint2D32f(cos(angle), sin(angle));

	// there is a father
	CvPoint2D32f father_x = father->get_x_axis();
	CvPoint2D32f father_y = father->get_y_axis();
	double x = father_x.x * cos(angle) + father_y.x * sin(angle);
	double y = father_x.y * cos(angle) + father_y.y * sin(angle);
	CvPoint2D32f ans = cvPoint2D32f(x, y);
	normalize(ans);
	return ans;
}

CvPoint2D32f Member::get_y_axis() {
	CvPoint2D32f x_axis = get_x_axis();
	return cvPoint2D32f(-x_axis.y, x_axis.x);
}

CvPoint3D32f Member::get_absolute_position() {
	if (father == NULL) {
		CvPoint3D32f ans = body->position;
		ans.x += abscissae_of_fixation;
		return ans;
	}

	CvPoint3D32f ans = father->get_absolute_position();
	CvPoint2D32f father_x = father->get_x_axis();
	ans.x += abscissae_of_fixation * father_x.x;
	ans.y += abscissae_of_fixation * father_x.y;

	return ans;
}

double Member::get_absolute_rotation_rad() {
	CvPoint2D32f axis_x = get_x_axis();
	return atan2(axis_x.y, axis_x.x);
}

VirtualBody::VirtualBody() {
	t = 0;
	opengl_make_dancing = false;
}

VirtualBody::~VirtualBody() {
}

Human::Human() {
	position = cvPoint3D32f(0, 0, 0);

	Member* torso = new Member(NULL, this);
	torso->abscissae_of_fixation = 0;
	torso->angle = -PI / 2;
	torso->length = 60;

	/* head */
	Member* head = new Member(torso, this);
	head->abscissae_of_fixation = -.5;
	head->angle = -PI;
	head->length = 30;

	/* left arm */
	Member* shoulder_left = new Member(torso, this);
	shoulder_left->abscissae_of_fixation = torso->length * 1.5f / 10;
	shoulder_left->angle = PI / 2;
	shoulder_left->length = 30;

	Member* arm_left = new Member(shoulder_left, this);
	arm_left->abscissae_of_fixation = shoulder_left->length;
	arm_left->angle = -3 * PI / 8;
	arm_left->length = 25;

	Member* forearm_left = new Member(arm_left, this);
	forearm_left->abscissae_of_fixation = arm_left->length;
	forearm_left->angle = -PI / 8;
	forearm_left->length = 35;

	/* right arm */
	Member* shoulder_right = new Member(torso, this);
	shoulder_right->abscissae_of_fixation
			= shoulder_left->abscissae_of_fixation;
	shoulder_right->angle = -PI / 2;
	shoulder_right->length = shoulder_left->length;

	Member* arm_right = new Member(shoulder_right, this);
	arm_right->abscissae_of_fixation = shoulder_right->length;
	arm_right->angle = 3 * PI / 8;
	arm_right->length = arm_left->length;

	Member* forearm_right = new Member(arm_right, this);
	forearm_right->abscissae_of_fixation = arm_right->length;
	forearm_right->angle = PI / 8;
	forearm_right->length = forearm_left->length;

	/* left leg */
	Member* crotch_left = new Member(torso, this);
	crotch_left->abscissae_of_fixation = torso->length;
	crotch_left->angle = PI / 2;//*PI * (1.f / 2 - 1.f / 8);
	crotch_left->length = 15;

	Member* shin_left = new Member(crotch_left, this);
	shin_left->abscissae_of_fixation = crotch_left->length;
	shin_left->angle = -PI / 2 + PI / 12;
	shin_left->length = 55;

	Member* leg_left = new Member(shin_left, this);
	leg_left->abscissae_of_fixation = shin_left->length;
	leg_left->angle = -PI / 12;
	leg_left->length = 60;

	/* right leg */
	Member* crotch_right = new Member(torso, this);
	crotch_right->abscissae_of_fixation = torso->length;
	crotch_right->angle = -PI / 2;
	crotch_right->length = crotch_left->length;

	Member* shin_right = new Member(crotch_right, this);
	shin_right->abscissae_of_fixation = crotch_right->length;
	shin_right->angle = PI / 2 - PI / 12;
	shin_right->length = shin_left->length;

	Member* leg_right = new Member(shin_right, this);
	leg_right->abscissae_of_fixation = shin_right->length;
	leg_right->angle = PI / 12;
	leg_right->length = leg_left->length;

}

Gingy::Gingy() {
	// clear the members
	for (vector<Member*>::iterator it = members.begin(); it < members.end(); ++it)
		(*it)->set_not_drawn();

	head()->abscissae_of_fixation = head()->length * .05f;

	// shoulders
	Member* shoulder_left = members.at(2);
	shoulder_left->abscissae_of_fixation += torso()->length * 0.1f;
	Member* shoulder_right = members.at(5);
	shoulder_right->abscissae_of_fixation
			= shoulder_left->abscissae_of_fixation;

	arm_left()->abscissae_of_fixation = shoulder_left->length * 7.f / 10;
	arm_left()->angle = 0;
	arm_right()->abscissae_of_fixation = arm_left()->abscissae_of_fixation;
	arm_right()->angle = 0;

	forearm_left()->abscissae_of_fixation = arm_left()->length * 2.f / 3;
	forearm_left()->angle = 0;
	forearm_right()->abscissae_of_fixation = arm_right()->length * 2.f / 3;
	forearm_right()->angle = 0;

	shin_left()->length *= .35;
	shin_left()->angle = -PI / 2;
	shin_right()->length = shin_left()->length;
	shin_right()->angle = PI / 2;

	leg_left()->abscissae_of_fixation = shin_left()->length;
	leg_left()->angle = 0;
	leg_right()->abscissae_of_fixation = shin_right()->length;
	leg_right()->angle = 0;

	torso()->set_rtg_file("gingy/torso.rtg", 60);
	head()->set_rtg_file("gingy/head.rtg", 60);
	arm_left()->set_rtg_file("gingy/arm.rtg", 60);
	arm_right()->set_rtg_file("gingy/arm.rtg", 60);
	forearm_left()->set_rtg_file("gingy/forearm.rtg", 60);
	forearm_right()->set_rtg_file("gingy/forearm.rtg", 60);
	shin_left()->set_rtg_file("gingy/shin_left.rtg", 60);
	shin_right()->set_rtg_file("gingy/shin_right.rtg", 60);
	leg_left()->set_rtg_file("gingy/leg.rtg", 60);
	leg_right()->set_rtg_file("gingy/leg.rtg", 60);
}

Human::~Human() {
	// delete members
	for (int var = members.size() - 1; var >= 0; --var)
		delete members.at(var);
	members.clear();
}
