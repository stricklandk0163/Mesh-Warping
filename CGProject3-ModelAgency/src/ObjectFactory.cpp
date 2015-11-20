#define _USE_MATH_DEFINES
#include <cmath>
#include "ObjectFactory.h"
#include "ObjectNode.h"
#include <QFileInfo>
#include <QMessageBox>

ObjectNode* ObjectFactory::loadMesh(const QString &pMeshFile)
{
    // Allocate a new node object
    ObjectNode* lObject = new ObjectNode();

    // Read an OBJ mesh file (Note: this will automatically center and scale the object to unit size)
    lObject->readFile(pMeshFile.toStdString(), true, true);
    lObject->meshGroupsToChildren();
    lObject->setName(QFileInfo(pMeshFile).baseName());

    return lObject;
}

ObjectNode* ObjectFactory::buildCube()
{
    // A counter used to name the cubes
    static int lsCubeCount = 1;

    // Allocate a new node object
    ObjectNode* lObject = new ObjectNode();
    lObject->setName(QString("Cube %1").arg(lsCubeCount));
    lsCubeCount++;

    // Add the bottom four vertices
    lObject->addVertex(-1.0, -1.0, -1.0);  // 0
    lObject->addVertex( 1.0, -1.0, -1.0);  // 1
    lObject->addVertex( 1.0, -1.0,  1.0);  // 2
    lObject->addVertex(-1.0, -1.0,  1.0);  // 3

    // Add the top four vertices
    lObject->addVertex(-1.0,  1.0, -1.0);  // 4
    lObject->addVertex( 1.0,  1.0, -1.0);  // 5
    lObject->addVertex( 1.0,  1.0,  1.0);  // 6
    lObject->addVertex(-1.0,  1.0,  1.0);  // 7

    // Add the six face normals
    lObject->addNormal( 1.0, 0.0, 0.0);    // 0
    lObject->addNormal(-1.0, 0.0, 0.0);    // 1

    lObject->addNormal(0.0,  1.0, 0.0);    // 2
    lObject->addNormal(0.0, -1.0, 0.0);    // 3

    lObject->addNormal(0.0, 0.0,  1.0);    // 4
    lObject->addNormal(0.0, 0.0, -1.0);    // 5

    // Add the 12 faces (six square faces split into triangles)

    // Bottom face (-Y)
    lObject->addTriangle(0, 1, 2, 3, 3, 3);
    lObject->addTriangle(2, 3, 0, 3, 3, 3);

    // Top face (Y)
    lObject->addTriangle(7, 6, 5, 2, 2, 2);
    lObject->addTriangle(5, 4, 7, 2, 2, 2);

    // Front face (Z)
    lObject->addTriangle(3, 2, 6, 4, 4, 4);
    lObject->addTriangle(6, 7, 3, 4, 4, 4);

    // Right face (X)
    lObject->addTriangle(2, 1, 5, 0, 0, 0);
    lObject->addTriangle(5, 6, 2, 0, 0, 0);

    // Back face (-Z)
    lObject->addTriangle(1, 0, 4, 5, 5, 5);
    lObject->addTriangle(4, 5, 1, 5, 5, 5);

    // Left face (-X)
    lObject->addTriangle(0, 3, 7, 1, 1, 1);
    lObject->addTriangle(7, 4, 0, 1, 1, 1);

    // Process the vertex, normal and face data just added to the object
    if(!lObject->parseRawData())
    {
        printf("There was an error creating the cube node object.\n");
        fflush(stdout);

        // Cleanup
        delete lObject;
        return NULL;
    }

    return lObject;
}

ObjectNode* ObjectFactory::buildCylinder(int pSlices)
{
    // A counter used to name the cylinders
    static int lsCylinderCount = 1;

    // Allocate a new node object
    ObjectNode* lObject = new ObjectNode();
    lObject->setName(QString("Cylinder %1").arg(lsCylinderCount));
    lsCylinderCount++;

    //Variable for conversion
    double degreesToRadians = M_PI / 180.0;

    //Figure out change in angle per slice
    double deltaTheta = 360.0/pSlices;

    //Add a vertex and normal at center of the top of the cylinder
    lObject->addVertex(0,1,0);
    lObject->addNormal(0,1,0);

    //Create top face of cylinder
    for(double theta = -180.0; theta <= 180.0; theta +=deltaTheta)
    {
        double thetaRadians = theta*degreesToRadians;
        lObject->addVertex(sin(thetaRadians), 1,cos(thetaRadians));
        lObject->addNormal(sin(thetaRadians), 0,cos(thetaRadians));

    }

    //Add a vertex and normal at the center of the bottom of the cylinder
    lObject->addVertex(0,-1,0);
    lObject->addNormal(0,-1,0);

    //Create top face of cylinder
    for(double theta = -180.0; theta <= 180.0; theta +=deltaTheta)
    {
        double thetaRadians = theta*degreesToRadians;
        lObject->addVertex(sin(thetaRadians), -1,cos(thetaRadians));

    }

    //Draw triangles which form squares which form the sides of the cylinder
    for(int i = 1; i<=pSlices; i++)
    {
        lObject->addTriangle(0,i, i+1,  0, 0, 0);
        lObject->addTriangle( i+pSlices+2,pSlices+2, i+pSlices+3, pSlices+2, pSlices+2,  pSlices+2);
        lObject->addTriangle(i,i+pSlices+3, i+1,i,i+1,i+1);
        lObject->addTriangle(i+pSlices+2,i+pSlices+3,i,i, i+1, i );
    }

    // Process the vertex, normal and face data just added to the object
    if(!lObject->parseRawData())
    {
        printf("There was an error creating the cylinder node object.\n");
        fflush(stdout);

        // Cleanup
        delete lObject;
        return NULL;
    }

    return lObject;
}

ObjectNode* ObjectFactory::buildSphere(int pSlices, int pStacks)
{
    // A counter used to name the spheres
    static int lsSphereCount = 1;

    // Allocate a new node object
    ObjectNode* lObject = new ObjectNode();
    lObject->setName(QString("Sphere %1").arg(lsSphereCount));
    lsSphereCount++;

    //The change in phi (hieght of the stacks we will be using)
    double deltaPhi = 180.0/pStacks;

    //The change in tehta used to compute the distance of each vertice at each level (stack)
    double deltaTheta = 360.0/pSlices;

    //Multiply a degree amount by this value to convert it to radians
    double degreesToRadians = M_PI/180;

    //We want to work from the bottom of the sphere to the top
    for(double phi = -90.0; phi<=90.0; phi+=deltaPhi)
    {
        //the height of the stack we are on in radians
        double phir1 = phi*degreesToRadians;

        //Work our way around each stack in the sphere
        for(float theta = -180.0; theta <=180.0; theta += deltaTheta)
        {
            //The change in the angle between each vertice on the stack
            float thetar = theta*degreesToRadians;

            lObject->addVertex(sin(thetar)*cos(phir1), cos(thetar)*cos(phir1), sin(phir1));
            lObject->addNormal(sin(thetar)*cos(phir1), cos(thetar)*cos(phir1), sin(phir1));
        }
    }
    //Create the top of the sphere using a triangle fan
    for(int k = 0; k<=pSlices; k++)
    {
        lObject->addTriangle(0, pSlices+k+1, pSlices+k+2, 0, pSlices+k+1, pSlices+k+2);
    }

    //Create the rest of the sphere by connecting the vertices we calculated earlier
    //Loop which allows us to work with every stack except the top one
    for(int j = 1; j<pStacks; j++)
    {
        //Loop that allows us to work with every slice on each stack
        for(int k = 0; k<pSlices; k++)
        {
            //Creates triangles which connect both vertices in a slice on a stack to the first vertice on the stack one lower than it in the same slice
            lObject->addTriangle(j*(pSlices+1)+k,(j+1)*(pSlices+1)+k, j*(pSlices+1)+k+1,  j*(pSlices+1)+k,(j+1)*(pSlices+1)+k, j*(pSlices+1)+k+1 );

            //Creates triangles which connect both vertices in a slice on a stack to the second vertice on the stack one higher than it in the same slice
            lObject->addTriangle((j+1)*(pSlices+1)+k,(j+1)*(pSlices+1)+k+1,j*(pSlices+1)+k+1, (j+1)*(pSlices+1)+k, (j+1)*(pSlices+1)+k+1,j*(pSlices+1)+k+1 );

        }
    }


    // Process the vertex, normal and face data just added to the object
    if(!lObject->parseRawData())
    {
        printf("There was an error creating the sphere node object.\n");
        fflush(stdout);

        // Cleanup
        delete lObject;
        return NULL;
    }

    return lObject;
}

ObjectNode* ObjectFactory::buildSculpture()
{
    // Allocate a new node object
    ObjectNode* lObject = new ObjectNode();
    lObject->setName("Sculpture");

    //Make the struts of the object
    ObjectNode *strut1;
    strut1 = buildSculptureStrut();
    strut1->setTranslate(1.5, 0, 0);
    strut1->setRotate(0,90,0);
    lObject->addChild(strut1);

    ObjectNode *strut2;
    strut2 = buildSculptureStrut();
    strut2->setTranslate(1.5, 0, 0);
    strut2->setRotate(0,-90,0);
    lObject->addChild(strut2);

    ObjectNode *strut3;
    strut3 = buildSculptureStrut();
    strut3->setTranslate(1.5, 0, 0);
    strut3->setRotate(0,0,0);
    lObject->addChild(strut3);

    ObjectNode *strut4;
    strut4 = buildSculptureStrut();
    strut4->setTranslate(1.5, 0, 0);
    strut4->setRotate(0,180,0);
    lObject->addChild(strut4);


    //Make the base of the object
    ObjectNode *base;
    base = buildCylinder(20);
    base->setScale(1.9f, 0.01f, 1.9f);
    lObject->addChild(base);

    //Make the strut connector (point where the struts meet)
    ObjectNode *strutConnector;
    strutConnector = buildCylinder(20);
    strutConnector->setTranslate(0,7.4f, 0);
    strutConnector->setScale(0.2f,0.2f,0.2f);
    lObject->addChild(strutConnector);

    //Make a string to hang our discoball from
    ObjectNode *discoBallString;
    discoBallString = buildCylinder(20);
    discoBallString->setTranslate(0,6.0f, 0);
    discoBallString->setScale(0.05f,0.2f,0.05f);
    lObject->addChild(discoBallString);

    //Make a disco ball to hang from our string!
    ObjectNode *discoBall;
    discoBall = buildSphere(20,20);
    discoBall->setTranslate(0,4.0f, 0);
    discoBall->setScale(0.25f,0.25f,0.25f);
    lObject->addChild(discoBall);
    return lObject;
}

//Makes a strut for the sculpture
ObjectNode* ObjectFactory::buildSculptureStrut()
{
    // Allocate a new node object
    ObjectNode* lObject = new ObjectNode();
    lObject->setName("SculptureStrut");

    //Strut is made out of cylinders which are progressively moved up and inward towards the center of the object while being rotated more each time
    ObjectNode *cylinder1;
    cylinder1 = buildCylinder(20);
    cylinder1->setScale(0.2f, 0.2f, 0.2f);
    cylinder1->setTranslate(1,1,0);
    lObject->addChild(cylinder1);

    ObjectNode *cylinder2;
    cylinder2 = buildCylinder(20);
    cylinder2->setScale(0.2f, 0.2f, 0.2f);
    cylinder2->setTranslate(1.4f, 2.4f, 0);
    cylinder2->setRotate(0,0,15);
    lObject->addChild(cylinder2);

    ObjectNode *cylinder3;
    cylinder3 = buildCylinder(20);
    cylinder3->setScale(0.2f, 0.2f, 0.2f);
    cylinder3->setTranslate(2.2f, 3.6f, 0);
    cylinder3->setRotate(0,0,30);
    lObject->addChild(cylinder3);

    ObjectNode *cylinder4;
    cylinder4 = buildCylinder(20);
    cylinder4->setScale(0.2f, 0.2f, 0.2f);
    cylinder4->setTranslate(3.3f, 4.6f, 0);
    cylinder4->setRotate(0,0,45);
    lObject->addChild(cylinder4);

    ObjectNode *cylinder5;
    cylinder5 = buildCylinder(20);
    cylinder5->setScale(0.2f, 0.2f, 0.2f);
    cylinder5->setTranslate(4.6f, 5.2f, 0);
    cylinder5->setRotate(0,0,60);
    lObject->addChild(cylinder5);

    ObjectNode *cylinder6;
    cylinder6 = buildCylinder(20);
    cylinder6->setScale(0.2f, 0.2f, 0.2f);
    cylinder6->setTranslate(6.0f, 5.6f, 0);
    cylinder6->setRotate(0,0,75);
    lObject->addChild(cylinder6);

    ObjectNode *cylinder7;
    cylinder7 = buildCylinder(20);
    cylinder7->setScale(0.2f, 0.2f, 0.2f);
    cylinder7->setTranslate(7.45f, 5.65f, 0);
    cylinder7->setRotate(0,0,90);
    lObject->addChild(cylinder7);

    return lObject;
}

//Builds a humanoid with a torso, arms, legs and a head
ObjectNode* ObjectFactory::buildHumanoid()
{
    // Allocate a new node object
    ObjectNode* lObject = new ObjectNode();
    lObject->setName("Humanoid");

    ObjectNode* torso = new ObjectNode();
    torso = buildTorso();
    lObject->addChild(torso);

    ObjectNode* leftArm = new ObjectNode();
    leftArm = buildArm();
    leftArm->setTranslate(0.0f,.37f,1.0f);
    leftArm->setName("Left Arm");
    torso->addChild(leftArm);

    ObjectNode* rightArm = new ObjectNode();
    rightArm = buildArm();
    rightArm->setRotate(0, 180, 0);
    rightArm->setTranslate(0.0f,.37f,1.0f);
    rightArm->setName("Right Arm");
    torso->addChild(rightArm);

    ObjectNode* leftLeg = new ObjectNode();
    leftLeg = buildLeg();
    leftLeg->setTranslate(0.0f,1.1f,-0.4f);
    torso->addChild(leftLeg);

    ObjectNode* rightLeg = new ObjectNode();
    rightLeg = buildLeg();
    rightLeg->setTranslate(0.0f,1.1f,0.4f);
    torso->addChild(rightLeg);

    ObjectNode* head = new ObjectNode();
    head = buildHead();
    head->setTranslate(0.0f, 1.0f, 0.0f);
    torso->addChild(head);

    return lObject;
}

//Builds the torso/ shoulders of the humanoid
ObjectNode* ObjectFactory::buildTorso()
{
    ObjectNode* lObject = new ObjectNode();
    lObject->setName("Torso");

    ObjectNode* body = new ObjectNode();
    body->setName("Body");
    body = buildCube();
    body->setScale(.53f,.98f,.75f);
    lObject->addChild(body);

    ObjectNode* shoulders = new ObjectNode();
    shoulders->setName("Shoulders");
    shoulders = buildCylinder(20);
    shoulders->setScale(.4f,.37f,.95f);
    shoulders->setRotate(90,0,0);
    shoulders->setTranslate(0,0,-1);
    lObject->addChild(shoulders);

    return lObject;

}

//Builds an arm with a upper arm lower arm and hand
ObjectNode* ObjectFactory::buildArm()
{
    ObjectNode* arm = new ObjectNode();
    arm->setName("Arm");

    ObjectNode* upperArmPivot = new ObjectNode();
    upperArmPivot= buildSphere(20,20);
    upperArmPivot->setScale(.35f,.35f,.35f);
    upperArmPivot->setName("Upper Arm Pivot");
    arm->addChild(upperArmPivot);

    ObjectNode* upperArm = new ObjectNode();
    upperArm = buildCylinder(20);
    upperArm->setRotate(90,0,0);
    upperArm->setTranslate(0,1,0);
    upperArm->setName("Upper Arm Cylinder");
    upperArmPivot->addChild(upperArm);

    ObjectNode* lowerArmPivot = new ObjectNode();
    lowerArmPivot = buildSphere(20, 20);
    lowerArmPivot->setTranslate(0.0f,1.0f,0.0f);
    lowerArmPivot->setName("Lower Arm Pivot");
    upperArm->addChild(lowerArmPivot);
    
    ObjectNode* lowerArm = new ObjectNode();
    lowerArm = buildCylinder(20);
    lowerArm->setTranslate(0.0f,1.0f,0.0f);
    lowerArm->setName("Lower Arm");
    lowerArmPivot->addChild(lowerArm);

    ObjectNode* handPivot = new ObjectNode();
    handPivot = buildSphere(20,20);
    handPivot->setTranslate(0.0f,1.0f,0.0f);
    handPivot->setName("HandPivot");
    lowerArm->addChild(handPivot);

    ObjectNode* hand = new ObjectNode();
    hand = buildCube();
    hand->setName("Hand");
    hand->setRotate(0,0,45);
    handPivot->addChild(hand);

    return arm;
}

//Buid the head/ neck of the humanoid
ObjectNode* ObjectFactory::buildHead()
{
    ObjectNode* head = new ObjectNode();
    head->setName("Head");

    ObjectNode* neck = new ObjectNode();
    neck= buildSphere(20, 20);
    neck->setScale(.4f,.4f,.4f);
    neck->setName("Neck");
    head->addChild(neck);

    ObjectNode* face = new ObjectNode();
    face = buildCube();
    face->setScale(1.2f,1.2f,1.2f);
    face->setTranslate(0,1.2f,0);
    face->setName("Face");
    neck->addChild(face);


    return head;
}

ObjectNode* ObjectFactory::buildLeg()
{
    ObjectNode* leg = new ObjectNode();
    leg->setRotate(180,0,0);
    leg->setName("Leg");

    ObjectNode* upperLegPivot = new ObjectNode();
    upperLegPivot= buildSphere(20,20);
    upperLegPivot->setScale(.35f,.35f,.35f);
    upperLegPivot->setName("Upper Leg Pivot");
    leg->addChild(upperLegPivot);

    ObjectNode* upperLeg = new ObjectNode();
    upperLeg = buildCylinder(20);
    upperLeg->setTranslate(0,1,0);
    upperLeg->setName("Upper Leg Cylinder");
    upperLegPivot->addChild(upperLeg);

    ObjectNode* lowerLegPivot = new ObjectNode();
    lowerLegPivot = buildSphere(20, 20);
    lowerLegPivot->setTranslate(0.0f,1.0f,0.0f);
    lowerLegPivot->setName("Lower Leg Pivot");
    upperLeg->addChild(lowerLegPivot);

    ObjectNode* lowerLeg = new ObjectNode();
    lowerLeg = buildCylinder(20);
    lowerLeg->setTranslate(0.0f,1.0f,0.0f);
    lowerLeg->setName("Lower Leg");
    lowerLegPivot->addChild(lowerLeg);

    ObjectNode* ankle = new ObjectNode();
    ankle = buildSphere(20,20);
    ankle->setTranslate(0.0f,1.0f,0.0f);
    ankle->setName("Ankle");
    lowerLeg->addChild(ankle);

    ObjectNode* foot = new ObjectNode();
    foot = buildCube();
    foot->setName("Foot");
    ankle->addChild(foot);

    return leg;
}

ObjectNode* ObjectFactory::buildExtraObject1(double pParam1, double pParam2, double pParam3)
{
    // These commands will suppress the "Unused parameter" warnings
    Q_UNUSED(pParam1); Q_UNUSED(pParam2); Q_UNUSED(pParam3);

    // Allocate a new node object
    ObjectNode* lObject = new ObjectNode();

    // TODO: Create a node object for some other object as suggested in the instructions or approved by the professor
    //    - You may or may not need to use the parameters they are just there in case you need them (it will depend on what shape you choose)
    //    - See buildCube for an example of how to use the ObjectNode object lObject
    //    - Use addVertex, addNormal and addTriangle
    //    - Be sure to leave in the allocation above and the call to parseRawData() below

    // Process the vertex, normal and face data just added to the object
    if(!lObject->parseRawData(false, false, true))
    {
        printf("There was an error creating the node object for extra credit object 1.\n");
        fflush(stdout);

        // Cleanup
        delete lObject;
        return NULL;
    }

    return lObject;
}

ObjectNode* ObjectFactory::buildExtraObject2(double pParam1, double pParam2, double pParam3)
{
    // These commands will suppress the "Unused parameter" warnings
    Q_UNUSED(pParam1); Q_UNUSED(pParam2); Q_UNUSED(pParam3);

    // Allocate a new node object
    ObjectNode* lObject = new ObjectNode();

    // TODO: Create a node object for some other object as suggested in the instructions or approved by the professor
    //    - You may or may not need to use the parameters they are just there in case you need them (it will depend on what shape you choose)
    //    - See buildCube for an example of how to use the ObjectNode object lObject
    //    - Use addVertex, addNormal and addTriangle
    //    - Be sure to leave in the allocation above and the call to parseRawData() below

    // Process the vertex, normal and face data just added to the object
    if(!lObject->parseRawData(false, false, true))
    {
        printf("There was an error creating the node object for extra credit object 2.\n");
        fflush(stdout);

        // Cleanup
        delete lObject;
        return NULL;
    }

    return lObject;
}
