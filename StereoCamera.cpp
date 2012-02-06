#include "StereoCamera.h"

#include <cmath>
#include <GL/gl.h>

void StereoCamera::beginEye(const Eye &e) const
{
	float left, right;

	float b = mA - mES2;
	float c = mA + mES2;

	left    = -(e == Left ? b : c) * mNearClippingDistance/mConvergence;
	right   =  (e == Left ? c : b) * mNearClippingDistance/mConvergence;

	// Set the Projection Matrix
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glFrustum(left, right, mBottom, mTop,
			  mNearClippingDistance, mFarClippingDistance);

	// Displace the world to right
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef((e == Left ? mES2 : -mES2), 0.0f, 0.0f);
}

void StereoCamera::finishEye() const
{
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

StereoCamera::StereoCamera(
	float Convergence,
	float EyeSeparation,
	float AspectRatio,
	float FOV,
	float NearClippingDistance,
	float FarClippingDistance)
{
	mConvergence            = Convergence;
	mEyeSeparation          = EyeSeparation;
	mAspectRatio            = AspectRatio;
	mFOV                    = FOV * M_PI / 180.0f;
	mNearClippingDistance   = NearClippingDistance;
	mFarClippingDistance    = FarClippingDistance;
	const float fovTan = tan(mFOV/2);
	mTop = mNearClippingDistance * fovTan;
	mBottom = -mTop;
	mA = mAspectRatio * fovTan * mConvergence;
	mES2 = mEyeSeparation/2;
}

float StereoCamera::fov() const
{
	return mFOV * M_1_PI * 180.0f;
}

float StereoCamera::aspect() const
{
	return mAspectRatio;
}

float StereoCamera::near() const
{
	return mNearClippingDistance;
}

float StereoCamera::far() const
{
	return mFarClippingDistance;
}

void StereoCamera::setAspectRatio(const float &ar)
{
	mAspectRatio            = ar;
	const float fovTan = tan(mFOV/2);
	mA = mAspectRatio * fovTan * mConvergence;
}
