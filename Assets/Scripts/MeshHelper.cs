using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class MeshHelper
{
	static List<Vector3> vertices;
	static List<Vector3> normals;
	static List<int> indices;
	static Dictionary<uint,int> newVertices;
	
	static int GetNewVertex(int i1, int i2)
	{
		// We have to test both directions since the edge; we want to watch for reversed directions
		uint t1 = ((uint)i1 << 16) | (uint)i2;
		uint t2 = ((uint)i2 << 16) | (uint)i1;
		if (newVertices.ContainsKey(t2))
			return newVertices[t2];

		if (newVertices.ContainsKey(t1))
			return newVertices[t1];

		// Generate vertex:
		int newIndex = vertices.Count;
		newVertices.Add(t1,newIndex);
		
		// Calculate new vertex
		vertices.Add((vertices[i1] + vertices[i2]) * 0.5f);
		normals.Add((normals[i1] + normals[i2]).normalized);
		
		return newIndex;
	}	

	// Pass mesh into function, then manipulate mesh's triangles and vertices
	public static void Subdivide(Mesh mesh)
	{
		newVertices = new Dictionary<uint,int>();
		vertices = new List<Vector3>(mesh.vertices);
		normals = new List<Vector3>(mesh.normals);
		indices = new List<int>();

		int[] triangles = mesh.triangles;

		// Create points to seperate triangle into multiple triangles
		for (int i = 0; i < triangles.Length; i += 3)
		{
			int i1 = triangles[i + 0];
			int i2 = triangles[i + 1];
			int i3 = triangles[i + 2];
			
			int a = GetNewVertex(i1, i2);
			int b = GetNewVertex(i2, i3);
			int c = GetNewVertex(i3, i1);
			indices.Add(i1);   indices.Add(a);   indices.Add(c);
			indices.Add(i2);   indices.Add(b);   indices.Add(a);
			indices.Add(i3);   indices.Add(c);   indices.Add(b);
			indices.Add(a );   indices.Add(b);   indices.Add(c); // The center triangle
		}

		mesh.vertices = vertices.ToArray();
		mesh.normals = normals.ToArray();
		mesh.triangles = indices.ToArray();
		
		// Erase arrays to free them up
		newVertices = null;
		vertices = null;
		normals = null;
		indices = null;
	}
}