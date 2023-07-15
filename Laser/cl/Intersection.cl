typedef struct Intersection
{
	float3 P; 
	float3 N;
	float u, v; // Barycentric coordinates
	uint TriangleIndex;
} Intersection;