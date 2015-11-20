#-------------------------------------------------
#
# Project created by Seth Berrier 2012-09-10
# - Updated 10/22/2013 by Seth Berrier
#
#-------------------------------------------------
# ModelAgency.pro:
#   This file defines all the pieces of our
# program for 'qmake', a component of Qt.  It is
# processed by qmake in order to generate the
# build files for your system (usually Makefiles
# but could be a .sln or an xcode project).  You
# can open this file directly in Qt Creator to
# edit and manage the project in Qt's own IDE.
#-------------------------------------------------

# The Qt modules we need (GUIs, OpenGL and the core of Qt)
QT += core gui opengl

# Make sure each object will get rebuilt when a dependent header changes
CONFIG += depend_includepath

# If using Qt 5 or greater then we need to add the 'widgets' module as well
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# The name of this application (and it's type, an app)
TARGET = ModelAgency
TEMPLATE = app

# Where to put all the intermediate files generated during compilation
CONFIG(debug, debug|release) {
    OBJECTS_DIR = Debug/
    MOC_DIR = Debug/moc/
    UI_DIR = Debug/ui/
} else {
    OBJECTS_DIR = Release/
    MOC_DIR = Release/moc/
    UI_DIR = Release/ui/
}

# Specify the opengl libraries explicitly (qmake doesn't include them on windows)
win32 {
    LIBS += -lopengl32 -lglu32
}

# Add this so we don't have to append 'include' in front of every .h file
INCLUDEPATH += include/

# All the source code that is part of this project
SOURCES += src/main.cpp\
    src/ModelAgencyMainWindow.cpp \
    src/OpenGLRenderable.cpp \
    src/MeshGLWidget.cpp \
    src/GL3DShaderWidget.cpp \
    src/GLRenderMatrixContext.cpp \
    src/View.cpp \
    src/ObjMesh.cpp \
    src/MaterialProperties.cpp \
    src/Tesselator.cpp \
    src/Trackball.cpp \
    src/TrackballWidget.cpp \
    src/HierarchicalMeshModel.cpp \
    src/ObjectNode.cpp \
    src/ObjectFactory.cpp

HEADERS += include/ModelAgencyMainWindow.h \
    include/OpenGLRenderable.h \
    include/CGCommon.h \
    include/MeshGLWidget.h \
    include/GL3DShaderWidget.h \
    include/GLRenderMatrixContext.h \
    include/View.h \
    include/ObjMesh.h \
    include/MaterialProperties.h \
    include/Tesselator.h \
    include/Trackball.h \
    include/TrackballWidget.h \
    include/ObjectNode.h \
    include/HierarchicalMeshModel.h \
    include/ObjectFactory.h \
    include/msinttypes.h

# The 'form' files that define the layout of the GUI and various dialogs
FORMS += forms/ModelAgencyMainWindow.ui \
    forms/CylinderDialog.ui \
    forms/SphereDialog.ui \
    forms/GenericDialog.ui

# The shaders used by OpenGL
OTHER_FILES += \
    shaders/Material.vert \
    shaders/Material.frag

# Resource file that includes the shaders
RESOURCES += ModelAgency.qrc
