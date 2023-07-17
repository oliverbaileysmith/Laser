typedef struct ImageProps
{
	uint Width;
	uint Height;
	float AspectRatio;
	uint TileWidth;
	uint TileHeight;
	uint nRows;
	uint nColumns;
	int Dummy; // "Format" in host program
} ImageProps;