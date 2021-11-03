#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <array>

#include "Vertex.h"

class Mesh
{
public :

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;
};