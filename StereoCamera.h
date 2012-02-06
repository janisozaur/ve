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

	enum Eye {
		Left,
		Right
	};

	void beginEye(const Eye &e) const;
	void finishEye() const;
	void setAspectRatio(const float &ar);
	float fov() const;
	float aspect() const;
	float near() const;
	float far() const;

private:
	float mConvergence;
	float mEyeSeparation;
	float mAspectRatio;
	float mFOV;
	float mNearClippingDistance;
	float mFarClippingDistance;
	float mTop;
	float mBottom;
	float mA;
	float mES2;
};

#endif // STEREOCAMERA_H
