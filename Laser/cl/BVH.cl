#include "Bounds.cl"

typedef struct BVHLinearNode
{
	Bounds Bounds;
	uint SecondChildOffset; // First child is next node in array
	uint FirstTriangle;
	uint nTriangles;
	uint SplitAxis;
} BVHLinearNode;

bool intersectBVH(Ray* ray, __global Vertex* vertices,
	__global Triangle* triangles, __global Material* materials,
	__global mat4* transforms, __global BVHLinearNode* bvh, float* t,
	float3* n, Intersection* isect,
	__global RenderStats* renderStats)
{
	bool hit = false;

	float3 invDir = (float3)(1.0f / ray->dir.x, 1.0f / ray->dir.y, 1.0f / ray->dir.z);
    int dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };

	float currentT = INFINITY;

	uint current = 0;
	uint toVisitOffset = 0;
	uint nodesToVisit[64];
	
	while (true)
	{
		BVHLinearNode node = bvh[current];

		// If ray hits current node
		if (intersectBounds(ray, &node.Bounds))
		{
			// If node is leaf
			if (node.nTriangles > 0)
			{
				// For each triangle in leaf node
				for (int i = 0; i < node.nTriangles; i++)
				{	
					uint triIndex = node.FirstTriangle + i;

					// Local copy of transformation matrix
					mat4 transform;
					transform[0] = transforms[triangles[triIndex].Transform][0];
					transform[1] = transforms[triangles[triIndex].Transform][1];
					transform[2] = transforms[triangles[triIndex].Transform][2];
					transform[3] = transforms[triangles[triIndex].Transform][3];

					// Triangle vertices
					float3 v0 = vertices[triangles[triIndex].v0].Position;
					float3 v1 = vertices[triangles[triIndex].v1].Position;
					float3 v2 = vertices[triangles[triIndex].v2].Position;
		
					// Transformed vertices
					v0 = multMat4Point(&transform, &v0);
					v1 = multMat4Point(&transform, &v1);
					v2 = multMat4Point(&transform, &v2);

					float u, v;
					
					// If ray intersects triangle
					if (intersectTriangle(ray, v0, v1, v2, &currentT, n, &u, &v, renderStats))
					{
						hit = true;
						
						// Update intersection if closer hit
						if (currentT != 0.0f && currentT < *t)
						{
							*t = currentT;
							isect->P = ray->orig + *t * ray->dir;
							isect->N = *n;
							isect->TriangleIndex = triIndex;
							isect->u = u;
							isect->v = v;
						}
					}
				}
				// Break if done, otherwise update toVisitOffset
				if (toVisitOffset == 0)
					break;
				current = nodesToVisit[--toVisitOffset];
			}

			// If node is interior
			else
			{
				// Determine which child to visit first based on ray direction in split axis
				if (dirIsNeg[node.SplitAxis])
				{
					nodesToVisit[toVisitOffset++] = current + 1;
					current = node.SecondChildOffset;
				}
				else
				{
					nodesToVisit[toVisitOffset++] = node.SecondChildOffset;
					current++;
				}
			}
		}

		// If ray misses current node
		else
		{
			// Break if done, otherwise update toVisitOffset
			if (toVisitOffset == 0)
				break;
			current = nodesToVisit[--toVisitOffset];
		}
	}
	return hit;
}