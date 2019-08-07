#include <iostream>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "main.h"
#include "string"

int const FPS = 60;

void keyPressed(unsigned char key, int xx, int yy) {
  if (key == 27) exit(0);
  Game::Instance()->keyStatus[key] = true;
}

void keyUp(unsigned char key, int x, int y) {
  Game::Instance()->keyStatus[key] = false;
}

void specialPressed(int key, int x, int y) {
  Game::Instance()->specialStatus[key] = true;
}

void specialUp(int key, int x, int y) {
  Game::Instance()->specialStatus[key] = false;
}

void renderScene(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();
  Game::Instance()->refresh();
  glutSwapBuffers();
}

void Initialization() {
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_COLOR_MATERIAL);
  // glEnable(0x809D);
  // glEnable(GL_MULTISAMPLE_ARB);
  // glEnable(GL_MULTISAMPLE);
  // glEnable(GL_POINT_SMOOTH);
  // glEnable(GL_LINE_SMOOTH);
  // glEnable(GL_POLYGON_SMOOTH);
  // glEnable(GL_SMOOTH);
}

void changeSize(int w, int h) {
  if (h == 0) h = 1;
  float ratio = w * 1.0 / h;
  Game::Instance()->setSize(w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, w, h);
  gluPerspective(60.0f, ratio, 0.1f, 100000.0f);
  glMatrixMode(GL_MODELVIEW);
  glClearColor(0.3398, 0.9803, 1.0, 0);
  Initialization();
}

void getKeyStatus(int value) {
  Game::Instance()->moveCar();
  Game::Instance()->changeCamera();
  glutTimerFunc(1000 / FPS, getKeyStatus, 0);
}

void refreshScene(int value) {
  glutPostRedisplay();
  glutTimerFunc(1000 / FPS, refreshScene, 1);
}

int main(int argc, char **argv) {
  string mapName = "map.txt";
  if (argc > 1 && argv[1][0] == '-' && argv[1][1] == 'm') mapName = argv[2];
  Game::Instance(mapName);

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(800, 600);
  glutCreateWindow("My 3D Car");

  Initialization();

  glutDisplayFunc(renderScene);
  glutReshapeFunc(changeSize);
  // glutIdleFunc(renderScene);

  glutKeyboardFunc(keyPressed);
  glutKeyboardUpFunc(keyUp);
  glutSpecialFunc(specialPressed);
  glutSpecialUpFunc(specialUp);
  glutTimerFunc(1000 / FPS, getKeyStatus, 0);
  glutTimerFunc(1000 / FPS, refreshScene, 1);

  glutMainLoop();

  return 1;
}