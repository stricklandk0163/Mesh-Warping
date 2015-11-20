#include "CGCommon.h"

#include "GL3DShaderWidget.h"

#include <QGLShaderProgram>

GL3DShaderWidget::GL3DShaderWidget(QWidget *parent, QGLWidget *share) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent, share),
    ViewMatrixOpenGLRenderable(GENERAL_FOV, 0.1, 100.0)
{
    checkOpenGLPixelFormat(QGLFormat(QGL::SampleBuffers));
}

GL3DShaderWidget::GL3DShaderWidget(QGLFormat fmt, QWidget *parent, QGLWidget *share) :
    QGLWidget(fmt, parent, share), ViewMatrixOpenGLRenderable(GENERAL_FOV, 0.1, 100.0)
{
    checkOpenGLPixelFormat(fmt);
}

GL3DShaderWidget::GL3DShaderWidget(double pFOV, double pNear, double pFar, QWidget *parent, QGLWidget *share) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent, share),
    ViewMatrixOpenGLRenderable(pFOV, pNear, pFar)
{
    checkOpenGLPixelFormat(QGLFormat(QGL::SampleBuffers));
}

GL3DShaderWidget::GL3DShaderWidget(QGLFormat fmt, double pFOV, double pNear, double pFar, QWidget *parent, QGLWidget *share) :
    QGLWidget(fmt, parent, share), ViewMatrixOpenGLRenderable(pFOV, pNear, pFar)
{
    checkOpenGLPixelFormat(fmt);
}

GL3DShaderWidget::~GL3DShaderWidget() {}

inline void GL3DShaderWidget::checkOpenGLPixelFormat(const QGLFormat &fmt)
{
    QGLFormat lRealFormat = format();

    if(fmt.depth() && !lRealFormat.depth()) printf("OpenGL Buffer Warning: depth buffer requested but NOT granted.\n");
    if(fmt.alpha() && !lRealFormat.alpha()) printf("OpenGL Buffer Warning: alpha buffer requested but not granted.\n");

    if(!fmt.doubleBuffer() && lRealFormat.doubleBuffer()) printf("OpenGL Buffer Warning: single buffer requested but NOT granted.\n");
    if(fmt.doubleBuffer() && !lRealFormat.doubleBuffer()) printf("OpenGL Buffer Warning: double buffer requested but NOT granted.\n");
    if(fmt.stereo() && !lRealFormat.stereo()) printf("OpenGL Buffer Warning: stereo buffer requested but NOT granted.\n");

    if(!fmt.rgba() && lRealFormat.rgba()) printf("OpenGL Buffer Warning: indexed color mode requested but NOT granted.\n");
    if(fmt.rgba() && !lRealFormat.rgba()) printf("OpenGL Buffer Warning: RGBA color mode requested but NOT granted.\n");
    if(fmt.sampleBuffers() && !lRealFormat.sampleBuffers()) printf("OpenGL Buffer Warning: multisample buffers requested but NOT granted.\n");

    if(fmt.accum() && !lRealFormat.accum()) printf("OpenGL Buffer Warning: accumulation buffer requested but not granted.\n");
    if(fmt.stencil() && !lRealFormat.stencil()) printf("OpenGL Buffer Warning: stencil buffer requested but not granted.\n");
    fflush(stdout);
}

// GL Widget Functions
void GL3DShaderWidget::initializeGL()
{    
    OpenGLRenderable::initializeGLRenderable(false);
}

void GL3DShaderWidget::resizeGL(int pW, int pH)
{
    OpenGLRenderable::resizeGLRenderable(pW, pH);
}

void GL3DShaderWidget::setZClipping(double pNear, double pFar)
{
    double lAspect = mW/(double)mH;

    CG_GL_ERROR_CHECK
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluPerspective(mHorizFOV/lAspect, lAspect, pNear, pFar);

    CG_GL_ERROR_CHECK
    glMatrixMode(GL_MODELVIEW);
}

void GL3DShaderWidget::paintGL()
{
    // If it's not yet visible don't try and draw (you can upset OpenGL on some systems if you do)
    if(!isVisible()) return;

    CG_GL_ERROR_CHECK

    // Make sure this OpenGL context is current
#ifdef GLCONTEXT_DEBUG
    fprintf(stderr, "Making current (%s:%d)\n", __FILE__, __LINE__);
    fflush(stderr);
#endif
    makeGLContextCurrent();
    CG_GL_ERROR_CHECK

    // Clear the buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CG_GL_ERROR_CHECK

    // Allow special processing before drawContents() (implement in child)
    // Note: buffer clears before but not after, shader is not bound
    if(mShader != NULL) mShader->release();
    CG_GL_ERROR_CHECK
    prePaint();

    CG_GL_ERROR_CHECK

    // Bind the shader and draw the maincontents
    if(mShader != NULL) mShader->bind();
    else glColor4f(0.0, 0.0, 0.0, 1.0);
    drawContents();

    CG_GL_ERROR_CHECK

    // Allow special procesing after drawContents() but before buffer swap (implement in child)
    // Note: shader is not bound
    if(mShader != NULL) mShader->release();
    postPaintPreSwap();

    CG_GL_ERROR_CHECK

    // Swap back and front buffers
    // NO!  Qt does this automatically and doing it manually will cause a
    // double swap on some OSs and seisure inducing flashing.
//    swapBuffers();
}

// Default does nothing
void GL3DShaderWidget::drawContents() {}
