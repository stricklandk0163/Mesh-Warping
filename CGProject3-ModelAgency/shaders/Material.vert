// Information about the geometry at the surface point (in global coordinates)
varying vec3 VGlobal;		// View vector in global coordinates
varying vec3 LGlobal;		// Light vector in global coordinates
varying vec3 NGlobal;		// Normal vector in global coordinates

varying vec3 TGlobal;           // Surface tangent vector in global coordinates
varying vec3 BGlobal;           // Surface bi-tangent vector in global coordinates

// Important information about the lighting and viewing situation
uniform vec3 lightPos;		// Position of the point light source (in global coordinates)
uniform vec3 cameraPos;		// Position of the camera (in global coordinates)

// Manual transformation matrices (for OpenGL 3.1+)
uniform mat4 modelMatrix;       // Transformation from local to global coordinates (for vertices)
uniform mat3 normalMatrix;      // Transformation from local to global coordinates (for normal vector)
uniform mat4 viewMatrix;        // Transformation from global coordinates to camera coordinates
uniform mat4 projMatrix;        // Matrix to achieve projection from 3D to 2D

// Surface tangent vector (in model space coordinates)
attribute vec3 tangent;

// Render modes
#define MODE_SOLID_COLOR    0
#define MODE_NORMALS	    1
#define MODE_TEX_CORDS	    2
#define MODE_PHONG_SHADED   3
uniform int curRenderMode;

// Transform the geometry and compute some vectors for use in the fragment shader
void main(void)
{
    // Global vertex position
    vec4 POSGlobal = (modelMatrix*gl_Vertex);

    if(curRenderMode != MODE_PHONG_SHADED)
    {
        // Just pass through color, texture and un-transformed Normal
        NGlobal = gl_Normal;
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_FrontColor = gl_BackColor = gl_Color;
    }
    else
    {
        // Compute view, light and normal vectors in eye space (for use in fragment shader)
        VGlobal = cameraPos - POSGlobal.xyz;
        LGlobal = lightPos - POSGlobal.xyz;

        NGlobal = (normalMatrix*gl_Normal);
        TGlobal = (normalMatrix*tangent);
        BGlobal = cross(NGlobal, TGlobal);

        // Pass-through texture coordinates un-transformed as well as front and back colors
        gl_TexCoord[0] = gl_MultiTexCoord0;
        gl_FrontColor = gl_BackColor = gl_Color;
    }

    // Transform the vertex to screen coordinates
    gl_Position = projMatrix * viewMatrix * POSGlobal;
}
