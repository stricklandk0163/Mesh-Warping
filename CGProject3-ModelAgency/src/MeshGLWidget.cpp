#include "CGCommon.h"

#include "ObjectNode.h"

#include "MaterialProperties.h"
#include "View.h"

#include "MeshGLWidget.h"

// For trackball rotation
#include <QTimer>
#include <QQuaternion>
#include <QMatrix4x4>

#include <QGLShaderProgram>
#include <QKeyEvent>

// Different wireframe drawing modes
#define WIREFRAME_NONE          0
#define WIREFRAME_OVER_MESH     1
#define WIREFRAME_ONLY          2
#define WIREFRAME_MODE_COUNT    3

MeshGLWidget::MeshGLWidget(QWidget *parent, QGLWidget *share) :
    TrackballWidget(parent, share)
{
    // Initialize all pointers
    mObject = NULL;
    mPhongMtrl = NULL;

    // Initialize rendering options
    mDrawAxies = false;
    mDrawNormals = false;
    mWireframeMode = WIREFRAME_NONE;

    // Setup a few views of the scene
    mPose = CameraPose::MakeFromLookAt(0, 0, 6.0);

    // Give strong focus so widget responds to key presses
    setFocusPolicy(Qt::StrongFocus);

    // Set window title to something reasonable
    setWindowTitle("Mesh Viewer");

    enableIdleRefresh();
}

MeshGLWidget::MeshGLWidget(QGLFormat fmt, QWidget *parent, QGLWidget *share) :
    TrackballWidget(fmt, parent, share)
{
    // Initialize all pointers
    mObject = NULL;
    mPhongMtrl = NULL;

    // Initialize rendering options
    mDrawAxies = false;
    mDrawNormals = false;
    mWireframeMode = WIREFRAME_NONE;

    // Setup a few views of the scene
    mPose = CameraPose::MakeFromLookAt(0, 0, 6.0);

    // Give strong focus so widget responds to key presses
    setFocusPolicy(Qt::StrongFocus);

    // Set window title to something reasonable
    setWindowTitle("Mesh Viewer");

    enableIdleRefresh();
}

MeshGLWidget::~MeshGLWidget()
{
    if(mPhongMtrl != NULL) delete mPhongMtrl;
    if(mObject != NULL) delete mObject;
}

void MeshGLWidget::enableAxies(bool pEnable)
{
    mDrawAxies = pEnable;
    updateGL();
}

void MeshGLWidget::enableNormals(bool pEnable)
{
    mDrawNormals = pEnable;
    updateGL();
}

void MeshGLWidget::changeRenderingMode(int pNewMode)
{
    if(mPhongMtrl != NULL)
    {
        mPhongMtrl->setRenderMode(pNewMode);
        updateGL();
    }
}

void MeshGLWidget::cycleWireframeMode()
{
    mWireframeMode = (mWireframeMode + 1)%WIREFRAME_MODE_COUNT;
    updateGL();
}

void MeshGLWidget::computePerFaceNormals()
{
    if(mObject != NULL)
    {
        mObject->computePerFaceNormals(true);
        updateGL();
    }
}

void MeshGLWidget::smoothAllNormals()
{
    if(mObject != NULL)
    {
        mObject->smoothNormals();
        updateGL();
    }
}

void MeshGLWidget::takeObject(ObjectNode* pNewObject)
{
    if(mObject != NULL) delete mObject;
    mObject = pNewObject;
    updateGL();
}

const ObjectNode* MeshGLWidget::getObject() const { return mObject; }

void MeshGLWidget::initializeGL()
{
    // Initialize parent first
    GL3DShaderWidget::initializeGL();
    glClearColor(0.0f, 0.4f, 0.4f, 0.0f);

    // Make diffuse material property track current color
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

    // Build material
    mPhongMtrl = new MaterialProperties();
    mPhongMtrl->setDiffuse(200, 200, 200);
    mPhongMtrl->setSpecular(0, 0, 0);

    // Load/compile our shader
    initShader(":/ModelAgency/Material.vert", ":/ModelAgency/Material.frag");
}

void MeshGLWidget::setShaderDefaults()
{
    if(mShader != NULL)
    {
        CG_GL_ERROR_CHECK
        mShader->bind();
        setCameraView(mPose);

        if(mPhongMtrl != NULL) mPhongMtrl->TellOpenGL(mShader);
    }
}

void MeshGLWidget::drawContents()
{
    if(mPhongMtrl == NULL) return;

    // Pass shader values to GLSL
    mPhongMtrl->TellOpenGL(mShader);

    CG_GL_ERROR_CHECK
    // Apply rotation to model-view
    mMatrixContext.pushModelMatrix();

        // Main rotation
        ApplyTrackballRotation();

        // Draw the optional global axies
        if(mDrawAxies) drawAxies();

        // Draw the mesh contents
        drawObject();

        // Draw the mesh normals
        if(mDrawNormals)
        {
            glColor3f(0.0, 0.0, 1.0);
            MaterialProperties::solidColorOnly(mShader);
            mObject->drawNormals(mMatrixContext);
        }

    mMatrixContext.popModelMatrix();
}

void MeshGLWidget::drawObject()
{
    static GLfloat sBlack[4] = { 0.0, 0.0, 0.0, 1.0 };

    if(mObject == NULL) return;

    bool lWiref = (mWireframeMode > WIREFRAME_NONE);
    bool lSolid = (mWireframeMode < WIREFRAME_ONLY);

    // Draw the mesh shape
    glPushAttrib(GL_LIGHTING_BIT | GL_POLYGON_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
    if(lSolid)
    {
        if(lWiref)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(1.0, 1.0);
        }

        mObject->draw(mMatrixContext, mShader);
    }

    if(lWiref)
    {
        if(lSolid) glDisable(GL_POLYGON_OFFSET_FILL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        mPhongMtrl->solidColorOnly(mShader);

        glMaterialfv(GL_FRONT, GL_DIFFUSE, sBlack);
        glColor4fv(sBlack);

        mObject->draw(mMatrixContext);

        mPhongMtrl->TellOpenGL(mShader);
    }
    glPopAttrib();
}

void MeshGLWidget::drawAxies()
{
    static GLfloat sRed[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
    static GLfloat sDarkRed[4] = { 0.7f, 0.0f, 0.0f, 1.0f };
    static GLfloat sGreen[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
    static GLfloat sBlue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };

    glPushAttrib(GL_LINE_BIT | GL_CURRENT_BIT);
    glLineWidth(4);

    mPhongMtrl->solidColorOnly(mShader);

    glMaterialfv(GL_FRONT, GL_DIFFUSE, sRed);
    glBegin(GL_LINES);
        glColor4fv(sRed);
        glVertex3f(-10, 0.0, 0.0); glVertex3f(10, 0.0, 0.0);

        glColor4fv(sGreen);
        glVertex3f(0.0, -10, 0.0); glVertex3f(0.0, 10, 0.0);

        glColor4fv(sBlue);
        glVertex3f(0.0, 0.0, -10.0); glVertex3f(0.0, 0.0, 10);
    glEnd();

    static float lBoxSize = 1.0f;
    glBegin(GL_LINE_STRIP);
        glColor4fv(sDarkRed);
        glVertex3f(-lBoxSize, -lBoxSize, lBoxSize);
        glVertex3f( lBoxSize, -lBoxSize, lBoxSize);
        glVertex3f( lBoxSize,  lBoxSize, lBoxSize);
        glVertex3f(-lBoxSize,  lBoxSize, lBoxSize);
        glVertex3f(-lBoxSize, -lBoxSize, lBoxSize);

        glVertex3f(-lBoxSize, -lBoxSize, -lBoxSize);
        glVertex3f( lBoxSize, -lBoxSize, -lBoxSize);
        glVertex3f( lBoxSize,  lBoxSize, -lBoxSize);
        glVertex3f(-lBoxSize,  lBoxSize, -lBoxSize);
        glVertex3f(-lBoxSize, -lBoxSize, -lBoxSize);
    glEnd();

    glBegin(GL_LINES);
        glVertex3f( lBoxSize, -lBoxSize,  lBoxSize);
        glVertex3f( lBoxSize, -lBoxSize, -lBoxSize);

        glVertex3f( lBoxSize,  lBoxSize,  lBoxSize);
        glVertex3f( lBoxSize,  lBoxSize, -lBoxSize);

        glVertex3f(-lBoxSize,  lBoxSize,  lBoxSize);
        glVertex3f(-lBoxSize,  lBoxSize, -lBoxSize);
    glEnd();

    glPopAttrib();
    mPhongMtrl->TellOpenGL(mShader);
}

void MeshGLWidget::keyPressEvent(QKeyEvent *event)
{
    static double sPrevFOV = mHorizFOV;

    makeGLContextCurrent();
    switch(event->key())
    {
//        // Changing the render mode
//        case Qt::Key_1: case Qt::Key_2: case Qt::Key_3: case Qt::Key_4:
//        {
//            if(mPhongMtrl != NULL)
//            {
//                mPhongMtrl->setRenderMode(event->key() - Qt::Key_1);
//                updateGL();
//            }
//        }
//        break;

//        // Compute per-face normals
//        case Qt::Key_N:
//            if(mObject != NULL)
//            {
//                mObject->computePerFaceNormals(true);
//                updateGL();
//            }
//        break;

//        // Compute per-face normals
//        case Qt::Key_S:
//            if(mObject != NULL)
//            {
//                mObject->smoothNormals();
//                updateGL();
//            }
//        break;

//        // Enable/disable the axies
//        case Qt::Key_A:
//            mDrawAxies = !mDrawAxies;
//            updateGL();
//        break;

//        // Change the wireframe mode
//        case Qt::Key_W:
//            mWireframeMode = (mWireframeMode + 1)%WIREFRAME_MODE_COUNT;
//            updateGL();
//        break;

//        // Toggle drawing the normals
//        case Qt::Key_Tab:
//            mDrawNormals = !mDrawNormals;
//            updateGL();
//        break;

        // Toggle zooming in and out
        case Qt::Key_Space:
        {
            if(mHorizFOV != 15)
            {
                sPrevFOV = mHorizFOV;
                mHorizFOV = 15;
            }
            else mHorizFOV = sPrevFOV;

            resizeGLRenderable(mW, mH);
            updateGL();
        }
        break;
    }

    // Pass up inheritance chain
    GL3DShaderWidget::keyPressEvent(event);
}
