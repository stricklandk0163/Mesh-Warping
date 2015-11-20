#include "CGCommon.h"

#include "GLRenderMatrixContext.h"
#include "View.h"
#include <iostream>
#include <QGLShaderProgram>

GLRenderMatrixContext::GLRenderMatrixContext()
{
    //Set all three matrices to identity
    mProjMatrix.setToIdentity();
    mModelMatrix.setToIdentity();
    mViewMatrix.setToIdentity();
    mAutoPassToGL = false;
    mShader = NULL;
}

// Set the shader object so that you can syncronize the matrices with
// your openGL shader code.  You must still call the pass*ToOpenGL methods
// yourself to syncronize the matrices.  To avoid this, call
// enableAutoPassingToGL instead and supply your shader there.
void GLRenderMatrixContext::setShader(QGLShaderProgram* pShader)
{
    mShader = pShader;
    passProjToOpenGL(mShader);
    passModelToOpenGL(mShader);
    passViewToOpenGL(mShader);
}

// Setup auto passing.  This will automatically pass any changes to the matrices
// into the shader when the changes happen.  It may end up wasting work but it
// ensures they are always in sync.
void GLRenderMatrixContext::enableAutoPassingToGL(bool pEnable, QGLShaderProgram *pShader)
{
    mAutoPassToGL = pEnable;
    if(mAutoPassToGL)
    {
        if(pShader != NULL)
        {
            mShader = pShader;
            mShader->bind();
        }

        passProjToOpenGL(mShader);
        passModelToOpenGL(mShader);
        passViewToOpenGL(mShader);
    }
}

// Send information to OpenGL
void GLRenderMatrixContext::passProjToOpenGL(QGLShaderProgram *pShader)
{
    CG_GL_ERROR_CHECK

    // If a shader is provided then pass as uniform variable
    if(pShader != NULL)
    {
        pShader->setUniformValue("projMatrix", mProjMatrix);
        CG_GL_ERROR_CHECK
    }
    else // Use compatibility profile and OpenGL ModelView matrix
    {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(mProjMatrix.constData());
        CG_GL_ERROR_CHECK
    }
}

void GLRenderMatrixContext::passModelToOpenGL(QGLShaderProgram *pShader)
{
    CG_GL_ERROR_CHECK

    // If a shader is provided then pass as uniform variable
    if(pShader != NULL)
    {
        pShader->setUniformValue("modelMatrix", mModelMatrix);
        pShader->setUniformValue("normalMatrix", mModelMatrix.normalMatrix());
        CG_GL_ERROR_CHECK
    }
    else // Use compatibility profile and OpenGL ModelView matrix
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(mModelMatrix.constData());
        CG_GL_ERROR_CHECK
    }
}

void GLRenderMatrixContext::passViewToOpenGL(QGLShaderProgram *pShader)
{
    CG_GL_ERROR_CHECK
    if(pShader == NULL)
    {
        fprintf(stderr, "GL Render Matrix Context: Cannot set view matrix without a shader.\n");
        fflush(stderr);
        return;
    }

    pShader->setUniformValue("viewMatrix", mViewMatrix);
    CG_GL_ERROR_CHECK
}

// Directly manipulate the proj matrix
void GLRenderMatrixContext::loadProjIdentity()
{
    //Set to identity
    mProjMatrix.setToIdentity();

    // Pass to shader
    if(mAutoPassToGL) passProjToOpenGL(mShader);
}

void GLRenderMatrixContext::loadProjMatrix(const QMatrix4x4& pProjMatrix)
{
    mProjMatrix = pProjMatrix;

    // Pass to shader
    if(mAutoPassToGL) passProjToOpenGL(mShader);
}

void GLRenderMatrixContext::multProjMatrix(const QMatrix4x4& pMatrix)
{
    //Set proj matrix to identity
    mProjMatrix *= pMatrix;

    // Pass to shader
    if(mAutoPassToGL) passProjToOpenGL(mShader);
}

// Directly manipulate the model matrix
void GLRenderMatrixContext::loadModelIdentity()
{
    //Set model matrix to identity
    mModelMatrix.setToIdentity();

    // Pass to shader
    if(mAutoPassToGL) passModelToOpenGL(mShader);
}

void GLRenderMatrixContext::loadModelMatrix(const QMatrix4x4 &pModelMatrix)
{
    mModelMatrix = pModelMatrix;

    // Pass to shader
    if(mAutoPassToGL) passModelToOpenGL(mShader);
}

void GLRenderMatrixContext::multModelMatrix(const QMatrix4x4 &pMatrix)
{
    mModelMatrix *= pMatrix;

    // Pass to shader
    if(mAutoPassToGL) passModelToOpenGL(mShader);
}

// Directly manipulate the view matrix
void GLRenderMatrixContext::loadViewIdentity()
{
    //Set view matrix to identity
    mViewMatrix.setToIdentity();

    // Pass to shader
    if(mAutoPassToGL) passViewToOpenGL(mShader);
}

void GLRenderMatrixContext::loadViewMatrix(const QMatrix4x4 &pViewMatrix)
{
    mViewMatrix = pViewMatrix;

    // Pass to shader
    if(mAutoPassToGL) passViewToOpenGL(mShader);
}

void GLRenderMatrixContext::multViewMatrix(const QMatrix4x4 &pMatrix)
{
    mViewMatrix *= pMatrix;

    // Pass to shader
    if(mAutoPassToGL) passViewToOpenGL(mShader);
}

// Different ways to build the projection matrix
void GLRenderMatrixContext::frustum(float pLeft, float pRight, float pBottom, float pTop, float pNear, float pFar)
{
    // build the projeciton matrix from the generic frustrum dimensions
    mProjMatrix.frustum(pLeft, pRight, pBottom, pTop, pNear, pFar);
    // Pass to shader
    if(mAutoPassToGL) passProjToOpenGL(mShader);
}

void GLRenderMatrixContext::perspective(float pVerticalFOV, float pAspectRatio, float pNear, float pFar)
{
    // build the projection matrix as a perspective projection (QMatrix4x4 makes this easy)
    mProjMatrix.perspective(pVerticalFOV, pAspectRatio, pNear, pFar);
    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passProjToOpenGL(mShader);
}

void GLRenderMatrixContext::ortho(float pLeft, float pRight, float pBottom, float pTop, float pNear, float pFar)
{
    // build the projeciton matrix from the generic frustrum as orthographic (QMatrix4x4 makes this easy)
    mProjMatrix.ortho(pLeft, pRight, pBottom, pTop, pNear, pFar);
    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passProjToOpenGL(mShader);
}

void GLRenderMatrixContext::ortho2D(float pLeft, float pRight, float pBottom, float pTop)
{
    // build the projeciton matrix from the generic frustrum as orthographic
    // set near to -1 and far to 1
    mProjMatrix.ortho(pLeft, pRight, pBottom, pTop, -1, 1 );
    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passProjToOpenGL(mShader);
}

// Setup the view matrix
void GLRenderMatrixContext::lookAt(float pEyeX, float pEyeY, float pEyeZ,
                                   float pAtX, float pAtY, float pAtZ,
                                   float pUpX, float pUpY, float pUpZ)
{
    //Use the lookAt algorithm to make the camera look at the object
    mViewMatrix.lookAt(QVector3D(pEyeX, pEyeY, pEyeZ),
                       QVector3D(pAtX,pAtY,pAtZ),
                       QVector3D(pUpX,pUpY,pUpZ));

    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passViewToOpenGL(mShader);
}

void GLRenderMatrixContext::pose(CameraPose* pPose)
{
    // Sanity check
    if(pPose == NULL) return;

    // Get orientation information
    const float* pM3 = pPose->GetOrientationMatrix();
    QMatrix4x4 lOrient(pM3[0], pM3[3], pM3[6], 0.0,
                       pM3[1], pM3[4], pM3[7], 0.0,
                       pM3[2], pM3[5], pM3[8], 0.0,
                          0.0,    0.0,    0.0, 1.0);
    mViewMatrix *= lOrient;

    // Get position information
    float pos[3];
    pPose->GetPosition(pos[0], pos[1], pos[2]);
    mViewMatrix.translate(-pos[0], -pos[1], -pos[2]);
    mViewMatrix.optimize();

    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passViewToOpenGL(mShader);
}

// Transform the view matrix
void GLRenderMatrixContext::translateView(double pTx, double pTy, double pTz)
{
    // apply a translation to the view matrix
    mViewMatrix.translate(pTx,pTy,pTz);
    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passViewToOpenGL(mShader);
}

void GLRenderMatrixContext::rotateView(double pAngleInDegrees, double pRx, double pRy, double pRz)
{
    //apply a rotation to the view matrix
    mViewMatrix.rotate(pAngleInDegrees, pRx, pRy, pRz);
    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passViewToOpenGL(mShader);
}

void GLRenderMatrixContext::scaleView(double pSx, double pSy, double pSz)
{
    // apply a scale to the view matrix
    mViewMatrix.scale(pSx, pSy, pSz);
    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passViewToOpenGL(mShader);
}

// Apply one of the three basic transformations to the model matrix
void GLRenderMatrixContext::translate(double pTx, double pTy, double pTz)
{
    // apply a translation to the model matrix
    mModelMatrix.translate(pTx,pTy,pTz);
    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passModelToOpenGL(mShader);
}

void GLRenderMatrixContext::rotate(double pAngleInDegrees, double pRx, double pRy, double pRz)
{
    // apply a rotation to the model matrix
    mModelMatrix.rotate(pAngleInDegrees, pRx, pRy, pRz);
    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passModelToOpenGL(mShader);
}

void GLRenderMatrixContext::scale(double pSx, double pSy, double pSz)
{
    // apply a scale to the model matrix
    mModelMatrix.scale(pSx, pSy, pSz);
    // Pass the new matrix to OpenGL
    if(mAutoPassToGL) passModelToOpenGL(mShader);
}

// Manage the internal matrix stacks
//Push the projection matrix onto the projection matrix stack
void GLRenderMatrixContext::pushProjMatrix()
{
    mProjStack.push(mProjMatrix);
    if(mAutoPassToGL) passProjToOpenGL(mShader);
}
//Pop the top projection matrix off of the projection matrix stack
void GLRenderMatrixContext::popProjMatrix()
{
    if(!mProjStack.empty())
    {
        mProjMatrix=mProjStack.pop();
        if(mAutoPassToGL) passProjToOpenGL(mShader);
    }
    else
        std::cerr<<"The Projection Stack is empty";
}
//Push the model matrix onto the model matrix stack
void GLRenderMatrixContext::pushModelMatrix()
{
    mModelStack.push(mModelMatrix);
    if(mAutoPassToGL) passModelToOpenGL(mShader);
}
//Pop the top model matrix off of the model matrix stack
void GLRenderMatrixContext::popModelMatrix()
{
    if(!mModelStack.empty())
    {
        mModelMatrix=mModelStack.pop();
        if(mAutoPassToGL) passModelToOpenGL(mShader);
    }
    else
        std::cerr<<"The Model Stack is empty";

}
//Push the view matrix onto the view matrix stack
void GLRenderMatrixContext::pushViewMatrix()
{
    mViewStack.push(mViewMatrix);
    if(mAutoPassToGL) passViewToOpenGL(mShader);
}
//Pop the view matrix off of the view matrix stack
void GLRenderMatrixContext::popViewMatrix()
{
    if(!mViewStack.empty())
    {
        mViewMatrix=mViewStack.pop();
        if(mAutoPassToGL) passViewToOpenGL(mShader);
    }
    else
        std::cerr<<"The View Stack is empty";
}
