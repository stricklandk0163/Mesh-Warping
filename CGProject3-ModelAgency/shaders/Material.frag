// Information about the geometry at the surface point (in global coordinates)
varying vec3 VGlobal;		// View vector in global coordinates
varying vec3 LGlobal;		// Light vector in global coordinates
varying vec3 NGlobal;		// Normal vector in global coordinates

varying vec3 TGlobal;           // Surface tangent vector in global coordinates
varying vec3 BGlobal;           // Surface bi-tangent vector in global coordinates

// Render modes
#define MODE_SOLID_COLOR    0
#define MODE_NORMALS	    1
#define MODE_TEX_CORDS	    2
#define MODE_PHONG_SHADED   3
uniform int curRenderMode;

// Retrieve the normal from the map (in global coordinates)
vec3 NormalMapLookup();

// Compute Phong reflection model (using halfway vector a la Blinn)
void main(void)
{
    // Normalize the interpolated global vectors
    vec3 V = normalize(VGlobal);
    vec3 L = normalize(LGlobal);
    vec3 N = normalize(NGlobal);

    // Switch on the current render mode
    if(curRenderMode == MODE_SOLID_COLOR)
    {
        // Show a solid color without any shading
        gl_FragColor = gl_Color;
    }
    else if(curRenderMode == MODE_NORMALS)
    {
        // Assign the normal vector to the surface color
        // This allows you to visualize the normals and check that they are correct.
	gl_FragColor = vec4(N/2.0 + 0.5, 1.0);
    }
    else if(curRenderMode == MODE_TEX_CORDS)
    {
        // Assign the texture coordinates to the surface color
        // This allows you to visualize the normals and check that they are correct.
        vec3 T = normalize(gl_TexCoord[0].xyz);
	gl_FragColor = vec4(T/2.0 + 0.5, 1.0);
    }
    else // MODE_PHONG_SHADED
    {
        // Make sure the normal points outward (handy for backwards polygon winding)
        N = faceforward(N, -V, N);

        // Compute diffuse weight (Lambert's Cosine Law)
        float diffuseWeight = clamp(dot(L, N), 0.0, 1.0);

        // Compute specular weight (Phong Lobe)
        vec3 H = normalize(L + V);
        float NdotH = max(0.0, dot(N, H));
        float specularWeight = clamp(pow(NdotH, gl_FrontMaterial.shininess), 0.0, 1.0);

        // Combine Color Components
        vec4 shadedColor = gl_FrontMaterial.emission;                           // Emissive component
        shadedColor = shadedColor + gl_FrontMaterial.ambient;                   // Ambient component
        shadedColor = shadedColor + (diffuseWeight*gl_FrontMaterial.diffuse);   // Diffuse Component
        shadedColor = shadedColor + (specularWeight*gl_FrontMaterial.specular); // Specular Component

	gl_FragColor = shadedColor;
    }
}
