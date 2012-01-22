#include "Model.h"
#include "Displayer.h"
#include "GLDebugDrawer.h"
#include "StereoCamera.h"

#include <QFile>
#include <QTemporaryFile>
#include <GL/glu.h>
#include <GL/freeglut.h>

#include <QDebug>

Displayer::Displayer(QWidget *parent) :
	QGLWidget(parent),
	//mBroadphase(new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000))),
	mBroadphase(new btDbvtBroadphase()),
	mCollisionConfiguration(new btDefaultCollisionConfiguration()),
	mDispatcher(new btCollisionDispatcher(mCollisionConfiguration)),
	mSolver(new btSequentialImpulseConstraintSolver),
	mDynamicsWorld(new btDiscreteDynamicsWorld(mDispatcher, mBroadphase,
			mSolver, mCollisionConfiguration)),
	mGroundShape(new btStaticPlaneShape(btVector3(0, 1, 0), 1)),
	mFallShape(new btSphereShape(1)),
	mPoolTableMotionState(new btDefaultMotionState(
			btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)))),
	mGroundMotionState(new btDefaultMotionState(
			btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)))),
	mGroundRigidBodyCI(0, mGroundMotionState, mGroundShape, btVector3(0, 0, 0)),
	mGroundRigidBody(new btRigidBody(mGroundRigidBodyCI)),
	mFallMotionState(new btDefaultMotionState(
			btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 100, 0)))),
	mControllerMotionState(new btDefaultMotionState(
			btTransform(btQuaternion(0, 0, 0, 1), btVector3(1.25f, 1, 0)))),
	mQuadric(gluNewQuadric()),
	mStereoCam(1500.0f,     // Convergence
			   1.0f,       // Eye Separation
			   1.3333f,     // Aspect Ratio
			   45.0f,       // FOV along Y in degrees
			   10.0f,       // Near Clipping Distance
			   2000.0f)   // Far Clipping Distance)
{
	mBackgroundColor.setRgb(qRgb(100, 100, 100));
	mPoolTableCollider = NULL;
	mDynamicsWorld->setGravity(btVector3(0, -10, 0));
	mDynamicsWorld->addRigidBody(mGroundRigidBody);
	btScalar mass = 1;
	btVector3 fallInertia(0, 0, 0);
	mFallShape->calculateLocalInertia(mass, fallInertia);
	mFallRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(mass,
									mFallMotionState, mFallShape, fallInertia);
	mFallRigidBody = new btRigidBody(*mFallRigidBodyCI);
	mDynamicsWorld->addRigidBody(mFallRigidBody);
	mControllerRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,
									mControllerMotionState, mFallShape);
	mControllerRigidBody = new btRigidBody(*mControllerRigidBodyCI);
	mControllerRigidBody->setCollisionFlags(
									mControllerRigidBody->getCollisionFlags() |
									btCollisionObject::CF_KINEMATIC_OBJECT);
	mControllerRigidBody->setActivationState(DISABLE_DEACTIVATION);
	mDynamicsWorld->addRigidBody(mControllerRigidBody);
	gluQuadricNormals(mQuadric, GLU_SMOOTH);

	light_ambient = new GLfloat[4];
	light_ambient[0] = 0.5f;
	light_ambient[1] = 0.5f;
	light_ambient[2] = 0.5f;
	light_ambient[3] = 1.0;
	light_ambient_position = new GLfloat[4];
	light_ambient_position[0] = 0.0;
	light_ambient_position[1] = 0.0;
	light_ambient_position[2] = 2.0;
	light_ambient_position[3] = 1.0;
	whiteDiffuseLight = new GLfloat[4];
	whiteDiffuseLight[0] = 0.7;
	whiteDiffuseLight[1] = 0.7;
	whiteDiffuseLight[2] = 0.7;
	whiteDiffuseLight[3] = 1.0;
	blackAmbientLight = new GLfloat[3];
	blackAmbientLight[0] = 1.0;
	blackAmbientLight[1] = 0.0;
	blackAmbientLight[2] = 0.0;
	whiteSpecularLight = new GLfloat[4];
	whiteSpecularLight[0] = 0.8;
	whiteSpecularLight[1] = 0.8;
	whiteSpecularLight[2] = 0.8;
	whiteSpecularLight[3] = 1.0;

	connect(&mPhysicsTimer, SIGNAL(timeout()), this, SLOT(timeout()));
	connect(&mDrawTimer, SIGNAL(timeout()), this, SLOT(update()));

	mDrawTimer.start(20);
	mPhysicsTimer.start(10);
	mTime.start();

	// nasty workaround for lib3ds ability to load only files by their name and
	// including the model file to project source
	QFile poolTableModelFile(":/models/pooltable.3ds");
	poolTableModelFile.open(QIODevice::ReadOnly);
	QTemporaryFile *tempFile = QTemporaryFile::createLocalFile(poolTableModelFile);
	mPoolTableDisplayModel = new Model(tempFile->fileName().toLocal8Bit().constData());
	poolTableModelFile.close();
	delete tempFile;

	poolTableModelFile.setFileName(":/models/pooltable_physics.3ds");
	poolTableModelFile.open(QIODevice::ReadOnly);
	tempFile = QTemporaryFile::createLocalFile(poolTableModelFile);
	mPoolTablePhysicsModel = new Model(tempFile->fileName().toLocal8Bit().constData());
	poolTableModelFile.close();
	delete tempFile;

	QVector<Triangle> faces = mPoolTablePhysicsModel->getFaces();
	mPoolTableCollider = new btTriangleMesh;
	for (int i = 0; i < faces.count(); i++) {
		btVector3 one(faces.at(i).vertices[0].x(), faces.at(i).vertices[0].y(),
					  faces.at(i).vertices[0].z());
		btVector3 two(faces.at(i).vertices[1].x(), faces.at(i).vertices[1].y(),
					  faces.at(i).vertices[1].z());
		btVector3 tri(faces.at(i).vertices[2].x(), faces.at(i).vertices[2].y(),
					  faces.at(i).vertices[2].z());
		mPoolTableCollider->addTriangle(one, two, tri);
	}
	mPoolTableCollisionShape = new btBvhTriangleMeshShape(mPoolTableCollider,
														  false);
	mPoolTableRigidBodyCI = new btRigidBody::btRigidBodyConstructionInfo(0,
							mPoolTableMotionState, mPoolTableCollisionShape);
	mPoolTableRigidBody = new btRigidBody(*mPoolTableRigidBodyCI);
	mPoolTableRigidBody->setCollisionFlags(
									mPoolTableRigidBody->getCollisionFlags() |
									btCollisionObject::CF_KINEMATIC_OBJECT);
	mPoolTableRigidBody->setActivationState(DISABLE_DEACTIVATION);
	mPoolTableRigidBody->getMotionState()->getWorldTransform(mPoolTableTrans);
	btQuaternion rot = mPoolTableTrans.getRotation();
	rot.setRotation(btVector3(1, 0, 0), -SIMD_HALF_PI);
	mPoolTableTrans.setRotation(rot);
	mPoolTableMotionState->setWorldTransform(mPoolTableTrans);
	mDynamicsWorld->addRigidBody(mPoolTableRigidBody);
	mPoolTableRigidBody->setFriction(1.0f);
	mFallRigidBody->setFriction(1.0f);
	mFallRigidBody->setDamping(0.0f, 0.5f);
	qDebug() << "table friction:" << mPoolTableRigidBody->getFriction() << "linear damping:" << mPoolTableRigidBody->getLinearDamping() << "angular damping:" << mPoolTableRigidBody->getAngularDamping();
	qDebug() << "ball friction:" << mFallRigidBody->getFriction() << "linear damping:" << mFallRigidBody->getLinearDamping() << "angular damping:" << mFallRigidBody->getAngularDamping();

	mDebugDrawer = new GLDebugDrawer;
	mDynamicsWorld->setDebugDrawer(mDebugDrawer);
}

Displayer::~Displayer()
{
	qDebug() << "Displayer dtor";
	delete mDebugDrawer;
	delete mPoolTableRigidBody;
	delete mPoolTableRigidBodyCI;
	delete mPoolTableCollisionShape;
	delete mPoolTableCollider;
	delete mPoolTableMotionState;
	gluDeleteQuadric(mQuadric);
	delete mControllerRigidBody;
	delete mControllerRigidBodyCI;
	delete mControllerMotionState;
	delete light_ambient;
	delete light_ambient_position;
	delete whiteDiffuseLight;
	delete blackAmbientLight;
	delete whiteSpecularLight;
	delete mGroundShape;
	delete mFallShape;
	delete mFallMotionState;
	delete mFallRigidBody;
	delete mFallRigidBodyCI;
	delete mGroundRigidBody;
	delete mGroundMotionState;
	delete mDynamicsWorld;
	delete mSolver;
	delete mDispatcher;
	delete mCollisionConfiguration;
	delete mBroadphase;
	delete mPoolTableDisplayModel;
	delete mPoolTablePhysicsModel;
}

void Displayer::moveMarker(QVector3D pos)
{
	/*if (which != Yellow) {
		return;
	}*/
	mControllerTrans.getOrigin().setX(pos.x());
	mControllerTrans.getOrigin().setY(pos.y());
	mControllerTrans.getOrigin().setZ(pos.z() - 25);
	btVector3 diff = mControllerTrans.getOrigin() - mTrans.getOrigin();
	QVector3D d(diff.getX(), diff.getY(), diff.getZ());
	qDebug() << "moving marker yellow to" << pos << d;
}

QString Displayer::getGLInfo()
{
	QStringList result;
	result << QString("average fps: ") + QString::number(mFpsCounter.getFps());
	result << QString("average rame time: ") + QString::number(mFpsCounter.getFrameTime());
	result << QString("vendor: ") + QString((char *)glGetString(GL_VENDOR));
	result << QString("renderer: ") + QString((char *)glGetString(GL_RENDERER));
	result << QString("version: ") + QString((char *)glGetString(GL_VERSION));
#ifdef GL_SHADING_LANGUAGE_VERSION
	result << QString("glsl: ") + QString((char *)glGetString(GL_SHADING_LANGUAGE_VERSION));
#else
	result << QString("glsl: Sorry, but your toolchain says there is no such enum GL_SHADING_LANGUAGE_VERSION, you must be running prehistoric opengl version.");
#endif
	result << QString("extensions: ") + QString((char *)glGetString(GL_EXTENSIONS));
	return result.join("\n");
}

void Displayer::initializeGL()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glShadeModel(GL_SMOOTH);
	glLightfv(GL_LIGHT0, GL_POSITION, light_ambient_position);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, whiteDiffuseLight);
	glLightfv(GL_LIGHT0, GL_SPECULAR, whiteSpecularLight);
	mPoolTableDisplayModel->loadTextures();
	mPoolTableDisplayModel->createDisplayList();
}

void Displayer::resizeGL(int w, int h)
{
	mWidth = w;
	mHeight = h;
	glViewport(0, 0, (GLint)w, (GLint)h);
	qDebug() << "resize (" << w << ", " << h << ")";
}

void PlaceSceneElements()
{
	// translate to appropriate depth along -Z
	glTranslatef(0.0f, 0.0f, -1800.0f);

	// rotate the scene for viewing
	glRotatef(-60.0f, 1.0f, 0.0f, 0.0f);
	glRotatef(-45.0f, 0.0f, 0.0f, 1.0f);

	// draw intersecting tori
	glPushMatrix();
		glTranslatef(0.0f, 0.0f, 240.0f);
		glRotatef(90.0f, 1.0f, 0.0f, 0.0f);
		glColor3f(0.2, 0.2, 0.6);
		glutSolidTorus(40, 200, 20, 30);
		glColor3f(0.7f, 0.7f, 0.7f);
		glutWireTorus(40, 200, 20, 30);
	glPopMatrix();

	glPushMatrix();
		glTranslatef(240.0f, 0.0f, 240.0f);
		glColor3f(0.2, 0.2, 0.6);
		glutSolidTorus(40, 200, 20, 30);
		glColor3f(0.7f, 0.7f, 0.7f);
		glutWireTorus(40, 200, 20, 30);
	glPopMatrix();
}

void Displayer::paintGL()
{
	mFpsCounter.frameStart();
	qglClearColor(mBackgroundColor);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//glTranslatef(0, -2, 0);
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(0 + mCameraDiff.x(), 100 + mCameraDiff.y(), 0,
			  0, 1, 0,
			  0, 0, 1);
	btVector3 cam(0, 70, -30);
	btVector3 target = mPoolTableTrans.getOrigin();
	btVector3 cam2obj = target - cam;
	//target += btVector3(-25, 0, 15);
	//gluLookAt(0, 60, 80, 0, 26, 0, 0, 1, 0);

//	gluLookAt(cam[0], cam[1], cam[2], target[0], target[1], target[2], 0, 1, 0);
	static float zdist = 0.0f;
//	static int delta = 1;
//	if (zdist > 0 || zdist < -800) {
//		delta = -delta;
//	}
//	zdist += delta;
	mStereoCam.beginEye(StereoCamera::Left);
	glColorMask(true, false, false, false);
	//PlaceSceneElements();
	glTranslatef(0.0f, 0.0f, zdist);
	glRotatef(-(mPoolTableTrans.getRotation().getAngle() / SIMD_2_PI) * 360, 1, 0, 0);
	glTranslatef(mPoolTableTrans.getOrigin().getX(),
				 mPoolTableTrans.getOrigin().getY(),
				 mPoolTableTrans.getOrigin().getZ());
	mPoolTableDisplayModel->draw();
	mStereoCam.finishEye();
	glClear(GL_DEPTH_BUFFER_BIT);
	mStereoCam.beginEye(StereoCamera::Right);
	glColorMask(false, true, true, false);
	//PlaceSceneElements();
	glTranslatef(0.0f, 0.0f, zdist);
	glRotatef(-(mPoolTableTrans.getRotation().getAngle() / SIMD_2_PI) * 360, 1, 0, 0);
	glTranslatef(mPoolTableTrans.getOrigin().getX(),
				 mPoolTableTrans.getOrigin().getY(),
				 mPoolTableTrans.getOrigin().getZ());
	mPoolTableDisplayModel->draw();
	mStereoCam.finishEye();
	glColorMask(true, true, true, true);
	mDebugDrawer->setDebugMode(1);
	mDynamicsWorld->debugDrawWorld();
	mFpsCounter.frameEnd();
}

void Displayer::update()
{
	updateGL();
	QGLWidget::update();
}

void Displayer::timeout()
{
	int elapsed = mTime.restart();
	//qDebug() << "elapsed:" << elapsed;
	//float delta = 0.5f * (float)elapsed / mPhysicsTimer.interval();
	//QVector3D pos(3, 9, 15);
	//pos.setX(mControllerTrans.getOrigin().x());
	//pos.setY(mControllerTrans.getOrigin().y());
	//pos.setZ(mControllerTrans.getOrigin().z());
	//mRotation.rotate(delta, 0, 1, 0);
	//pos = pos * mRotation;
	//btVector3 btPos(pos.x(), pos.y(), pos.z());
	//mControllerTrans.setOrigin(btPos);
	mControllerMotionState->setWorldTransform(mControllerTrans);
	mDynamicsWorld->stepSimulation((float)elapsed / 1000.f, 5);
	mFallMotionState->getWorldTransform(mTrans);
}

void Displayer::setRelativeCameraPos(QPointF p)
{
	mCameraDiff = p * 20;
}
