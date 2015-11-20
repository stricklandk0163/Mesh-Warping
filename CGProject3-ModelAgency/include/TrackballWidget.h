#ifndef TRACKBALLWIDGET_H
#define TRACKBALLWIDGET_H

#include "GL3DShaderWidget.h"

// For trackball rotation
class QTimer;
class QMouseEvent;
#include <QPointF>
#include <Trackball.h>

class TrackballWidget : public GL3DShaderWidget
{
    Q_OBJECT

public:
    TrackballWidget(QWidget *parent = NULL, QGLWidget *share = NULL);
    TrackballWidget(QGLFormat fmt, QWidget *parent = NULL, QGLWidget *share = NULL);
    virtual ~TrackballWidget();

    void enableTrackballRotation();
    void disableTrackballRotation();

    void enableIdleRefresh();
    void disableIdleRefresh();

    void freezeTrackball();

    void clearRotation();
    void addManualRotation(const QVector3D &pAxis, float pAngle);
    void setManualRotation(const QVector3D &pAxis, float pAngle);

public slots:
    virtual void updateGL();

protected slots:
    virtual void updateGLFromTimer();

protected:
    QTimer *mIdleTimer;
    TrackBall mTrackBall;
    bool mTrackballEnabled;

    inline QPointF NormalizePixelPos(const QPoint& p);

    void ApplyTrackballToPoint(float &pX, float &pY, float &pZ);
    void ApplyTrackballToVector(float &pi, float &pj, float &pk);

    void ApplyTrackballRotationGLSL();
    void ApplyTrackballRotation();

    // Overriding mouse event methods
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
};

#endif // TRACKBALLWIDGET_H
