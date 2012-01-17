#ifndef DISPLAYWIDGET_H
#define DISPLAYWIDGET_H

#include <QGLWidget>
#include <QVector3D>
#include <QMatrix4x4>

class DisplayWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit DisplayWidget(QWidget *parent = 0);

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    
signals:
    
public slots:
    void updateScene();
    void setVector(QVector3D vec);
    void setMatrix(const QMatrix4x4 &mat);

private:
    QVector3D mVec;
    QMatrix4x4 mMat;
    GLfloat *light_ambient, *light_ambient_position;
    GLfloat *whiteDiffuseLight, *blackAmbientLight, *whiteSpecularLight;
};

#endif // DISPLAYWIDGET_H
