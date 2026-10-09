/*
 TCG6223 Computer Graphics
 FIST, Multimedia University

 CGLabmain.hpp

 Objective: Header File for Main Program (for ALL Labs)

 Copyright (C)Ya-Ping Wong <ypwong@mmu.edu.my>

 This file (CGLabmain.hpp) can be distributed to the students

 INSTRUCTIONS
 ============
 Please refer to CGLabmain.cpp for instructions

 SPECIAL NOTES
 =============
 * This header file will be used for ALL Labs (Lab01 to Lab12) and
   you do not need to modify anything in this file.

 CHANGE LOG
 ==========
*/

#ifndef YP_CGLABMAIN_HPP
#define YP_CGLABMAIN_HPP

#include <string>
#include <cmath>
#include <iostream>
#include <GL/glut.h>

using namespace std;

void myDisplayFunc(void);
void myReshapeFunc(int width, int height);
void myKeyboardFunc(unsigned char key, int x, int y);
void mySpecialFunc(int key, int x, int y);
void myMouseFunc(int button, int state, int x, int y);
void myMotionFunc(int x, int y);
void myTimerFunc(int value);

void myDataInit();
void myViewingInit();
void myInit();
void myWelcome();
void myLightingInit();

static GLfloat myblack[]   = { 0.0f, 0.0f, 0.0f, 1.0f };
static GLfloat mywhite[]   = { 1.0f, 1.0f, 1.0f, 1.0f };
static GLfloat myred[]     = { 1.0f, 0.0f, 0.0f, 1.0f };
static GLfloat mygreen[]   = { 0.0f, 1.0f, 0.0f, 1.0f };
static GLfloat myblue[]    = { 0.0f, 0.0f, 1.0f, 1.0f };
static GLfloat mycyan[]    = { 0.0f, 1.0f, 1.0f, 1.0f };
static GLfloat mymagenta[] = { 1.0f, 0.0f, 1.0f, 1.0f };
static GLfloat myyellow[]  = { 1.0f, 1.0f, 0.0f, 1.0f };

static GLfloat mygray10Percent[] = { 0.1f, 0.1f, 0.1f, 1.0f };
static GLfloat mygray20Percent[] = { 0.2f, 0.2f, 0.2f, 1.0f };
static GLfloat mygray30Percent[] = { 0.3f, 0.3f, 0.3f, 1.0f };

//***********************************
struct MyWindow
{
 string title;
 GLint posX;
 GLint posY;
 GLint width;
 GLint height;
};

struct MyWorld
{
 GLdouble rotateX, rotateY, rotateZ;
 GLdouble posX, posY, posZ;
 GLdouble scaleX, scaleY, scaleZ;
 void move(GLfloat xinc, GLfloat yinc, GLfloat zinc)
 {
  posX += xinc;
  posY += yinc;
  posZ += zinc;
 }
 void rotate(GLfloat xinc, GLfloat yinc, GLfloat zinc)
 {
  rotateX += xinc;
  rotateY += yinc;
  rotateZ += zinc;
 }
};

struct MyViewer
{
 GLdouble eyeX, eyeY, eyeZ;
 GLdouble centerX, centerY, centerZ;
 GLdouble upX, upY, upZ;

 GLdouble zNear, zFar;   // The distance to the near/far clipping plane
 GLdouble fieldOfView;   // Angle at Y direction
 GLdouble aspectRatio;   // width / height
};

struct MySetting
{
 GLdouble posInc;
 GLdouble angleInc;
 int mouseX, mouseY;
 bool mouseRightMode;
 bool mouseLeftMode;

 bool shadingMode;
};

class MyAxis
{
 private:
    GLfloat xlen, ylen, zlen;
    GLfloat xred, xgreen, xblue;
    GLfloat yred, ygreen, yblue;
    GLfloat zred, zgreen, zblue;
    GLint    linestipplerepeatfactor;
    GLfloat  linewidth;
    GLushort linestipplepattern;
    bool     shoulddraw;
 public:
    MyAxis()
    {
     setXColor(1.0f, 0.0f, 0.0f);
     setYColor(0.0f, 1.0f, 0.0f);
     setZColor(0.0f, 0.0f, 1.0f);
     setLength(20.0f, 20.0f, 20.0f);
     setLineStipple(1, 0xffff);
     setLineWidth(1);
     shoulddraw = true;
    }

    void setLength(GLfloat xLen, GLfloat yLen, GLfloat zLen)
       { xlen = xLen; ylen = yLen; zlen = zLen; }
    void setXColor(GLfloat r, GLfloat g, GLfloat b)
       { xred = r; xgreen = g; xblue = b; }
    void setYColor(GLfloat r, GLfloat g, GLfloat b)
       { yred = r; ygreen = g; yblue = b; }
    void setZColor(GLfloat r, GLfloat g, GLfloat b)
       { zred = r; zgreen = g; zblue = b; }
    void setLineWidth(GLfloat lineWidth)
       { linewidth = lineWidth; }
    void setLineStipple(GLint repeatfactor, GLushort pattern)
       { linestipplerepeatfactor = repeatfactor, linestipplepattern = pattern; }

    void draw()
    {
     if (!shoulddraw) return;

     GLboolean lightingIsOn;
     glGetBooleanv(GL_LIGHTING, &lightingIsOn);
     glDisable(GL_LIGHTING);

     GLint oldlinestipplerepeatfactor;
     GLint oldlinestipplepattern;
     GLfloat  oldlinewidth;
     GLboolean stippleIsOn;
     glGetIntegerv(GL_LINE_STIPPLE_REPEAT, &oldlinestipplerepeatfactor);
     glGetIntegerv(GL_LINE_STIPPLE_PATTERN, &oldlinestipplepattern);
     glGetFloatv(GL_LINE_WIDTH, &oldlinewidth);
     glGetBooleanv(GL_LINE_STIPPLE,&stippleIsOn);

     glEnable(GL_LINE_STIPPLE);
     glLineWidth(linewidth);
     glLineStipple(linestipplerepeatfactor, linestipplepattern);
     glBegin(GL_LINES);
        //x-axis
        glColor3f(xred, xgreen, xblue);

        glVertex3f( 0.0, 0.0, 0.0);
        glVertex3f(xlen, 0.0, 0.0);
        //x-axis arrow
        glVertex3f(    xlen, 0.0,  0.0);
        glVertex3f(0.9*xlen, 0.0,  0.05*xlen);
        glVertex3f(    xlen, 0.0,  0.0);
        glVertex3f(0.9*xlen, 0.0, -0.05*xlen);

        //y-axis
        glColor3f(yred, ygreen, yblue);
        glVertex3f(0.0,  0.0, 0.0);
        glVertex3f(0.0, ylen, 0.0);
        //y-axis arrow
        glVertex3f(       0.0,     ylen, 0.0);
        glVertex3f( 0.05*ylen, 0.9*ylen, 0.0);
        glVertex3f(       0.0,     ylen, 0.0);
        glVertex3f(-0.05*ylen, 0.9*ylen, 0.0);

        //z-axis
        glColor3f(zred, zgreen, zblue);
        glVertex3f(0.0, 0.0,  0.0);
        glVertex3f(0.0, 0.0, zlen);
        //z-axis arrow
        glVertex3f(0.0, 0.0, zlen);
        glVertex3f( 0.05*zlen, 0.0, 0.9*zlen);
        glVertex3f(       0.0, 0.0,     zlen);
        glVertex3f(-0.05*zlen, 0.0, 0.9*zlen);
     glEnd();

     glLineWidth(oldlinewidth);
     glLineStipple(oldlinestipplerepeatfactor, oldlinestipplepattern);

     if (stippleIsOn==GL_FALSE)
        glDisable(GL_LINE_STIPPLE);

     if (lightingIsOn==GL_TRUE)
        glEnable(GL_LIGHTING);
    } //void draw()

    void toggle()
    {
     shoulddraw = !shoulddraw;
    }
    void enable()
    {
     shoulddraw = true;
    }
    void disable()
    {
     shoulddraw = false;
    }
};

#endif //YP_CGLABMAIN_HPP
