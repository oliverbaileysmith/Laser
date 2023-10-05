#ifndef IMAGE_CL
#define IMAGE_CL

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

#endif // IMAGE_CL
