#include "MainWindow.h"

#include <QtGui/QApplication>
#include <QDebug>
#include <cv.h>
#include <highgui.h>
#include <vector>

using namespace cv;
using namespace std;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return a.exec();
}
