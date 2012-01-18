#ifndef STEREOCAMERA_H
#define STEREOCAMERA_H

class StereoCamera
{
public:
	StereoCamera(
		float Convergence,
		float EyeSeparation,
		float AspectRatio,
		float FOV,
		float NearClippingDistance,
		float FarClippingDistance
	);

	void applyLeftFrustum() const;
	void applyRightFrustum() const;

private:
	float mConvergence;
	float mEyeSeparation;
	float mAspectRatio;
	float mFOV;
	float mNearClippingDistance;
	float mFarClippingDistance;
};

#endif // STEREOCAMERA_H
