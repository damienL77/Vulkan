#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <array>

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>

#include "volk/volk.h"

struct SurfaceModel {
    glm::vec3 camPos;
	glm::vec3 albedo;
	float metallic;
	float roughness;
	float ao;
}; 