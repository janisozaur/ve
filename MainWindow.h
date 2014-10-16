#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSemaphore>

class BallTrackingThread;
class MoveThread;
class PSMoveForm;
class GLForm;
class Displayer;
class WiiMarkerTracker;
class SpeechRecognition;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void showImage();

private slots:
	void on_pushButton_clicked();

	void on_setLowerPushButton_clicked();

	void on_setUpperPushButton_clicked();

	void on_erosionSlider_valueChanged(int value);

	void on_dilutionSlider_valueChanged(int value);

	void on_moveConnectPushButton_clicked();

	void on_scanPushButton_clicked();

	void releaseWiiSemaphore();
	void releaseMoveSemaphore();

private:
	void connectSignals();

	Ui::MainWindow *ui;
	BallTrackingThread *mBallTracker;
	MoveThread *mMoveThread;
	PSMoveForm *mMoveForm;
	WiiMarkerTracker *mWMT;
	GLForm *mGLForm;
	Displayer *mDisplayer;
	QSemaphore mWiiSemaphore;
	QSemaphore mMoveSemaphore;
	SpeechRecognition *mSpeech;
};

#endif // MAINWINDOW_H
