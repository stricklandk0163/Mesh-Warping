#ifndef GLRENDERMATRIXCONTEXT_H
#define GLRENDERMATRIXCONTEXT_H

#include <QStack>
#include <QMatrix4x4>
#include <QVector3D>

// Forward declaration
class QGLShaderProgram;
class CameraPose;

class GLRenderMatrixContext
{
public:
    // Construct a new matrix context
    GLRenderMatrixContext();

    // Setup auto passing of data to OpenGL
    void setShader(QGLShaderProgram* pShader);
    void enableAutoPassingToGL(bool pEnable, QGLShaderProgram* pShader = NULL);

    // Send information to OpenGL
    void passProjToOpenGL(QGLShaderProgram *pShader = NULL);
    void passModelToOpenGL(QGLShaderProgram *pShader = NULL);
    void passViewToOpenGL(QGLShaderProgram *pShader = NULL);

    // Directly manipulate the proj matrix
    void loadProjIdentity();
    void loadProjMatrix(const QMatrix4x4 &pProjMatrix);
    void multProjMatrix(const QMatrix4x4 &pMatrix);

    // Directly manipulate the model matrix
    void loadModelIdentity();
    void loadModelMatrix(const QMatrix4x4 &pModelMatrix);
    void multModelMatrix(const QMatrix4x4 &pMatrix);

    // Directly manipulate the view matrix
    void loadViewIdentity();
    void loadViewMatrix(const QMatrix4x4 &pViewMatrix);
    void multViewMatrix(const QMatrix4x4 &pMatrix);

    // Different ways to setup a projection
    void frustum(float pLeft, float pRight, float pBottom, float pTop, float pNear, float pFar);
    void perspective(float pVerticalFOV, float pAspectRatio, float pNear, float pFar);
    void ortho(float pLeft, float pRight, float pBottom, float pTop, float pNear, float pFar);
    void ortho2D(float pLeft, float pRight, float pBottom, float pTop);

    // Setup the view matrix
    void lookAt(float pEyeX, float pEyeY, float pEyeZ,
                float pAtX,  float pAtY,  float pAtZ,
                float pUpX,  float pUpY,  float pUpZ);

    void pose(CameraPose* pPose);

    // Transform the view matrix
    void translateView(double pTx, double pTy, double pTz);
    void rotateView(double pAngleInDegrees, double pRx, double pRy, double pRz);
    void scaleView(double pSx, double pSy, double pSz);

    // Apply one of the three basic transformations to the model matrix
    void translate(double pTx, double pTy, double pTz);
    void rotate(double pAngleInDegrees, double pRx, double pRy, double pRz);
    void scale(double pSx, double pSy, double pSz);

    // Manage the internal matrix stacks
    void pushProjMatrix();
    void popProjMatrix();

    void pushModelMatrix();
    void popModelMatrix();

    void pushViewMatrix();
    void popViewMatrix();

protected:
    // The current projection, model and view matrices
    QMatrix4x4 mProjMatrix;
    QMatrix4x4 mModelMatrix;
    QMatrix4x4 mViewMatrix;

    //QMatrix4x4 Stacks
    QStack<QMatrix4x4> mProjStack;
    QStack<QMatrix4x4> mModelStack;
    QStack<QMatrix4x4> mViewStack;

    // Enable automatic passing of info to OpenGL
    bool mAutoPassToGL;
    QGLShaderProgram* mShader;
};

#endif // GLRENDERMATRIXCONTEXT_H
