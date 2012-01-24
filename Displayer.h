#ifndef DISPLAYER_H
#define DISPLAYER_H

#include "FPSCounter.h"
#include "StereoCamera.h"

#include <QGLWidget>
#ifdef Q_OS_LINUX
#include <bullet/btBulletDynamicsCommon.h>
#else
#include <src/btBulletDynamicsCommon.h>
#endif
#include <QTime>
#include <QTimer>
#include <QVector3D>
#include <QMatrix4x4>

class GLUquadric;
class Model;
struct Triangle;
class GLDebugDrawer;

class Displayer : public QGLWidget
{
	Q_OBJECT
public:
	explicit Displayer(QWidget *parent = 0);
	~Displayer();
	void resize(int w, int h);
	QString getGLInfo();

private:
	QColor mBackgroundColor;

	// Bullet Physics variables, as seen on
	// http://www.bulletphysics.org/mediawiki-1.5.8/index.php?title=Hello_World
	btBroadphaseInterface* mBroadphase;
	btDefaultCollisionConfiguration* mCollisionConfiguration;
	btCollisionDispatcher* mDispatcher;
	btSequentialImpulseConstraintSolver* mSolver;
	btDiscreteDynamicsWorld* mDynamicsWorld;
	btCollisionShape* mGroundShape;
	btCollisionShape* mFallShape;
	btCollisionShape *mPoolTableCollisionShape;
	btRigidBody::btRigidBodyConstructionInfo *mPoolTableRigidBodyCI;
	btDefaultMotionState* mPoolTableMotionState;
	btDefaultMotionState* mGroundMotionState;
	btRigidBody::btRigidBodyConstructionInfo mGroundRigidBodyCI;
	btRigidBody* mGroundRigidBody;
	btRigidBody* mPoolTableRigidBody;
	btDefaultMotionState* mFallMotionState;
	btRigidBody::btRigidBodyConstructionInfo *mFallRigidBodyCI;
	btRigidBody* mFallRigidBody;

	QVector<btRigidBody *> mBallsRigidBody;
	QVector<btRigidBody::btRigidBodyConstructionInfo *> mBallsRigidBodyCI;
	QVector<btMotionState *> mBallsMotionState;
	QVector<btCollisionShape *> mBallsShape;

	btDefaultMotionState* mControllerMotionState;
	btRigidBody::btRigidBodyConstructionInfo *mControllerRigidBodyCI;
	btRigidBody* mControllerRigidBody;

	btTriangleMesh *mPoolTableCollider;

	btTransform mTrans, mControllerTrans, mPoolTableTrans;
	QTimer mPhysicsTimer, mDrawTimer;
	QTime mTime;
	GLUquadric *mQuadric;
	GLfloat *light_ambient, *light_ambient_position, *whiteDiffuseLight,
			*blackAmbientLight, *whiteSpecularLight;
	int mWidth, mHeight;
	Model *mPoolTableDisplayModel, *mPoolTablePhysicsModel;
	GLDebugDrawer *mDebugDrawer;
	FpsCounter mFpsCounter;
	QMatrix4x4 mRotation;
	StereoCamera mStereoCam;
	QPointF mCameraDiff;

protected:
	void initializeGL();
	void paintGL();
	void resizeGL(int w, int h);

signals:

private slots:
	void timeout();

public slots:
	void update();
	void moveMarker(QVector3D pos);
	void setRelativeCameraPos(QPointF p);
};

#endif // DISPLAYER_H
