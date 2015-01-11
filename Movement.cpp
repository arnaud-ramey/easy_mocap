/*
 * Movement.cpp
 *
 *  Created on: Jan 12, 2010
 *      Author: arnaud
 */

/*!
 * refresh the display
 */
void refresh() {
	glutPostRedisplay();
}

void reload_view() {
	glMatrixMode( GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(X, Y, Z, X + lx, Y + ly, Z + lz, 0.0f, 1.0f, 0.0f);
	refresh();
}

/*! Reshape callback */
void reshape(int w, int h) {
	cout << "reshape()" << endl;

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	h = (h <= 0 ? 1 : h);

	width = w;
	height = h;

	// Reset the coordinate system before modifying
	glMatrixMode( GL_PROJECTION);
	glLoadIdentity();

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);

	// Set the clipping volume
	gluPerspective(45, 1.0f * w / h, 1, 1000);

	reload_view();
}

/*!
 * apply a rotation of an angle theta on place on lx0 and lz0
 * @param theta angle in radians
 * @param lx0
 * @param lz0
 */
void applyRotation(double theta, float* lx0, float* lz0) {
	double newLx = cos(theta) * *lx0 - sin(theta) * *lz0;
	double newLz = sin(theta) * *lx0 + cos(theta) * *lz0;
	*lx0 = newLx;
	*lz0 = newLz;
}

/*!
 * rotate the point of view from an angle
 * @param theta the angle in radians
 */
void rotate(double theta) {
	applyRotation(theta, &lx, &lz);
	gluLookAt(X, Y, Z, X + lx, Y + ly, Z + lz, 0.0f, 1.0f, 0.0f);
	reload_view();
}

/*!
 * advance the point of view from a certain distance
 * @param step the distance
 */
void advance(double step) {
	X += step * lx;
	Y += step * ly;
	Z += step * lz;
	gluLookAt(X, Y, Z, X + lx, Y + ly, Z + lz, 0.0f, 1.0f, 0.0f);
	reload_view();
}

/*!
 * advance perpediculary to the direction looked
 * @param step the distance to advance
 */
void strafe(double step) {
	float lx2 = lx, lz2 = lz;
	applyRotation(PI / 2.0f, &lx2, &lz2);
	X += step * lx2;
	Z += step * lz2;
	gluLookAt(X, Y, Z, X + lx, Y + ly, Z + lz, 0.0f, 1.0f, 0.0f);
	reload_view();
}

/*!
 * the routine for handling key input
 * @param key the input key
 * @param x
 * @param y
 */
void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	//just to remove unused variables
	x = x;
	y = y;

	//case 'q':
	//case 'Q':
	//case 27:
	//	exit(0);
	//	break;
case '1':
	strafe(-5.f);
	break;
case '3':
	strafe(5.f);
	break;
case '4':
	rotate(-.1f);
	break;
case '6':
	rotate(.1f);
	break;
case '8':
	advance(5.f);
	break;
case '2':
	advance(-5.f);
	break;
case '+':
	Y += 10;
	reload_view();
	break;
case '-':
	Y -= 10;
	reload_view();
	break;

default:
	break;//Don't do anything if any other keys are pressed
	}
}
