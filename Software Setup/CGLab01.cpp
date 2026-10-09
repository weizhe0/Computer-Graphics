/*
 TCG6223 Computer Graphics
 FIST, Multimedia University

 CGLab01.cpp

 Objective: Lab01 Demo Models / World

 Copyright (C) Ya-Ping Wong <ypwong@mmu.edu.my>

 This file (CGLab01.cpp) can be distributed to the students

 INSTRUCTIONS
 ============
 Please refer to CGLabmain.cpp for instructions

 CHANGE LOG
 ==========
*/

#include <GL/glut.h>
#include <string>
#include <fstream>
#include "CGLab01.hpp"

using namespace CGLab01;

void computeSurfaceNormal( GLfloat vertex1[],
                           GLfloat vertex2[],
                           GLfloat vertex3[],
                           GLfloat normal[])
{
 GLfloat v1[3], v2[3];
 v1[0] = vertex3[0] - vertex2[0];
 v1[1] = vertex3[1] - vertex2[1];
 v1[2] = vertex3[2] - vertex2[2];
 v2[0] = vertex1[0] - vertex2[0];
 v2[1] = vertex1[1] - vertex2[1];
 v2[2] = vertex1[2] - vertex2[2];
 normal[0] = v1[1]*v2[2] - v1[2]*v2[1];
 normal[1] = v1[2]*v2[0] - v1[0]*v2[2];
 normal[2] = v1[0]*v2[1] - v1[1]*v2[0];
 GLfloat magnitude = sqrt(   normal[0]*normal[0]
                           + normal[1]*normal[1]
                           + normal[2]*normal[2] );
 normal[0] /= magnitude;
 normal[1] /= magnitude;
 normal[2] /= magnitude;
}

void SimplePolygon::draw()
{
 glDisable(GL_CULL_FACE);

 glBegin(GL_QUADS);
    //bottom
    glColor3f ( 1.0f,  0.0f,  0.0f );
    //glNormal3f( 0.0f, -1.0f,  0.0f );
	glVertex3f( 2.0f, -5.0f, -5.0f );
	glVertex3f( 2.0f, -5.0f,  5.0f );
	glVertex3f(-2.0f, -5.0f,  5.0f );
	glVertex3f(-2.0f, -5.0f, -5.0f );
    //right
    glColor3f ( 0.0f,  1.0f,  0.0f );
    //glNormal3f(0.96f,-0.29f,  0.0f );
	glVertex3f( 5.0f,  5.0f, -2.0f );
	glVertex3f( 5.0f,  5.0f,  2.0f );
	glVertex3f( 2.0f, -5.0f,  5.0f );
	glVertex3f( 2.0f, -5.0f, -5.0f );
    //top
	glColor3f ( 0.0f,  0.0f,  1.0f );
    //glNormal3f( 0.0f,  1.0f,  0.0f );
	glVertex3f(-5.0f,  5.0f, -2.0f );
	glVertex3f(-5.0f,  5.0f,  2.0f );
	glVertex3f( 5.0f,  5.0f,  2.0f );
    glVertex3f( 5.0f,  5.0f, -2.0f );
    //left
    glColor3f ( 1.0f,  1.0f,  0.0f );
    //glNormal3f(-0.96f,-0.29f, 0.0f );
    glVertex3f(-2.0f, -5.0f, -5.0f );
	glVertex3f(-2.0f, -5.0f,  5.0f );
	glVertex3f(-5.0f,  5.0f,  2.0f );
	glVertex3f(-5.0f,  5.0f, -2.0f );
 glEnd();

 glEnable(GL_CULL_FACE);
}

void SimpleTriangles::draw()
{
//glDisable(GL_CULL_FACE); //disable face culling
glFrontFace(GL_CW);
glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 0.0f, 1.0f );  glVertex3f( 0.0f,  5.0f,  0.0f );
    glColor3f(1.0f, 0.0f, 0.0f );  glVertex3f(-5.0f, -5.0f,  5.0f );
    glColor3f(0.0f, 1.0f, 0.0f );  glVertex3f( 5.0f, -5.0f,  5.0f );

    glColor3f(0.0f, 0.0f, 1.0f );  glVertex3f( 0.0f,  5.0f,  0.0f );
    glColor3f(0.0f, 1.0f, 0.0f );  glVertex3f( 5.0f, -5.0f,  5.0f );
    glColor3f(0.0f, 1.0f, 1.0f );  glVertex3f( 5.0f, -5.0f, -5.0f );

    glColor3f(0.0f, 0.0f, 1.0f );  glVertex3f( 0.0f,  5.0f,  0.0f );
    glColor3f(0.0f, 1.0f, 1.0f );  glVertex3f( 5.0f, -5.0f, -5.0f );
    glColor3f(1.0f, 1.0f, 0.0f );  glVertex3f(-5.0f, -5.0f, -5.0f );

    glColor3f(0.0f, 0.0f, 1.0f );  glVertex3f( 0.0f,  5.0f,  0.0f );
    glColor3f(1.0f, 1.0f, 0.0f );  glVertex3f(-5.0f, -5.0f, -5.0f );
    glColor3f(1.0f, 0.0f, 0.0f );  glVertex3f(-5.0f, -5.0f,  5.0f );
glEnd();

//glEnable(GL_CULL_FACE);  //restore back the default setting
}






void SimpleBox::draw()
{
 static GLfloat vertices[8][3] =
    { {-5.0f, -5.0f,  5.0f},     // vertex 0
      { 5.0f, -5.0f,  5.0f},     // vertex 1
      { 5.0f, -5.0f, -5.0f},     // vertex 2
      {-5.0f, -5.0f, -5.0f},     // vertex 3
      {-5.0f,  5.0f,  5.0f},     // vertex 4
      { 5.0f,  5.0f,  5.0f},     // vertex 5
      { 5.0f,  5.0f, -5.0f},     // vertex 6
      {-5.0f,  5.0f, -5.0f},     // vertex 7
    };

 static int faces[6*4] =
	{
		3, 0, 4, 7,		//face 0
		3, 2, 1, 0,		//face 1
		2, 3, 7, 6,		//face 2
		1, 2, 6, 5,		//face 3
		4, 5, 6, 7,		//face 4
		0, 1, 5, 4		//face 5
	};

 static GLfloat normals[6][3] =
	{	{ -1.0f,  0.0f,  0.0f},   // face 0
		{  0.0f, -1.0f,  0.0f},   // face 1
		{  0.0f,  0.0f, -1.0f},   // face 2
		{  1.0f,  0.0f,  0.0f},   // face 3
		{  0.0f,  1.0f,  0.0f},   // face 4
		{  0.0f,  0.0f,  1.0f},   // face 5
	};

 int numberOfFaces = 6;
 int verticesPerFace = 4;

 int n = 0;
 glColor3f(1.0f, 0.0f, 1.0f);
 for (int j=0; j<numberOfFaces; j++)
 {
    glBegin(GL_POLYGON);
       glNormal3fv(normals[j]);
       for (int k=0; k<verticesPerFace; k++)
       {
          glVertex3fv(vertices[ faces[n++] ]);
       }
    glEnd();
 }
}

void SimpleTeapot::draw()
{
 glColor3f(1.0f, 1.0f, 0.0f );
 glFrontFace(GL_CW);
 glutSolidTeapot(5);
 glFrontFace(GL_CCW);
}

SimpleBouncingBall::SimpleBouncingBall()
{
 timetick = 0;
 vel0  =  0.1;
 accel = -0.0005;
}

void SimpleBouncingBall::draw()
{
 float y = vel0*timetick + 0.5*accel*timetick*timetick;
 if (y < 0.0)
 {
    y = 0.0;
    timetick=0;
 }

 glColor3f(0.0f, 0.0f, 1.0f );
 glTranslatef(0.0f, y, 0.0f);
 glutSolidSphere(2.0,16,16);
 glutPostRedisplay();
}

void SimpleBouncingBall::tickTime(long int elapseTime)
{
 timetick += elapseTime;
};

void MyModelLoader::load(string filename, float scale)
{
 int numberOfVertices;
 int numberOfFaces;
 ifstream fin;
 fin.open(filename.c_str());

 if (fin.good())
 {
    fin >> numberOfVertices;
    fin >> numberOfFaces;
    int varraysize = numberOfVertices*3;
    vertices.reserve(varraysize);
    faces.reserve(numberOfFaces*4); //this is just the minimum

    float coord;
    for (int i=0; i<varraysize && fin.good(); i++)
    {
       fin >> coord;
       vertices.push_back(coord*scale);
    }
    int vcount,index;
    for (int i=0; i<numberOfFaces && fin.good(); ++i)
    {
       fin >> vcount;
       faces.push_back(vcount);
       for (int j=0; j<vcount && fin.good(); ++j)
       {
          fin >> index;
          faces.push_back(index);
       }
    }
    fin.close();
 }

 vector<int>::iterator itr;
 GLfloat normal[3];

 stanforddragon = glGenLists(1);
 glNewList(stanforddragon, GL_COMPILE);
    glColor3f(1.0f, 0.8f, 0.4f);
    int vcount;  //number of vertices for a particular polygon
    for (itr = faces.begin(); itr!=faces.end();)
    {
       vcount = *itr;
       itr++;
       int i1, i2, i3;
       i1 = (*itr)     * 3;
       i2 = (*(itr+1)) * 3;
       i3 = (*(itr+2)) * 3;
       computeSurfaceNormal( &vertices[i1],&vertices[i2],&vertices[i3],
                             normal );
       glBegin(GL_POLYGON);
          glNormal3fv(normal);
          for (int k=0; k<vcount; k++)
          {
             glVertex3fv( &vertices[(*itr)*3] );
             itr++;
          }
       glEnd();
    }
 glEndList();
}

void MyModelLoader::draw()
{
 glCallList(stanforddragon);
}

void MyVirtualWorld::draw()
{
 glPushMatrix();
    glTranslatef(-10.0f, 0.0f, 0.0f);
    simplepolygon.draw();
 glPopMatrix();

 glPushMatrix();
    glTranslatef( 0.0f, -3.0f, 0.0f);
    glScalef( 1.0f, 0.2f, 1.0f);
    simplebox.draw();
 glPopMatrix();

 glPushMatrix();
    glTranslatef(  0.0f, 0.0f, -10.0f);
    simpletriangles.draw();
 glPopMatrix();

 glPushMatrix();
    glTranslatef(  0.0f, 0.0f,  10.0f);
    simpleteapot.draw();
 glPopMatrix();

 glPushMatrix();
    simplebouncingball.draw();
 glPopMatrix();

 glPushMatrix();
    glTranslatef( 15.0f, -10.0f, 0.0f);
    mymodelloader.draw();
 glPopMatrix();
}
