#include <QApplication>
#include "ModelAgencyMainWindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Force use of desktop OpenGL drivers and not ANGLE or OpenGL ES
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);

    // Fixes QListWidget selection when running in OS X and compiling against Qt 5
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    ModelAgencyMainWindow w;
    w.show();
    
    return a.exec();
}
