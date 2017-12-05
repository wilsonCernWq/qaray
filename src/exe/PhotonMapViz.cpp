//-------------------------------------------------------------------------------
///
/// \author  Cem Yuksel (www.cemyuksel.com)
/// \date    December 1, 2015
///
/// \brief   Photon map visualization tool
///
//-------------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <GL/glut.h>

#define WINDOW_SIZE 800

float viewAngle1 = -60, viewAngle2 = -120, viewTransZ = -200;
int mouseMode;
int mouseX, mouseY;
bool showPhotonColor = false;

enum MouseModes {
  MOUSE_MODE_NONE,
  MOUSE_MODE_VIEW_ROTATE,
  MOUSE_MODE_VIEW_ZOOM,
};

//-------------------------------------------------------------------------------

struct Photon {
  float pos[3];
  float power;
  unsigned char color[3];
  unsigned char
      planeAndDirZ;  // splitting plane for kd-tree and one bit for determining the z direction
  short dirX, dirY;   // photon direction
};

int numPhotons = 0;
Photon *photons = NULL;

//-------------------------------------------------------------------------------

void Display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  glPushMatrix();

  // Set view
  glTranslatef(0, 0, viewTransZ);
  glRotatef(viewAngle1, 1, 0, 0);
  glRotatef(viewAngle2, 0, 0, 1);

  // Draw photons
  glColor3f(1, 1, 1);
  glDrawArrays(GL_POINTS, 0, numPhotons);

  const float lineSize = 100;
  glBegin(GL_LINES);
  glColor3f(1, 0, 0);
  glVertex3f(0, 0, 0);
  glVertex3f(lineSize, 0, 0);
  glColor3f(0, 1, 0);
  glVertex3f(0, 0, 0);
  glVertex3f(0, lineSize, 0);
  glColor3f(0, 0, 1);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 0, lineSize);
  glEnd();

  glPopMatrix();

  glutSwapBuffers();
}

//-------------------------------------------------------------------------------

void Reshape(int w, int h)
{
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  float r = (float) w / float(h);
  gluPerspective(60, r, 0.02, 1000.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

//-------------------------------------------------------------------------------

void Mouse(int button, int state, int x, int y)
{
  mouseX = x;
  mouseY = y;
  if (state == GLUT_DOWN) {
    switch (button) {
      case GLUT_LEFT_BUTTON: mouseMode = MOUSE_MODE_VIEW_ROTATE;
        break;
      case GLUT_RIGHT_BUTTON: mouseMode = MOUSE_MODE_VIEW_ZOOM;
        break;
    }
  } else {
    mouseMode = MOUSE_MODE_NONE;
  }
  glutPostRedisplay();
}

//-------------------------------------------------------------------------------

#define VIEW_ROTATE_INC 0.2f
#define VIEW_ZOOM_INC 0.5f

void MouseMove(int x, int y)
{

  switch (mouseMode) {
    case MOUSE_MODE_VIEW_ROTATE: viewAngle1 -= VIEW_ROTATE_INC * (mouseY - y);
      viewAngle2 -= VIEW_ROTATE_INC * (mouseX - x);
      break;
    case MOUSE_MODE_VIEW_ZOOM: viewTransZ += VIEW_ZOOM_INC * (mouseY - y);
      break;
  }

  mouseX = x;
  mouseY = y;

  glutPostRedisplay();
}

//-------------------------------------------------------------------------------

void Keyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case 27:    // ESC
      exit(0);
      break;
    case ' ': showPhotonColor = !showPhotonColor;
      if (showPhotonColor) {
        glEnableClientState(GL_COLOR_ARRAY);
      } else {
        glDisableClientState(GL_COLOR_ARRAY);
      }
      glutPostRedisplay();
      break;
  }
}

//-------------------------------------------------------------------------------

int main(int argc, char **argv)
{
  glutInit(&argc, argv);

  char defaultFileName[] = "photonmap.dat";
  char *fname = defaultFileName;
  if (argc > 1) {
    fname = argv[1];
  }


  /////////////////////////////////////////////////////////////////////////////////
  // Read the photon map
  /////////////////////////////////////////////////////////////////////////////////
  FILE *fp = fopen(fname, "rb");
  if (fp == NULL) {
    printf("ERROR: Cannot open file \"%s\".\n", fname);
    return -1;
  }
  int n = 0;
  Photon buffer;
  for (; !feof(fp); n++) {
    fread(&buffer, sizeof(Photon), 1, fp);
  }
  n--;

  if (n <= 0) {
    printf("ERROR: No photons found.\n");
  } else {
    photons = new Photon[n];
    rewind(fp);
    int np = fread(photons, sizeof(Photon), n, fp);
    numPhotons = np;
    printf("%d photons read.\n", np);
  }
  fclose(fp);
  if (n <= 0) return -2;
  /////////////////////////////////////////////////////////////////////////////////


  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  if (glutGet(GLUT_SCREEN_WIDTH) > 0 && glutGet(GLUT_SCREEN_HEIGHT) > 0) {
    glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - WINDOW_SIZE) / 2,
                           (glutGet(GLUT_SCREEN_HEIGHT) - WINDOW_SIZE) / 2);
  } else glutInitWindowPosition(50, 50);
  glutInitWindowSize(WINDOW_SIZE, WINDOW_SIZE);

  glutCreateWindow("Photon Map Viz");
  glutDisplayFunc(Display);
  glutReshapeFunc(Reshape);
  glutMouseFunc(Mouse);
  glutMotionFunc(MouseMove);
  glutKeyboardFunc(Keyboard);

  glClearColor(0, 0, 0, 0);
  glPointSize(1.0);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, sizeof(Photon), photons[0].pos);
  if (showPhotonColor) glEnableClientState(GL_COLOR_ARRAY);
  glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(Photon), photons[0].color);

  glutMainLoop();

  delete[] photons;

  return 0;
}

//-------------------------------------------------------------------------------
