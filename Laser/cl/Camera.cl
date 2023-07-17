typedef struct CameraProps
{
	float3 Position;
	float3 Target;
	float3 UpperLeftCorner;
	float3 ViewportHorizontal;
	float3 ViewportVertical;
	float VerticalFOV;
	float AspectRatio;
	float FocalLength;
} CameraProps;