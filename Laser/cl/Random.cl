#ifndef RANDOM_CL
#define RANDOM_CL

uint PCGHash(uint seed)
{
	uint state = seed * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
	return (word >> 22u) ^ word;
}

float randomFloat(uint *seed)
{
	*seed = PCGHash(*seed);
	return (float)*seed / (float)0xFFFFFFFFu;
}

float3 randomFloat3InUnitDisk(uint *seed)
{
	float3 point;
	while (true)
	{
		point = (float3)(randomFloat(seed) * 2.0f - 1.0f,
			randomFloat(seed) * 2.0f - 1.0f, 0.0f);
		if (length(point) < 1.0f)
			break;
	}
	return point;
}

#endif // RANDOM_CL
