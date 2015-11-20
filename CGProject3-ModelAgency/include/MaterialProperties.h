#ifndef METAMORPH_MATERIALPROPERTIES_H
#define METAMORPH_MATERIALPROPERTIES_H

#include "CGCommon.h"

#include <QColor>

class QGLShaderProgram;

#define PHONG_MATERIAL_MODE_SOLID_COLOR     0
#define PHONG_MATERIAL_MODE_NORMALS         1
#define PHONG_MATERIAL_MODE_TEX_CORDS	    2
#define PHONG_MATERIAL_MODE_SHADED          3

class MaterialProperties
{
public:
    MaterialProperties();
    MaterialProperties(const MaterialProperties &pMP);
    ~MaterialProperties();

    // Getters
    const QColor& getAmbient() const;
    const QColor& getDiffuse() const;
    const QColor& getSpecular() const;

    float getSpecularExp() const;
    int getRenderMode() const;

    // Setters
    void setAmbient(const QColor& pA);
    void setAmbient(int pRed, int pGreen, int pBlue);

    void setDiffuse(const QColor& pD);
    void setDiffuse(int pRed, int pGreen, int pBlue);

    void setSpecular(const QColor& pS);
    void setSpecular(int pRed, int pGreen, int pBlue);

    void setSpecularExp(float pExp);
    void setRenderMode(int pNewMode);

    // Pass data to shader
    void TellOpenGL(QGLShaderProgram *pShader, bool pIgnoreMode = false, unsigned int pFace = 0) const;

    // Easy access to alternate rendering modes without changing the internal data
    static void solidColorOnly(QGLShaderProgram *pShader);
    static void visualizeNormals(QGLShaderProgram *pShader);
    static void visualizeTextureCoordinates(QGLShaderProgram *pShader);

protected:
    QColor mKa, mKd, mKs;
    float mExp; int mCurMode;
};

#endif // METAMORPH_MATERIALPROPERTIES_H
