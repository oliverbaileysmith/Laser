uint PCGHash(uint seed)
{
	uint state = seed * 747796405u + 2891336453u;
	uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float randomFloat(uint* seed)
{
	*seed = PCGHash(*seed);
	return (float)*seed / (float)0xFFFFFFFFu;
}