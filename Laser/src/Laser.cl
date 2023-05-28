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
	*t = - (dot(*norm, ray->m_Origin) + D) / dot(*norm, ray->m_Direction);
	float3 pHit = ray->m_Origin + *t * ray->m_Direction;

	return false;
}

__kernel void Laser(__global float3* output, int imageWidth, int imageHeight, float aspectRatio, float viewportWidth, float viewportHeight, float focalLength, float3 cameraOrigin, float3 lowerLeftCorner, __global float3* triangle)
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
	ray.m_Direction = lowerLeftCorner;
	ray.m_Direction.x += fx * viewportWidth;
	ray.m_Direction.y += fy * viewportHeight;
	ray.m_Direction -= cameraOrigin;
	ray.m_Direction = normalize(ray.m_Direction);

	float t = 0.0f;
	float3 norm = (float3)(0.0f,0.0f,0.0f);
	float3 v0 = triangle[0];
	float3 v1 = triangle[1];
	float3 v2 = triangle[2];

	intersectTriangle(&ray, &v0, &v1, &v2, &t, &norm);

	//if (ray.m_Direction.x > 0.5)
	//	output[workItemID] = (float3)(1.0f,0.0f,0.0f);
	//else
	//	output[workItemID] = (float3)(0.0f,0.0f,0.0f);
	//output[workItemID] = fabs(ray.m_Direction);
	output[workItemID] = norm;
}