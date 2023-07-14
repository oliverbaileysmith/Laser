struct Intersection
{
	float3 P;
	float3 N;
	uint TriangleIndex;
	float u, v; // Barycentric coordinates
};