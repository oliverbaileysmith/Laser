typedef float4 mat4[4];

// Transform a point (uses homogenous weight w = 1)
float3 multMat4Point(mat4* m, float3* point)
{
	float x = (*m)[0][0]*point->x + (*m)[1][0]*point->y + (*m)[2][0]*point->z + (*m)[3][0];
	float y = (*m)[0][1]*point->x + (*m)[1][1]*point->y + (*m)[2][1]*point->z + (*m)[3][1];
	float z = (*m)[0][2]*point->x + (*m)[1][2]*point->y + (*m)[2][2]*point->z + (*m)[3][2];
	float w = (*m)[0][3]*point->x + (*m)[1][3]*point->y + (*m)[2][3]*point->z + (*m)[3][3];

	// Divide by w (homogenous weight)
	if (w == 1)
		return (float3)(x, y, z);
    else
		return (float3)(x, y, z) / w;
}