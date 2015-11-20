/** \file ObjMesh.h
 * \brief ObjMesh.h contains the class ObjMesh
 *
 * ObjMesh.h contains the class ObjMesh and nothing else.  It is the header file for ObjMesh.cpp.
 **/

#ifndef MESH_OBJECT
#define MESH_OBJECT

#include "CGCommon.h"

#include <vector>
#include <string>
using namespace std;

//! Object for creating tightly packed arrays to be passed to VBO creation
#pragma pack(push, 1)
class vertex { public: GLfloat X, Y, Z; };
#pragma pack(pop)

//! Helper function for easily filling a vector object.
GLvoid setVertex(vertex *V, GLfloat X, GLfloat Y, GLfloat Z);

class ObjMesh;

//! This object abstracts groups of faces into individual objects storing indices into the raw vectors
struct MeshGroup
{
    MeshGroup(ObjMesh* newParent = NULL);
    MeshGroup(ObjMesh* newParent, vector<string> groupNames);

    ostream& output(ostream &out);

    void addGroupNames(vector<string> groupNames);
    bool isEmpty();

    ObjMesh* parent;
    vector<uint32_t> groups;          //!< Indices into the 'names' array
    vector<uint32_t> triangleV;       //!< Indices into the 'rawVerts' array
    vector<uint32_t> triangleN;       //!< Indices into the 'rawNorms' array
    vector<uint32_t> triangleC;       //!< Indices into the 'rawColors' array
    vector<uint32_t> triangleTex;     //!< Indices into the 'rawTexCords' array
    vector<uint32_t> triangleTan;     //!< Indices into the 'rawTangents' array

    //!< Indicate if optional parameters are present
    GLboolean hasNormals, hasColors, hasTexCoords, hasTangents;
};

//! Functions and variables for loading and manipulating vertex and face triangle/quad meshes
/**
 * <p>This class provides functions and variables to manage a vertex mesh.  The mesh is a collection of vertices, faces
 * that are built from those vertices and normals to describe the direction of the faces.  This class loads meshes
 * from wavefront obj files and stores them in what is called a Vertex Buffer Object (VBO).  The VBO can be pre-loaded
 * to the video card and rendered repeatedly with a minimum of OpenGL API calls streamlining the rendering process.
 * Loading of the mesh file can be done immediately or delayed until the first time the mesh is drawn.</p>
 *
 * <p>At present, only mesh files made entirely of triangular faces are supported.  Mesh files with quad or polygonal
 * faces or a mix of all three are not guaranteed to work properly.  The variables are there to support such files but
 * the functionality may not be complete.  Testing should be done to verify.</p>
 **/
class ObjMesh
{
protected:
    bool mWarned;               //!< remember if we have warned the user about tesselation problems.
    vector<string> names;       //!< Names of all the groups in this mesh
    vector<bool> enableGrp;     //!< Enabling and disabling of each group
    uint32_t counter;           //!< A counter to avoid duplicate group names

    /// \name Bounding box variables
    //@{
    GLfloat minX;			//!< Smallest vertex X dimension
    GLfloat maxX;			//!< Largest vertex X dimension

    GLfloat minY;			//!< Smallest vertex Y dimension
    GLfloat maxY;			//!< Largest vertex Y dimension

    GLfloat minZ;			//!< Smallest vertex Z dimension
    GLfloat maxZ;			//!< LArgest vertex Z dimension

    GLfloat center[3];		//!< Geometric center (computed from bounding box variables)
    GLfloat unitScale;      //!< Scaling value necessary to tightly fit bounding box inside unit cube
    //@}

    /// \name Raw mesh data storage
    //@{
    vector<float> rawVerts;		//!< Array of triangle vertices (always in groups of three)
    vector<float> rawNorms;		//!< Normals for the vertices (always in groups of three)
    vector<float> rawColors;	//!< Colors for the vertices (always in groups of three)
    vector<float> rawTexCords;	//!< Texture Coordinates for the vertices (always in groups of three)
    vector<float> rawTangents;	//!< Tangent vectors for the vertices (always in groups of three)
    //@}

    /// \name Mesh groups and VBOs
    //@{
    vector<MeshGroup*> group;
    //@}

    /// \name Boolean state variables
    //@{
    GLboolean LOADED;	//!< Has the mesh file been read (or has data been generated)?
    //@}

    GLvoid tesselateTriangle(vector<uint32_t> &polyV, vector<uint32_t> &polyN, vector<uint32_t> &polyT);
    GLvoid computeFaceTangents(float v1[9], float n[9], float tex[9], float* tan);
    GLvoid computeTriangleNormal(uint32_t idx1, uint32_t idx2, uint32_t idx3, double N[]);

public:
    /// \name Constructors &amp; Destructors
    //@{
    //! Standard empty constructor, initializes all members with reasonable default values
    /**
     * This is the standard constructor.  It initializes all members with reasonable default values.
     **/
    ObjMesh();

    //! Standard destructor, frees all dynamically allocated memory
    /**
     * This is the standard destructor function.  It frees all dynamically allocated memory for this object that will
     * not be used anywhere else after it is deleted.
     **/
    virtual ~ObjMesh();
    //@}

    // Directly add raw data (vertices and normals only for now)
    GLvoid addGroup(std::string groupName);
    uint32_t addRawVertex(const float X, const float Y, const float Z);
    uint32_t addRawNormal(const float i, const float j, const float k);
    GLvoid addTriangle(const uint32_t vA, const uint32_t vB, const uint32_t vC,
                       const uint32_t nA, const uint32_t nB, const uint32_t nC);

    // Parse the raw data (added with above three functions)
    bool parseRawData(bool pAllowEmpty = false);

    /// \name Object drawing functions for OpenGL and Cg
    //@{
    //! Draw this object using OpenGL commands
    /**
     * This function draws the geometry contained in this object using OpenGL commands.  It first pushes
     * the transformation matrix and multiplies it with this object's matrix.  Then, when finished drawing,
     * it should pop the matrix back to its original state.
     **/
    GLvoid draw();
    GLvoid drawNormals();

    void drawDirect(uint32_t i) const;
    void drawNormalsDirect(uint32_t i) const;

    void drawDirect(const MeshGroup* curGroup) const;
    void drawNormalsDirect(const MeshGroup* curGroup) const;

    /// \name File input/output functions
    //@{
    //! Reads a text based wavefront .obj file
    /**
     * This function will read the raw vertex, normal and face data from a wavefront .obj file.
     * Seperate mesh groups are created for each object/group in the .obj file.  If rescale is
     * true then the entire obj contents are scaled to unit size in each dimension and if
     * recenter is true, the vertices are moved so that their geometric average is at the origin.
     **/
    virtual GLvoid readFile(string filename);

    bool writeOBJFile(string filename);

    //@}

    void computeMeshTangents(bool replaceExisting = false);

    bool weldVertices(double angleThreshold);
    bool computePerFaceNormals(bool overwrite = false);
    GLvoid smoothNormals(double angleThreshold);

    uint32_t getNamedGroupCount();
    string getGroupName(uint32_t idx);

    void enableGroup(uint32_t idx);
    void disableGroup(uint32_t idx);
    void toggleGroup(uint32_t idx);

    bool isGroupEnabled(uint32_t idx);
    bool doesGroupHaveNormals(uint32_t idx);
    bool doesGroupHaveColors(uint32_t idx);
    bool doesGroupHaveTexCoords(uint32_t idx);
    bool doesGroupHaveTangents(uint32_t idx);

    MeshGroup* extractMeshGroup(uint32_t i);
    uint32_t getMeshGroupCount();

    uint32_t getVertexCount();
    uint32_t getFaceCount();

    GLvoid getCenter(double &pX, double &pY, double &pZ) const;
    double getUnitScale() const;

    bool compare(const ObjMesh &other);

    friend struct MeshGroup;
};

#endif
