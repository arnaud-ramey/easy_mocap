/*
 * Drawing.cpp
 *
 *  Created on: Jan 24, 2010
 *      Author: arnaud
 *
 * drawing functions
 */

extern "C" {
#include "libs/glee/GLee.h"
#include "libs/glut/glut.h"
#include "libs/rtg/RTGlib.h"
}
#include "libs/bitmap/Bitmap.h"
#include "Textures.cpp"

#define  TEX_UNIT_1				GL_TEXTURE0_ARB
#define  TEX_UNIT_2				GL_TEXTURE1_ARB
//#define  DEVIL_TEX_ID 			2


/*!
 * the implementation of a man
 */
class Hero {
public:
	string filename;
	RTGFile *rtg_file;
	//	double scale;
	double x, y, z, angle_rad;

	Hero(string filename) {
		this->filename = filename;
		rtg_file = RTG_Parse(filename.c_str(), 1);
	}

	void setPos(double X, double Y, double Z, double Angle_rad) {
		x = X;
		y = Y;
		z = Z;
		angle_rad = Angle_rad;
	}
};

static void draw_cylinder(CvPoint3D32f origin, double rotation_radians,
		double diameter, double length) {
	glPushMatrix();
	// go to the origin
	glTranslatef(origin.x, origin.y, origin.z);
	// rotate the good angle
	glRotatef(90, 0, 1, 0);
	glRotatef(-rotation_radians * 180.f / PI, 1, 0, 0);
	// draw the cylinder
	GLUquadric *myQuad = gluNewQuadric();
	gluCylinder(myQuad, diameter, diameter, length, 10, 10);
	gluDeleteQuadric(myQuad);
	glPopMatrix();
}

/*!
 * draw the axis arrow in three colors
 */
static void draw_axes(void) {
	glPushMatrix();

	glTranslatef(-150, 0, 0);
	glLineWidth(2.0);
	int size = 20;

	glBegin(GL_LINES);
	glColor3f(1, 0, 0); // X axis is red.
	glVertex3f(0, 0, 0);
	glVertex3f(size, 0, 0);
	glColor3f(0, 1, 0); // Y axis is green.
	glVertex3f(0, 0, 0);
	glVertex3f(0, size, 0);
	glColor3f(0, 0, 1); // z axis is blue.
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, size);
	glEnd();

	glPopMatrix();
}

class HillLocation {
public:
	int x;
	int z;
	int deepness;
	int radius;
	float color;
};

class TreeLocation {
public:
	CvPoint3D32f origin;
	int height;
	int green_size;
	float color;
};

vector<HillLocation> hills;
vector<TreeLocation> trees;

static inline void draw_hill(HillLocation* h) {
	glPushMatrix();
	glTranslated(h->x, -h->deepness, h->z);
	glColor3f(0, h->color, 0); // green
	glutSolidSphere(h->radius, 20, 20);
	glPopMatrix();
}

static inline void draw_tree(TreeLocation* h) {
	glPushMatrix();
	glColor3f(.2, 0, 0); // red
	draw_cylinder(h->origin, PI / 2, h->height / 10, h->height);
	glTranslatef(h->origin.x, h->origin.y + h->height, h->origin.z);
	glColor3f(0, h->color, 0); // green
	glutSolidSphere(h->green_size, 20, 20);
	glPopMatrix();
}

static void draw_sky() {
	/* draw the sky */
	glClearColor(0.5f, 0.5f, 1.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Hero *dog;
static void drawRTG(RTGFile *file, bool useTextures, double x, double y,
		double z, double scale, double angle_rad);

static void draw_background() {
	// draw the ground -> grass
	glColor3f(0, 1, 0); // green
	int diam = 600;
	int y_floor = -110;
	load_texture("grass.bmp", TEX_UNIT_1);
	glBegin(GL_QUADS);
	glVertex3f(-diam, y_floor, -diam);
	glTexCoord2d(0, 0);
	glVertex3f(-diam, y_floor, +diam);
	glTexCoord2d(0, 1);
	glVertex3f(+diam, y_floor, +diam);
	glTexCoord2d(1, 1);
	glVertex3f(+diam, y_floor, -diam);
	glTexCoord2d(1, 0);
	glNormal3f(0, 1, 0);
	glEnd();
	unload_texture_unit(TEX_UNIT_1);

	// draw some random hills
	if (hills.size() == 0) {
		/* initialize random seed: */
		srand(time(NULL));

		for (int i = 0; i < 20; ++i) {
			HillLocation hill;
			hill.x = -diam + rand() % (2 * diam);
			hill.z = -diam + rand() % (2 * diam);
			hill.radius = 500 + rand() % 300;
			hill.deepness = hill.radius * (100 - rand() % 8) / 100.f;
			hill.deepness -= y_floor;
			hill.color = (50 + rand() % 50) / 100.f;
			hills.push_back(hill);
		}
		for (int i = 0; i < 5; ++i) {
			TreeLocation tree;
			tree.origin.x = -diam + rand() % (2 * diam);
			tree.origin.z = -diam + rand() % (2 * diam);
			tree.origin.y = y_floor;
			tree.height = 200 + rand() % 200;
			tree.green_size = tree.height * (30 + rand() % 20) / 100;
			tree.color = (rand() % 100) / 100.f;
			trees.push_back(tree);
		}
	}

	//		for (vector<HillLocation>::iterator it = hills.begin(); it < hills.end(); ++it)
	//		draw_hill(&(*it));
	for (vector<TreeLocation>::iterator it = trees.begin(); it < trees.end(); ++it)
		draw_tree(&(*it));

	// draw the dog
	if (!dog) {
		dog = new Hero("rtg/dog.rtg");
		int x = -400;
		int z = -60;
		dog->setPos(x, y_floor, z, 0);
	}
	drawRTG(dog->rtg_file, true, dog->x, dog->y, dog->z, 10, PI / 2);
}

static void setMaterial(RTGMaterial *material) {
	/* Sätt material här */
	if (material->texFile != NULL) {
		load_texture(material->texFile, TEX_UNIT_1);
	}
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, material->amb);
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, material->dif);
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, material->spc);
	//	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shn);
	//	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, material->em);

	glColor3fv(material->amb);
	//	glColor3f(1, 0, 0);
}

static inline void crossProduct(RTGVec3* a, RTGVec3* b, RTGVec3* c) {
	a->x = b->y * c->z - c->y * b->z;
	a->y = b->z * c->x - c->z * b->x;
	a->z = b->x * c->y - c->x * b->y;
}

static inline void diff(RTGVec3* ans, RTGVec3* b, RTGVec3* c) {
	ans->x = b->x - c->x;
	ans->y = b->y - c->y;
	ans->z = b->z - c->z;
}

static void drawObject(RTGFile *file, RTGObject *obj, bool useTextures) {
	unsigned int i, j;
	RTGVertex *v;
	RTGVec3 *normal;
	RTGVec3 *point;
	RTGVec2 *tex_coords;

	//	RTGVec3 *prev_point1 = NULL, *prev_point2 = NULL;
	//	RTGVec3 diff1, diff2;
	//	RTGVec3 normal2;

	if (useTextures)
		setMaterial(file->mat[obj->mat]);

	for (i = 0; i < obj->npoly; i++) {
		glBegin(GL_POLYGON);
		for (j = 0; j < obj->poly[i]->nvert; j++) {
			v = &obj->poly[i]->v[j];

			normal = &obj->norm[v->n];
			glNormal3f(normal->x, normal->y, normal->z);

			tex_coords = &obj->tex[v->t];
			glTexCoord2f(tex_coords->x, tex_coords->y);

			point = &obj->pos[v->p];
			glVertex3f(point->x, point->y, point->z);

			//			prev_point2 = prev_point1;
			//			prev_point1 = point;
		}
		glEnd();

		//		diff(&diff1, point, prev_point1);
		//		diff(&diff2, prev_point1, prev_point2);
		//		crossProduct(&normal2, &diff1, &diff2);
		//		glNormal3f(normal2.x, normal2.y, normal2.z);
	}

	unload_texture_unit(TEX_UNIT_1);
}

static void drawNode(RTGFile *file, RTGNode *node, bool useTextures) {
	unsigned int i;

	/* Om noden har geometri, rita den */
	if (node->obj) {
		drawObject(file, node->obj, useTextures);
	}
	/* Om noden har barn, rita dem */
	for (i = 0; i < node->nchildren; i++) {
		drawNode(file, node->children[i], useTextures);
	}

}

/*!
 *
 * @param file
 * @param useTextures
 * @param x
 * @param y
 * @param z
 * @param scale
 * @param angle
 */
static void drawRTG(RTGFile *file, bool useTextures, double x, double y,
		double z, double scale, double angle_rad) {
	unsigned int i;

	glPushMatrix();
	/* Applicera transformation här! */
	glTranslated(x, y, z);
	glScaled(scale, scale, scale);
	//	glRotated(angle, 0, 1, 0);

	// rotate the good angle
	//glRotatef(90, 0, 1, 0);
	glRotatef(angle_rad * 180.f / PI - 90.f, 0, 0, 1);

	/* Rita alla toppniv<8C>noder */
	for (i = 0; i < file->nnodes; i++) {
		drawNode(file, file->nodes[i], useTextures);
	}

	glPopMatrix();
}

//static inline void drawRTG(RTGFile *file, bool useTextures) {
//	drawRTG(file, useTextures, 0, 0, 0, 1, 0);
//}

/*!
 * where we store the heroes
 */
class Hero; // forward declaration
vector<Hero> heroes;

/*!
 * draw the hero
 */
static inline void drawHero(Hero* h, bool useTextures) {
	drawRTG(h->rtg_file, useTextures, h->x, h->y, h->z, 1.f, h->angle_rad);
}

/*!
 * find the index of the hero in the vector
 * @return -1 if not found
 */
static int find_hero_index(string filename) {
	int rep = 0;
	for (vector<Hero>::iterator it = heroes.begin(); it < heroes.end(); ++it) {
		if (it->filename.find(filename) != string::npos)
			return rep;
		++rep;
	}
	return -1;
}

/*!
 * draw the hero
 */
static Hero* get_hero_by_name(string filename) {
	int index = find_hero_index(filename);
	// load if needed
	if (index == -1) {
		// create filename
		ostringstream filename_full;
		filename_full << "rtg/" << filename;

		// display
		cout << "Hero missing ! '" << filename_full.str() << "'. Loading it !"
				<< endl;

		Hero h(filename_full.str());
		heroes.push_back(h);
		index = heroes.size() - 1;
	}
	return &heroes.at(index);
}

