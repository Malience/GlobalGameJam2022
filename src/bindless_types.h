#pragma once

#include "glm/glm.hpp"

#include <cstdint>

struct DrawData {
	int positionOffset;
	int normalOffset;
	int tangentOffset;
	//int bitangentOffset;

	int texCoord0Offset;
	//int texCoord1Offset;
	int materialOffset;

	int mvpOffset;
	//int invMVPOffset;

	int padding0;
	int padding1;
};

struct SceneData {
	glm::vec4 cameraPosition;
	glm::vec4 lightDir;
	glm::vec4 lightColor;

	float directionalLightPower;
	uint32_t activeLights;
	int pad0;
	int pad1;
};

struct DirLight {
	glm::vec4 lightDir;
	glm::vec4 lightColor;
	float directionalLightPower;
};

struct PBRMaterial {
	glm::vec4 tint;

	float metallic;
	float roughness;
	float ao;

	int albedoTexture;
	int normalTexture;
	//int roughnessTexture;
	int pad0;
	int pad1;
	int pad2;
	//int pad3;
};

struct Light {
	glm::vec4 position;
	glm::vec4 direction;
	glm::vec4 color;

	uint32_t type;
	uint32_t pad0;
	uint32_t pad1;
	uint32_t pad2;
};