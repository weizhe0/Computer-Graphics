/*
 TCG6223 Computer Graphics
 FIST, Multimedia University

 CGLab01.hpp

 Objective: Header File for Lab01 Demo Models / World

 Copyright (C)Ya-Ping Wong <ypwong@mmu.edu.my>

 This file (CGLab01.hpp) can be distributed to the students

 INSTRUCTIONS
 ============
 Please refer to CGLabmain.cpp for instructions

 SPECIAL NOTES
 =============
 * Try loading the high-polygons version of
   the Dragon model, it will be slow in slower PC.
 * Try out other models from http://graphics.stanford.edu/data/3Dscanrep/
   However, you will need to modify the file to comform to the
   format that is being used in this program.

 CHANGE LOG
 ==========
*/

#ifndef YP_CGLAB01_HPP
#define YP_CGLAB01_HPP

#include "CGLabmain.hpp"
#include <string>
#include <vector>

namespace CGLab01 {

class SimplePolygon
{
 public:
    void draw();
};

class SimpleTriangles
{
 public:
    void draw();
};

class SimpleBox
{
 public:
    void draw();
};

class SimpleTeapot
{
 public:
    void draw();
};

class SimpleBouncingBall
{
 public:
    SimpleBouncingBall();
    void draw();
    void tickTime(long int elapseTime);
 private:
    long int timetick;
    float vel0;
    float accel;
};

class MyModelLoader
{
 public:
    MyModelLoader()
    {
    }
    ~MyModelLoader()
    {
    }
    //load a model and scale it
    void load(string filename, float scale = 1.0);
    void draw();
 private:
    vector<GLfloat> vertices;
    vector<int> faces;
    GLuint stanforddragon; //for generating display list
};

//------------------------------------
//the main program will call methods from this class
class MyVirtualWorld
{
 public:
    SimplePolygon      simplepolygon;
    SimpleBox          simplebox;
    SimpleTriangles    simpletriangles;
    SimpleTeapot       simpleteapot;
    SimpleBouncingBall simplebouncingball;
    MyModelLoader      mymodelloader;

    long int timeold, timenew, elapseTime;

    void draw();

    void tickTime()
    {

     timenew    = glutGet(GLUT_ELAPSED_TIME);
     elapseTime = timenew - timeold;
     timeold    = timenew;

     simplebouncingball.tickTime(elapseTime);
    }

    //for any one-time only initialization of the
    //   virtual world before any rendering takes place
    //   BUT after OpenGL has been initialized
    void init()
    {
     glEnable(GL_LIGHTING);

     //Low-polygons dragon (5835 triangles)
     //mymodelloader.load("data/model_rose",100);

     //Try this:
     //High-polygons dragon (original model of Stanford Dragon)
     // (871414 triangles) will take some minutes for it to get loaded
     //mymodelloader.load("data/model_highpolygonstanforddragon.txt",100);

     //mymodelloader.load("data/model_rose.txt", 0.2);

     mymodelloader.load("data/model_shuttle.txt", 0.1);

     timeold = glutGet(GLUT_ELAPSED_TIME);

    }
};

}; //end of namespace CGLab01

#endif //YP_CGLAB01_HPP
