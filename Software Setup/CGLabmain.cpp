/*
 TCG6223 Computer Graphics
 FIST, Multimedia University

 CGLabmain.cpp

 Objective: Main Program for Lab01 to Lab11

 NOTE: For Lab12 and Lab13, a modified version (CGLab12and13main.cpp)
       will be provided.

 Copyright (C) by Ya-Ping Wong <ypwong@mmu.edu.my>

 This file (CGLabmain.cpp) can be distributed to the students

 INSTRUCTIONS
 ============
 How to compile and run:
 * For each of the lab, you need the files below:
   a. CGLabmain.cpp => The file you are reading now
   b. CGLabmain.hpp => Header file to be used with CGLabmain.cpp
   c. CGLabxx.cpp => Program for lab number 'xx', 'xx' being the lab
                     number such as 01, 03 ... 10, 11
   d. CGLabxx.hpp => Header file to be used with CGLabxx.cpp
 * Make sure you are including the correct include file
   in CGLabmain.cpp (this file) such as:
      #include "CGLabxx.hpp"   where 'xx' is the lab number
 * Make sure gl.h, glu.h and glut.h are in the 'include' path
 * Make sure opengl32.dll, glu32.dll and glut32.dll are in the 'system32' path
 * Make sure libopengl32.a, libglu32.a and libglut32.a are in the
   'lib' path and included in your project file if you are using an IDE
 * To compile using a command line using the GCC, type the command as below:
      g++ CGLabmain.cpp CGLabxx.cpp -o CGLabxx.exe -lopengl32 -lglu32 -lglut32
      [replace 'xx' with the lab number such as 01, 03 ... 10, 11]

 How to modify:
 =============
 * All the user-defined drawing functions are called from
   the draw() function of class MyVirtualWorld which
   must be declared in CGLabxx.hpp
 * myvirtualworld is declared in CGLabmain.cpp as an instance
   of class MyVirtualWorld
 * MyVirtualWorld must implement the following member functions:
   a. draw()     => will be called by myDisplayFunc() of CGLabmain.cpp
   b. tickTime() => will be called by myDisplayFunc() of CGLabmain.cpp
                    for animation purposes
   c. init()     => will be called by myInit() of CGLabmain.cpp
                    to carry up one-time setup/initialization before
                    any rendering BUT after OpenGL has been initialized
 * All classes and variables for each lab are defined in their own
   namespace to avoid names clashing of variable and type names.
   Specifically each CGLabxx.hpp declared and defined a MyVirtualWorld
   class, thus in CGLabmain.cpp, you need to specify from which
   namespace the MyVirtualWorld that you wish to use. Thus, you will
   need to modify the line (in the beginning of this program):
      CGLabxx::MyVirtualWorld myvirtualworld;
   and change the 'xx' to the lab number that you wish to use.

 SPECIAL NOTES
 =============
 * This main program is only for Lab01, Lab03 to Lab11
   (Lab02 uses Lab01 material).
   For Lab12 and Lab13, use CGLab12and13main.cpp

 TO DO
 =====
*/

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <cmath>
#include <GL/glut.h>

#include "CGLabmain.hpp"

//Ideally, you should include only the files that you wish
//  to use, however, all of them are included here just for
//  convenience.
#include "CGLab01.hpp"

//IMPORTANT:
//  Change the namespace scope below corresponding to
//  to the lab number which you wish to use.
using CGLab01::MyVirtualWorld;

MyVirtualWorld myvirtualworld;

using namespace std;

MyWindow   window;
MyWorld    world;
MyViewer   viewer;
MySetting  setting;
MyAxis     worldaxis;

void myDisplayFunc(void)
{
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 glPushMatrix();

    glTranslatef(world.posX, world.posY, world.posZ);
    glRotatef(world.rotateX, 1.0f, 0.0f, 0.0f);
    glRotatef(world.rotateY, 0.0f, 1.0f, 0.0f);
    glRotatef(world.rotateZ, 0.0f, 0.0f, 1.0f);
    glScalef(world.scaleX, world.scaleY, world.scaleZ);

    worldaxis.draw();

    myvirtualworld.draw();

 glPopMatrix();

 glFlush();   // send any buffered output to be rendered
 glutSwapBuffers();

 myvirtualworld.tickTime(); //tick the clock
 glutPostRedisplay();//force openGL to call myDisplayFunc() again
}

void myReshapeFunc(int width, int height)
{
 window.width  = width;
 window.height = height;
 glViewport(0, 0, width, height);
}

void myKeyboardFunc(unsigned char key, int x, int y)
{
 GLfloat xinc,yinc,zinc;
 xinc = yinc = zinc = 0.0;
 switch (key)
 {
    case 'a': case 'A': xinc = -setting.posInc;  break;
    case 'd': case 'D': xinc =  setting.posInc;  break;
    case 'q': case 'Q': yinc = -setting.posInc;  break;
    case 'e': case 'E': yinc =  setting.posInc;  break;
    case 'w': case 'W': zinc = -setting.posInc;  break;
    case 's': case 'S': zinc =  setting.posInc;  break;

    case 27  : exit(1); break;
 }

 world.move(xinc, yinc, zinc);

 glutPostRedisplay();
}

void mySpecialFunc(int key, int x, int y)
{
 switch (key)
 {
    case GLUT_KEY_DOWN  : world.rotateX -= setting.angleInc;  break;
    case GLUT_KEY_UP    : world.rotateX += setting.angleInc;  break;
    case GLUT_KEY_LEFT  : world.rotateY -= setting.angleInc;  break;
    case GLUT_KEY_RIGHT : world.rotateY += setting.angleInc;  break;
    case GLUT_KEY_HOME  : myDataInit(); break;
 	case GLUT_KEY_F1    : setting.shadingMode = !setting.shadingMode;
                          if (setting.shadingMode)
                          	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                            else
	                          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                          break;
	case GLUT_KEY_F2    : worldaxis.toggle();
	                      break;
	case GLUT_KEY_F3    : GLboolean lightingIsOn;
                          glGetBooleanv(GL_LIGHTING, &lightingIsOn);
                          if (lightingIsOn==GL_TRUE)
                             glDisable(GL_LIGHTING);
                             else  glEnable(GL_LIGHTING);
                          break;
 }
 glutPostRedisplay();
}

void myMouseFunc(int button, int state, int x, int y)
{
 y = window.height - y;
 switch (button)
 {
    case GLUT_RIGHT_BUTTON:
       if (state==GLUT_DOWN && !setting.mouseRightMode)
       {
          setting.mouseX = x;
          setting.mouseY = y;
          setting.mouseRightMode = true;
       }
       if (state==GLUT_UP && setting.mouseRightMode)
       {
          setting.mouseRightMode = false;
       }
       break;
    case GLUT_LEFT_BUTTON:
       if (state==GLUT_DOWN && !setting.mouseLeftMode)
       {
          setting.mouseX = x;
          setting.mouseY = y;
          setting.mouseLeftMode = true;
       }
       if (state==GLUT_UP &&  setting.mouseLeftMode)
       {
          setting.mouseLeftMode = false;
       }
       break;
 }
}

void myMotionFunc(int x, int y)
{
 y = window.height - y;
 GLint xinc = x - setting.mouseX;
 GLint yinc = y - setting.mouseY;

 if(setting.mouseRightMode)
 {
    world.rotate(0.0f, 0.0f, -xinc*0.5);
 }
 if(setting.mouseLeftMode)
 {
    world.rotate(-yinc*0.5, xinc*0.5, 0.0f);
 }

 setting.mouseX = x;
 setting.mouseY = y;
 glutPostRedisplay();
}

void myDataInit()
{
 window.title = "TCG6223 Computer Graphics";
 window.posX = 100;
 window.posY = 100;
 window.width  = 800;
 window.height = 500;

 world.rotateX  = 0.0;
 world.rotateY  = 0.0;
 world.rotateZ  = 0.0;
 world.posX     = 0.0;
 world.posY     = 0.0;
 world.posZ     = 0.0;
 world.scaleX   = 1.0;
 world.scaleY   = 1.0;
 world.scaleZ   = 1.0;

 viewer.eyeX    = 0.0;
 viewer.eyeY    = 0.0;
 viewer.eyeZ    = 40.0;
 viewer.centerX = 0.0;
 viewer.centerY = 0.0;
 viewer.centerZ = 0.0;
 viewer.upX     = 0.0;
 viewer.upY     = 1.0;
 viewer.upZ     = 0.0;
 viewer.zNear   = 0.1;
 viewer.zFar    = 500.0;
 viewer.fieldOfView = 60.0;
 viewer.aspectRatio = static_cast<GLdouble> (window.width) / window.height;

 setting.posInc   = 1.0;
 setting.angleInc = 2.0;
 setting.mouseX   = 0;
 setting.mouseY   = 0;

 setting.mouseRightMode = false;
 setting.mouseLeftMode = false;

 setting.shadingMode = true;
}

void myViewingInit()
{
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluPerspective(viewer.fieldOfView,
                viewer.aspectRatio,
                viewer.zNear,
                viewer.zFar);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 gluLookAt(viewer.eyeX,   viewer.eyeY,   viewer.eyeZ,
           viewer.centerX,viewer.centerY,viewer.centerZ,
           viewer.upX,    viewer.upY,    viewer.upZ );
}

void myLightingInit()
{
 static GLfloat  ambient[] = { 0.0f,  0.0f,  0.0f, 1.0f };
 static GLfloat  diffuse[] = { 1.0f,  1.0f,  1.0f, 1.0f };
 static GLfloat specular[] = { 1.0f,  1.0f,  1.0f, 1.0f };
 static GLfloat  specref[] = { 1.0f,  1.0f,  1.0f, 1.0f };
 static GLfloat position[] = {10.0f, 10.0f, 10.0f, 1.0f };
  short shininess = 128;

 glDisable(GL_LIGHTING);
 glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
 glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
 glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
 glLightfv(GL_LIGHT0, GL_POSITION, position);
 glEnable(GL_LIGHT0);

 glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
 glEnable(GL_COLOR_MATERIAL);

 glMaterialfv(GL_FRONT, GL_SPECULAR, specref);
 glMateriali(GL_FRONT, GL_SHININESS, shininess);

 glEnable(GL_NORMALIZE);
}

void myInit()
{
 myDataInit();

 glutInitDisplayMode( GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
 glutInitWindowPosition(window.posX, window.posY); // Set top-left position
 glutInitWindowSize(window.width, window.height); //Set width and height
 glutCreateWindow(window.title.c_str());// Create display window

 glutDisplayFunc(myDisplayFunc);  // Specify the display callback function
 glutReshapeFunc(myReshapeFunc);
 glutKeyboardFunc(myKeyboardFunc);
 glutSpecialFunc(mySpecialFunc);
 glutMotionFunc(myMotionFunc);
 glutMouseFunc(myMouseFunc);

 glPointSize(4.0);
 glEnable(GL_DEPTH_TEST);
 glDepthFunc(GL_LESS);
 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 glFrontFace(GL_CCW);
 glShadeModel (GL_SMOOTH);
 glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

 glEnable(GL_CULL_FACE);

 myViewingInit();

 myLightingInit();

 myvirtualworld.init();
}

void myWelcome()
{
 cout << "*****************************************************************\n";
 cout << "*                   TCG6223 Computer Graphics                   *\n";
 cout << "*                  FIST, Multimedia University                  *\n";
 cout << "*****************************************************************\n";
 cout << "| Press:                                                        |\n";
 cout << "|   <a>,<d>,<w>,<s>,<q>,<e> => move world                       |\n";
 cout << "|   <arrows>                => rotate world                     |\n";
 cout << "|   HOME                    => restore defaults                 |\n";
 cout << "|   ESC                     => exit                             |\n";
 cout << "|                                                               |\n";
 cout << "|   F1                      => toggle shading / wire-frame mode |\n";
 cout << "|   F2                      => toggle rendering of axes         |\n";
 cout << "|   F3                      => toggle lighting on / off         |\n";
 cout << "|                                                               |\n";
 cout << "| Mouse (Left Drag or Right Drag) => rotate world               |\n";
 cout << "|                                                               |\n";
 cout << "*****************************************************************\n";
 cout << "|                      H A V E   F U N  !!!                     |\n";
 cout << "*****************************************************************\n";
}

//--------------------------------------------------------------------
int main(int argc, char **argv)
{
 glutInit(&argc, argv);

 myWelcome();

 myInit();

 glutMainLoop(); // Display everything and wait
}
//--------------------------------------------------------------------
