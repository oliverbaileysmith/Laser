struct Ray
{
	float3 m_Origin;
	float3 m_Direction;
};

__kernel void Laser(__global float3* output, int imageWidth, int imageHeight, float aspectRatio, float viewportWidth, float viewportHeight, float focalLength, float3 cameraOrigin, float3 lowerLeftCorner)
{
	const int workItemID = get_global_id(0);
	int x = workItemID % imageWidth;
	int y = workItemID / imageWidth;
	float fx = (float)x / (float)(imageWidth - 1);
	float fy = (float)y / (float)(imageHeight - 1);
	struct Ray ray;
	ray.m_Origin = cameraOrigin;
	ray.m_Direction = lowerLeftCorner;
	ray.m_Direction.x += fx * viewportWidth;
	ray.m_Direction.y += fy * viewportHeight;
	ray.m_Direction -= cameraOrigin;
	ray.m_Direction = normalize(ray.m_Direction);
	output[workItemID] = fabs(ray.m_Direction);
}