using UnityEngine;
using UnityEngine.UI;
using System.Collections;

public class ObjectController : MonoBehaviour {

	public Text percentText;
	public GameObject objectOne;
	public GameObject objectTwo;
	public float rotateSpeed = 1;
	public float percentObjectTwo = 0.1f;

	private Mesh meshOne;
	private Mesh meshTwo;
	private Mesh curMesh;

	void Start()
	{
		meshOne = objectOne.GetComponent<MeshFilter> ().mesh;
		meshTwo = objectTwo.GetComponent<MeshFilter> ().mesh;
		curMesh = transform.GetComponent<MeshFilter> ().mesh;
		percentText.text = "Percent Object Two: " + percentObjectTwo;
		RecalculateVertices();
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
			RecalculateVertices();
		}
		if(Input.GetKey(KeyCode.Alpha2))
		{
			percentObjectTwo -= 0.5f;
			percentText.text = "Percent Object Two: " + percentObjectTwo;
			RecalculateVertices();
		}
		if(Input.GetKey(KeyCode.Space))
		{
			RecalculateVertices();
		}
	}

	void RecalculateVertices()
	{
		Vector3[] vertices = new Vector3[curMesh.vertexCount];
		for(int i = 0; i < curMesh.vertexCount; i++)
		{
			float x_one = meshOne.vertices[i].x;
			float y_one = meshOne.vertices[i].y;
			float z_one = meshOne.vertices[i].z;

			float x_two = meshTwo.vertices[i].x;
			float y_two = meshTwo.vertices[i].y;
			float z_two = meshTwo.vertices[i].z;

			vertices[i] = new Vector3(x_one + ((x_two - x_one) * percentObjectTwo/100), y_one + ((y_two - y_one) * percentObjectTwo/100), z_one + ((z_two - z_one) * percentObjectTwo/100));
		}
		curMesh.Clear ();
		curMesh.vertices = vertices;
		curMesh.normals = meshOne.normals;
		curMesh.uv = meshOne.uv;
		curMesh.triangles = meshOne.triangles;
		curMesh.RecalculateBounds();
		curMesh.Optimize();
	}
}
