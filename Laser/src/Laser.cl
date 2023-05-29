struct Ray
{
	float3 m_Origin;
	float3 m_Direction;
};

bool intersectTriangle(struct Ray* ray, float3* v0, float3* v1, float3* v2, float* t, float3* norm)
{
	// calculate triangle/plane normal (this can be precalculated on CPU in future)
	float3 v0v1 = *v1 - *v0;
	float3 v0v2 = *v2 - *v0;
	*norm = cross(v0v1, v0v2);

	// calculate intersection point of ray and the plane in which the triangle lies
	float D = - dot(*norm, *v0); // D is the distance from the origin to the plane, parallel to the plane's normal
	float normDotRayDir = dot(*norm, ray->m_Direction);
    if (fabs(normDotRayDir) < 0.00001f) // test for ray parallel to triangle before continuing, preventing division by 0 when calculating t
        return false;

	*t = - (dot(*norm, ray->m_Origin) + D) / normDotRayDir;
	if (*t < 0.00001f) // test if hit point is behind or "inside" camera
		return false;

	float3 p = ray->m_Origin + *t * ray->m_Direction;

	// check if intersection point is inside triangle
	float3 C;
	
	float3 edge0 = *v1 - *v0; // first edge
	float3 v0p = p - *v0;
	C = cross(edge0, v0p);
	if (dot(*norm, C) < 0.0f)
		return false;

	float3 edge1 = *v2 - *v1; // second edge
	float3 v1p = p - *v1;
	C = cross(edge1, v1p);
	if (dot(*norm, C) < 0.0f)
		return false;

	float3 edge2 = *v0 - *v2; // third edge
	float3 v2p = p - *v2;
	C = cross(edge2, v2p);
	if (dot(*norm, C) < 0.0f)
		return false;

	return true;
}

__kernel void Laser(__global float3* output, int imageWidth, int imageHeight, float aspectRatio, float viewportWidth, float viewportHeight, float focalLength, float3 cameraOrigin, float3 upperLeftCorner, __global float3* triangle)
{
	// calculate pixel coordinates
	const int workItemID = get_global_id(0);
	int x = workItemID % imageWidth;
	int y = workItemID / imageWidth;
	float fx = (float)x / (float)(imageWidth - 1);
	float fy = (float)y / (float)(imageHeight - 1);

	// generate ray
	struct Ray ray;
	ray.m_Origin = cameraOrigin;
	ray.m_Direction = upperLeftCorner;
	ray.m_Direction.x += fx * viewportWidth;
	ray.m_Direction.y -= fy * viewportHeight;
	ray.m_Direction -= cameraOrigin;
	ray.m_Direction = normalize(ray.m_Direction);

	float t = 0.0f;
	float3 norm = (float3)(0.0f,0.0f,0.0f);
	float3 v0 = triangle[0];
	float3 v1 = triangle[1];
	float3 v2 = triangle[2];

	if (intersectTriangle(&ray, &v0, &v1, &v2, &t, &norm))
		output[workItemID] = (float3)(1.0f,0.0f,0.0f);
	else
		output[workItemID] = (float3)(0.0f,0.0f,0.0f);
}