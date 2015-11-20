#ifndef MESHGLWIDGET_H
#define MESHGLWIDGET_H

#include "TrackballWidget.h"

class QKeyEvent;
class CameraPose;
class MaterialProperties;
class ObjectNode;

class MeshGLWidget : public TrackballWidget
{
    Q_OBJECT

public:
    MeshGLWidget(QWidget *parent = 0, QGLWidget *share = 0);
    MeshGLWidget(QGLFormat fmt, QWidget *parent = 0, QGLWidget *share = 0);
    ~MeshGLWidget();

    // Set a new mesh object (also takes ownership and will destory)
    void takeObject(ObjectNode* pNewObject);
    const ObjectNode* getObject() const;

public slots:
    // Change rendering modes
    void enableAxies(bool pEnable);
    void enableNormals(bool pEnable);
    void changeRenderingMode(int pNewMode);
    void cycleWireframeMode();

    // Recompute normals
    void computePerFaceNormals();
    void smoothAllNormals();

protected:
    // The object to render
    ObjectNode *mObject;

    // Material properties and rendering options
    MaterialProperties *mPhongMtrl;
    bool mDrawAxies, mDrawNormals;
    int mWireframeMode;

    // Keyboard interaction
    virtual void keyPressEvent(QKeyEvent *event);

    // Overrides of GL3DShaderWidget functions
    virtual void initializeGL();
    virtual void drawContents();
    virtual void setShaderDefaults();

    // Draw the current mesh to OpenGL
    virtual void drawObject();

    // Helper function to draw global axies
    virtual void drawAxies();
};

#endif // MESHGLWIDGET_H
