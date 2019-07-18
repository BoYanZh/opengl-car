#include <gl/glut.h>

void Initialization() {
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  GLfloat lightSpecular[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat lightPosition[] = {0.5, 0.5, 4.0, 0.0};

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //指定混合函数
  glShadeModel(GL_SMOOTH);

  glMaterialfv(GL_FRONT, GL_SPECULAR, lightSpecular);
  glMaterialf(GL_FRONT, GL_SHININESS, 100.0);
  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

  glEnable(GL_BLEND);           //启用混合状态
  glEnable(GL_LIGHTING);        //启用光照
  glEnable(GL_LIGHT0);          //打开光源0
  glEnable(GL_DEPTH_TEST);      //启用深度检测
  glEnable(GL_COLOR_MATERIAL);  //材质跟踪当前绘图色
}

void OnDisplay(void) {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //双缓冲机制

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glPushMatrix();  //装载
  {
    glTranslatef(0.0f, 0.0f, -3.0f);
    glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    glutSolidTorus(1.0f, 2.0f, 30.0f, 30.0f);  //绘制圆环
  }
  glPopMatrix();  //装出

  glPushMatrix();
  {
    glTranslatef(1.0f, 1.0f, 3.0f);
    glColor4f(0.0f, 1.0f, 0.0f, 0.5);
    glutSolidSphere(2.0f, 30.0f, 30.0f);  //绘制球体
  }
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-1, -1, 4);
  glColor4f(0.0f, 0.0f, 1.0f, 0.5);
  glBegin(GL_QUADS);  //绘制四边形
  glVertex3f(0, 0, 0);
  glVertex3f(5, 0, 0);
  glVertex3f(5, 5, 0);
  glVertex3f(0, 5, 0);
  glEnd();
  glPopMatrix();

  glPushMatrix();
  glColor4f(0.0f, 1.0f, 1.0f, 0.5);
  glTranslatef(-1, -1, 5);
  glRotatef(60, 0, 0, 1);
  glBegin(GL_QUADS);
  glVertex3f(0, 0, 0);
  glVertex3f(5, 0, 0);
  glVertex3f(5, 5, 0);
  glVertex3f(0, 5, 0);
  glEnd();
  glPopMatrix();

  glutSwapBuffers();
}

void OnReShape(int w, int h) {
  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);  //将当前矩阵指定为投影模式
  glLoadIdentity();

  if (h != 0) {
    GLfloat aspect = GLfloat(w) / GLfloat(h);

    if (w < h) {
      glOrtho(-6.0f, 6.0f, -6.0f * aspect, 6.0f * aspect, -6.0f,
              6.0f);  //三维正交投影
    } else {
      glOrtho(-6.0f / aspect, 6.0f / aspect, -6.0f, 6.0f, -6.0f, 6.0f);
    }
  }
  glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(600, 480);
  glutCreateWindow("OpenGL透明");

  glutReshapeFunc(OnReShape);
  glutDisplayFunc(OnDisplay);

  Initialization();
  glutMainLoop();  //主程序循环
}