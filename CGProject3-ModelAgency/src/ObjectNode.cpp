#include "ObjectNode.h"

#include <QGLShaderProgram>
#include <qalgorithms.h>

#include "ObjMesh.h"
#include "MaterialProperties.h"

ObjectNode ObjectNode::badNode;
int ObjectNode::msObjectCount = 1;

ObjectNode::ObjectNode()
{
    // Make a unique name for this mesh
    mName = QString("Mesh Object %1").arg(msObjectCount);
    msObjectCount++;

    // Set the parent-child metrics
    mChildIndex = -1;
    mParentMesh = NULL;

    // Always start with first child
    mChildIteratorIndex = 0;

    // Initialize all transformation variables
    mTx = mTy = mTz = 0.0;
    mSx = mSy = mSz = 1.0;
    mRx = mRy = mRz = 0.0;    
    mRotCentX = mRotCentY = mRotCentZ = 0.0;

    // Make a new empty mesh for this node
    mNodeMesh = new ObjMesh();
    mMtrl = NULL;
    mMeshGroup = NULL;
    mDontDrawMesh = false;
}

ObjectNode::ObjectNode(ObjMesh *pMesh, MeshGroup *pGroup)
{
    // Set the parent-child metrics
    mChildIndex = -1;
    mParentMesh = NULL;

    // Always start with first child
    mChildIteratorIndex = 0;

    // Initialize all transformation variables
    mTx = mTy = mTz = 0.0;
    mSx = mSy = mSz = 1.0;
    mRx = mRy = mRz = 0.0;
    mRotCentX = mRotCentY = mRotCentZ = 0.0;

    mMtrl = NULL;

    mNodeMesh = pMesh;
    mMeshGroup = pGroup;
    mDontDrawMesh = false;
}

ObjectNode::~ObjectNode()
{
    // Delete the children first
    qDeleteAll(mNodeChildren);
    mNodeChildren.clear();

    // Delete this node's mesh (unless this is a group node)
    if(mMeshGroup == NULL) delete mNodeMesh;

    // Delete material if there is any
    delete mMtrl;
}

// Access/Set name
const QString& ObjectNode::getName() const { return mName; }
void ObjectNode::setName(QString pName) { mName = pName; }

// Mesh metrics
int ObjectNode::getVertexCount() const { return (int)(mMeshGroup==NULL?mNodeMesh->getVertexCount():0); }
int ObjectNode::getFaceCount() const { return (int)(mMeshGroup==NULL?mNodeMesh->getFaceCount():mMeshGroup->triangleV.size()/3); }

void ObjectNode::meshGroupsToChildren()
{
    for(uint32_t i=0; i < mNodeMesh->getMeshGroupCount(); i++)
    {
        MeshGroup* lCurGroup = mNodeMesh->extractMeshGroup(i);
        ObjectNode* lCurObj = new ObjectNode(mNodeMesh, lCurGroup);
        lCurObj->setName(QString::fromStdString(mNodeMesh->getGroupName(lCurGroup->groups[0])));
        addChild(lCurObj);
    }
    mDontDrawMesh = true;
}

// Parent-child metrics
int ObjectNode::childIndex() const { return mChildIndex; }
ObjectNode* ObjectNode::parentMesh() { return mParentMesh; }
int ObjectNode::getChildCount() const { return mNodeChildren.size(); }

void ObjectNode::setParentChildMetrics(ObjectNode* pParentMesh, int pChildIndex)
{
    mParentMesh = pParentMesh;
    mChildIndex = pChildIndex;
}

// Management of node children links
ObjectNode* ObjectNode::makeNewChild(QString pName)
{
    ObjectNode* lNewChild = new ObjectNode();
    if(!pName.isNull()) lNewChild->setName(pName);
    addChild(lNewChild);
    return lNewChild;
}

void ObjectNode::addChild(ObjectNode* pChild)
{
    if(pChild != NULL && pChild != &badNode)
    {
        pChild->setParentChildMetrics(this, mNodeChildren.size());
        mNodeChildren.push_back(pChild);
    }
}

ObjectNode* ObjectNode::getChild(int index)
{
    if(index < 0 || index > mNodeChildren.size()) return &badNode;
    return mNodeChildren.at(index);
}

QList<ObjectNode *> &ObjectNode::getChildren()
    { return mNodeChildren; }

// Iterate through the node children
ObjectNode* ObjectNode::getFirstChild()
{
    mChildIteratorIndex = 0;
    return getChild();
}

ObjectNode* ObjectNode::getNextChild()
{
    mChildIteratorIndex++;
    return getChild(mChildIteratorIndex);
}

void ObjectNode::setMaterials(MaterialProperties* pNewMat) { mMtrl = pNewMat; }
MaterialProperties* ObjectNode::getMaterials() { return mMtrl; }

void ObjectNode::setCenterOfRotation(float pX, float pY, float pZ) { mRotCentX = pX; mRotCentY = pY; mRotCentZ = pZ; }
void ObjectNode::getCenterOfRotation(float &pX, float &pY, float &pZ) const { pX = mRotCentX; pY = mRotCentY; pZ = mRotCentZ; }

void ObjectNode::getTranslation(float &pX, float &pY, float &pZ) const { pX = mTx; pY = mTy; pZ = mTz; }
void ObjectNode::getScale(float &pX, float &pY, float &pZ) const { pX = mSx; pY = mSy; pZ = mSz; }
void ObjectNode::getRotation(float &pX, float &pY, float &pZ) const { pX = mRx; pY = mRy; pZ = mRz; }

// Explicitly set this object's transformation
void ObjectNode::setTranslate(float pX, float pY, float pZ) { mTx = pX; mTy = pY; mTz = pZ; }
void ObjectNode::setScale(float pX, float pY, float pZ) { mSx = pX; mSy = pY; mSz = pZ; }
void ObjectNode::setRotate(float pX, float pY, float pZ) { mRx = pX; mRy = pY; mRz = pZ; }

// Adjust this object's transformation
void ObjectNode::addTranslate(float pX, float pY, float pZ) { mTx += pX; mTy += pY; mTz += pZ; }
void ObjectNode::addScale(float pX, float pY, float pZ) { mTx *= pX; mTy *= pY; mTz *= pZ; }
void ObjectNode::addRotate(float pX, float pY, float pZ) { mRx += pX; mRy += pY; mRz += pZ; }

// Methods that pass through to the Mesh
void ObjectNode::addGroup(std::string groupName) { mNodeMesh->addGroup(groupName); }
uint32_t ObjectNode::addVertex(float pX, float pY, float pZ) { return mNodeMesh->addRawVertex(pX, pY, pZ); }
uint32_t ObjectNode::addNormal(float pI, float pJ, float pK) { return mNodeMesh->addRawNormal(pI, pJ, pK); }

void ObjectNode::addTriangle(uint32_t vA, uint32_t vB, uint32_t vC, uint32_t nA, uint32_t nB, uint32_t nC)
{
    mNodeMesh->addTriangle(vA, vB, vC, nA, nB, nC);
}

// Parse the raw data (added with above four functions)
bool ObjectNode::parseRawData(bool pReScale, bool pReCenter, bool pAllowEmpty)
{
    // Not valid for a mesh group node
    if(mMeshGroup != NULL) return false;

    // Process raw data in the mesh
    if(mNodeMesh->parseRawData(pAllowEmpty))
    {
        // Scale to unit size based on the 'unit scale' of the mesh
        if(pReScale)
        {
            double lUnitScale = mNodeMesh->getUnitScale();
            setScale(lUnitScale, lUnitScale, lUnitScale);
        }

        // Translate so the object is roughly centered
        if(pReCenter)
        {
            double lXc, lYc, lZc;
            mNodeMesh->getCenter(lXc, lYc, lZc);
            setTranslate(-lXc, -lYc, -lZc);
        }

        // Indicate success
        return true;
    }

    // Indicate failure
    return false;
}

// Recompute all vertex normals so there is 1 per face
void ObjectNode::computePerFaceNormals(bool pOverwrite)
{
    // Not valid for a mesh group node
    if(mMeshGroup != NULL) return;

    // Compute face normals for the node mesh
    mNodeMesh->computePerFaceNormals(pOverwrite);

    // Iterate over all childen and compute their face normals
    QList<ObjectNode*>::iterator i = mNodeChildren.begin();
    while(i != mNodeChildren.end())
    {
        (*i)->computePerFaceNormals(pOverwrite);
        i++;
    }
}

// Smooth all the normals by averaging neighboring vertices
void ObjectNode::smoothNormals()
{
    // Not valid for a mesh group node
    if(mMeshGroup != NULL) return;

    // Smooth the node meshes face normals
    mNodeMesh->smoothNormals(15);

    // Iterate over all childen and smooth their normals
    QList<ObjectNode*>::iterator i = mNodeChildren.begin();
    while(i != mNodeChildren.end())
    {
        (*i)->smoothNormals();
        i++;
    }
}

// Read an OBJ file into the mesh for this node
bool ObjectNode::readFile(std::string pFilename, bool pReScale, bool pReCenter)
{
    // Not valid for a mesh group node
    mMeshGroup = NULL;

    // Read object data from an OBJ file
    mNodeMesh->readFile(pFilename);

    // Scale to unit size based on the 'unit scale' of the mesh
    if(pReScale)
    {
        double lUnitScale = mNodeMesh->getUnitScale();
        setScale(lUnitScale, lUnitScale, lUnitScale);
    }

    // Translate so the object is roughly centered
    if(pReCenter)
    {
        double lXc, lYc, lZc;
        mNodeMesh->getCenter(lXc, lYc, lZc);
        setTranslate(-lXc, -lYc, -lZc);
    }

    // For now, always indicate success
    return true;
}

// Write the mesh for this node to an OBJ file
bool ObjectNode::writeFile(std::string pFilename) const
{
    // Not valid for a mesh group node
    if(mMeshGroup != NULL) return false;

    // Read object data from an OBJ file
    return mNodeMesh->writeOBJFile(pFilename);
}

// Draw this object and all of its children
// Make sure that the transformation of this object is applied to all its children
void ObjectNode::draw(GLRenderMatrixContext &pMatrix, QGLShaderProgram *pShader) const
{
    // Save the model transformation matrix within the given GLRenderMatrixContext
    pMatrix.pushModelMatrix();
    applyTransformation(pMatrix);

    // Draw the node mesh
    if(mMtrl != NULL && pShader != NULL) mMtrl->TellOpenGL(pShader, true);
    if(mMeshGroup == NULL)
    {
        if(!mDontDrawMesh) mNodeMesh->draw();
    }
    else mNodeMesh->drawDirect(mMeshGroup);

    // Iterate over all childen and draw them
    QList<ObjectNode*>::const_iterator i = mNodeChildren.begin();
    while(i != mNodeChildren.end())
    {
        glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT);
        (*i)->draw(pMatrix, pShader);
        glPopAttrib();

        i++;
    }

    // Restore the model transformation matrix within the given GLRenderMatrixContext
    pMatrix.popModelMatrix();
}

// Draw this object and all of its children
// Make sure that the transformation of this object is applied to all its children
void ObjectNode::drawNormals(GLRenderMatrixContext &pMatrix) const
{
    // Save the model transformation matrix within the given GLRenderMatrixContext
    applyTransformation(pMatrix);
    pMatrix.pushModelMatrix();

    // Draw the node mesh
    if(mMeshGroup == NULL)
    {
        if(!mDontDrawMesh) mNodeMesh->drawNormals();
    }
    else mNodeMesh->drawNormalsDirect(mMeshGroup);

    // Iterate over all childen and draw them
    QList<ObjectNode*>::const_iterator i = mNodeChildren.begin();
    while(i != mNodeChildren.end())
    {
        (*i)->drawNormals(pMatrix);
        i++;
    }

    //Restore the model transformation matrix within the given GLRenderMatrixContext
    pMatrix.popModelMatrix();
}

inline void ObjectNode::applyTransformation(GLRenderMatrixContext &pMatrix) const
{
    // Apply the transformation for this object to the given GLRenderMatrixContext reference.
    pMatrix.scale(mSx, mSy, mSz);
    pMatrix.translate(-mRotCentX, -mRotCentY, -mRotCentZ);
    pMatrix.rotate(mRz,0,0,1);
    pMatrix.rotate(mRx,1,0,0);
    pMatrix.rotate(mRy,0,1,0);
    pMatrix.translate(mRotCentX,mRotCentY,mRotCentZ);
    pMatrix.translate(mTx,mTy,mTz);

}
