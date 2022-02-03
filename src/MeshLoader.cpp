#include "MeshLoader.h"

#include "ResourceSystem.h"

#include "edl/logger.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <unordered_set>;
#include <iostream>

namespace edl {

struct objsucks {
    objsucks(tinyobj::index_t i) {
        x = i.vertex_index;
        y = i.normal_index;
        z = i.texcoord_index;
    }
    uint32_t x, y, z;

    bool operator==(const objsucks& other) const {
        return (x == other.x) && (y == other.y) && (z == other.z);
    }
};

struct objsucks_hash {
    std::size_t operator () (const objsucks& p) const {
        auto h1 = std::hash<uint32_t>{}(p.x);
        auto h2 = std::hash<uint32_t>{}(p.y);
        auto h3 = std::hash<uint32_t>{}(p.z);


        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 ^ h2 ^ h3;
    }
};

void loadMesh(res::Toolchain& toolchain, res::Resource& res) {
    edl::ResourceSystem& system = toolchain.getTool<edl::ResourceSystem>("system");

    edl::res::allocateResourceData(res, sizeof(Mesh), *system.allocator);
    Mesh& meshres = edl::res::getResourceData<Mesh>(res);

    tinyobj::attrib_t attrib = {};
    std::vector<tinyobj::shape_t> shapes = {};
    std::vector<tinyobj::material_t> materials = {};

    std::string warn = "";
    std::string err = "";

    std::cout << "--- Loading File: " << res.path << " START!" << std::endl;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, res.path.c_str());
    std::cout << "--- Loading File: " << res.path << " END!" << std::endl;

    if (!warn.empty()) {
        //log::warn("AssetLoading", warn);
    }

    if (!err.empty()) {
        log::error("AssetLoading", err);
    }

    auto& shape = shapes[0];
    auto& mesh = shape.mesh;

    uint32_t vertexCount = mesh.indices.size();
    uint32_t indexCount = mesh.indices.size();

    std::vector<glm::vec4> positions(vertexCount);
    std::vector<glm::vec4> normals(vertexCount);
    std::vector<glm::vec4> tangents(vertexCount);
    std::vector<unsigned short> indices(indexCount);
    //std::vector<glm::vec4> bitangents(vertexCount);

    std::vector<glm::vec2> texCoords0(vertexCount);

    auto& v = attrib.vertices;
    auto& n = attrib.normals;
    auto& tv = attrib.texcoords;

    std::unordered_map<objsucks, uint32_t, objsucks_hash> indexMap;

    size_t nextIndex = 0;
    size_t index_offset = 0;
    for (size_t f = 0; f < mesh.num_face_vertices.size(); f++) {
        auto idx0 = mesh.indices[index_offset + 0];
        auto idx1 = mesh.indices[index_offset + 1];
        auto idx2 = mesh.indices[index_offset + 2];

        glm::vec3 v0 = glm::vec3(attrib.vertices[3 * idx0.vertex_index + 0], attrib.vertices[3 * idx0.vertex_index + 1], attrib.vertices[3 * idx0.vertex_index + 2]);
        glm::vec3 v1 = glm::vec3(attrib.vertices[3 * idx1.vertex_index + 0], attrib.vertices[3 * idx1.vertex_index + 1], attrib.vertices[3 * idx1.vertex_index + 2]);
        glm::vec3 v2 = glm::vec3(attrib.vertices[3 * idx2.vertex_index + 0], attrib.vertices[3 * idx2.vertex_index + 1], attrib.vertices[3 * idx2.vertex_index + 2]);

        glm::vec2 uv0 = glm::vec2(attrib.texcoords[2 * idx0.texcoord_index + 0], attrib.texcoords[2 * idx0.texcoord_index + 1]);
        glm::vec2 uv1 = glm::vec2(attrib.texcoords[2 * idx1.texcoord_index + 0], attrib.texcoords[2 * idx1.texcoord_index + 1]);
        glm::vec2 uv2 = glm::vec2(attrib.texcoords[2 * idx2.texcoord_index + 0], attrib.texcoords[2 * idx2.texcoord_index + 1]);

        glm::vec3 e10 = v1 - v0;
        glm::vec3 e20 = v2 - v0;
        glm::vec2 duv10 = uv1 - uv0;
        glm::vec2 duv20 = uv2 - uv0;

        float denom = 1.0f / (duv10.x * duv20.y - duv20.x * duv10.y);

        glm::vec4 tangent;
        tangent.x = f * (duv20.y * e10.x - duv10.y * e20.x);
        tangent.y = f * (duv20.y * e10.y - duv10.y * e20.y);
        tangent.z = f * (duv20.y * e10.z - duv10.y * e20.z);
        tangent.w = 0.0f;

        //TODO: Consider smoothing?

        for (int i = 0; i < 3; i++) {
            auto idx = mesh.indices[index_offset + i];
            objsucks o(idx);
            uint32_t oidx = 0;
            if (indexMap.find(o) != indexMap.end()) {
                oidx = indexMap.at(o);
            }
            else {
                oidx = nextIndex;
                nextIndex++;
                indexMap.insert({ o, oidx });
            }

            positions[oidx].x = v[idx.vertex_index * 3 + 0];
            positions[oidx].y = v[idx.vertex_index * 3 + 1];
            positions[oidx].z = v[idx.vertex_index * 3 + 2];
            positions[oidx].w = 1.0f;

            normals[oidx].x = n[idx.normal_index * 3 + 0];
            normals[oidx].y = n[idx.normal_index * 3 + 1];
            normals[oidx].z = n[idx.normal_index * 3 + 2];
            normals[oidx].w = 0.0f;

            texCoords0[oidx].x = tv[idx.texcoord_index * 2 + 0];
            texCoords0[oidx].y = tv[idx.texcoord_index * 2 + 1];

            tangents[oidx] = tangent;

            indices[index_offset + i] = oidx;
        }

        index_offset += 3;
    }

    meshres.indexCount = indexCount;

    vertexCount = nextIndex;

    meshres.positionOffset = edl::getStorageBufferIndex(system.positionBuffer, vertexCount);
    meshres.normalOffset = edl::getStorageBufferIndex(system.normalBuffer, vertexCount);
    meshres.tangentOffset = edl::getStorageBufferIndex(system.normalBuffer, vertexCount);
    //uint32_t bitangentOffset = edl::getStorageBufferIndex(system.normalBuffer, vertexCount);
    meshres.texCoord0Offset = edl::getStorageBufferIndex(system.texCoord0Buffer, vertexCount);

    meshres.indexOffset = edl::getStorageBufferIndex(system.indexBuffer, indexCount);

    edl::updateStorageBuffer(system.stagingBuffer, system.positionBuffer, meshres.positionOffset, positions.data(), vertexCount);
    edl::updateStorageBuffer(system.stagingBuffer, system.normalBuffer, meshres.normalOffset, normals.data(), vertexCount);
    edl::updateStorageBuffer(system.stagingBuffer, system.normalBuffer, meshres.tangentOffset, tangents.data(), vertexCount);
    //edl::updateStorageBuffer(system.stagingBuffer, system.normalBuffer, bitangentOffset, bitangents.data(), vertexCount);
    edl::updateStorageBuffer(system.stagingBuffer, system.texCoord0Buffer, meshres.texCoord0Offset, texCoords0.data(), vertexCount);

    edl::updateStorageBuffer(system.stagingBuffer, system.indexBuffer, meshres.indexOffset, indices.data(), indexCount);

    res.status = edl::res::ResourceStatus::LOADED;
}

}