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
#include "CGLabMinecraft.hpp"

//IMPORTANT:
//  Change the namespace scope below corresponding to
//  to the lab number which you wish to use.
using CGLabMinecraft::MyVirtualWorld;

MyVirtualWorld myvirtualworld;

using namespace std;

MyWindow   window;
MyWorld    world;
MyViewer   viewer;
MySetting  setting;
MyAxis     worldaxis;

bool gameStarted = false;
int menuChoice = 0;
const int MENU_ITEM_COUNT = 4;
const int CHARACTER_COUNT = 3;
const char* CHARACTER_NAMES[CHARACTER_COUNT] = {"Creeper", "Enderman", "Blaze"};
int playerOneChoice = 0;
int playerTwoChoice = 1;

void drawMenuText(float x, float y, const string& text, void* font)
{
 glRasterPos2f(x, y);
 for (string::const_iterator it = text.begin(); it != text.end(); ++it)
 {
    glutBitmapCharacter(font, *it);
 }
}

int menuTextWidth(const string& text, void* font)
{
 int width = 0;
 for (string::const_iterator it = text.begin(); it != text.end(); ++it)
 {
    width += glutBitmapWidth(font, *it);
 }
 return width;
}

void drawCenteredMenuText(float centerX, float y, const string& text, void* font)
{
 drawMenuText(centerX - menuTextWidth(text, font) * 0.5f, y, text, font);
}

int wrapIndex(int value, int count)
{
 while (value < 0)
 {
    value += count;
 }
 return value % count;
}

void keepCharacterChoicesDifferent(int changedPlayer)
{
 if (playerOneChoice != playerTwoChoice)
 {
    return;
 }

 if (changedPlayer == 0)
 {
    playerTwoChoice = wrapIndex(playerTwoChoice + 1, CHARACTER_COUNT);
 }
 else
 {
    playerOneChoice = wrapIndex(playerOneChoice + 1, CHARACTER_COUNT);
 }
}

void changeCharacterChoice(int delta)
{
 if (menuChoice == 0)
 {
    playerOneChoice = wrapIndex(playerOneChoice + delta, CHARACTER_COUNT);
    keepCharacterChoicesDifferent(0);
 }
 else if (menuChoice == 1)
 {
    playerTwoChoice = wrapIndex(playerTwoChoice + delta, CHARACTER_COUNT);
    keepCharacterChoicesDifferent(1);
 }
}

void setHighlightedCharacter(int index)
{
 if (menuChoice == 0)
 {
    playerOneChoice = wrapIndex(index, CHARACTER_COUNT);
    keepCharacterChoicesDifferent(0);
 }
 else if (menuChoice == 1)
 {
    playerTwoChoice = wrapIndex(index, CHARACTER_COUNT);
    keepCharacterChoicesDifferent(1);
 }
}

void changeMenuSelection(int delta)
{
 menuChoice = wrapIndex(menuChoice + delta, MENU_ITEM_COUNT);
}

void drawMainMenu()
{
 glMatrixMode(GL_PROJECTION);
 glPushMatrix();
 glLoadIdentity();
 gluOrtho2D(0.0, window.width, 0.0, window.height);

 glMatrixMode(GL_MODELVIEW);
 glPushMatrix();
 glLoadIdentity();

 glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);
 glDisable(GL_LIGHTING);
 glDisable(GL_TEXTURE_2D);
 glDisable(GL_DEPTH_TEST);

 glColor3f(0.03f, 0.03f, 0.04f);
 glBegin(GL_QUADS);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(static_cast<float>(window.width), 0.0f);
    glVertex2f(static_cast<float>(window.width), static_cast<float>(window.height));
    glVertex2f(0.0f, static_cast<float>(window.height));
 glEnd();

 const float centerX = window.width * 0.5f;
 const float centerY = window.height * 0.5f;
 const float titleY = centerY + 150.0f;
 const float subtitleY = titleY - 42.0f;
 const float firstItemY = centerY + 28.0f;
 const float helpY = centerY - 230.0f;

 glColor3f(0.35f, 0.95f, 0.25f);
 drawCenteredMenuText(centerX, titleY, "MINECRAFT BATTLE", GLUT_BITMAP_TIMES_ROMAN_24);

 glColor3f(0.82f, 0.82f, 0.88f);
 drawCenteredMenuText(centerX, subtitleY, "Choose your players", GLUT_BITMAP_HELVETICA_18);

 const string items[MENU_ITEM_COUNT] =
 {
    string("P1 Character: ") + CHARACTER_NAMES[playerOneChoice],
    string("P2 Character: ") + CHARACTER_NAMES[playerTwoChoice],
    "Start Game",
    "Quit"
 };

 for (int i = 0; i < MENU_ITEM_COUNT; ++i)
 {
    const float y = firstItemY - i * 48.0f;
    string rowText = items[i];
    if (i < 2)
    {
       rowText += "        <  >";
    }

    const float rowX = centerX - menuTextWidth(rowText, GLUT_BITMAP_HELVETICA_18) * 0.5f;
    if (menuChoice == i)
    {
       glColor3f(0.72f, 0.15f, 1.0f);
       drawMenuText(rowX - 40.0f, y, ">", GLUT_BITMAP_HELVETICA_18);
       glColor3f(1.0f, 1.0f, 1.0f);
    }
    else
    {
       glColor3f(0.55f, 0.55f, 0.60f);
    }
    drawMenuText(rowX, y, items[i], GLUT_BITMAP_HELVETICA_18);

    if (i < 2)
    {
       glColor3f(0.42f, 0.62f, 0.90f);
       drawMenuText(rowX + menuTextWidth(items[i] + "        ", GLUT_BITMAP_HELVETICA_18),
                    y, "<  >", GLUT_BITMAP_HELVETICA_18);
    }
 }

 glColor3f(0.45f, 0.45f, 0.50f);
 drawCenteredMenuText(centerX, helpY,
                      "UP/DOWN select row    LEFT/RIGHT change character    ENTER confirm",
                      GLUT_BITMAP_HELVETICA_12);
 drawCenteredMenuText(centerX, helpY - 25.0f,
                      "P1: WASD + G/H      P2: Arrow Keys + 8/9",
                      GLUT_BITMAP_HELVETICA_12);

 glPopAttrib();
 glPopMatrix();
 glMatrixMode(GL_PROJECTION);
 glPopMatrix();
 glMatrixMode(GL_MODELVIEW);
}

void chooseMenuItem()
{
 if (menuChoice == 0 || menuChoice == 1)
 {
    changeCharacterChoice(1);
 }
 else if (menuChoice == 2)
 {
    myvirtualworld.setPlayerCharacters(playerOneChoice, playerTwoChoice);
    gameStarted = true;
 }
 else
 {
    exit(0);
 }
}

void resetGameplayView()
{
 world.rotateX  = 0.0;
 world.rotateY  = 0.0;
 world.rotateZ  = 0.0;
 world.posX     = 0.0;
 world.posY     = 0.0;
 world.posZ     = 0.0;
 world.scaleX   = 1.0;
 world.scaleY   = 1.0;
 world.scaleZ   = 1.0;
 myvirtualworld.resetView();
}

void returnToMainMenu()
{
 gameStarted = false;
 menuChoice = 0;
 resetGameplayView();
}

void myDisplayFunc(void)
{
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 if (!gameStarted)
 {
    drawMainMenu();
    glFlush();
    glutSwapBuffers();
    return;
 }

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

 viewer.aspectRatio = static_cast<GLdouble>(width) / static_cast<GLdouble>(height);
 myViewingInit();
}

void myKeyboardFunc(unsigned char key, int x, int y)
{
 if (!gameStarted)
 {
    switch (key)
    {
       case 13:
          chooseMenuItem();
          break;
       case 'w': case 'W':
          changeMenuSelection(-1);
          break;
       case 's': case 'S':
          changeMenuSelection(1);
          break;
       case 'a': case 'A':
          changeCharacterChoice(-1);
          break;
       case 'd': case 'D':
          changeCharacterChoice(1);
          break;
       case '1':
       case '2':
       case '3':
          setHighlightedCharacter(key - '1');
          break;
       case 'q': case 'Q':
       case 27:
          exit(0);
          break;
       default:
          break;
    }
    glutPostRedisplay();
    return;
 }

 if (key == 27)
 {
    returnToMainMenu();
    glutPostRedisplay();
    return;
 }

 if (myvirtualworld.handleKeyboard(key))
 {
    glutPostRedisplay();
    return;
 }

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
 }

 world.move(xinc, yinc, zinc);

 glutPostRedisplay();
}

void mySpecialFunc(int key, int x, int y)
{
 if (!gameStarted)
 {
    switch (key)
    {
       case GLUT_KEY_UP:
          changeMenuSelection(-1);
          break;
       case GLUT_KEY_DOWN:
          changeMenuSelection(1);
          break;
       case GLUT_KEY_LEFT:
          changeCharacterChoice(-1);
          break;
       case GLUT_KEY_RIGHT:
          changeCharacterChoice(1);
          break;
       default:
          break;
    }
    glutPostRedisplay();
    return;
 }

 if (myvirtualworld.handleSpecial(key))
 {
    glutPostRedisplay();
    return;
 }

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
    myvirtualworld.rotateView(0.0f, 0.0f, -xinc*0.5f);
 }
 if(setting.mouseLeftMode)
 {
    world.rotate(-yinc*0.5, xinc*0.5, 0.0f);
    myvirtualworld.rotateView(-yinc*0.5f, xinc*0.5f, 0.0f);
 }

 setting.mouseX = x;
 setting.mouseY = y;
 glutPostRedisplay();
}

void myDataInit()
{
 window.title = "Blind Boxes Collection - Minecraft Battle";
 window.posX = 100;
 window.posY = 100;
 window.width  = 1100;
 window.height = 720;

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
 viewer.eyeY    = 7.0;
 viewer.eyeZ    = 18.0;
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

 myvirtualworld.resetView();
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
 cout << "*             Blind Boxes Collection: Minecraft Battle          *\n";
 cout << "*****************************************************************\n";
 cout << "| Press:                                                        |\n";
 cout << "|   Menu: choose Creeper, Enderman, or Blaze before battle       |\n";
 cout << "|   P1: W/A/S/D move, G primary skill, H secondary skill         |\n";
 cout << "|   P2: Arrow keys move, 8 primary skill, 9 secondary skill      |\n";
 cout << "|   R                       => reset battle                     |\n";
 cout << "|   HOME                    => restore defaults                 |\n";
 cout << "|   ESC                     => menu in battle / exit on menu    |\n";
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
