#include "DisplayWidget.h"

#include <GL/glu.h>
#include <QTimer>
#include <GL/freeglut.h>
#include <QCoreApplication>
#include <cmath>

DisplayWidget::DisplayWidget(QWidget *parent) :
    QGLWidget(parent)
{
    QTimer *t = new QTimer(this);
    connect(t, SIGNAL(timeout()), this, SLOT(updateScene()));
    t->start(20);
}

void DisplayWidget::initializeGL()
{
    QCoreApplication *app = QCoreApplication::instance();
    int argc = 0;
    char *argv = NULL;
    glutInit(&argc, &argv);
    // Set up the rendering context, define display lists etc.:
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    light_ambient = new GLfloat[4];
    light_ambient[0] = 1.0;
    light_ambient[1] = 1.0;
    light_ambient[2] = 1.0;
    light_ambient[3] = 1.0;
    light_ambient_position = new GLfloat[4];
    light_ambient_position[0] = 2.0;
    light_ambient_position[1] = 2.0;
    light_ambient_position[2] = 2.0;
    light_ambient_position[3] = 1.0;
    whiteDiffuseLight = new GLfloat[3];
    whiteDiffuseLight[0] = 1.0;
    whiteDiffuseLight[1] = 1.0;
    whiteDiffuseLight[2] = 1.0;

    glLightfv(GL_LIGHT0, GL_POSITION, light_ambient_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDiffuseLight);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void DisplayWidget::resizeGL(int width, int height)
{
    // setup viewport, projection etc.:
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void DisplayWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    //gluLookAt(30, 30, 30, 0, 0, 0, -1, 1, -1);
    glTranslatef(0.0f, 0.0f, -6.0f);
    glMultMatrixd(mMat.constData());
    //glRotatef(std::acos(QVector3D::dotProduct(mMat.row(2).toVector3D(), QVector3D(0, 1, 0))) * 180 / M_PI * (mMat.row(2).z() > 0 ? 1 : -1), 1, 0, 0);
#ifdef FREEGLUT
    glutSolidTeapot(1);
#endif
    glBegin(GL_LINES);
        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(mVec.x(), mVec.y(), mVec.z());
        glColor3f(1, 1, 0);
        glVertex3f(-1, -1, -1);
        glVertex3f(0, 0, 0);
    glEnd();
}

void DisplayWidget::updateScene()
{
    updateGL();
}

void DisplayWidget::setVector(QVector3D vec)
{
    mVec = vec;
}

void DisplayWidget::setMatrix(const QMatrix4x4 &mat)
{
    mMat = mat;
}
