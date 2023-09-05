
#pragma once

// std
#include <iostream>
#include <string>
#include <vector>

// glm
#include <glm/glm.hpp>

// stb
#include <stb_image.h>


class Texture {
private:
	glm::ivec2 m_size{0,0};
	std::vector<float> m_data;

public:
	Texture() {}

	// create a texture from a file
	// supports JPEG, PNG, TGA, BMP and a few others
	Texture(const std::string &filename) {
		int w, h, n;

		stbi_set_flip_vertically_on_load(true);
		unsigned char *img = stbi_load(filename.c_str(), &w, &h, &n, 3);

		if (!img) {
			std::cerr << "Error: Failed to load image " << filename << " : " << stbi_failure_reason();
			throw std::runtime_error("Failed to load image " + filename);
		}

		m_data = std::vector<float>(w * h * 3);
		for (int i = 0; i < w * h * 3; i++)
			m_data[i] = img[i] / 255.f;
		m_size = glm::ivec2(w, h);
		stbi_image_free(img);
	}


	// fetch the value of a single channel of a texel
	float texel(float x, float y, int n) const {
		// wrap coordinates
		int mx = int(glm::floor(x)) % m_size.x;
		mx += (mx < 0) ? m_size.x : 0;

		int my = int(glm::floor(y)) % m_size.y;
		my += (my < 0) ? m_size.y : 0;
		
		// fetch
		return m_data.at(n + (mx + my*m_size.x)*3);
	}

	// fetch the value of a single channel of a texel
	float texel(const glm::ivec2 &p, int n) const {
		return texel(float(p.x), float(p.y), n);
	}

	// sample given a range of uv in [0, 1]^2
	// provides wrapping for values outside that range
	glm::vec3 sample(float u, float v) const {
		float x = u * m_size.x + 0.5f;
		float y = v * m_size.y + 0.5f;
		return glm::vec3(texel(x, y, 0), texel(x, y, 1), texel(x, y, 2));
	}

	// sample given a range of uv in [0, 1]^2
	// provides wrapping for values outside that range
	glm::vec3 sample(const glm::vec2 &uv) const {
		glm::vec2 p = uv * glm::vec2(m_size) + glm::vec2(0.5f);
		return glm::vec3(texel(p, 0), texel(p, 1), texel(p, 2));
	}
};