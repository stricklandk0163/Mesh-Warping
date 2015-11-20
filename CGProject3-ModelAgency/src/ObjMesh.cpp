#include "ObjMesh.h"
#include "Tesselator.h"

#include <float.h>
#include <math.h>
#ifndef M_PI
#define M_PI	3.14159265358979
#endif

#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include <fstream>
#include <sstream>
using namespace std;


#include <QtAlgorithms>
#include <QList>
bool vertexSort(const vertex &s1, const vertex &s2)
{
    if(fabs(s1.X - s2.X) < 1e-8)
    {
        if(fabs(s1.Y - s2.Y) < 1e-8)
        {
            return s1.Z < s2.Z;
        }
        else return (s1.Y < s2.Y);
    }
    else return (s1.X < s2.X);
}

double vertexDist(const vertex &A, const vertex &B)
{
    vertex C = { A.X - B.X, A.Y - B.Y, A.Z - B.Z };
    return sqrt(C.X*C.X + C.Y*C.Y + C.Z*C.Z);
}

// If you find your program crashes when it tries to tesselate a face then uncomment this line
//#define DISABLE_FACE_TESSELATION

#define CHECK_OPENGL_ERROR(A)  { \
    GLuint err = glGetError(); \
    if(err != GL_NO_ERROR) cout << "Line " << (A) << ": " << gluErrorString(err) << endl; \
    }

struct sortVertex
{
    GLfloat X, Y, Z;
    uint32_t oldIdx;
};

struct sortFace
{
    uint32_t group, i;
};

bool operator<(const sortVertex &A, const sortVertex &B)
{
    return ((A.X < B.X) ||
            (A.X == B.X && A.Y < B.Y) ||
            (A.X == B.X && A.Y == B.Y && A.Z < B.Z));
}

bool operator==(const sortVertex &A, const sortVertex &B)
{
    return (fabs(A.X - B.X) < 0.0000001 && fabs(A.Y - B.Y) < 0.0000001 && fabs(A.Z - B.Z) < 0.0000001);
}

bool operator!=(const sortVertex &A, const sortVertex &B)
{
    if(fabs(A.X - B.X) >= 0.0000001) return true;
    else if(fabs(A.Y - B.Y) >= 0.0000001) return true;
    else if(fabs(A.Z - B.Z) >= 0.0000001) return true;
    else return false;
}

GLvoid setVertex(vertex *V, GLfloat X, GLfloat Y, GLfloat Z)
{ V->X = X; V->Y = Y; V->Z = Z; }

MeshGroup::MeshGroup(ObjMesh* newParent)
{
    // Copy parent pointer
    parent = newParent;

    // If no parent was provided, ignore groups
    if(parent != NULL)
    {
        // Create a new unique name
        char namestr[20];
        sprintf(namestr, "unnamed%.4d", (int)parent->counter);
        string name = namestr; parent->counter++;

        uint32_t j;
        for(j=0; j<parent->names.size(); j++)
            if(parent->names[j] == name) break;
        if(j == parent->names.size())
        {
            parent->names.push_back(name);
            parent->enableGrp.push_back(true);
        }

        // Add group name index to list
        groups.push_back(j);
    }

    // Default values
    hasNormals = hasColors = hasTexCoords = hasTangents = GL_FALSE;
}

MeshGroup::MeshGroup(ObjMesh* newParent, vector<string> groupNames)
{
    // Copy parent pointer
    parent = newParent;

    // If no parent was provided, ignore groups
    if(parent != NULL)
    {
        // Build the list of group indicators
        for(uint32_t i=0; i<groupNames.size(); i++)
        {
            // Check if group name has already been used. If not, add it to the list.
            uint32_t j;
            for(j=0; j<parent->names.size(); j++)
                if(parent->names[j] == groupNames[i]) break;
            if(j == parent->names.size())
            {
                parent->names.push_back(groupNames[i]);
                parent->enableGrp.push_back(true);
            }

            // Add group name index to list
            groups.push_back(j);
        }
    }

    // Default values
    hasNormals = hasColors = hasTexCoords = hasTangents = GL_FALSE;
}

ostream& MeshGroup::output(ostream &out)
{
    // Sanity check
    if(isEmpty()) return out;

    // Output the group name line (if any)
    if(groups.size() > 0 && parent != NULL)
    {
        out << "g";
        for(uint32_t i=0; i<groups.size(); i++)
            out << " " << parent->names[groups[i]];
        out << endl;
    }

    // Output the faces
    for(uint32_t i=0; i<triangleV.size(); i++)
    {
        // Every third token should start a new line
        if(i%3 == 0) out << "f";

        // Every face has vertices
        out << " " << (triangleV[i]+1);

        // If we also have texcoords or normals add those too
        if(hasTexCoords || hasNormals)
        {
            out << "/";
            if(hasTexCoords && i<triangleTex.size()) out << (triangleTex[i]+1);
            if(hasNormals && i<triangleN.size()) out << "/" << (triangleN[i]+1);
        }

        // End the line every third token before starting a new one
        if(i%3 == 2) out << endl;
    }

    return out;
}

bool MeshGroup::isEmpty()
{
    return (triangleV.empty() && triangleN.empty() && triangleC.empty() &&
            triangleTex.empty() && triangleTan.empty());
}

ObjMesh::ObjMesh() : rawVerts(), rawNorms(), rawColors(), rawTexCords(), rawTangents(), group()
{
    maxX = maxY = maxZ = -FLT_MAX;
    minX = minY = minZ = FLT_MAX;
    mWarned = false;

    center[0] = center[1] = center[2] = 0.0;
    unitScale = 1.0;

    counter = 0;
}

ObjMesh::~ObjMesh()
{
    for(uint32_t i=0; i<group.size(); i++)
        delete group[i];
    group.clear();
}

void ObjMesh::enableGroup(uint32_t idx) { if(idx < enableGrp.size()) enableGrp[idx] = true; }
void ObjMesh::disableGroup(uint32_t idx) { if(idx < enableGrp.size()) enableGrp[idx] = false; }
void ObjMesh::toggleGroup(uint32_t idx) { if(idx < enableGrp.size()) enableGrp[idx] = !enableGrp[idx]; }

uint32_t ObjMesh::getNamedGroupCount() { return (uint32_t)names.size(); }
string ObjMesh::getGroupName(uint32_t idx) { if(idx < names.size()) return names[idx]; return ""; }

bool ObjMesh::isGroupEnabled(uint32_t idx)
{
    if(idx >= enableGrp.size()) return false;
    return enableGrp[idx];
}

bool ObjMesh::doesGroupHaveNormals(uint32_t idx)
{
    if(idx >= group.size()) return false;
    return group[idx]->hasNormals;
}

bool ObjMesh::doesGroupHaveColors(uint32_t idx)
{
    if(idx >= group.size()) return false;
    return group[idx]->hasColors;
}

bool ObjMesh::doesGroupHaveTexCoords(uint32_t idx)
{
    if(idx >= group.size()) return false;
    return group[idx]->hasTexCoords;
}

bool ObjMesh::doesGroupHaveTangents(uint32_t idx)
{
    if(idx >= group.size()) return false;
    return group[idx]->hasTangents;
}

MeshGroup* ObjMesh::extractMeshGroup(uint32_t i)
{
    if(i >= group.size()) return NULL;
    MeshGroup* extractMe = group[i];
//    group.erase(group.begin()+i);
    return extractMe;
}

uint32_t ObjMesh::getMeshGroupCount() { return (int)group.size(); }

uint32_t ObjMesh::getVertexCount() { return (uint32_t)(rawVerts.size()/3.0); }
uint32_t ObjMesh::getFaceCount()
{
    uint32_t faceCount = 0;
    for(uint32_t i=0; i<group.size(); i++)
        faceCount += (uint32_t)(group[i]->triangleV.size()/3.0);
    return faceCount;
}

GLvoid ObjMesh::draw()
{
    CG_GL_ERROR_CHECK;

    // Loop through the groups drawing each one individually
    for(uint32_t i=0; i<group.size(); i++)
        drawDirect(i);

    CG_GL_ERROR_CHECK;
}

inline void ObjMesh::drawDirect(uint32_t i) const
{
    // Is the raw data available to fall back onto?
    if(rawVerts.empty())
    {
        cout << "Error: Raw vertex data missing for group" << i << ".  Cannot draw.\n";
        return;
    }

    // Are the raw indices still available?
    if(i > group.size())
    {
        cout << "Error: Raw triangle index data missing for group" << i << ".  Cannot draw.\n";
        return;
    }

    // Draw the group directly (less efficient)
    MeshGroup* curGroup = group[i];
    drawDirect(curGroup);
}

inline void ObjMesh::drawDirect(const MeshGroup* curGroup) const
{
    // Is this mesh group part of at least 1 enabled named group
    bool groupEnabled = false;
    for(uint32_t j=0; j<curGroup->groups.size(); j++)
        groupEnabled = (groupEnabled || enableGrp[curGroup->groups[j]]);

    if(!groupEnabled) return;

    glBegin(GL_TRIANGLES);
    for(uint32_t j=0; j<curGroup->triangleV.size(); j++)
    {
        if(!rawTexCords.empty() && curGroup->hasTexCoords)
        {
            glTexCoord3f(rawTexCords[3*curGroup->triangleTex[j] + 0],
                         rawTexCords[3*curGroup->triangleTex[j] + 1],
                         rawTexCords[3*curGroup->triangleTex[j] + 2]);
        }

        if(!rawColors.empty() && curGroup->hasColors)
        {
            glColor3f(rawColors[3*curGroup->triangleC[j] + 0],
                      rawColors[3*curGroup->triangleC[j] + 1],
                      rawColors[3*curGroup->triangleC[j] + 2]);
        }

        if(!rawNorms.empty() && curGroup->hasNormals)
        {
            glNormal3f(rawNorms[3*curGroup->triangleN[j] + 0],
                       rawNorms[3*curGroup->triangleN[j] + 1],
                       rawNorms[3*curGroup->triangleN[j] + 2]);
        }

        glVertex3f(rawVerts[3*curGroup->triangleV[j] + 0],
                   rawVerts[3*curGroup->triangleV[j] + 1],
                   rawVerts[3*curGroup->triangleV[j] + 2]);

    }
    glEnd();
}

GLvoid ObjMesh::drawNormals()
{
    CG_GL_ERROR_CHECK;
    // Loop through the groups drawing each one individually
    for(uint32_t i=0; i<group.size(); i++)
        drawNormalsDirect(i);

    CG_GL_ERROR_CHECK;
}

inline void ObjMesh::drawNormalsDirect(uint32_t i) const
{
    // Is the raw data available to fall back onto?
    if(rawVerts.empty() || rawNorms.empty())
    {
        cout << "Error: Raw vertex data missing for group" << i << ".  Cannot draw.\n";
        return;
    }

    // Are the raw indices still available?
    if(i > group.size())
    {
        cout << "Error: Raw triangle index data missing for group" << i << ".  Cannot draw.\n";
        return;
    }

    // Draw the group directly (less efficient)
    MeshGroup* curGroup = group[i];
    drawNormalsDirect(curGroup);
}

inline void ObjMesh::drawNormalsDirect(const MeshGroup* curGroup) const
{
    // Is this mesh group part of at least 1 enabled named group
    bool groupEnabled = false;
    for(uint32_t j=0; j<curGroup->groups.size(); j++)
        groupEnabled = (groupEnabled || enableGrp[curGroup->groups[j]]);

    if(!groupEnabled) return;

    glBegin(GL_LINES);
    for(uint32_t j=0; j<curGroup->triangleV.size(); j++)
    {
        float A[3] = {rawVerts[3*curGroup->triangleV[j] + 0],
                      rawVerts[3*curGroup->triangleV[j] + 1],
                      rawVerts[3*curGroup->triangleV[j] + 2]};

        float B[3] = {A[0] + rawNorms[3*curGroup->triangleN[j] + 0],
                      A[1] + rawNorms[3*curGroup->triangleN[j] + 1],
                      A[2] + rawNorms[3*curGroup->triangleN[j] + 2]};

        glVertex3fv(A);
        glVertex3fv(B);
    }
    glEnd();
}

GLvoid ObjMesh::readFile(string filename)
{
    // Clean up any lingering data
    rawVerts.clear(); rawNorms.clear(); rawColors.clear();
    rawTexCords.clear(); rawTangents.clear();

    for(uint32_t i=0; i<group.size(); i++)
        delete group[i];
    group.clear();

    uint32_t lineNum = 0;
    MeshGroup* curGroup = NULL;

    // Try to open file
    ifstream fin;
    fin.open(filename.c_str());
    if(!fin.is_open()) return;

    // Parse file
    stringstream linein;
    string firstToken, restOfLine;
    while(!fin.eof() && !fin.bad())
    {
        // Ignore comments (remember to skip to end of line)
        if(fin.peek() == '#') { fin.ignore(256, '\n'); continue; }

        // Read characters up to white-space, null character or EOF
        fin >> firstToken;

        // Read the rest of the line (and any line-breaks)
        getline(fin, restOfLine);
        while(restOfLine.find('\\') != string::npos)
        {
            string tempLine;
            restOfLine[restOfLine.find('\\')] = ' ';
            getline(fin, tempLine);
            restOfLine.append(tempLine);
        }

        // Turn line into stringstrem
        linein.clear();
        linein.str(restOfLine);

        // Parse the different line types
        if(firstToken == "g")
        {
            // If it's not empty (and not NULL), store the current group
            if(curGroup != NULL)
            {
                if(curGroup->isEmpty()) delete curGroup;
                else group.push_back(curGroup);
            }

            // Build list of group names
            vector<string> groupNames; string curName;
            linein >> curName;
            while(!linein.fail())
            {
                if(curName != "") groupNames.push_back(curName);
                linein >> curName;
            }

            // Create a new group
            curGroup = new MeshGroup(this, groupNames);
        }

        //		To properly implement smoothing and merging groups they must be completely
        //		Independent from polygonal groups.  This would require big changes and they
        //		don't come up often enough to warrant this change.  They can be safely
        //		ignored if you are not going to do any merging or smoothing.

        //		else if(firstToken == "s")
        //		{
        //		}

        //		else if(firstToken == "mg")
        //		{
        //		}
        //
        else if(firstToken == "v")
        {
            // Read a vertex
            double X, Y, Z;
            linein >> X >> Y >> Z;

            // Check for formatted input failure
            if(!linein.fail())
            {
                // Store vertex
                rawVerts.push_back(X);
                rawVerts.push_back(Y);
                rawVerts.push_back(Z);

                // Update maximums and minimums
                if(X > maxX) maxX = X; if(X < minX) minX = X;
                if(Y > maxY) maxY = Y; if(Y < minY) minY = Y;
                if(Z > maxZ) maxZ = Z; if(Z < minZ) minZ = Z;

                //				printf("\tv (%f, %f, %f)\n", X, Y, Z);
            }
            else cout << "Vertex line error: '" << restOfLine << "'\n";
        }

        else if(firstToken == "vn")
        {
            // Read a normal
            double i, j, k;
            linein >> i >> j >> k;

            // Check for formatted input failure
            if(!linein.fail())
            {
                // Store normal
                rawNorms.push_back(i);
                rawNorms.push_back(j);
                rawNorms.push_back(k);

                //				printf("\tvn (%f, %f, %f)\n", i, j, k);
            }
            else cout << "Normal line error: '" << restOfLine << "'\n";
        }

        else if(firstToken == "vt")
        {
            // Read a texture coordinate (first just 2D)
            double u, v, w = 0.0;
            linein >> u >> v;

            // Check for formatted input failure
            if(!linein.fail())
            {
                // Store texcoord
                rawTexCords.push_back(u);
                rawTexCords.push_back(v);

                // Check for a third dimension
                linein >> w;
                if(!linein.fail()) rawTexCords.push_back(w);
                else rawTexCords.push_back(0.0);

                //				printf("\tvt (%f, %f, %f)\n", u, v, w);
            }
            else cout << "Texture line error: '" << restOfLine << "'\n";
        }

        else if(firstToken == "f")
        {
            // Temporary storage for vertex, normal and texture indices
            vector<uint32_t> tempV, tempTex, tempN;

            // If we get here with a NULL curGroup then it is an unnamed group
            if(curGroup == NULL) curGroup = new MeshGroup(this);

            // Parse each face token
            string curToken;
            while(!linein.eof() && !linein.bad())
            {
                // Move to the next token and check for failure
                linein >> curToken;
                if(linein.fail()) continue;

                // Count the '/' separators and replace with spaces
                std::size_t pos = curToken.find('/', 0), sepCount = 0;
                while(pos != string::npos)
                {
                    sepCount++;
                    curToken[pos] = ' ';
                    pos = curToken.find('/', pos);
                }

                // Extract indices
                uint32_t V = 0, N = 0, T = 0;
                stringstream faceTok(curToken, ios_base::in);
                faceTok >> V;

                if(faceTok.fail()) { cout << "Bad face line token: missing vertex.\n"; continue; }
                tempV.push_back(V);

                if(sepCount > 0)
                {
                    // Attempt to read texture and normal indices
                    faceTok >> T >> N;

                    // Check for failure to read (assume failure indicates only 1 number read)
                    if(!faceTok.fail())
                    {
                        curGroup->hasTexCoords = GL_TRUE;
                        curGroup->hasNormals = GL_TRUE;
                    }
                    else
                    {
                        if(sepCount == 1)
                        {
                            // Only one separator means texture coords included but no normals
                            curGroup->hasTexCoords = GL_TRUE;
                            curGroup->hasNormals = GL_FALSE;
                        }
                        else if(sepCount == 2)
                        {
                            // Two separators means texture coords left blank but normals included
                            curGroup->hasTexCoords = GL_FALSE;
                            curGroup->hasNormals = GL_TRUE;

                            // Switch data to proper variables
                            N = T; T = 0;
                        }
                        else
                        {
                            // Should never have more than two separators
                            cout << "Face line error: too many '/' seperators.\n";
                            continue;
                        }
                    }

                    tempTex.push_back(T);
                    tempN.push_back(N);
                }

                // Only vertices provided, push zeros for other data
                else
                {
                    curGroup->hasTexCoords = GL_FALSE;
                    curGroup->hasNormals = GL_FALSE;
                    tempTex.push_back(0);
                    tempN.push_back(0);
                }
            }

            // Triangulate non-triangular faces
            if(tempV.size() > 3)
            {
#ifdef DISABLE_FACE_TESSELATION
                if(!mWarned)
                {
                    mWarned = true;
                    printf("WARNING!!! Non-triangular face encountered BUT tesselation is disabled.  You may see noticable artifacts.\n");
                    fflush(stdout);
                }

                // Make [0 1 2] into a triangle
                for(uint32_t j=0; j<3; j++)
                {
                    curGroup->triangleV.push_back(tempV[j]-1);
                    if(curGroup->hasNormals && tempN.size() > j) curGroup->triangleN.push_back(tempN[j]-1);
                    if(curGroup->hasTexCoords && tempTex.size() > j) curGroup->triangleTex.push_back(tempTex[j]-1);
                }

                // Make [2 3 0] into a triangle
                for(uint32_t j=2; j<5; j++)
                {
                    if(j==4)
                    {
                        curGroup->triangleV.push_back(tempV[0]-1);
                        if(curGroup->hasNormals && tempN.size() > 0) curGroup->triangleN.push_back(tempN[0]-1);
                        if(curGroup->hasTexCoords && tempTex.size() > 0) curGroup->triangleTex.push_back(tempTex[0]-1);
                    }
                    else
                    {
                        curGroup->triangleV.push_back(tempV[j]-1);
                        if(curGroup->hasNormals && tempN.size() > j) curGroup->triangleN.push_back(tempN[j]-1);
                        if(curGroup->hasTexCoords && tempTex.size() > j) curGroup->triangleTex.push_back(tempTex[j]-1);
                    }
                }
#else
                if(!mWarned)
                {
                    mWarned = true;
                    printf("Non-triangular face encountered, attempting to triangulate.\n");
                    printf("If your program crashes, see ObjMesh.cpp and uncomment the line 19, '#define DISABLE_FACE_TESSELATION'.\n");
                    fflush(stdout);
                }

                tesselateTriangle(tempV, tempN, tempTex);
                if(tempV.size()%3 != 0)
                {
                    printf("ERROR: triangulation did not return triangles (%d).\n", (int)tempV.size());
                    for(uint32_t i=0; i<tempV.size(); i+=3)
                    {
                        printf("\tf%d %d/%d/%d", (int)i/3, (int)tempV[i], (int)tempN[i], (int)tempTex[i]);
                        if(i+1 < tempV.size()) printf(" %d/%d/%d", (int)tempV[i+1], (int)tempN[i+1], (int)tempTex[i+1]);
                        if(i+2 < tempV.size()) printf(" %d/%d/%d", (int)tempV[i+2], (int)tempN[i+2], (int)tempTex[i+2]);
                        printf("\n");
                    }
                }
                else
                {
                    for(uint32_t j=0; j<tempV.size(); j++)
                    {
                        curGroup->triangleV.push_back(tempV[j]-1);
                        if(curGroup->hasNormals && tempN.size() > j) curGroup->triangleN.push_back(tempN[j]-1);
                        if(curGroup->hasTexCoords && tempTex.size() > j) curGroup->triangleTex.push_back(tempTex[j]-1);
                    }
                }
#endif
            }
            else
            {
                for(uint32_t j=0; j<3; j++)
                {
                    curGroup->triangleV.push_back(tempV[j]-1);
                    if(curGroup->hasNormals && tempN.size() > j) curGroup->triangleN.push_back(tempN[j]-1);
                    if(curGroup->hasTexCoords && tempTex.size() > j) curGroup->triangleTex.push_back(tempTex[j]-1);
                }
            }

            lineNum++;
        }
    }

    fin.close();

    // Store the last group
    group.push_back(curGroup);

    // Compute bounding box and optionally use to re-scale and re-center
    parseRawData();
}

GLvoid ObjMesh::addGroup(string groupName)
{
    // Add group to list (all new raw triangles will be added to this group)
    vector<string> groupNames;
    groupNames.push_back(groupName);
    group.push_back(new MeshGroup(this, groupNames));
}

uint32_t ObjMesh::addRawVertex(const float X, const float Y, const float Z)
{
    // Push in the new vertex
    rawVerts.push_back(X);
    rawVerts.push_back(Y);
    rawVerts.push_back(Z);

    // Update the max's and min's
    if(X > maxX) maxX = X; if(X < minX) minX = X;
    if(Y > maxY) maxY = Y; if(Y < minY) minY = Y;
    if(Z > maxZ) maxZ = Z; if(Z < minZ) minZ = Z;

    return (rawVerts.size()/3)-1;
}

uint32_t ObjMesh::addRawNormal(const float i, const float j, const float k)
{
    // Push in the new normal
    rawNorms.push_back(i);
    rawNorms.push_back(j);
    rawNorms.push_back(k);

    return (rawNorms.size()/3)-1;
}

GLvoid ObjMesh::addTriangle(const uint32_t vA, const uint32_t vB, const uint32_t vC,
                            const uint32_t nA, const uint32_t nB, const uint32_t nC)
{
    // Make sure there's at least one group to hold the triangle data
    if(group.size() == 0) group.push_back(new MeshGroup(this));

    // Add the indices to the most recently added group
    group[group.size()-1]->triangleV.push_back(vA);
    group[group.size()-1]->triangleV.push_back(vB);
    group[group.size()-1]->triangleV.push_back(vC);

    if(!rawNorms.empty() && 3*nA < rawNorms.size() && 3*nB < rawNorms.size() && 3*nC < rawNorms.size())
    {
        group[group.size()-1]->hasNormals = true;
        group[group.size()-1]->triangleN.push_back(nA);
        group[group.size()-1]->triangleN.push_back(nB);
        group[group.size()-1]->triangleN.push_back(nC);
    }
}

bool ObjMesh::parseRawData(bool pAllowEmpty)
{
    if(rawVerts.empty() || group.empty()) return pAllowEmpty;

    // Set default values for center and scale
    center[0] = (maxX + minX)/2.0,
    center[1] = (maxY + minY)/2.0,
    center[2] = (maxZ + minZ)/2.0;

    // Compute maximum dimension of axis-aligned bounding box
    GLfloat width = maxX - minX, height = maxY - minY, depth = maxZ - minZ;
    GLfloat maxDim = max(width, max(height, depth));

    // Uniform scale by 2.0/<largest dimension> (ensure between [-1.0 1.0] in all dims)
    unitScale = 2.0/maxDim;

    unsigned int lNumberOfTris = 0;
    for(int i=0; i<(int)group.size(); i++)
        lNumberOfTris += (unsigned int)group[i]->triangleV.size()/3;

    // Output bounding box info
    printf("Mesh loaded with %u vertices and %u triangles in %u parts.\n",
           (unsigned int)rawVerts.size()/3, lNumberOfTris, (unsigned int)group.size());
    printf("\tBounding Box: (%.4f, %.4f, %.4f) -> (%.4f, %.4f, %.4f)\n", minX, minY, minZ, maxX, maxY, maxZ);
    printf("\tCenter:       (%.4f, %.4f, %.4f)\n", center[0], center[1], center[2]);
    printf("\tSize:         <%.4f, %.4f, %.4f> - scale by %.4f\n", maxX - minX, maxY - minY, maxZ - minZ, unitScale);
    fflush(stdout);

    return true;
}

GLvoid ObjMesh::getCenter(double &pX, double &pY, double &pZ) const
{
    pX = center[0];
    pY = center[1];
    pZ = center[2];
}

double ObjMesh::getUnitScale() const { return unitScale; }

GLvoid ObjMesh::tesselateTriangle(vector<uint32_t> &polyV, vector<uint32_t> &polyN, vector<uint32_t> &polyT)
{
    // Build list of vertices, normals and texture coords
    vector<double*> verts;
    for(uint32_t i=0; i<polyV.size(); i++)
    {
        // Allocate a continuous memory array to hold 3 doubles and 3 indices
        double* vertData = (double*)malloc(4*sizeof(double) + 3*sizeof(uint32_t));
        uint32_t* extraData = (uint32_t*)(vertData + 4);

        uint32_t idx = polyV[i]-1;
        vertData[0] = rawVerts[3*idx+0];
        vertData[1] = rawVerts[3*idx+1];
        vertData[2] = rawVerts[3*idx+2];
        vertData[3] = 0.0;

        extraData[0] = polyV[i];
        extraData[1] = polyN[i];
        extraData[2] = polyT[i];

        verts.push_back(vertData);
    }

    // Initiate the tesselation
    Tesselator myTess;
    if(!myTess.process(verts))
    {
        // Tesselation created new vertices which is not yet properly implemented
        // Need to fall back to simpler solution (assume it's a quad and make two triangles)

        // Save a copy of the 4th indices
        uint32_t tempV = polyV[3];
        uint32_t tempN = polyN[3];
        uint32_t tempT = polyT[3];

        // Remove any indices beyond the 3rd
        polyV.resize(3); polyN.resize(3); polyT.resize(3);

        // Add the 3rd, 4th and 1st indices on as a second triangle
        polyV.push_back(polyV[2]);
        polyN.push_back(polyN[2]);
        polyT.push_back(polyT[2]);

        polyV.push_back(tempV);
        polyN.push_back(tempN);
        polyT.push_back(tempT);

        polyV.push_back(polyV[0]);
        polyN.push_back(polyN[0]);
        polyT.push_back(polyT[0]);
    }
    else
    {
        // Retrieve the results
        polyV.clear(); polyN.clear(); polyT.clear();
        for(uint32_t i=0; i<myTess.getResultCount(); i++)
        {
            uint32_t* data = (uint32_t*)myTess.getResultVertex(i);
            if(data[0] != 0)
            {
                polyV.push_back(data[0]);
                polyN.push_back(data[1]);
                polyT.push_back(data[2]);
            }
        }
    }

    // Clean up
    for(uint32_t i=0; i<verts.size(); i++)
        free(verts[i]);
}

bool ObjMesh::weldVertices(double angleThreshold)
{
    // Sort all the vertices
    list<sortVertex> sortedRawVerts;
    for(uint32_t i=0; i<rawVerts.size()/3.0; i++)
    {
        sortVertex newVert = { rawVerts[3*i+0], rawVerts[3*i+1], rawVerts[3*i+2], i };
        sortedRawVerts.push_back(newVert);
    }

    sortedRawVerts.sort();

    // Build face membership lists
    vector<sortFace> *faces = new vector<sortFace>[rawVerts.size()/3];
    for(uint32_t j=0; j<group.size(); j++)
    {
        // Get current group pointer
        MeshGroup* curGroup = group[j];
        if(curGroup == NULL) continue;

        // Compute vertex face membership lists
        for(uint32_t i=0; i<curGroup->triangleV.size(); i+=3)
        {
            uint32_t A = curGroup->triangleV[i];
            uint32_t B = curGroup->triangleV[i+1];
            uint32_t C = curGroup->triangleV[i+2];

            sortFace thisFaceA = { j, i };
            sortFace thisFaceB = { j, i+1 };
            sortFace thisFaceC = { j, i+2 };
            faces[A].push_back(thisFaceA);
            faces[B].push_back(thisFaceB);
            faces[C].push_back(thisFaceC);
        }
    }

    // Move through the list of sorted vertices and remove duplicates
    // re-arranging the face pointers as needed
    uint32_t remCount = 0, curVert = 1;
    double progress = 0.0, cosThresh = cos(angleThreshold/180.0*M_PI);
    list<sortVertex>::iterator i, A, B;
    for(i = sortedRawVerts.begin(); i != sortedRawVerts.end(); i++, curVert++)
    {
        A = B = i; B++;

        // If no consecutive equal vertices, skip it
        if((*A) != (*B)) continue;

        // Compute an average normal for A
        uint32_t AIdx = (*A).oldIdx;
        double avgN[3] = { 0.0, 0.0, 0.0 };
        //		Vector3D avgN(0, 0, 0);
        for(uint32_t k=0; k<faces[AIdx].size(); k++)
        {
            // Retrieve group that face belongs to
            MeshGroup *curGroup = group[ faces[AIdx][k].group ];
            if(curGroup == NULL || !curGroup->hasNormals) continue;

            // Retrieve normal corresponding to face and add to N
            uint32_t Nidx = curGroup->triangleN[ faces[AIdx][k].i ];
            avgN[0] += rawNorms[3*Nidx+0];
            avgN[1] += rawNorms[3*Nidx+1];
            avgN[2] += rawNorms[3*Nidx+2];
        }

        // Make unit length
        double len = sqrt(VEC_LEN_SQ(avgN));
        if(len > 1e-10) { NORMALIZE(avgN, len); }

        // Identify consecutive equal vertices
        while((*A) == (*B))
        {
            // Replace any references to vertex B with references to vertex A
            uint32_t BIdx = (*B).oldIdx;
            for(uint32_t j=0; j<faces[BIdx].size(); j++)
            {
                // Retrieve group that face belongs to
                MeshGroup *curGroup = group[ faces[BIdx][j].group ];
                if(curGroup->hasNormals)
                {
                    // Retrieve normal corresponding to face
                    uint32_t Nidx = curGroup->triangleN[ faces[BIdx][j].i ];
                    double N[3] = {rawNorms[3*Nidx+0], rawNorms[3*Nidx+1], rawNorms[3*Nidx+2]};

                    // compute dot product to compare with threshold
                    float dotP = DOTPROD(avgN, N);

                    // Skip this face if below threshold
                    if(dotP < cosThresh) continue;
                }

                // Rewrite the face pointer to point to A instead
                group[ faces[BIdx][j].group ]->triangleV[ faces[BIdx][j].i ] = AIdx;
            }
            B++; remCount++;
        }

        // Advance A one (so that the first vertex is preserved) and erase from A to B
        A++; sortedRawVerts.erase(A, B);

        // Show progress
        double newProgress = 100.0*(curVert/(double)sortedRawVerts.size());
        if(newProgress >= progress)
        {
            cout << "\rWelding duplicate vertices: " << progress << "% "
                 << "(" << curVert << "/" << sortedRawVerts.size()
                 << " - Removed " << remCount << ")";
            cout.flush();
            progress += 1;
        }
    }

    if(remCount == 0) cout << "No duplicates found.\n";

    // Clean up
    delete [] faces;
    return (remCount > 0);
}

bool ObjMesh::computePerFaceNormals(bool overwrite)
{
    // Loop through the groups
    bool changesMade = false;
    if(overwrite) rawNorms.clear();

    for(uint32_t j=0; j<group.size(); j++)
    {
        MeshGroup *curGroup = group[j];
        if(curGroup == NULL) continue;

        // Does this group already have normals?
        if(curGroup->hasNormals && !overwrite) continue;

        // Remove any existing normals
        curGroup->triangleN.clear();
        changesMade = true;

        for(uint32_t i=0; i<curGroup->triangleV.size(); i+=3)
        {
            // Compute the normal for the current triangle
            double N[3];
            computeTriangleNormal(curGroup->triangleV[i], curGroup->triangleV[i+1],
                                  curGroup->triangleV[i+2], N);

            // Add this normal and update the triangle's normal index
            rawNorms.push_back(N[0]);
            rawNorms.push_back(N[1]);
            rawNorms.push_back(N[2]);

            uint32_t normalIndex = (uint32_t)(rawNorms.size()-3)/3;
            curGroup->triangleN.push_back(normalIndex);
            curGroup->triangleN.push_back(normalIndex);
            curGroup->triangleN.push_back(normalIndex);
        }

        curGroup->hasNormals = true;
    }

    return changesMade;
}

inline GLvoid ObjMesh::computeTriangleNormal(uint32_t idx1, uint32_t idx2, uint32_t idx3, double N[])
{
    // Construct edge vectors
    double A[3], B[3];
    A[0] = rawVerts[3*idx1+0] - rawVerts[3*idx2+0];
    A[1] = rawVerts[3*idx1+1] - rawVerts[3*idx2+1];
    A[2] = rawVerts[3*idx1+2] - rawVerts[3*idx2+2];

    B[0] = rawVerts[3*idx1+0] - rawVerts[3*idx3+0];
    B[1] = rawVerts[3*idx1+1] - rawVerts[3*idx3+1];
    B[2] = rawVerts[3*idx1+2] - rawVerts[3*idx3+2];

    // Compute their cross product & normalize
    CROSSPROD(A, B, N);
    double len = sqrt(VEC_LEN_SQ(N));
    if(len > 1e-10) { NORMALIZE(N, len); }
}

GLvoid ObjMesh::smoothNormals(double angleThreshold)
{
    (void)angleThreshold;

    // Create storage for the new per-vertex normals
    float* smoothNorms = new float[rawVerts.size()];

    // Loop through groups and smooth only within a single group
    for(uint32_t j=0; j<group.size(); j++)
    {
        // Get current group pointer
        MeshGroup* curGroup = group[j];
        if(curGroup == NULL) continue;

        // Compute vertex face membership lists
        vector<double> faceNorms;
        vector<uint32_t> *faces = new vector<uint32_t>[rawVerts.size()/3];
        for(uint32_t i=0; i<curGroup->triangleV.size(); i+=3)
        {
            // Grab the three vertex indexes for this face
            uint32_t A = curGroup->triangleV[i];
            uint32_t B = curGroup->triangleV[i+1];
            uint32_t C = curGroup->triangleV[i+2];

            // Update the face membership list for those vertices (they are members of face i)
            faces[A].push_back(i);
            faces[B].push_back(i);
            faces[C].push_back(i);

            // Compute the normal for face i and store it
            double N[3];
            computeTriangleNormal(A, B, C, N);
            faceNorms.push_back(N[0]);
            faceNorms.push_back(N[1]);
            faceNorms.push_back(N[2]);
        }

        // Create a new normal for each vertex in the membership list
        for(uint32_t i=0; i<rawVerts.size()/3; i++)
        {
            // Skip any empty lists (vertex is not in any face in this group)
            if(faces[i].size() == 0) continue;

            // Add up face normals and then normalize
            double newN[3] = { 0.0, 0.0, 0.0 };
            for(uint32_t k=0; k<faces[i].size(); k++)
            {
                newN[0] += faceNorms[faces[i][k] + 0];
                newN[1] += faceNorms[faces[i][k] + 1];
                newN[2] += faceNorms[faces[i][k] + 2];
            }
            double len = sqrt(VEC_LEN_SQ(newN));
            if(len > 1e-10) { NORMALIZE(newN, len); }

            smoothNorms[3*i+0] = newN[0];
            smoothNorms[3*i+1] = newN[1];
            smoothNorms[3*i+2] = newN[2];
        }

        // Replace the triangleN indices to match vertex indices
        curGroup->triangleN.clear();
        for(uint32_t i=0; i<curGroup->triangleV.size(); i++)
            curGroup->triangleN.push_back(curGroup->triangleV[i]);

        // Clean up
        delete [] faces;
    }

    // Replace rawNorms with smoothNorms
    rawNorms.clear();
    for(uint32_t i=0; i<rawVerts.size(); i++)
        rawNorms.push_back(smoothNorms[i]);

    // Clean up
    delete [] smoothNorms;
}

void ObjMesh::computeMeshTangents(bool replaceExisting)
{
    // Check for existing tangents and clear out if allowed
    if(rawTangents.size() != 0 && !replaceExisting) return;
    rawTangents.clear();

    // Loop through the groups and create tangents from
    // texture coordinates
    for(uint32_t grp=0; grp<group.size(); grp++)
    {
        MeshGroup* curGroup = group[grp];

        // Check for existing tangents
        if(curGroup->hasTangents && !replaceExisting)
            continue;

        // Check for normals and texture coordinates (required)
        if(!curGroup->hasNormals || !curGroup->hasTexCoords)
            continue;

        // Compute tangents for each face
        for(uint32_t i=0; i<curGroup->triangleV.size(); i+=3)
        {
            // Copy indices into rawVerts and rawTex
            uint32_t v1 = curGroup->triangleV[i+0];
            uint32_t v2 = curGroup->triangleV[i+1];
            uint32_t v3 = curGroup->triangleV[i+2];

            uint32_t n1 = curGroup->triangleN[i+0];
            uint32_t n2 = curGroup->triangleN[i+1];
            uint32_t n3 = curGroup->triangleN[i+2];

            uint32_t w1 = curGroup->triangleTex[i+0];
            uint32_t w2 = curGroup->triangleTex[i+1];
            uint32_t w3 = curGroup->triangleTex[i+2];

            // Build arrays for the tangent function
            float V[9] = {
                rawVerts[3*v1+0], rawVerts[3*v1+1], rawVerts[3*v1+2],
                rawVerts[3*v2+0], rawVerts[3*v2+1], rawVerts[3*v2+2],
                rawVerts[3*v3+0], rawVerts[3*v3+1], rawVerts[3*v3+2]
            };

            float N[9] = {
                rawNorms[3*n1+0], rawNorms[3*n1+1], rawNorms[3*n1+2],
                rawNorms[3*n2+0], rawNorms[3*n2+1], rawNorms[3*n2+2],
                rawNorms[3*n3+0], rawNorms[3*n3+1], rawNorms[3*n3+2]
            };

            float tex[9] = {
                rawTexCords[3*w1+0], rawTexCords[3*w1+1], rawTexCords[3*w1+2],
                rawTexCords[3*w2+0], rawTexCords[3*w2+1], rawTexCords[3*w2+2],
                rawTexCords[3*w3+0], rawTexCords[3*w3+1], rawTexCords[3*w3+2]
            };

            float tan[9];

            // Compute per-vertex tangents for this face
            computeFaceTangents(V, N, tex, (float*)tan);

            // Add to raw tangents and group indices
            for(uint32_t j=0; j<3; j++)
            {
                rawTangents.push_back(tan[j*3+0]);
                rawTangents.push_back(tan[j*3+1]);
                rawTangents.push_back(tan[j*3+2]);
                curGroup->triangleTan.push_back((uint32_t)(rawTangents.size()/3.0)-1);
            }
        }

        // Indicate tangents are now available
        curGroup->hasTangents = GL_TRUE;
    }
}

GLvoid ObjMesh::computeFaceTangents(float v[9], float n[9], float tex[9], float* tan)
{
    // Compute vertex gradients
    float x1 = v[3] - v[0];  //v2.X - v1.X;
    float x2 = v[6] - v[0];  //v3.X - v1.X;

    float y1 = v[4] - v[1];  //v2.Y - v1.Y;
    float y2 = v[7] - v[1];  //v3.Y - v1.Y;

    float z1 = v[5] - v[2];  //v2.Z - v1.Z;
    float z2 = v[8] - v[2];  //v3.Z - v1.Z;

    // Compute texture gradients
    float s1 = tex[3] - tex[0];  //w2.X - w1.X;
    float s2 = tex[6] - tex[0];  //w3.X - w1.X;

    float t1 = tex[4] - tex[1];  //w2.Y - w1.Y;
    float t2 = tex[7] - tex[1];  //w3.Y - w1.Y;

    // Compute normalization factor
    float r = 1.0F/(s1*t2 - s2*t1);

    // Store values (normalized)
    float sdir[3] = { (t2*x1 - t1*x2)*r, (t2*y1 - t1*y2)*r, (t2*z1 - t1*z2)*r };
    float tdir[3] = { (s1*x2 - s2*x1)*r, (s1*y2 - s2*y1)*r, (s1*z2 - s2*z1)*r };

    // Compute per-vertex tangent
    for(uint32_t i=0; i<3; i++)
    {
        // Gram-Schmidt orthogonalize
        double NDotT = n[3*i+0]*sdir[0] + n[3*i+1]*sdir[1] + n[3*i+2]*sdir[2];
        double A[3] = { sdir[0] - n[3*i+0]*NDotT, sdir[1] - n[3*i+1]*NDotT, sdir[2] - n[3*i+2]*NDotT };

        // Normalize
        double length = sqrt(A[0]*A[0] + A[1]*A[1] + A[2]*A[2]);
        A[0] /= length; A[1] /= length; A[2] /= length;

        // Calculate handedness and flip tangent if necessary
        double nCROSSt[3] = { n[3*i+1]*sdir[2] - n[3*i+2]*sdir[1],
                              n[3*i+2]*sdir[0] - n[3*i+0]*sdir[2],
                              n[3*i+0]*sdir[1] - n[3*i+1]*sdir[0] };

        double dotProd = nCROSSt[0]*tdir[0] + nCROSSt[1]*tdir[1] + nCROSSt[2]*tdir[2];

        if(dotProd < 0.0)
        {
            // Flip tangent
            A[0] = -A[0];
            A[1] = -A[1];
            A[2] = -A[2];
        }

        // Store
        tan[3*i+0] = A[0];
        tan[3*i+1] = A[1];
        tan[3*i+2] = A[2];
    }
}

bool ObjMesh::writeOBJFile(string filename)
{
    // Open the file handle
    ofstream fout; fout.open(filename.c_str());
    if(!fout.is_open()) return false;

    // Count total number of faces
    uint32_t totalFaces = 0;
    for(uint32_t i=0; i<group.size(); i++)
        if(group[i] != NULL) totalFaces += (uint32_t)group[i]->triangleV.size()/3;

    // Write a brief header
    fout << "# " << filename << " generated by lightfield library from an ObjMesh object." << endl;
    fout << "# " << rawVerts.size()/3 << " vertices, ";
    if(rawNorms.size() > 0) fout << rawNorms.size()/3 << " normals, ";
    if(rawTexCords.size() > 0) fout << rawTexCords.size()/2 << " texture coordinates, ";
    fout << "and " << totalFaces << " faces in " << group.size() << " groups." << endl << endl;

    // Write the raw arrays
    for(uint32_t i=0; i<rawVerts.size()-2; i+=3)
        fout << "v " << rawVerts[i] << " " << rawVerts[i+1] << " " << rawVerts[i+2] << endl;
    fout << endl;

    if(rawNorms.size() > 0)
    {
        for(uint32_t i=0; i<rawNorms.size()-2; i+=3)
            fout << "vn " << rawNorms[i] << " " << rawNorms[i+1] << " " << rawNorms[i+2] << endl;
        fout << endl;
    }

    if(rawTexCords.size() > 0)
    {
        for(uint32_t i=0; i<rawTexCords.size()-2; i+=3)
            fout << "vt " << rawTexCords[i] << " " << rawTexCords[i+1] << " " << rawTexCords[i+2] << endl;
        fout << endl;
    }

    // Write each polygon group
    for(uint32_t i=0; i<group.size(); i++)
    {
        MeshGroup* curGrp = group[i];
        if(curGrp == NULL) continue;

        // Write the group name list
        fout << "g ";
        for(uint32_t j=0; j<curGrp->groups.size(); j++)
            fout << names[curGrp->groups[j]] << " ";
        fout << endl;

        // Write out all the faces (in our case, always triangles)
        for(uint32_t j=0; j<curGrp->triangleV.size()-2; j+=3)
        {
            if(curGrp->hasTexCoords && curGrp->hasNormals)
            {
                fout << "f " << curGrp->triangleV[j]+1 << "/" << curGrp->triangleTex[j]+1 << "/" << curGrp->triangleN[j]+1 << " "
                             << curGrp->triangleV[j+1]+1 << "/" << curGrp->triangleTex[j+1]+1 << "/" << curGrp->triangleN[j+1]+1 << " "
                             << curGrp->triangleV[j+2]+1 << "/" << curGrp->triangleTex[j+2]+1 << "/" << curGrp->triangleN[j+2]+1 << endl;
            }
            else if(curGrp->hasTexCoords)
            {
                fout << "f " << curGrp->triangleV[j]+1 << "/" << curGrp->triangleTex[j]+1 << " "
                             << curGrp->triangleV[j+1]+1 << "/" << curGrp->triangleTex[j+1]+1 << " "
                             << curGrp->triangleV[j+2]+1 << "/" << curGrp->triangleTex[j+2]+1 << endl;
            }
            else if(curGrp->hasNormals)
            {
                fout << "f " << curGrp->triangleV[j]+1 << "//" << curGrp->triangleN[j]+1 << " "
                             << curGrp->triangleV[j+1]+1 << "//" << curGrp->triangleN[j+1]+1 << " "
                             << curGrp->triangleV[j+2]+1 << "//" << curGrp->triangleN[j+2]+1 << endl;
            }
            else
            {
                fout << "f " << curGrp->triangleV[j]+1 << " "
                             << curGrp->triangleV[j+1]+1 << " "
                             << curGrp->triangleV[j+2]+1 << endl;
            }
        }

        fout << endl;
    }

    // Close the file
    fout.close();
    return true;
}

bool ObjMesh::compare(const ObjMesh &other)
{
    // Build local copies of vertices and normals
    QList<vertex> myVerts, myNorms;
    for(int i=0; i<(int)rawVerts.size() || i < (int)rawNorms.size(); i+=3)
    {
        if(i+2 < (int)rawVerts.size())
        {
            vertex curVert = { rawVerts[i+0], rawVerts[i+1], rawVerts[i+2] };
            myVerts.push_back(curVert);
        }

        if(i+2 < (int)rawNorms.size())
        {
            vertex curNorm = { rawVerts[i+0], rawVerts[i+1], rawVerts[i+2] };
            myNorms.push_back(curNorm);
        }
    }

    QList<vertex> otherVerts, otherNorms;
    for(int i=0; i<(int)other.rawVerts.size() || i < (int)other.rawNorms.size(); i+=3)
    {
        if(i+2 < (int)other.rawVerts.size())
        {
            vertex curVert = { other.rawVerts[i+0], other.rawVerts[i+1], other.rawVerts[i+2] };
            otherVerts.push_back(curVert);
        }

        if(i+2 < (int)other.rawNorms.size())
        {
            vertex curNorm = { other.rawVerts[i+0], other.rawVerts[i+1], other.rawVerts[i+2] };
            otherNorms.push_back(curNorm);
        }
    }

    // Sort local copies
    qSort(myVerts.begin(), myVerts.end(), vertexSort);
    qSort(myNorms.begin(), myNorms.end(), vertexSort);

    qSort(otherVerts.begin(), otherVerts.end(), vertexSort);
    qSort(otherNorms.begin(), otherNorms.end(), vertexSort);

    // Compare and report statistics
    double diffV = 0;
    for(int i=0; i<myVerts.size() && i < otherVerts.size(); i++)
    {
        diffV += vertexDist(myVerts[i], otherVerts[i]);
    }

    double diffN = 0;
    for(int i=0; i<myNorms.size() && i < otherNorms.size(); i++)
    {
        diffN += vertexDist(myNorms[i], otherNorms[i]);
    }

    printf("\tVertex: %d -- %d (%.6f)\n", myVerts.size(), otherVerts.size(), diffV);
    printf("\tNormal: %d -- %d (%.6f)\n", myNorms.size(), otherNorms.size(), diffN);

    return (diffV < 1e-6 && diffN < 1e-6);
}
