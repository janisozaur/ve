#ifndef TEXTUREINFO_H
#define TEXTUREINFO_H

#include <QImage>
#include <GL/gl.h>

struct TextureInfo {
	float scaleX;
	float scaleY;
	QImage glImg;
	GLuint texId;
};

#endif // MYTEXTUREINFO_H
