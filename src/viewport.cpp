//------------------------------------------------------------------------------
///
/// \file       viewport.cpp 
/// \author     Cem Yuksel (www.cemyuksel.com)
/// \version    11.0
/// \date       November 6, 2017
///
/// \brief Example source for CS 6620 - University of Utah.
///
//------------------------------------------------------------------------------

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#include "viewport.h"
#include "scene.h"
#include "objects/objects.h"
#include "lights/lights.h"
#include "materials/materials.h"
#include "textures/texture.h"
#include "globalvar.h"
#include <stdlib.h>
#include <chrono>

#ifdef USE_GUI
# ifdef USE_GLUT
#  ifdef __APPLE__
#   include <GLUT/glut.h>
#  else
#   include <GL/glut.h>
#  endif
# else

#  include <GL/freeglut.h>

# endif
#endif

//------------------------------------------------------------------------------

enum Mode {
  MODE_READY,    // Ready to render
  MODE_RENDERING,  // Rendering the image
  MODE_RENDER_DONE  // Rendering is finished
};

enum ViewMode {
  VIEWMODE_OPENGL,
  VIEWMODE_IMAGE,
  VIEWMODE_Z,
  VIEWMODE_SAMPLECOUNT,
  VIEWMODE_IRRADCOMP,
};

enum MouseMode {
  MOUSEMODE_NONE,
  MOUSEMODE_DEBUG,
  MOUSEMODE_ROTATE,
};

#ifdef USE_GUI
static Mode mode = MODE_READY;    // Rendering mode
static ViewMode viewMode = VIEWMODE_OPENGL;  // Display mode
static MouseMode mouseMode = MOUSEMODE_NONE;  // Mouse mode
static int mouseX = 0, mouseY = 0;
static float viewAngle1 = 0, viewAngle2 = 0;
static GLuint viewTexture;
static int dofDrawCount = 0;
static Color3f *dofImage = NULL;
static Color3c *dofBuffer = NULL;
#define MAX_DOF_DRAW 32
#endif
//------------------------------------------------------------------------------

void GlutDisplay();

void GlutReshape(int w, int h);

void GlutIdle();

void GlutKeyboard(unsigned char key, int x, int y);

void GlutMouse(int button, int state, int x, int y);

void GlutMotion(int x, int y);

//------------------------------------------------------------------------------

void ShowViewport()
{
#ifdef USE_GUI
  int argc = 1;
  char argstr[] = "raytrace";
  char *argv = argstr;
  glutInit(&argc, &argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
  if (glutGet(GLUT_SCREEN_WIDTH) > 0 && glutGet(GLUT_SCREEN_HEIGHT) > 0) {
    glutInitWindowPosition(
        (glutGet(GLUT_SCREEN_WIDTH) - scene.camera.imgWidth) / 2,
        (glutGet(GLUT_SCREEN_HEIGHT) - scene.camera.imgHeight)
                               / 2);
  } else glutInitWindowPosition(50, 50);
  glutInitWindowSize(scene.camera.imgWidth, scene.camera.imgHeight);
  glutCreateWindow("Ray Tracer - CS 6620");
  glutDisplayFunc(GlutDisplay);
  glutReshapeFunc(GlutReshape);
  glutIdleFunc(GlutIdle);
  glutKeyboardFunc(GlutKeyboard);
  glutMouseFunc(GlutMouse);
  glutMotionFunc(GlutMotion);
  Color3f bg = scene.background.GetColor();
  glClearColor(bg.r, bg.g, bg.b, 0);
  glPointSize(3.0);
  glEnable(GL_CULL_FACE);
  float zero[] = {0, 0, 0, 0};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, zero);
  glEnable(GL_NORMALIZE);
  glLineWidth(2);

  if (scene.camera.depthOfField > 0) {
    dofBuffer = new Color3c[scene.camera.imgWidth * scene.camera.imgHeight];
    dofImage = new Color3f[scene.camera.imgWidth * scene.camera.imgHeight];
    memset(dofImage,
           0,
           scene.camera.imgWidth * scene.camera.imgHeight * sizeof(Color3f));
  }

  glGenTextures(1, &viewTexture);
  glBindTexture(GL_TEXTURE_2D, viewTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glutMainLoop();
#endif
}

//------------------------------------------------------------------------------

void GlutReshape(int w, int h)
{
#ifdef USE_GUI
  if (w != scene.camera.imgWidth || h != scene.camera.imgHeight) {
    glutReshapeWindow(scene.camera.imgWidth, scene.camera.imgHeight);
  } else {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float r = (float) w / float(h);
    gluPerspective(scene.camera.fovy, r, 0.02, 1000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
  }
#endif
}

//------------------------------------------------------------------------------
#ifdef USE_GUI

void DrawNode(Node *node)
{
  glPushMatrix();
  const Material *mtl = node->GetMaterial();
  if (mtl) mtl->SetViewportMaterial();
  Matrix3 tm = node->GetTransform();
  Point3 p = node->GetPosition();
  Point3 v0 = glm::column(tm, 0);
  Point3 v1 = glm::column(tm, 1);
  Point3 v2 = glm::column(tm, 2);
  float m[16] = {v0.x, v0.y, v0.z, 0,
                 v1.x, v1.y, v1.z, 0,
                 v2.x, v2.y, v2.z, 0,
                 p.x, p.y, p.z, 1};
  glMultMatrixf(m);
  Object *obj = node->GetNodeObj();
  if (obj) obj->ViewportDisplay(mtl);
  for (int i = 0; i < node->GetNumChild(); i++) {
    DrawNode(node->GetChild(i));
  }
  glPopMatrix();
}

#endif
//------------------------------------------------------------------------------
#ifdef USE_GUI

void DrawScene()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  const TextureMap *bgMap = scene.background.GetTexture();
  if (bgMap) {
    glDepthMask(GL_FALSE);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    Color3f c = scene.background.GetColor();
    glColor3f(c.r, c.g, c.b);
    if (bgMap->SetViewportTexture()) {
      glEnable(GL_TEXTURE_2D);
      glMatrixMode(GL_TEXTURE);
      Matrix3 tm = bgMap->GetInverseTransform();
      Point3 p = tm * bgMap->GetPosition();
      Point3 v0 = glm::column(tm, 0);
      Point3 v1 = glm::column(tm, 1);
      Point3 v2 = glm::column(tm, 2);
      float m[16] = {v0.x, v0.y, v0.z, 0,
                     v1.x, v1.y, v1.z, 0,
                     v2.x, v2.y, v2.z, 0,
                     -p.x, -p.y, -p.z, 1};
      glLoadMatrixf(m);
      glMatrixMode(GL_MODELVIEW);
    } else {
      glDisable(GL_TEXTURE_2D);
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0, 1);
    glVertex2f(-1, -1);
    glTexCoord2f(1, 1);
    glVertex2f(1, -1);
    glTexCoord2f(1, 0);
    glVertex2f(1, 1);
    glTexCoord2f(0, 0);
    glVertex2f(-1, 1);
    glEnd();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glDepthMask(GL_TRUE);
    glDisable(GL_TEXTURE_2D);
  }
  glEnable(GL_LIGHTING);
  glEnable(GL_DEPTH_TEST);
  glPushMatrix();
  Point3 p = scene.camera.pos;
  Point3 t = scene.camera.pos + scene.camera.dir * scene.camera.focalDistance;
  Point3 u = scene.camera.up;
  if (scene.camera.depthOfField > 0) {
    Point3 v = glm::cross(scene.camera.dir, scene.camera.up);
    float r = sqrtf(float(rand()) / RAND_MAX) * scene.camera.depthOfField;
    float a = float(M_PI) * 2.0f * float(rand()) / RAND_MAX;
    p += r * cosf(a) * v + r * sinf(a) * u;
  }
  gluLookAt(p.x, p.y, p.z, t.x, t.y, t.z, u.x, u.y, u.z);
  glRotatef(viewAngle1, 1, 0, 0);
  glRotatef(viewAngle2, 0, 0, 1);
  if (scene.lights.size() > 0) {
    for (unsigned int i = 0; i < scene.lights.size(); i++) {
      scene.lights[i]->SetViewportLight(i);
    }
  } else {
    float white[] = {1, 1, 1, 1};
    float black[] = {0, 0, 0, 0};
    Point4 p(scene.camera.pos, 1);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, black);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
    glLightfv(GL_LIGHT0, GL_SPECULAR, white);
    glLightfv(GL_LIGHT0, GL_POSITION, &p.x);
  }
  DrawNode(&(scene.rootNode));
  glPopMatrix();
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glDisable(GL_TEXTURE_2D);
}

#endif
//------------------------------------------------------------------------------
#ifdef USE_GUI

void DrawImage(void *data, GLenum type, GLenum format)
{
  glBindTexture(GL_TEXTURE_2D, viewTexture);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGB,
               renderImage.GetWidth(),
               renderImage.GetHeight(),
               0,
               format,
               type,
               data);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  glEnable(GL_TEXTURE_2D);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glColor3f(1, 1, 1);
  glBegin(GL_QUADS);
  glTexCoord2f(0, 1);
  glVertex2f(-1, -1);
  glTexCoord2f(1, 1);
  glVertex2f(1, -1);
  glTexCoord2f(1, 0);
  glVertex2f(1, 1);
  glTexCoord2f(0, 0);
  glVertex2f(-1, 1);
  glEnd();
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glDisable(GL_TEXTURE_2D);
}

#endif
//------------------------------------------------------------------------------
#ifdef USE_GUI

void DrawProgressBar(float done)
{
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glBegin(GL_LINES);
  glColor3f(1, 1, 1);
  glVertex2f(-1, -1);
  glVertex2f(done * 2 - 1, -1);
  glColor3f(0, 0, 0);
  glVertex2f(done * 2 - 1, -1);
  glVertex2f(1, -1);
  glEnd();
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

#endif
//------------------------------------------------------------------------------
#ifdef USE_GUI

void DrawRenderProgressBar()
{
  int rp = renderImage.GetNumRenderedPixels();
  int np = renderImage.GetWidth() * renderImage.GetHeight();
  if (rp >= np) return;
  float done = (float) rp / (float) np;
  DrawProgressBar(done);
}

#endif

//------------------------------------------------------------------------------
void GlutDisplay()
{
#ifdef USE_GUI
  switch (viewMode) {
    case VIEWMODE_OPENGL:
      if (dofImage) {
        if (dofDrawCount < MAX_DOF_DRAW) {
          DrawScene();
          glReadPixels(0,
                       0,
                       scene.camera.imgWidth,
                       scene.camera.imgHeight,
                       GL_RGB,
                       GL_UNSIGNED_BYTE,
                       dofBuffer);
          for (int i = 0, y = 0; y < scene.camera.imgHeight; y++) {
            int j = (scene.camera.imgHeight - y - 1) * scene.camera.imgWidth;
            for (int x = 0; x < scene.camera.imgWidth; x++, i++, j++) {
              dofImage[i] =
                  (dofImage[i] * float(dofDrawCount) + ToColor(dofBuffer[j]))
                      / float(dofDrawCount + 1);
            }
          }
          dofDrawCount++;
        }
        glClear(
            GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        DrawImage(dofImage, GL_FLOAT, GL_RGB);
        if (dofDrawCount < MAX_DOF_DRAW) {
          DrawProgressBar(float(dofDrawCount) / MAX_DOF_DRAW);
          glutPostRedisplay();
        }
      } else {
        DrawScene();
      }
      break;
    case VIEWMODE_IMAGE:
      DrawImage(renderImage.GetPixels(),
                GL_UNSIGNED_BYTE,
                GL_RGB);
      DrawRenderProgressBar();
      break;
    case VIEWMODE_Z:
      if (!renderImage.GetZBufferImage())
        renderImage.ComputeZBufferImage();
      DrawImage(renderImage.GetZBufferImage(), GL_UNSIGNED_BYTE, GL_LUMINANCE);
      break;
    case VIEWMODE_SAMPLECOUNT:
      if (!renderImage.GetSampleCountImage())
        renderImage.ComputeSampleCountImage();
      DrawImage(renderImage.GetSampleCountImage(),
                GL_UNSIGNED_BYTE,
                GL_LUMINANCE);
      break;
    case VIEWMODE_IRRADCOMP:
      if (renderImage.GetIrradianceComputationImage()) {
        DrawImage(renderImage.GetIrradianceComputationImage(),
                  GL_UNSIGNED_BYTE,
                  GL_LUMINANCE);
      }
      break;
  }
  glutSwapBuffers();
#endif
}

//------------------------------------------------------------------------------
void GlutIdle()
{
#ifdef USE_GUI
  if (mode == MODE_RENDERING) {
    if (renderImage.IsRenderDone()) {
      mode = MODE_RENDER_DONE;
      CleanRender();
    }
    glutPostRedisplay();
  }
  if (viewMode == VIEWMODE_IRRADCOMP) { glutPostRedisplay(); }
#endif
}

//------------------------------------------------------------------------------
void GlutKeyboard(unsigned char key, int x, int y)
{
#ifdef USE_GUI
  switch (key) {
    case 27:  // ESC
      KillRender();
      exit(0);
      break;
    case ' ':
      switch (mode) {
        case MODE_READY:mode = MODE_RENDERING;
          viewMode = VIEWMODE_IMAGE;
          if (dofImage) {
            Color3c *p = renderImage.GetPixels();
            for (int i = 0; i < scene.camera.imgWidth * scene.camera.imgHeight;
                 i++)
              p[i] = Color3c(dofImage[i]);
          } else {
            DrawScene();
            glReadPixels(0,
                         0,
                         renderImage.GetWidth(),
                         renderImage.GetHeight(),
                         GL_RGB,
                         GL_UNSIGNED_BYTE,
                         renderImage.GetPixels());
            {
              Color3c *c = renderImage.GetPixels();
              for (int y0 = 0, y1 = renderImage.GetHeight() - 1; y0 < y1;
                   y0++, y1--) {
                int i0 = y0 * renderImage.GetWidth();
                int i1 = y1 * renderImage.GetWidth();
                for (int x = 0; x < renderImage.GetWidth(); x++, i0++, i1++) {
                  Color3c t = c[i0];
                  c[i0] = c[i1];
                  c[i1] = t;
                }
              }
            }
          }
          BeginRender();
          break;
        case MODE_RENDERING:mode = MODE_READY;
          StopRender();
          glutPostRedisplay();
          break;
        case MODE_RENDER_DONE:mode = MODE_READY;
          viewMode = VIEWMODE_OPENGL;
          glutPostRedisplay();
          break;
      }
      break;
    case '1':viewAngle1 = viewAngle2 = 0;
      viewMode = VIEWMODE_OPENGL;
      glutPostRedisplay();
      break;
    case '2':viewMode = VIEWMODE_IMAGE;
      glutPostRedisplay();
      break;
    case '3':viewMode = VIEWMODE_Z;
      glutPostRedisplay();
      break;
    case '4':viewMode = VIEWMODE_SAMPLECOUNT;
      glutPostRedisplay();
      break;
    case '5':viewMode = VIEWMODE_IRRADCOMP;
      glutPostRedisplay();
      break;
  }
#endif
}
//------------------------------------------------------------------------------
#ifdef USE_GUI

void PrintPixelData(int x, int y)
{
  if (x < renderImage.GetWidth() && y < renderImage.GetHeight()) {
    Color3c *colors = renderImage.GetPixels();
    float *zbuffer = renderImage.GetZBuffer();
    int i = (y) * renderImage.GetWidth() + x;
    printf("Pixel [ %d, %d ] Color3c: %d, %d, %d   Z: %f\n",
           x, y, colors[i].r, colors[i].g, colors[i].b, zbuffer[i]);
  } else {
    printf("-- Invalid pixel (%d,%d) --\n", x, y);
  }
}

#endif

//------------------------------------------------------------------------------
void GlutMouse(int button, int state, int x, int y)
{
#ifdef USE_GUI
  if (state == GLUT_UP) {
    mouseMode = MOUSEMODE_NONE;
  } else {
    switch (button) {
      case GLUT_LEFT_BUTTON:mouseMode = MOUSEMODE_DEBUG;
        PrintPixelData(x, y);
        break;
      case GLUT_RIGHT_BUTTON:mouseMode = MOUSEMODE_ROTATE;
        mouseX = x;
        mouseY = y;
        break;
    }
  }
#endif
}

//------------------------------------------------------------------------------

void GlutMotion(int x, int y)
{
#ifdef USE_GUI
  switch (mouseMode) {
    case MOUSEMODE_DEBUG:PrintPixelData(x, y);
      break;
    case GLUT_RIGHT_BUTTON:viewAngle1 -= 0.2f * (mouseY - y);
      viewAngle2 -= 0.2f * (mouseX - x);
      mouseX = x;
      mouseY = y;
      glutPostRedisplay();
      break;
    default:break;
  }
#endif
}

//------------------------------------------------------------------------------
// Viewport Methods for various classes
//------------------------------------------------------------------------------
void Sphere::ViewportDisplay(const Material *mtl) const
{
#ifdef USE_GUI
  static GLUquadric *q = NULL;
  if (q == NULL) {
    q = gluNewQuadric();
    gluQuadricTexture(q, true);
  }
  gluSphere(q, 1, 50, 50);
#endif
}

void Plane::ViewportDisplay(const Material *mtl) const
{
#ifdef USE_GUI
  const int resolution = 32;
  float xyInc = 2.0f / resolution;
  float uvInc = 1.0f / resolution;
  glPushMatrix();
  glNormal3f(0, 0, 1);
  glBegin(GL_QUADS);
  float y1 = -1, y2 = xyInc - 1, v1 = 0, v2 = uvInc;
  for (int y = 0; y < resolution; y++) {
    float x1 = -1, x2 = xyInc - 1, u1 = 0, u2 = uvInc;
    for (int x = 0; x < resolution; x++) {
      glTexCoord2f(u1, v1);
      glVertex3f(x1, y1, 0);
      glTexCoord2f(u2, v1);
      glVertex3f(x2, y1, 0);
      glTexCoord2f(u2, v2);
      glVertex3f(x2, y2, 0);
      glTexCoord2f(u1, v2);
      glVertex3f(x1, y2, 0);
      x1 = x2;
      x2 += xyInc;
      u1 = u2;
      u2 += uvInc;
    }
    y1 = y2;
    y2 += xyInc;
    v1 = v2;
    v2 += uvInc;
  }
  glEnd();
  glPopMatrix();
#endif
}

void TriObj::ViewportDisplay(const Material *mtl) const
{
#ifdef USE_GUI
  unsigned int nextMtlID = 0;
  unsigned int nextMtlSwith = NF();
  if (mtl && NM() > 0) {
    mtl->SetViewportMaterial(0);
    nextMtlSwith = GetMaterialFaceCount(0);
    nextMtlID = 1;
  }
  glBegin(GL_TRIANGLES);
  for (unsigned int i = 0; i < NF(); i++) {
    while (i >= nextMtlSwith) {
      if (nextMtlID >= NM()) nextMtlSwith = NF();
      else {
        glEnd();
        nextMtlSwith += GetMaterialFaceCount(nextMtlID);
        mtl->SetViewportMaterial(nextMtlID);
        nextMtlID++;
        glBegin(GL_TRIANGLES);
      }
    }
    for (int j = 0; j < 3; j++) {
      if (HasTextureVertices()) glTexCoord3fv(&VT(FT(i).v[j]).x);
      if (HasNormals()) glNormal3fv(&VN(FN(i).v[j]).x);
      glVertex3fv(&V(F(i).v[j]).x);
    }
  }
  glEnd();
#endif
}

//------------------------------------------------------------------------------
void MtlBlinn_PathTracing::SetViewportMaterial(int subMtlID) const
{
#ifdef USE_GUI
  Color4f c;
  c = Color4f(diffuse.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, &c.r);
  c = Color4f(specular.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_SPECULAR, &c.r);
  glMaterialf(GL_FRONT, GL_SHININESS, specularGlossiness * 1.5f);
  c = Color4f(emission.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_EMISSION, &c.r);
  const TextureMap *dm = diffuse.GetTexture();
  if (dm && dm->SetViewportTexture()) {
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_TEXTURE);
    Matrix3 tm = dm->GetInverseTransform();
    Point3 p = tm * dm->GetPosition();
    Point3 v0 = glm::column(tm, 0);
    Point3 v1 = glm::column(tm, 1);
    Point3 v2 = glm::column(tm, 2);
    float m[16] = {v0.x, v0.y, v0.z, 0,
                   v1.x, v1.y, v1.z, 0,
                   v2.x, v2.y, v2.z, 0,
                   -p.x, -p.y, -p.z, 1};
    glLoadMatrixf(m);
    glMatrixMode(GL_MODELVIEW);
  } else {
    glDisable(GL_TEXTURE_2D);
  }
#endif
}

void MtlBlinn_MonteCarloGI::SetViewportMaterial(int subMtlID) const
{
#ifdef USE_GUI
  Color4f c;
  c = Color4f(diffuse.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, &c.r);
  c = Color4f(specular.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_SPECULAR, &c.r);
  glMaterialf(GL_FRONT, GL_SHININESS, glossiness * 1.5f);
  c = Color4f(emission.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_EMISSION, &c.r);
  const TextureMap *dm = diffuse.GetTexture();
  if (dm && dm->SetViewportTexture()) {
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_TEXTURE);
    Matrix3 tm = dm->GetInverseTransform();
    Point3 p = tm * dm->GetPosition();
    Point3 v0 = glm::column(tm, 0);
    Point3 v1 = glm::column(tm, 1);
    Point3 v2 = glm::column(tm, 2);
    float m[16] = {v0.x, v0.y, v0.z, 0,
                   v1.x, v1.y, v1.z, 0,
                   v2.x, v2.y, v2.z, 0,
                   -p.x, -p.y, -p.z, 1};
    glLoadMatrixf(m);
    glMatrixMode(GL_MODELVIEW);
  } else {
    glDisable(GL_TEXTURE_2D);
  }
#endif
}

void MtlBlinn_Basic::SetViewportMaterial(int subMtlID) const
{
#ifdef USE_GUI
  Color4f c;
  c = Color4f(diffuse.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, &c.r);
  c = Color4f(specular.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_SPECULAR, &c.r);
  glMaterialf(GL_FRONT, GL_SHININESS, glossiness * 1.5f);
  c = Color4f(emission.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_EMISSION, &c.r);
  const TextureMap *dm = diffuse.GetTexture();
  if (dm && dm->SetViewportTexture()) {
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_TEXTURE);
    Matrix3 tm = dm->GetInverseTransform();
    Point3 p = tm * dm->GetPosition();
    Point3 v0 = glm::column(tm, 0);
    Point3 v1 = glm::column(tm, 1);
    Point3 v2 = glm::column(tm, 2);
    float m[16] = {v0.x, v0.y, v0.z, 0,
                   v1.x, v1.y, v1.z, 0,
                   v2.x, v2.y, v2.z, 0,
                   -p.x, -p.y, -p.z, 1};
    glLoadMatrixf(m);
    glMatrixMode(GL_MODELVIEW);
  } else {
    glDisable(GL_TEXTURE_2D);
  }
#endif
}

void MtlPhong_Basic::SetViewportMaterial(int subMtlID) const
{
#ifdef USE_GUI
  Color4f c;
  c = Color4f(diffuse.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, &c.r);
  c = Color4f(specular.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_SPECULAR, &c.r);
  glMaterialf(GL_FRONT, GL_SHININESS, glossiness * 1.5f);
  c = Color4f(emission.GetColor(), 1.f);
  glMaterialfv(GL_FRONT, GL_EMISSION, &c.r);
  const TextureMap *dm = diffuse.GetTexture();
  if (dm && dm->SetViewportTexture()) {
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_TEXTURE);
    Matrix3 tm = dm->GetInverseTransform();
    Point3 p = tm * dm->GetPosition();
    Point3 v0 = glm::column(tm, 0);
    Point3 v1 = glm::column(tm, 1);
    Point3 v2 = glm::column(tm, 2);
    float m[16] = {v0.x, v0.y, v0.z, 0,
                   v1.x, v1.y, v1.z, 0,
                   v2.x, v2.y, v2.z, 0,
                   -p.x, -p.y, -p.z, 1};
    glLoadMatrixf(m);
    glMatrixMode(GL_MODELVIEW);
  } else {
    glDisable(GL_TEXTURE_2D);
  }
#endif
}

//------------------------------------------------------------------------------
bool TextureFile::SetViewportTexture() const
{
#ifdef USE_GUI
  if (viewportTextureID == 0) {
    std::vector<qaUCHAR> flip(width * height * 3, 0);
    for (int y = 0; y < height; ++y) {
      const auto dst_idx = 3 * ((height - y - 1) * width + 0);
      const auto src_idx = y * width + 0;
      std::memcpy(&flip[dst_idx],
                  &(data[src_idx].r),
                  3 * width * sizeof(qaUCHAR));
    }
    glGenTextures(1, &viewportTextureID);
    glBindTexture(GL_TEXTURE_2D, viewportTextureID);
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height,
                      GL_RGB, GL_UNSIGNED_BYTE, flip.data());
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  glBindTexture(GL_TEXTURE_2D, viewportTextureID);
  return true;
#else
  return false;
#endif

}

bool TextureChecker::SetViewportTexture() const
{
#ifdef USE_GUI
  if (viewportTextureID == 0) {
    const int texSize = 256;
    glGenTextures(1, &viewportTextureID);
    glBindTexture(GL_TEXTURE_2D, viewportTextureID);
    Color3c c[2] = {
        Color3c(color1 * 255.f),
        Color3c(color2 * 255.f)
    };
    Color3c *tex = new Color3c[texSize * texSize];
    for (int i = 0; i < texSize * texSize; i++) {
      int ix = (i % texSize) < 128 ? 0 : 1;
      if (i / 256 >= 128) ix = 1 - ix;
      tex[i] = c[ix];
    }
    gluBuild2DMipmaps(GL_TEXTURE_2D, 3, texSize, texSize,
                      GL_RGB, GL_UNSIGNED_BYTE, &tex[0].r);
    delete[] tex;
    glTexParameterf(GL_TEXTURE_2D,
                    GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  glBindTexture(GL_TEXTURE_2D, viewportTextureID);
  return true;
#else
  return false;
#endif
}

//------------------------------------------------------------------------------
void GenLight::SetViewportParam(int lightID,
                                Color4f ambient,
                                Color4f intensity,
                                Point4 pos) const
{
#ifdef USE_GUI
  glEnable(GL_LIGHT0 + lightID);
  glLightfv(GL_LIGHT0 + lightID, GL_AMBIENT, &ambient.r);
  glLightfv(GL_LIGHT0 + lightID, GL_DIFFUSE, &intensity.r);
  glLightfv(GL_LIGHT0 + lightID, GL_SPECULAR, &intensity.r);
  glLightfv(GL_LIGHT0 + lightID, GL_POSITION, &pos.x);
#endif
}
//------------------------------------------------------------------------------
