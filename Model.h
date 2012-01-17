#ifndef MODEL_H
#define MODEL_H

#define GL_GLEXT_PROTOTYPES
#define TEX_XSIZE 1024
#define TEX_YSIZE 1024

#include <QString>
#include <QMap>
#include <QVector>
#include <QVector3D>
#include <GL/gl.h>

struct Triangle
{
	QVector3D vertices[3];
};

struct Lib3dsFile;
struct TextureInfo;

class Model
{
public:
	Model(QString filename);
	void draw();
	void createDisplayList(GLfloat scale = 1);
	void loadTextures();
	QVector<Triangle> getFaces(GLfloat scale = 1);
	~Model();

protected:
	unsigned int countFaces();
	unsigned int mTotalFaces;
	Lib3dsFile *mModel;
	GLuint mVertexVBO, mNormalVBO;
	QMap<QString, TextureInfo *> mTexturesMap;
};

#endif // MODEL_H
