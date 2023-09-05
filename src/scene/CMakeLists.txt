
# Source files
set(sources
	"camera.hpp"
	"camera.cpp"

	"path_tracer.hpp"
	"path_tracer.cpp"

	"light.hpp"
	"light.cpp"

	"material.hpp"
	"material.cpp"

	"ray.hpp"

	"scene.hpp"
	"scene.cpp"

	"scene_object.hpp"
	"scene_object.cpp"

	"shape.hpp"
	"shape.cpp"

	"texture.hpp"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
