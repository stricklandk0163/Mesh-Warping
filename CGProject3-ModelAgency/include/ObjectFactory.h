#ifndef OBJECTFACTORY_H
#define OBJECTFACTORY_H

#include <QString>

class ObjectNode;

class ObjectFactory
{
public:
    ObjectFactory();
    ~ObjectFactory();

    // Read a mesh from the specified file
    static ObjectNode* loadMesh(const QString &pMeshFile);

    // Build simple fixed geometry meshes with these methods
    static ObjectNode* buildCube();
    static ObjectNode* buildCylinder(int pSlices);
    static ObjectNode* buildSphere(int pSlices, int pStacks);

    // Build hierarchical meshes composed of other ObjMeshes with these methods
    static ObjectNode* buildSculpture();
    static ObjectNode* buildHumanoid();

    // Helper functions
    static ObjectNode* buildSculptureStrut();
    static ObjectNode* buildTorso();
    static ObjectNode* buildArm();
    static ObjectNode* buildHead();
    static ObjectNode* buildLeg();

    // Some extra functions to fill in with extra credit objects (completing these is optional)
    static ObjectNode* buildExtraObject1(double pParam1, double pParam2, double pParam3);
    static ObjectNode* buildExtraObject2(double pParam1, double pParam2, double pParam3);
};

#endif // OBJECTFACTORY_H
