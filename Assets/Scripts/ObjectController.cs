using UnityEngine;
using UnityEngine.UI;
using System.Collections;

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
    private Matches[] matches;

	void Start()
	{
		meshOne = objectOne.GetComponent<MeshFilter> ().mesh;
		meshTwo = objectTwo.GetComponent<MeshFilter> ().mesh;
		curMesh = transform.GetComponent<MeshFilter> ().mesh;
		percentText.text = "Percent Object Two: " + percentObjectTwo;
        matches = vertexSort.SortVertices(meshOne.vertices, meshTwo.vertices);
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
                vertices[vertNum] = new Vector3(x_one + ((x_two - x_one) * percentObjectTwo / 100), y_one + ((y_two - y_one) * percentObjectTwo / 100), z_one + ((z_two - z_one) * percentObjectTwo / 100));
                vertNum++;
            }
		}
		curMesh.Clear ();
		curMesh.vertices = vertices;
		curMesh.uv = meshTwo.uv; //TODO: change meshOne.uv to a new variable, just like vertices is done
		curMesh.triangles = meshTwo.triangles;
        curMesh.RecalculateNormals();
		curMesh.RecalculateBounds();
		curMesh.Optimize();
	}
}
