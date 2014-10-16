#include "Model.h"
#include "TextureInfo.h"

#include <lib3ds/file.h>
#include <lib3ds/mesh.h>
#include <lib3ds/material.h>
#include <lib3ds/node.h>
#include <GL/glext.h>
#include <QImage>
#include <QGLWidget>
#include <cmath>

#include <QDebug>

Model::Model(QString filename) :
	mTotalFaces(0)
{
	mModel = lib3ds_file_load(filename.toLocal8Bit().constData());
	if (!mModel) {
		qWarning() << "Unable to load" << filename;
	}
}

// Count the total number of faces this model has
unsigned int Model::countFaces()
{
	if (mModel == NULL) {
		qDebug() << "tried to get faces from nonexistent model";
		return 0;
	}
	// use cache if possible
	if (mTotalFaces != 0) {
		return mTotalFaces;
	}
	Lib3dsMesh *mesh;
	// Loop through every mesh
	for (mesh = mModel->meshes; mesh != NULL; mesh = mesh->next) {
		// Add the number of faces this mesh has to the total faces
		mTotalFaces += mesh->faces;
		mesh->user.i = 0;
		mesh->user.p = NULL;
	}
	return mTotalFaces;
}

// scan model file and upload any textures found to opengl
// NOTE: requires active opengl context
void Model::loadTextures()
{
	if (mModel == NULL) {
		qDebug() << "tried to get faces from nonexistent model";
		return;
	}
//	glEnable(GL_TEXTURE_2D);
	for (Lib3dsMesh *mesh = mModel->meshes; mesh != NULL; mesh = mesh->next) {
		// this should equal to 0, as no list for the mesh should exist yet
		Q_ASSERT(mesh->user.i == 0);
		Lib3dsMaterial *oldmat = (Lib3dsMaterial *)-1;
		// Loop through every face
		for (unsigned int cur_face = 0; cur_face < mesh->faces; cur_face++) {
			Lib3dsFace *face = &mesh->faceL[cur_face];
			Lib3dsMaterial *mat = NULL;
			TextureInfo *texInfo = NULL;
			if (face->material[0]) {
				mat = lib3ds_file_material_by_name(mModel, face->material);
			}
			if (mat != oldmat) {
				if (mat != NULL) {
					QString name = QString(mat->texture1_map.name);
//					qDebug() << "name:" << name;
					if (!name.isEmpty()) {
						Lib3dsTextureMap *tex = &mat->texture1_map;
						QString texFile = QString(":/textures/%1")
													.arg(tex->name);
						bool cached;
						// try to find and use texture in cache
						if (!mTexturesMap.contains(name)) {
							QImage img(texFile);
							if (img.isNull()) {
								qDebug() << "failed to load" << name;
							}
							QImage glImg = QGLWidget::convertToGLFormat(img);
							texInfo = new TextureInfo;
							texInfo->glImg = glImg;
							texInfo->scaleX = (float)(glImg.width()) /
											  TEX_XSIZE;
							texInfo->scaleY = (float)(glImg.height()) /
											  TEX_YSIZE;
//							glGenTextures(1, &texInfo->texId);
							// we have a new texture, store it in cache
							mTexturesMap.insert(name, texInfo);
							tex->user.p = texInfo;
							cached = false;
//							glBindTexture(GL_TEXTURE_2D, texInfo->texId);
//							glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_XSIZE,
//										 TEX_YSIZE, 0, GL_RGBA,
//										 GL_UNSIGNED_BYTE, NULL);
//							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
//											GL_CLAMP);
//							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
//											GL_CLAMP);
//							glTexParameteri(GL_TEXTURE_2D,
//											GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//							glTexParameteri(GL_TEXTURE_2D,
//											GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//							glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
//									  GL_REPLACE);
//							glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
//											texInfo->glImg.width(),
//											texInfo->glImg.height(), GL_RGBA,
//											GL_UNSIGNED_BYTE,
//											texInfo->glImg.bits());
						} else {
							// get cached texture info and assign it to texInfo
							// (as it is used later on) and tex
							texInfo = mTexturesMap.value(name);
							tex->user.p = texInfo;
							cached = true;
						}
//						qDebug() << (QString("uploaded texture %1 (path: \"%2\", id: %3), %4, required by face %5")).arg(
//										name, texFile, QString::number(texInfo->texId), cached ? "cached" : "new",
//										QString::number(cur_face));
					} else {
//						qDebug() << "texture unloaded in face" << cur_face;
					}
				}
				oldmat = mat;
			}
		}
	}
//	glDisable(GL_TEXTURE_2D);
}

// Copy vertices and normals to the memory of the GPU
void Model::createDisplayList(GLfloat scale)
{
	if (mModel == NULL) {
		qDebug() << "tried to count faces of nonexistent model";
		return;
	}

	if (mTexturesMap.count() == 0) {
//		qDebug() <<
//				"there is no texture cached. did you run loadTextures() first?";
	}

	// Calculate the number of faces we have in total
	countFaces();

	// Allocate memory for our vertices and normals
	Lib3dsVector *vertices = new Lib3dsVector[mTotalFaces * 3];
	Lib3dsVector *normals = new Lib3dsVector[mTotalFaces * 3];

	unsigned int FinishedFaces = 0;
	// initialize to invalid value (not NULL)
	Lib3dsMaterial *oldmat = (Lib3dsMaterial *)-1;
	int cnt = 0;
	// Loop through all the meshes
	for (Lib3dsMesh *mesh = mModel->meshes; mesh != NULL; mesh = mesh->next) {
		lib3ds_mesh_calculate_normals(mesh, &normals[FinishedFaces * 3]);
		Q_ASSERT(mesh->user.i == 0);
		mesh->user.i = glGenLists(1);
		Q_ASSERT(mesh->user.i != 0);
		glNewList(mesh->user.i, GL_COMPILE);
//		glEnable(GL_TEXTURE_2D);
		glBegin(GL_TRIANGLES);
		qDebug() << "creating gl list" << mesh->user.i;
		// Loop through every face
		for (unsigned int cur_face = 0; cur_face < mesh->faces; cur_face++) {
			Lib3dsFace *face = &mesh->faceL[cur_face];
			Lib3dsMaterial *mat = NULL;
			TextureInfo *texInfo = NULL;
			if (face->material[0]) {
				mat = lib3ds_file_material_by_name(mModel, face->material);
			}
			if (mat != oldmat) {
				if (mat != NULL) {
					QString name = QString(mat->texture1_map.name);
					if (!name.isEmpty()) {
						// get cached texture info
						texInfo = mTexturesMap.value(name, NULL);
						// every texture should be cached now, if we spot one
						// that isn't, that's an error
						Q_ASSERT(texInfo != NULL);
						// texture binding should be done outside glBegin() .. glEnd()
//						glBindTexture(GL_TEXTURE_2D, texInfo->texId);
						qDebug() << QString("binding texture %1 (id: %2)").arg(
										name, QString::number(texInfo->texId));
					}
					//qDebug() << "setting material";
					glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse);
					glMaterialfv(GL_FRONT, GL_SPECULAR, mat->specular);
					glMaterialf(GL_FRONT, GL_SHININESS, pow(2, 10.0 *
															mat->shininess));
				} else {
					static const Lib3dsRgba a = {0.7, 0.7, 0.7, 1.0};
					static const Lib3dsRgba d = {0.7, 0.7, 0.7, 1.0};
					static const Lib3dsRgba s = {1.0, 1.0, 1.0, 1.0};
					glMaterialfv(GL_FRONT, GL_AMBIENT, a);
					glMaterialfv(GL_FRONT, GL_DIFFUSE, d);
					glMaterialfv(GL_FRONT, GL_SPECULAR, s);
					glMaterialf(GL_FRONT, GL_SHININESS, pow(2, 10.0 * 0.5));
				}
				oldmat = mat;
			} else if (mat != NULL && mat->texture1_map.name[0]) {
				Lib3dsTextureMap *tex = &mat->texture1_map;
				if (tex != NULL && tex->user.p != NULL) {
					texInfo = static_cast<TextureInfo *>(tex->user.p);
				}
			}
			// FIXME: TAG: texture
			// Found the texturing bug. Apparently, the glBindTexture() call
			// must be done outside of glBegin() ... glEnd(). Doing so for every
			// face introduces major performance penalty, so this has to be
			// cached.
			// TODO: extract all glBindTexture, glBegin and glEnd calls to be
			// made on per-texture basis. This could involve code marked with
			// 'texture' tag which is currently commented out.

			// TAG: texture
//			if (texInfo != NULL) {
//				glEnable(GL_TEXTURE_2D);
//				glBindTexture(GL_TEXTURE_2D, texInfo->texId);
//			}
			for (unsigned int i = 0; i < 3; i++) {
				glNormal3fv(normals[FinishedFaces * 3 + i]);
				Lib3dsVector pos;
				memcpy(pos, mesh->pointL[face->points[i]].pos, sizeof(pos));
				pos[0] *= scale;
				pos[1] *= scale;
				pos[2] *= scale;
				glVertex3fv(pos);
				if (texInfo != NULL) {
					cnt++;
					float u = mesh->texelL[face->points[i]][1] *
							texInfo->scaleX;
					float v = texInfo->scaleY -
							mesh->texelL[face->points[i]][0] * texInfo->scaleY;
					//qDebug() << FinishedFaces * 3 + i << "u:" << u << "v:" << v;
//					glTexCoord2f(u, v);
				}
			}
			// TAG: texture
//			if (texInfo != NULL) {
//				glDisable(GL_TEXTURE_2D);
//			}
			FinishedFaces++;
		}
		glEnd();
//		glDisable(GL_TEXTURE_2D);
		glEndList();
	}
	//qDebug() << "cnt:" << cnt;
	// Clean up our allocated memory
	delete[] vertices;
	delete[] normals;

	// still needed for drawing
	//lib3ds_file_free(mModel);
	//mModel = NULL;
}

QVector<Triangle> Model::getFaces(GLfloat scale)
{
	QVector<Triangle> faces;
	if (mModel == NULL) {
		qDebug() << "tried to get faces from nonexistent model";
		return faces;
	}
	for (Lib3dsMesh *mesh = mModel->meshes; mesh != NULL; mesh = mesh->next) {
		for (unsigned int cur_face = 0; cur_face < mesh->faces; cur_face++) {
			Lib3dsFace *face = &mesh->faceL[cur_face];
			Triangle t;
			for (int i = 0; i < 3; i++) {
				Lib3dsVector pos;
				memcpy(pos, mesh->pointL[face->points[i]].pos, sizeof(pos));
				pos[0] *= scale;
				pos[1] *= scale;
				pos[2] *= scale;
				t.vertices[i] = QVector3D(pos[0], pos[1], pos[2]);
			}
			faces.append(t);
		}
	}
	return faces;
}

void Model::draw()
{
	if (mTotalFaces == 0) {
		qDebug() << "tried to draw nonexistent model";
		return;
	}
	for (Lib3dsMesh *mesh = mModel->meshes; mesh != NULL; mesh = mesh->next) {
		glCallList(mesh->user.i);
	}
}

Model::~Model()
{
	qDebug() << "Model dtor";
	lib3ds_file_free(mModel);
	mModel = NULL;
}
