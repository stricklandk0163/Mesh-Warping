using UnityEngine;
using System.Collections;
using System.Collections.Generic;
using System;

public struct Matches
{
    public Vector3 parentVert;
    public List<Vector3> verts;
}

public class VertexSort : MonoBehaviour {
	public Matches[] SortVertices(Vector3[] object1, Vector3[] object2)
	{
        //High vert object will contain the object with a greater number of vertices
        Vector3[] highVertObject = null;

        //Low vert object will contain the object with a lower number of vertices
        Vector3[] lowVertObject = null;

        List<int> highVertMappedIndices = new List<int>();
        //First we find which object has more vertices (We will want to map from high vertice object to low vertice obect)
        if (object1.Length >= object2.Length)
        {
            highVertObject = object1;
            lowVertObject = object2;
        }
        else
        {
            highVertObject = object2;
            lowVertObject = object1;
        }

        //The matches struct will match each vertice on the small vert object to 1+ vertices on the high vert object
        Matches[] matches = new Matches[lowVertObject.Length];
        for (int i = 0; i < matches.Length; i++)
        {
            matches[i].parentVert = lowVertObject[i];
            matches[i].verts = new List<Vector3>();
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
        
        //Find which vertice on the high vert object each vertice on the low vert object is mapped closest to.  
        //We do not want a single vertice on low vert object to be mapped to multiple vertices on the high vert object
        for (int i = 0; i < matches.Length; i++ )
        {
            int minHighVertDistIndex = Int32.MaxValue;
            int minLowVertDistIndex = Int32.MaxValue;
            float minVertDistValue = float.MaxValue;
            //Find the minumum distance in the 2d array so we can map the two points together
            for(int j = 0; j < highVertObject.Length; j++)
            {
                for(int k = 0; k < lowVertObject.Length; k++)
                {
                    if(verticeDistances[j,k] < minVertDistValue)
                    {
                        minHighVertDistIndex = j;
                        minLowVertDistIndex = k;
                        minVertDistValue = verticeDistances[j, k];
                    }
                }
            }
            //Store the min value into the matches array
            matches[minLowVertDistIndex].verts.Add(highVertObject[minHighVertDistIndex]);
            //Add which vertice got mapped to the 
            highVertMappedIndices.Add(minHighVertDistIndex);
            //Maximize all values in the current lowvertdistindex column and minHighVertDistIndex row so that we don't end up mapping multiple values from the low vert shape into the high vert shape
            for(int j = 0; j < lowVertObject.Length; j++)
            {
                verticeDistances[minHighVertDistIndex, j] = float.MaxValue;
            }
            for(int j = 0; j < highVertObject.Length; j++)
            {
                verticeDistances[j, minLowVertDistIndex] = float.MaxValue;
            }
        }

        //Now we will need to recreate the 2d distance array
        for (int i = 0; i < highVertObject.Length; i++)
        {
            for (int j = 0; j < lowVertObject.Length; j++)
            {
                verticeDistances[i, j] = Vector3.Distance(highVertObject[i], lowVertObject[j]);
            }
        }

        //Now we need to map the remaining vertices on the highVertObject to a vertice on the low vert object
        //To do this we will find the closest vertice in the lowVertObject to each vertice in the highVert object and map them together
        for(int i = 0; i<highVertObject.Length; i++)
        {
            //We dont want to remap any vertices so if we find one that's already been mapped we will skip it
            if(!highVertMappedIndices.Contains(i))
            {
                int minLowVertIndex = 0;
                float minLowVertDist = float.MaxValue;
                for (int j = 0; j < lowVertObject.Length; j++ )
                {
                    if(verticeDistances[i,j]<minLowVertDist)
                    {
                        minLowVertIndex = j;
                        minLowVertDist = verticeDistances[i, j];
                    }
                }
                matches[minLowVertIndex].verts.Add(highVertObject[i]);
            }
             
        }

        //We want to insure that every vertice in the lowVert object is mapped to, so we will start out by mapping each lowVert vertice to a single highVert vertice

        //Take in two arrays and find pairs
        //Return a 2D array that describes which pairs are mathed together
        return matches;
	}

}
