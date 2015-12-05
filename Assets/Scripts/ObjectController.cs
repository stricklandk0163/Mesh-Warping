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

	private Mesh meshOne;
	private Mesh meshTwo;
	private Mesh curMesh;
	private Mesh largeMesh;
	private Mesh smallMesh;
    private Matches[] matches;

	void Start()
	{
		meshOne = objectOne.GetComponent<MeshFilter> ().mesh;
		meshTwo = objectTwo.GetComponent<MeshFilter> ().mesh;
		curMesh = transform.GetComponent<MeshFilter> ().mesh;

		largeMesh = (Mathf.Max (meshOne.vertexCount, meshTwo.vertexCount) == meshOne.vertexCount) ? meshOne : meshTwo;
		smallMesh = (largeMesh == meshOne) ? meshTwo : meshOne;
		
		percentText.text = "Percent Object Two: " + percentObjectTwo;
		SubdivideMesh ();

		largeMesh = (Mathf.Max (meshOne.vertexCount, meshTwo.vertexCount) == meshOne.vertexCount) ? meshOne : meshTwo;
		smallMesh = (largeMesh == meshOne) ? meshTwo : meshOne;

        //matches = vertexSort.SortVertices(meshOne.vertices, meshTwo.vertices);
	}

	// Update is called once per frame
	void Update ()
	{
		GetKeyInput ();
	}

	void GetKeyInput()
	{
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
		if(Input.GetKey(KeyCode.Alpha1))
		{
			percentObjectTwo += 0.5f;
			percentText.text = "Percent Object Two: " + percentObjectTwo;
		}
		if(Input.GetKey(KeyCode.Alpha2))
		{
			percentObjectTwo -= 0.5f;
			percentText.text = "Percent Object Two: " + percentObjectTwo;
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
		while (smallMesh.vertexCount*3 < largeMesh.vertexCount)
		{
			subdivisions++;
			MeshHelper.Subdivide (newMesh);
		}
		objectOne.GetComponent<MeshFilter>().mesh = (smallMesh == meshOne) ? newMesh : meshOne;
		objectTwo.GetComponent<MeshFilter>().mesh = (smallMesh == meshTwo) ? newMesh : meshTwo;
		print (subdivisions);
		print ("small: " + smallMesh.vertexCount + " large: " + largeMesh.vertexCount + " new: "+newMesh.vertexCount);
	}

	void RecalculateVertices()
	{
		Vector3[] vertices = new Vector3[largeMesh.vertexCount];
		List<int> triangles = new List<int>();

        int vertNum = 0;
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
				vertNum++;
            }
		}
		curMesh.Clear ();
		curMesh.vertices = vertices;
		curMesh.triangles = largeMesh.triangles;
		RecalculateUVs();
        curMesh.RecalculateNormals();
		curMesh.RecalculateBounds();
		curMesh.Optimize();
		print (largeMesh.vertexCount);
		print (curMesh.vertexCount);
		print (largeMesh.triangles.Length);
		print (curMesh.triangles.Length);
	}

	void RecalculateUVs()
	{
		Vector2[] uvs = new Vector2[curMesh.vertexCount];
		for (int i=0; i < uvs.Length; i++) {
			uvs[i] = new Vector2(curMesh.vertices[i].x, curMesh.vertices[i].z);
		}
		curMesh.uv = uvs;
	}
}
