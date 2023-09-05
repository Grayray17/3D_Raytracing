
# Source files
set(sources
	"cgra_gui.hpp"
	"cgra_gui.cpp"
	
	"cgra_shader.hpp"
	"cgra_shader.cpp"
)

# Add these sources to the project target
target_relative_sources(${CGRA_PROJECT} ${sources})
