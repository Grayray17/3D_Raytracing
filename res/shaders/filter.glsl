#version 330 core

uniform float uFrameTime;
uniform sampler2D uTexture0;

#ifdef _VERTEX_

void main() { }

#endif

#ifdef _GEOMETRY_

layout(points) in;
layout(triangle_strip, max_vertices = 3) out;

out vec2 v_tex_coord;

void main() {
    // output a single triangle that covers the whole screen
    
    gl_Position = vec4(3.0, 1.0, 0.0, 1.0);
    v_tex_coord = vec2(2.0, 1.0);
    EmitVertex();
    
    gl_Position = vec4(-1.0, 1.0, 0.0, 1.0);
    v_tex_coord = vec2(0.0, 1.0);
    EmitVertex();
    
    gl_Position = vec4(-1.0, -3.0, 0.0, 1.0);
    v_tex_coord = vec2(0.0, -1.0);
    EmitVertex();
    
    EndPrimitive();
    
}

#endif

#ifdef _FRAGMENT_

in vec2 v_tex_coord;

out vec4 f_color;

void main() {
	
	f_color = texture(uTexture0, v_tex_coord);
	// relevance to current time
	f_color.a = exp(-pow(10.0 * mod(uFrameTime - f_color.a, 100.0), 2.0));
	// bias away from 0 to avoid division problems later
	f_color.a += 0.001;
	f_color.rgb *= f_color.a;
	
}

#endif



























