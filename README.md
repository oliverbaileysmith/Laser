# Laser
A GPU path tracer in C++/OpenCL.
<figure>
    <img src="https://github.com/oliverbaileysmith/Laser/assets/64656218/aa96ca77-7c78-4abc-8794-6c88170b0795"
         alt="Utah Teapot rendered with Laser">
	<br>
    <figcaption>
		Sample output<br>
		Device: AMD Radeon RX 570<br>
		Triangles: 1,006<br>
		Render time: 4 minutes 49 seconds<br><br>
		Image size: 600x600<br>
		Samples per pixel: 4096<br>
		Max ray depth: 8 bounces<br>
		*Note: output was in .ppm, converted to .jpeg to display here
	</figcaption>
</figure>

## Features
- High levels of parallelism using GPU
- Direct and indirect lighting (global illumination)
- Triangle primitives (polygon primitives also supported through Assimp importer)
- Loading triangle meshes from external files
- Multiple objects in scene
- Transformation using translation, rotation, scale
- Physically based camera model with adjustable vFOV, focus distance, defocus blur (depth of field)
- Bounding Volume Heirarchy (BVH) acceleration structure
  - Automatic construction on CPU
  - Stack-based traversal on GPU
- Various materials
  - Diffuse
  - Metal
  - Glass
  - Area lights
- Smooth shading / soft shadows
- Adjustable samples per pixel for anti-aliasing and decreased noise
- Image output to .ppm

## Next steps
Planned features include:
- Build system
- Scene description format (so different scenes can be rendered without modifying code)
- Textures
- HDR
- Tone mapping
- Surface Area Heuristic (SAH) BVH construction
