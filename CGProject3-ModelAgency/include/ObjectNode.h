#ifndef OBJECTNODE_H
#define OBJECTNODE_H

#include "CGCommon.h"
#include <QList>
#include <QString>

#include "GLRenderMatrixContext.h"

// Forward declaration
class ObjMesh;
class MaterialProperties;
class QGLShaderProgram;
struct MeshGroup;

class ObjectNode
{
public:
    ObjectNode();
    ObjectNode(ObjMesh *pMesh, MeshGroup *pGroup);
    ~ObjectNode();

    const QString& getName() const;
    void setName(QString);

    // Mesh metrics
    int getVertexCount() const;
    int getFaceCount() const;

    // Parent-child metrics
    int childIndex() const;
    ObjectNode* parentMesh();
    int getChildCount() const;

    // Convert groups to children
    void meshGroupsToChildren();

    // Support for linking children to this object node
    ObjectNode* makeNewChild(QString pName = QString());

    void addChild(ObjectNode* pChild);
    ObjectNode *getChild(int index = 0);
    QList<ObjectNode*>& getChildren();

    // Iterate through children
    ObjectNode *getFirstChild();
    ObjectNode *getNextChild();

    void setMaterials(MaterialProperties* pNewMat);
    MaterialProperties* getMaterials();

    // Center of rotaiton
    void setCenterOfRotation(float pX, float pY, float pZ);
    void getCenterOfRotation(float &pX, float &pY, float &pZ) const;

    // Query the transformation values
    void getTranslation(float &pX, float &pY, float &pZ) const;
    void getScale(float &pX, float &pY, float &pZ) const;
    void getRotation(float &pX, float &pY, float &pZ) const;

    // Explicitly set this object's transformation
    void setTranslate(float pX, float pY, float pZ);
    void setScale(float pX, float pY, float pZ);
    void setRotate(float pX, float pY, float pZ);

    // Adjust this object's transformation
    void addTranslate(float pX, float pY, float pZ);
    void addScale(float pSx, float pY, float pZ);
    void addRotate(float pX, float pY, float pZ);

    // Methods that pass through to the Mesh
    void addGroup(std::string pGroupName);

    // Add a new raw vertex (supply the X, Y and Z coordinates of the vertex)
    uint32_t addVertex(float pX, float pY, float pZ);

    // Add a new raw normal vector (supply the i, j and k parts of the vector)
    uint32_t addNormal(float pI, float pJ, float pK);

    // Add a new triangle.  Supply and index into the raw vertex and normal arrays for each
    // of the triangles vertices (three vertex indices and three normal indices)
    void addTriangle(uint32_t vA, uint32_t vB, uint32_t vC,
                     uint32_t nA, uint32_t nB, uint32_t nC);

    // Parse the raw data (added with above three functions)
    bool parseRawData(bool pReScale = false, bool pReCenter = false, bool pAllowEmpty = false);

    // Managing mesh normals
    void computePerFaceNormals(bool pOverwrite = false);
    void smoothNormals();

    // Read an OBJ file into the mesh for this node (does not use any children)
    bool readFile(std::string pFilename, bool pReScale = false, bool pReCenter = false);

    // Save the mesh for this node as an OBJ file (does not save any children)
    bool writeFile(std::string pFilename) const;

    // Draw this object (and all of its children)
    void draw(GLRenderMatrixContext &pMatrix, QGLShaderProgram *pShader = NULL) const;

    // Draw this object's (and all of its children's) normals
    void drawNormals(GLRenderMatrixContext &pMatrix) const;

    // Used for a reference to a bad node (do not use directly except for comparison)
    static ObjectNode badNode;

protected:
    void setParentChildMetrics(ObjectNode* pParentMesh, int pChildIndex);
    void applyTransformation(GLRenderMatrixContext &pMatrix) const;

    // The name for this node
    QString mName;

    // Transformation variables
    float mTx, mTy, mTz;
    float mSx, mSy, mSz;
    float mRx, mRy, mRz;

    // Center of rotation
    float mRotCentX, mRotCentY, mRotCentZ;

    // For iterating through the children
    mutable int mChildIteratorIndex;

    // Pointer to parent and position in parent's list
    ObjectNode* mParentMesh;
    int mChildIndex;

    // The mesh or mesh group for this node
    ObjMesh *mNodeMesh;
    MeshGroup *mMeshGroup;
    bool mDontDrawMesh;

    // The materials for this node
    MaterialProperties *mMtrl;

    // Linked children nodes
    QList<ObjectNode*> mNodeChildren;

    // A counter for all nodes created so far
    static int msObjectCount;
};

#endif // OBJECTNODE_H
