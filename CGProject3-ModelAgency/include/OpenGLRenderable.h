#ifndef OPENGLRENDERABLE_H
#define OPENGLRENDERABLE_H

#include "CGCommon.h"
#include <QString>
#include <QMatrix4x4>
#include "GLRenderMatrixContext.h"

class QGLShaderProgram;

class OpenGLRenderable
{
public:
    OpenGLRenderable(double pFOV = GENERAL_FOV, double pNear = 0.1, double pFar = 100.0);
    OpenGLRenderable(bool p2DContent);
    ~OpenGLRenderable();

    // General function to read and compile a shader
    static void initShader(QGLShaderProgram* &pShader, const QString &pVTXShader, const QString &pFRGShader, bool pWithTans);

protected:
    // Camera parameters
    double mHorizFOV, mNear, mFar;
    bool mOrtho2D;

    // Is anti-aliasing enabled?
    bool mAllowAliasing;

    // Last seen width and height from resizeGL()
    int mW, mH;

    // GLSL Shader
    QGLShaderProgram* mShader;

    // Manual matrix management (for OpenGL 3.2+)
    GLRenderMatrixContext mMatrixContext;

    // Implement this in your child if you don't want to use the
    // currently enabled context or your child has it's own context.
    virtual bool makeGLContextCurrent() { return true; }

    // Internal OpenGL Initialization
    virtual void initializeGLRenderable(bool pAllowAliasing);
    virtual void resizeGLRenderable(int w, int h);
    virtual void initShader(const QString &pVTXShader, const QString &pFRGShader, bool pWithTans = false);

    // Customize for your shader (if any)
    virtual void setShaderDefaults() {}
};

/***********************************/

class CameraPose;
class CameraDistortion;

// Note: this class is designed for a particular type of shader that needs calculations
// in the local model space.  The shader should accept the eye position, light position
// and a 4x4 view matrix (with no model transformation).  See cpp file for details.
class ViewMatrixOpenGLRenderable : public OpenGLRenderable
{
public:
    ViewMatrixOpenGLRenderable(double pFOV = GENERAL_FOV, double pNear = 0.1, double pFar = 100.0);
    ~ViewMatrixOpenGLRenderable();

    virtual void setCameraView(const CameraPose* pPose = NULL, const CameraDistortion* pLens = NULL);
    virtual void setCameraView(QGLShaderProgram *pShader, const CameraPose* pPose = NULL, const CameraDistortion* pLens = NULL);

protected:
    CameraPose *mPose;
    QMatrix4x4 mViewMatrix;
};

/***********************************/
// Render 2D content with a fixed ratio that will fill as much of the viewport as possible
// Optionally allow zooming and panning
class FixedRatio2DGLRenderable : public OpenGLRenderable
{
public:
   FixedRatio2DGLRenderable(int pContentsWidth = 1, int pContentsHeight = 1, bool pEnableZoomAndPan = false);
   ~FixedRatio2DGLRenderable();

   // Controlling zooming and panning
   virtual void setZoom(float pZoom);
   virtual void setOffset(int pXOff, int pYOff);
   virtual void enableZoomAndPan(bool pEnable);

protected:
    virtual void resizeGLRenderable(int w, int h);
    virtual void calcWindowScale();
    virtual void drawContents();

    virtual void setContentsDimensions(int w, int h);

    // Adjust for panning offset
    void clampOffset();

    inline float getOffsetRangeX() const
    {
        float lZoomedWidth = mW*mWidthScale*mZoom;
        return MAX(0.0f, lZoomedWidth - mW);
    }

    inline float getOffsetRangeY() const
    {
        float lZoomedHeight = mH*mHeightScale*mZoom;
        return MAX(0.0f, lZoomedHeight - mH);
    }

    // Dimensions and scale for 2D content
    float mHeightScale, mWidthScale;
    int mContentsWidth, mContentsHeight;

    // Zooming and panning
    bool mEnableZoomAndPan;
    float mZoom, mXOff, mYOff;
};

#endif // OPENGLRENDERABLE_H
