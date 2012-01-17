#ifndef FPSCOUNTER_H
#define FPSCOUNTER_H

#include <QVector>
#include <QTime>

class FpsCounter
{
public:
	FpsCounter();
	float getFps();
	float getFrameTime();
	void frameStart();
	void frameEnd();

private:
	QVector<int> mFrameTimes;
	QVector<int> mFpsValues;
	QTime mFrameTime;
	int mFpsTime;
	QTime mLastTime;
	int mFrameIndex;
	int mFpsIndex;
	int mFrameCount;
};

#endif // FPSCOUNTER_H
