using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public struct Matches
{
    public Vector3 parentVert;
    public List<Vector3> verts = new List<Vector3>();
    //Vertex One = {1,2,1};  Vertex Two = {1,3,1};
}

public class VertexSort : MonoBehaviour {
	public Matches[] SortVertices(Vector3[] object1, Vector3[] object2)
	{
        //High vert object will contain the object with a greater number of vertices
        Vector3[] highVertObject = null;

        //Low vert object will contain the object with a lower number of vertices
        Vector3[] lowVertObject = null;

        //First we find which object has more vertices (We will want to map from high vertice object to low vertice obect)
        if(object1.Length >= object2.Length)
        {
            highVertObject = object1;
            lowVertObject = object2;
        }
        else
        {
            highVertObject = object2;
            lowVertObject = object1;
        }

        //Now we will want a way to compute distances for points in each array this will be accomplished with a 2D array
        //The rows of this array will be made up of the vertices of the vertices highVertObject while the columns are made up of the vertices of the lowVertObject
		float[,] verticeDistances = new float[highVertObject.Length, lowVertObject.Length];
        for(int i = 0; i < highVertObject.Length; i++)
        {
            for(int j = 0; j < lowVertObject.Length; j++)
            {
                verticeDistances[i, j] = Vector3.Distance(highVertObject[i], lowVertObject[j]);
            }
        }
        
        

        //We want to insure that every vertice in the lowVert object is mapped to, so we will start out by mapping each lowVert vertice to a single highVert vertice

        //Take in two arrays and find pairs
		//Return a 2D array that describes which pairs are mathed together
		return new Matches[1];
	}
}
