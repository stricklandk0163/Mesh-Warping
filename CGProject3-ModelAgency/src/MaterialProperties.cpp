#include "MaterialProperties.h"

#include <QGLShaderProgram>

MaterialProperties::MaterialProperties() :
        mKa(QColor::fromRgb(5, 5, 5)),
        mKd(QColor::fromRgb(200, 200, 200)),
        mKs(QColor::fromRgb(0, 0, 0))
{
    mExp = 128;
    mCurMode = PHONG_MATERIAL_MODE_SHADED;
}

MaterialProperties::MaterialProperties(const MaterialProperties &pMP)
{
    // Copy material properties
    mKa = pMP.mKa; mKd = pMP.mKd; mKs = pMP.mKs;
    mExp = pMP.mExp;

    // Always default to shaded
    mCurMode = PHONG_MATERIAL_MODE_SHADED;
}

MaterialProperties::~MaterialProperties()
{
}

// Getters
const QColor& MaterialProperties::getAmbient() const { return mKa; }
const QColor& MaterialProperties::getDiffuse() const { return mKd; }
const QColor& MaterialProperties::getSpecular() const { return mKs; }

float MaterialProperties::getSpecularExp() const { return mExp; }
int MaterialProperties::getRenderMode() const { return mCurMode; }

// Setters
void MaterialProperties::setAmbient(const QColor& pA) { mKa = pA; }
void MaterialProperties::setAmbient(int pRed, int pGreen, int pBlue)
{
    mKa = QColor::fromRgb(pRed, pGreen, pBlue);
}

void MaterialProperties::setDiffuse(const QColor& pD) { mKa = pD; }
void MaterialProperties::setDiffuse(int pRed, int pGreen, int pBlue)
{
    mKd = QColor::fromRgb(pRed, pGreen, pBlue);
}

void MaterialProperties::setSpecular(const QColor& pS) { mKa = pS; }
void MaterialProperties::setSpecular(int pRed, int pGreen, int pBlue)
{
    mKs = QColor::fromRgb(pRed, pGreen, pBlue);
}

void MaterialProperties::setSpecularExp(float pExp) { mExp = pExp; }
void MaterialProperties::setRenderMode(int pNewMode)
{
    if(pNewMode >= 0 && pNewMode <= 3)
        mCurMode = pNewMode;
}

void MaterialProperties::solidColorOnly(QGLShaderProgram *pShader)
{
    if(pShader != NULL)
    {
        pShader->bind();
        pShader->setUniformValue("curRenderMode", PHONG_MATERIAL_MODE_SOLID_COLOR);
    }
}

void MaterialProperties::visualizeNormals(QGLShaderProgram *pShader)
{
    if(pShader != NULL)
    {
        pShader->bind();
        pShader->setUniformValue("curRenderMode", PHONG_MATERIAL_MODE_NORMALS);
    }
}

void MaterialProperties::visualizeTextureCoordinates(QGLShaderProgram *pShader)
{
    if(pShader != NULL)
    {
        pShader->bind();
        pShader->setUniformValue("curRenderMode", PHONG_MATERIAL_MODE_TEX_CORDS);
    }
}

void MaterialProperties::TellOpenGL(QGLShaderProgram *pShader, bool pIgnoreMode, unsigned int pFace) const
{
    // Update the render mode
    if(pShader != NULL)
    {
        pShader->bind();
        if(!pIgnoreMode) pShader->setUniformValue("curRenderMode", mCurMode);
    }

    // Default to front face
    if(pFace == 0) pFace = GL_FRONT;

    // Set basic phong lighting parameters
    GLfloat mAmbientColor[4]  = { (GLfloat)mKa.redF(), (GLfloat)mKa.greenF(), (GLfloat)mKa.blueF(), 1.0 };
    GLfloat mDiffuseColor[4]  = { (GLfloat)mKd.redF(), (GLfloat)mKd.greenF(), (GLfloat)mKd.blueF(), 1.0 };
    GLfloat mSpecularColor[4] = { (GLfloat)mKs.redF(), (GLfloat)mKs.greenF(), (GLfloat)mKs.blueF(), 1.0 };

    glMaterialfv(pFace, GL_AMBIENT, mAmbientColor);
    glMaterialfv(pFace, GL_DIFFUSE, mDiffuseColor);
    glMaterialfv(pFace, GL_SPECULAR, mSpecularColor);
    glMaterialf(pFace, GL_SHININESS, mExp);

    // Dupilcate diffuse as primary color
    glColor4fv(mDiffuseColor);
}
