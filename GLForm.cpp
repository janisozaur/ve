#include "GLForm.h"
#include "ui_GLForm.h"

GLForm::GLForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GLForm)
{
    ui->setupUi(this);
}

GLForm::~GLForm()
{
    delete ui;
}

void GLForm::setVector(QVector3D vec)
{
    ui->widget->setVector(vec);
}

void GLForm::setMatrix(QMatrix4x4 vec)
{
    ui->widget->setMatrix(vec);
}
