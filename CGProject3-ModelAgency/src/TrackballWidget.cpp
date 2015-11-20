#include <CGCommon.h>

#include "TrackballWidget.h"

// For trackball rotation
#include <QTimer>
#include <QQuaternion>
#include <QMatrix4x4>

#include <QMouseEvent>
#include <QGLShaderProgram>

TrackballWidget::TrackballWidget(QWidget *parent, QGLWidget *share)
    : GL3DShaderWidget(parent, share), mTrackBall(0.03f, true, QVector3D(0, 1, 0), TrackBall::Sphere)
{
    mTrackballEnabled = true;

    // Update similar to an idle function
    mIdleTimer = new QTimer(NULL);
    mIdleTimer->setInterval(20);
    connect(mIdleTimer, SIGNAL(timeout()), this, SLOT(updateGLFromTimer()));
}

TrackballWidget::TrackballWidget(QGLFormat fmt, QWidget *parent, QGLWidget *share)
    : GL3DShaderWidget(fmt, parent, share), mTrackBall(0.03f, true, QVector3D(0, 1, 0), TrackBall::Sphere)
{
    mTrackballEnabled = true;

    // Update similar to an idle function
    mIdleTimer = new QTimer(NULL);
    mIdleTimer->setInterval(20);
    connect(mIdleTimer, SIGNAL(timeout()), this, SLOT(updateGLFromTimer()));
}

TrackballWidget::~TrackballWidget()
{
    if(mIdleTimer != NULL)
    {
        mIdleTimer->stop();
        delete mIdleTimer;
    }
}

// If the idle timer is active then don't do an update
void TrackballWidget::updateGL()
{
    if(mIdleTimer != NULL && mIdleTimer->isActive()) return;
    else GL3DShaderWidget::updateGL();
}

// A special update slot for the timer only (will always honor update request)
void TrackballWidget::updateGLFromTimer()
{
    GL3DShaderWidget::updateGL();
}

void TrackballWidget::enableTrackballRotation() { mTrackballEnabled = true; enableIdleRefresh(); }
void TrackballWidget::disableTrackballRotation() { mTrackballEnabled = false; disableIdleRefresh(); }

void TrackballWidget::enableIdleRefresh() { mIdleTimer->start(); }
void TrackballWidget::disableIdleRefresh() { mIdleTimer->stop(); }

void TrackballWidget::freezeTrackball()
{
    mTrackBall.freeze();
    updateGL();
}

void TrackballWidget::clearRotation()
{
    mTrackBall.clearRotation();
    updateGL();
}

void TrackballWidget::addManualRotation(const QVector3D &pAxis, float pAngle)
{
    mTrackBall.addRotation(pAxis, pAngle);
    updateGL();
}

void TrackballWidget::setManualRotation(const QVector3D &pAxis, float pAngle)
{
    mTrackBall.setRotation(pAxis, pAngle);
    updateGL();
}

void TrackballWidget::ApplyTrackballRotation()
{
    if(!mTrackballEnabled) return;

    // Convert to rotation matrix and apply
    QMatrix4x4 m; m.rotate(mTrackBall.rotation());
    mMatrixContext.multModelMatrix(m);
}

void TrackballWidget::ApplyTrackballToPoint(float &pX, float &pY, float &pZ)
{
    QMatrix4x4 m; m.rotate(mTrackBall.rotation());
    QVector4D lTrans = m.map(QVector4D(pX, pY, pZ, 1.0));

    pX = lTrans.x();
    pY = lTrans.y();
    pZ = lTrans.z();
}

void TrackballWidget::ApplyTrackballToVector(float &pi, float &pj, float &pk)
{
    QMatrix4x4 m; m.rotate(mTrackBall.rotation());
    QVector4D lTrans = m.map(QVector4D(pi, pj, pk, 0.0));

    pi = lTrans.x();
    pj = lTrans.y();
    pk = lTrans.z();
}

void TrackballWidget::ApplyTrackballRotationGLSL()
{
    if(!mTrackballEnabled || mShader == NULL) return;

    // Convert to rotation matrix
    QMatrix4x4 m; m.rotate(mTrackBall.rotation());

    // Pass to shader
    mShader->bind();
    mShader->setUniformValue("trackball", m);
}

inline QPointF TrackballWidget::NormalizePixelPos(const QPoint& p)
{
    return QPointF(2.0 * float(p.x()) / width() - 1.0,
                   1.0 - 2.0 * float(p.y()) / height());
}

void TrackballWidget::mouseMoveEvent(QMouseEvent *event)
{
    QGLWidget::mouseMoveEvent(event);
    if (event->isAccepted() || !mTrackballEnabled) return;

    makeGLContextCurrent();
    if (event->buttons() & Qt::LeftButton) {
        mTrackBall.move(NormalizePixelPos(event->pos()), QQuaternion());
        event->accept();
    } else {
         mTrackBall.release(NormalizePixelPos(event->pos()), QQuaternion());
    }

    updateGL();
}

void TrackballWidget::mousePressEvent(QMouseEvent *event)
{
    QGLWidget::mousePressEvent(event);
    if (event->isAccepted() || !mTrackballEnabled) return;

    if (event->buttons() & Qt::LeftButton) {
        mTrackBall.push(NormalizePixelPos(event->pos()), QQuaternion());
        event->accept();
    }

    updateGL();
}

void TrackballWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QGLWidget::mouseReleaseEvent(event);
    if (event->isAccepted() || !mTrackballEnabled) return;

    if (event->button() == Qt::LeftButton) {
        mTrackBall.release(NormalizePixelPos(event->pos()), QQuaternion());
        event->accept();
    }

    updateGL();
}
