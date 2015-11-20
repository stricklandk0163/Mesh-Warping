#ifndef GL3DSHADERWIDGET_H
#define GL3DSHADERWIDGET_H

#include <OpenGLRenderable.h>

#include <QGLWidget>
#include <QMatrix4x4>

class QGLShaderProgram;
class CameraPose;

class GL3DShaderWidget : public QGLWidget, public ViewMatrixOpenGLRenderable
{
    Q_OBJECT

public:
    GL3DShaderWidget(QWidget *parent = 0, QGLWidget *share = 0);
    GL3DShaderWidget(QGLFormat fmt, QWidget *parent = 0, QGLWidget *share = 0);
    GL3DShaderWidget(double pFOV, double pNear, double pFar, QWidget *parent = 0, QGLWidget *share = 0);
    GL3DShaderWidget(QGLFormat fmt, double pFOV, double pNear, double pFar, QWidget *parent = 0, QGLWidget *share = 0);

    virtual ~GL3DShaderWidget();

    virtual void setZClipping(double pNear, double pFar);

protected:
    // Compare the supplied pixel format with the one that was granted
    void checkOpenGLPixelFormat(const QGLFormat &fmt);

    // QGLWidget overloaded functions
    virtual void initializeGL();
    virtual void resizeGL(int w, int h);
    virtual void paintGL();

    // OpenGLRenderable overloaded functions
    virtual bool makeGLContextCurrent() { makeCurrent(); return true; }

    // Child widget should re-implement these as needed (particularly drawContents() )
    virtual void drawContents();
    virtual void setupWidget() {}
    virtual void prePaint() {}
    virtual void postPaintPreSwap() {}
};

#endif // GL3DSHADERWIDGET_H
