#version 330 core

uniform sampler2D uTexture0;
uniform float uExposure;
uniform float uFrameTime;

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

const vec4 sample_deltas[] = vec4[](
	vec4(0, 0, 1, 0),
	vec4(0, 0, 1, 1),
	vec4(0, 0, 1, 2),
	vec4(0, 0, 1, 3),
	vec4(0, 0, 0.5, 4),
	vec4(0, 0, 0.5, 5),
	vec4(-2, 0, 0.5, 2),
	vec4(2, 0, 0.5, 2),
	vec4(0, -2, 0.5, 2),
	vec4(0, 2, 0.5, 2),
	vec4(-3, -3, 0.3, 3),
	vec4(-3, 3, 0.3, 3),
	vec4(3, -3, 0.3, 3),
	vec4(3, 3, 0.3, 3)
);

void main() {
	
	vec4 color = vec4(0);
	float a = 1.0;
	
	// correct sample based on filtered input
	for (int i = 0; i < sample_deltas.length(); i++) {
		vec4 c = texture(uTexture0, v_tex_coord + sample_deltas[i].xy / vec2(textureSize(uTexture0, 0)), sample_deltas[i].w);
		color.rgb += c.rgb * a;
		color.a += a * c.a;
		a -= a * c.a * sample_deltas[i].z;
	}
	
	color.rgb /= color.a;

	// exposure (tone mapping)
	color.rgb = 1.0 - exp(-uExposure * color.rgb);

	// gamma correction
	color.rgb = pow(color.rgb, vec3(0.45));

	// dither
	// http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl
	float junk = fract(sin(dot(gl_FragCoord.xy + uFrameTime, vec2(12.9898, 78.233))) * 43758.5453);
	color.rgb += (junk-0.5) / 255.0;

	// output final color
	f_color = vec4(color.rgb, 1.0);
}

#endif



























