#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Tesselator.h"

GLenum Tesselator::triangleType = GL_TRIANGLES;
GLUtesselator* Tesselator::tobj = NULL;
vector<void*> Tesselator::result;
vector<newVertexData*> Tesselator::newVertices;

Tesselator::Tesselator() { if(tobj == NULL) tobj = gluNewTess(); }
Tesselator::~Tesselator()
{
    for(uint32_t i=0; i<newVertices.size(); i++)
	{
        delete (uint32_t*)newVertices[i]->extraOut;
		delete newVertices[i];
	}
	newVertices.clear();
}

uint32_t Tesselator::getResultCount() { return (uint32_t)result.size(); }

void* Tesselator::getResultVertex(uint32_t idx)
{
	if(idx < result.size()) return result[idx];
	else return NULL;
}

newVertexData* Tesselator::getNewData(uint32_t idx)
{
	if(idx < newVertices.size()) return newVertices[idx];
	else return NULL;
}

void Tesselator::linearizeResults()
{
	// Process TRIANGLE_FAN and TRIANGLE_STRIP results into TRIANGLES only
	vector<void*> linearResults;
	switch(triangleType)
	{
		case GL_TRIANGLE_FAN:
		{
			// Defines n-2 triangles (n is number of vertices)
			// Triangle i is defined by index 0, i+1, i+2
            for(uint32_t i=0; i<result.size()-2; i++)
			{
				linearResults.push_back(result[0]);
				linearResults.push_back(result[i+1]);
				linearResults.push_back(result[i+2]);
			}
		}
		break;

		case GL_TRIANGLE_STRIP:
		{
			// Defines n-2 triangles (n is number of vertices)
			// For odd n: Triangle i is defined by index i, i+1, i+2
			if(result.size()%2 == 1)
			{
                for(uint32_t i=0; i<result.size()-2; i++)
				{
					linearResults.push_back(result[i]);
					linearResults.push_back(result[i+1]);
					linearResults.push_back(result[i+2]);
				}
			}

			// For even n: Triangle i is defined by index i+1, i, i+2
			else
			{
                for(uint32_t i=0; i<result.size()-2; i++)
				{
					linearResults.push_back(result[i+1]);
					linearResults.push_back(result[i]);
					linearResults.push_back(result[i+2]);
				}
			}
		}
		break;

		// Any other type, do nothing
		default: return;
	}

	// Replace original results with linearResults
	result.clear();
    for(uint32_t i=0; i<linearResults.size(); i++)
		result.push_back(linearResults[i]);
}

bool Tesselator::process(vector<double*> &polyVerts)
{
	// Assign the GLU tesselation callback functions
#ifdef _WIN32
    gluTessCallback(tobj, GLU_TESS_VERTEX, (void (__stdcall*)())vertexCallback);
    gluTessCallback(tobj, GLU_TESS_BEGIN, (void (__stdcall*)())beginCallback);
    gluTessCallback(tobj, GLU_TESS_END, (void (__stdcall*)())endCallback);
    gluTessCallback(tobj, GLU_TESS_ERROR, (void (__stdcall*)())errorCallback);
    gluTessCallback(tobj, GLU_TESS_COMBINE, (void (__stdcall*)())combineCallback);
#else
	gluTessCallback(tobj, GLU_TESS_VERTEX, (GLvoid (*)())vertexCallback);
	gluTessCallback(tobj, GLU_TESS_BEGIN, (GLvoid (*)())beginCallback);
	gluTessCallback(tobj, GLU_TESS_END, (GLvoid (*)())endCallback);
	gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid (*)())errorCallback);
	gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid (*)())combineCallback);
#endif

	// Clear the results
	result.clear();
	triangleType = GL_TRIANGLES;

	// Set any tesselation properties
	gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);

	// Describe the polygon vertices
	gluTessBeginPolygon(tobj, NULL);
		gluTessBeginContour(tobj);

            for(uint32_t i = 0; i < polyVerts.size(); i++)
				gluTessVertex(tobj, polyVerts[i], (polyVerts[i] + 4));

		gluTessEndContour(tobj);
	gluTessEndPolygon(tobj);

	// Turn into pure triangle list
	linearizeResults();

	// All done
	if(newVertices.size() > 0) return false;
	return true;
}

void Tesselator::errorCallback(GLenum errorCode)
{
   const GLubyte *estring;
   estring = gluErrorString(errorCode);
   printf("Tessellation Error: %s\n", estring);
}

void Tesselator::beginCallback(GLenum which)
{
	triangleType = which;
}

void Tesselator::endCallback() { } //{ glEnd(); }

void Tesselator::vertexCallback(GLvoid *vertex)
{
	if(vertex == NULL) { printf("Error: null vertex data.\n"); return; }

	// Store vertex
	result.push_back(vertex);
}

void Tesselator::edgeFlagCallback(GLboolean flag) { (void)flag; }

void Tesselator::combineCallback(GLdouble coords[3], void *extraIn[4],
                                 GLfloat weight[4], void **dataOut)
{
	// Replace the V index with 0 and the N index with an index into the newVertices vector
    uint32_t* extraData = (uint32_t*)malloc(3*sizeof(uint32_t));
    extraData[0] = 0; extraData[1] = (uint32_t)newVertices.size();
	extraData[2] = 0;

	// Copy the data for external creation of the new vertex
	newVertexData* curData = new newVertexData();
	curData->coords[0] = coords[0];
	curData->coords[1] = coords[1];
	curData->coords[2] = coords[2];

	curData->extrasIn[0] = extraIn[0];
	curData->extrasIn[1] = extraIn[1];
	curData->extrasIn[2] = extraIn[2];
	curData->extrasIn[3] = extraIn[3];

	curData->weights[0] = weight[0];
	curData->weights[1] = weight[1];
	curData->weights[2] = weight[2];
	curData->weights[3] = weight[3];

	curData->extraOut = extraData;

	newVertices.push_back(curData);

	// Set the special extra data as output pointer
	*dataOut = extraData;
}
