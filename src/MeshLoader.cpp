#include "MeshLoader.h"

#include "ResourceSystem.h"

#include "edl/logger.h"

#include "tiny_obj_loader.h"

#include <iostream>

namespace edl {

void loadMesh(res::Toolchain& toolchain, res::Resource& res) {
    edl::ResourceSystem& system = toolchain.getTool<edl::ResourceSystem>("system");

    edl::res::allocateResourceData(res, sizeof(Mesh), *system.allocator);
    Mesh& meshres = edl::res::getResourceData<Mesh>(res);

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, res.path);

    if (!warn.empty()) {
        //log::warn("AssetLoading", warn);
    }

    if (!err.empty()) {
        log::error("AssetLoading", err);
    }

    auto& shape = shapes[0];
    auto& mesh = shape.mesh;

    uint32_t vsize = attrib.vertices.size() / 3;
    uint32_t nsize = attrib.normals.size() / 3;
    uint32_t tsize = attrib.texcoords.size() / 2;

    uint32_t largestBuffer = vsize > nsize ? (vsize > tsize ? 0 : 2) : (nsize > tsize ? 1 : 2);

    uint32_t vertexCount = largestBuffer == 0 ? vsize : (largestBuffer == 1 ? nsize : tsize);
    uint32_t indexCount = mesh.indices.size();

    meshres.positionOffset = edl::getStorageBufferIndex(system.positionBuffer, vertexCount);
    meshres.normalOffset = edl::getStorageBufferIndex(system.normalBuffer, vertexCount);
    meshres.tangentOffset = edl::getStorageBufferIndex(system.normalBuffer, vertexCount);
    //uint32_t bitangentOffset = edl::getStorageBufferIndex(system.normalBuffer, vertexCount);
    meshres.texCoord0Offset = edl::getStorageBufferIndex(system.texCoord0Buffer, vertexCount);

    meshres.indexOffset = edl::getStorageBufferIndex(system.indexBuffer, indexCount);

    std::vector<glm::vec4> positions(vertexCount);
    std::vector<glm::vec4> normals(vertexCount);
    std::vector<glm::vec4> tangents(vertexCount);
    std::vector<unsigned short> indices(indexCount);
    //std::vector<glm::vec4> bitangents(vertexCount);

    std::vector<glm::vec2> texCoords0(vertexCount);

    auto& v = attrib.vertices;
    auto& n = attrib.normals;
    auto& tv = attrib.texcoords;

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

        uint32_t oidx0 = largestBuffer == 0 ? idx0.vertex_index : (largestBuffer == 1 ? idx0.normal_index : idx0.texcoord_index);
        uint32_t oidx1 = largestBuffer == 0 ? idx1.vertex_index : (largestBuffer == 1 ? idx1.normal_index : idx1.texcoord_index);
        uint32_t oidx2 = largestBuffer == 0 ? idx2.vertex_index : (largestBuffer == 1 ? idx2.normal_index : idx2.texcoord_index);

        tangents[oidx0] = tangent;
        tangents[oidx1] = tangent;
        tangents[oidx2] = tangent;

        for (int i = 0; i < 3; i++) {
            auto idx = mesh.indices[index_offset + i];
            uint32_t oidx = largestBuffer == 0 ? idx.vertex_index : (largestBuffer == 1 ? idx.normal_index : idx.texcoord_index);

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

            indices[index_offset + i] = oidx;
        }

        index_offset += 3;
    }

    meshres.indexCount = indexCount;

    edl::updateStorageBuffer(system.stagingBuffer, system.positionBuffer, meshres.positionOffset, positions.data(), vertexCount);
    edl::updateStorageBuffer(system.stagingBuffer, system.normalBuffer, meshres.normalOffset, normals.data(), vertexCount);
    edl::updateStorageBuffer(system.stagingBuffer, system.normalBuffer, meshres.tangentOffset, tangents.data(), vertexCount);
    //edl::updateStorageBuffer(system.stagingBuffer, system.normalBuffer, bitangentOffset, bitangents.data(), vertexCount);
    edl::updateStorageBuffer(system.stagingBuffer, system.texCoord0Buffer, meshres.texCoord0Offset, texCoords0.data(), vertexCount);

    edl::updateStorageBuffer(system.stagingBuffer, system.indexBuffer, meshres.indexOffset, indices.data(), indexCount);

    res.status = edl::res::ResourceStatus::LOADED;
}

}