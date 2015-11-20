#ifndef TESSELATOR_H_
#define TESSELATOR_H_

#include "CGCommon.h"

#include <vector>
using namespace std;

struct newVertexData
{
	GLdouble coords[3];
	void* extrasIn[4];
	GLfloat weights[4];
	void* extraOut;
};

class Tesselator
{
private:
	static void beginCallback(GLenum which);
	static void endCallback();
	static void errorCallback(GLenum errorCode);
	static void vertexCallback(GLvoid *vertex);
	static void edgeFlagCallback(GLboolean flag);
	static void combineCallback(GLdouble coords[3], void *vertex_data[4],
                                    GLfloat weight[4], void **dataOut);

	static void linearizeResults();
	static GLenum triangleType;
	static GLUtesselator *tobj;

	static vector<void*> result;
	static vector<newVertexData*> newVertices;

public:
	Tesselator();
	virtual ~Tesselator();

	bool process(vector<double*> &polyVerts);

    static uint32_t getResultCount();
    static void* getResultVertex(uint32_t idx);
    static newVertexData* getNewData(uint32_t idx);
};

#endif /* TESSELATOR_H_ */
