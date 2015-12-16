using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using System.Collections.Generic;
using UnityEditor;

public class ObjectController : MonoBehaviour {

    public VertexSort vertexSort;
	public Text percentText;
	public GameObject objectOne;
	public GameObject objectTwo;
	public float rotateSpeed = 1;
	public float percentObjectTwo = 0.1f;
	public bool subdivide = true;

	private Mesh meshOne;
	private Mesh meshTwo;
	private Mesh curMesh;
	private Mesh largeMesh;
	private Mesh smallMesh;
    private Matches[] matches;

	void Start()
	{
		//Initialize all variables
		meshOne = objectOne.GetComponent<MeshFilter> ().mesh;
		meshTwo = objectTwo.GetComponent<MeshFilter> ().mesh;
		curMesh = transform.GetComponent<MeshFilter> ().mesh;

		largeMesh = (Mathf.Max (meshOne.vertexCount, meshTwo.vertexCount) == meshOne.vertexCount) ? meshOne : meshTwo;
		smallMesh = (largeMesh == meshOne) ? meshTwo : meshOne;
		percentText.text = "Percent Object Two: " + percentObjectTwo;

		//If subdivision is checked
		if(subdivide)
		{
			SubdivideMesh ();
			largeMesh = (Mathf.Max (meshOne.vertexCount, meshTwo.vertexCount) == meshOne.vertexCount) ? meshOne : meshTwo;
			smallMesh = (largeMesh == meshOne) ? meshTwo : meshOne;
		}

		//Calculate matches using VertexSort
        matches = vertexSort.SortVertices(meshOne.vertices, meshTwo.vertices);
	}
	
	void Update ()
	{
		GetKeyInput ();
	}

	void GetKeyInput()
	{
		//Rotates the warped object
		if(Input.GetKey(KeyCode.LeftArrow))
		{
			transform.Rotate(0, rotateSpeed * Time.deltaTime, 0);
		}
		if(Input.GetKey(KeyCode.RightArrow))
		{
			transform.Rotate(0, -rotateSpeed * Time.deltaTime, 0);
		}
		if(Input.GetKey(KeyCode.UpArrow))
		{
			transform.Rotate(rotateSpeed * Time.deltaTime,0,0);
		}
		if(Input.GetKey(KeyCode.DownArrow))
		{
			transform.Rotate(-rotateSpeed * Time.deltaTime,0,0);
		}

		//Transforms the warmped object
		if(Input.GetKey(KeyCode.Alpha1))
		{
			percentObjectTwo += 0.5f;
			percentText.text = "Percent Object Two: " + percentObjectTwo;
			//RecalculateVertices();
		}
		if(Input.GetKey(KeyCode.Alpha2))
		{
			percentObjectTwo -= 0.5f;
			percentText.text = "Percent Object Two: " + percentObjectTwo;
			//RecalculateVertices();
		}
		if(Input.GetKey(KeyCode.Space))
		{
			RecalculateVertices();
		}
	}

	void SubdivideMesh()
	{
		Mesh newMesh = smallMesh;
		int subdivisions = 0;
		//Subdivide the small mesh untill it's close in value to the larger mesh
		while (smallMesh.vertexCount*3 < largeMesh.vertexCount)
		{
			subdivisions++;
			MeshHelper.Subdivide (newMesh);
		}
		//Set small and large meshes to the the proper meshes
		objectOne.GetComponent<MeshFilter>().mesh = (smallMesh == meshOne) ? newMesh : meshOne;
		objectTwo.GetComponent<MeshFilter>().mesh = (smallMesh == meshTwo) ? newMesh : meshTwo;
		print (subdivisions);
		print ("small: " + smallMesh.vertexCount + " large: " + largeMesh.vertexCount + " new: "+newMesh.vertexCount);
	}

	void RecalculateVertices()
	{
		//Initialize new variables
		Vector3[] vertices = new Vector3[largeMesh.vertexCount];
		List<int> triangles = new List<int>();

		//Interpolate all vertices
		for(int i = 0; i < matches.Length; i++)
		{
            float x_one = matches[i].parentVert.x;
			float y_one = matches[i].parentVert.y;
			float z_one = matches[i].parentVert.z;
            for(int j = 0; j < matches[i].verts.Count; j++)
            {
                float x_two = matches[i].verts[j].x;
                float y_two = matches[i].verts[j].y;
                float z_two = matches[i].verts[j].z;
                vertices[matches[i].arrayLocations[j]] = new Vector3(x_one + ((x_two - x_one) * percentObjectTwo / 100), y_one + ((y_two - y_one) * percentObjectTwo / 100), z_one + ((z_two - z_one) * percentObjectTwo / 100));
            }
		}

		//Construct new mesh based on calculated values
		curMesh.Clear ();
		curMesh.vertices = vertices;
		curMesh.triangles = largeMesh.triangles;
		RecalculateUVs();
        curMesh.RecalculateNormals();
		curMesh.RecalculateBounds();
		curMesh.Optimize();
	}

	void RecalculateUVs()
	{
		//Create new uv mapping
		Vector2[] uvs = new Vector2[curMesh.vertexCount];
		for (int i=0; i < uvs.Length; i++) {
			uvs[i] = new Vector2(curMesh.vertices[i].x, curMesh.vertices[i].z);
		}
		curMesh.uv = uvs;
	}
}
