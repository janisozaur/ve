#ifndef GLFORM_H
#define GLFORM_H

#include <QWidget>
#include <QVector3D>
#include <QMatrix4x4>

namespace Ui {
class GLForm;
}

class GLForm : public QWidget
{
    Q_OBJECT
    
public:
    explicit GLForm(QWidget *parent = 0);
    ~GLForm();

public slots:
    void setVector(QVector3D vec);
    void setMatrix(QMatrix4x4 mat);
    
private:
    Ui::GLForm *ui;
};

#endif // GLFORM_H
