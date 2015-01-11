/*
 * BodyShower.cpp
 *
 *  Created on: Jan 12, 2010
 *      Author: arnaud
 */

/*! the position of the camera */
float X = 300.f, Y = -0.f, Z = 300.f;

/*! the direction of the look */
float lx = -1.0f, ly = 0.0f, lz = -1.0f;

/*! the params of the window */
int width, height;

/*! colors */
float colorRed[] = { 1.0f, 0.0f, 0.0f, 1.0f };
float colorBlue[] = { 0.0f, 0.0f, 1.0f, 1.0f };
float colorGreen[] = { 0.0f, 1.0f, 0.0f, 1.0f };
float colorWhite[] = { 0.0f, 1.0f, 0.0f, 1.0f };

#include "Drawing.cpp"
#include "Movement.cpp"
#include "ScreenShot.cpp"

Human* body;

void set_lights() {
    // Create light components
    GLfloat ambientLight[] = { 0.7f, 0.7f, 0.7f, 0.7f };
    GLfloat diffuseLight[] = { 1.f, 1.f, 1., 1.0f };
    GLfloat specularLight[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    double rayon = body->torso()->length * 3;
    GLfloat position[] = { rayon * cos(body->t / 100), 1.0f, rayon * sin(
                           body->t / 100), 1.0f };

    // draw the light
    //	glDisable(GL_LIGHTING);
    //	glPushMatrix();
    //	glTranslatef(position[0], position[1], position[2]);
    //	glColor3f(1, 0, 0);
    //	glutSolidSphere(rayon / 10, 15, 15);
    //	glPopMatrix();

    // Assign created components to GL_LIGHT0
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);

    /* set a light */
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);

    //	glutSolidTeapot(100);
}

void draw_body() {
    for (vector<Member*>::iterator member_it = body->members.begin(); member_it
         < body->members.end(); ++member_it) {
        /* set color */
        //	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, colorGreen);
        glColor3f(1, 1, 1); /// white

        Member *m = *member_it;
        //		m->display();

        CvPoint3D32f pos = m->get_absolute_position();
        if (m->type_of_drawin == DRAWN_CYLINDER)
            // draw a cylinder
            draw_cylinder(pos, m->get_absolute_rotation_rad(), m->length / 20,
                          m->length);
        else if (m->type_of_drawin == DRAWN_RTG) {
            Hero* h = get_hero_by_name(m->rtg_filename);
            //h->setPos(pos.x, pos.y, pos.z, m->get_absolute_rotation_rad());
            drawRTG(h->rtg_file, true, pos.x, pos.y, pos.z, m->rtg_scaling,
                    m->get_absolute_rotation_rad());
        }
    }
}

void draw_scene() {
    //	cout << "draw_scene()" << endl;
    cout << '^';

    //	draw_cylinder(cvPoint3D32f(0,0,0), 0, .1, 2);
    //	draw_cylinder(cvPoint3D32f(0,0,0), PI / 2, .05, 3);
    //	draw_cylinder(cvPoint3D32f(1,1,0), PI / 4, .05, 4);

    draw_sky();
    set_lights();
    if (body->torso()->type_of_drawin == DRAWN_RTG)
        draw_background();
    draw_axes();

    draw_body();

    if (SAVE_IMAGES)
        screenShot();
}

void display() {
    //	cout << "display()" << endl;
    //	double timer = (double) cvGetTickCount();
    //	double freq = (double) cvGetTickFrequency() * 1000.;

    // incr time
    body->t += .1;

    /* change angle */
    if (body->opengl_make_dancing) {
        body->head()->angle = PI + PI / 8 * cos(body->t / 8);
        body->arm_left()->angle = PI / 4 - PI / 8 * cos(body->t / 20);
        body->forearm_left()->angle = PI / 8 + PI / 8 * cos(body->t / 7);
        body->arm_right()->angle = PI / 8 * cos(body->t / 20);
        body->shin_right()->angle = PI / 4 + PI / 8 * cos(body->t / 12);
        body->leg_right()->angle = PI / 4 + PI / 8 * cos(body->t / 12);
    }

    glEnable(GL_DEPTH_TEST);

    draw_scene();

    glutSwapBuffers();

    //	double time = ((double) cvGetTickCount() - timer) / freq;
    //	cout << "Time for OpenGL rendering :" << time << endl;
}

void myExit() {
    cout << endl << endl << "myExit()" << endl;
    if (SAVE_IMAGES)
        screenShot_clean();
}

void start_bodyshower(Human* vb, int w /*= DEFAULT_OPENGL_WIDTH*/, int h /*= DEFAULT_OPENGL_HEIGHT*/) {
    printf("Keys:\n");
    printf("'8' : camera go forward\n");
    printf("'2' : camera go backward\n");
    printf("'1' : camera strafe left\n");
    printf("'3' : camera strafe right\n");
    printf("'4' : camera rotate left\n");
    printf("'6' : camera rotate right\n");
    printf("'+' : camera go higher\n");
    printf("'-' : camera go lower\n");
    printf("'q' : quit\n");
    printf("\n");

    //	glutInit(&argc, argv);
    int i = 0;
    glutInit(&i, NULL);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
    width = w;
    height = h;
    glutInitWindowSize(width, height);
    glutCreateWindow("easy_mocap");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(display);

    //	readTexture("textures/broodWarrior.bmp", DEVIL_TEX_ID);

    body = vb;

    atexit(myExit);
    glutMainLoop();
}

