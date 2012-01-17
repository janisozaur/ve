#include "FPSCounter.h"

FpsCounter::FpsCounter() :
	mFrameTimes(QVector<int>(10)),
	mFpsValues(QVector<int>(10)),
	mFpsTime(0),
	mFrameIndex(0),
	mFpsIndex(0),
	mFrameCount(0)
{
	mFrameTime.start();
}

float FpsCounter::getFps()
{
	float sum = 0;
	foreach (int item, mFpsValues) {
		sum += item;
	}
	return sum / mFpsValues.count();
}

float FpsCounter::getFrameTime()
{
	int sum = 0;
	foreach (int item, mFrameTimes) {
		sum += item;
	}
	return sum / (float)mFrameTimes.count();
}

void FpsCounter::frameStart()
{
	mFpsTime += mFrameTime.restart();
	mFrameCount++;
	if (mFpsTime >= 1000) {
		mFpsValues[mFpsIndex] = mFrameCount;
		mFpsIndex = (mFpsIndex + 1) % mFpsValues.count();
		mFrameCount = 0;
		mFpsTime = 0;
	}
}

void FpsCounter::frameEnd()
{
	mFrameTimes[mFrameIndex] = mFrameTime.elapsed();
	mFrameIndex = (mFrameIndex + 1) % mFrameTimes.count();
}
