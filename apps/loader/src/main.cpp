#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <filesystem>
#include <iostream>
#include <vector>

// #include <fastgltf/glm_element_traits.hpp> // Suporte opcional para glm
// #include <fastgltf/glm_element_traits.hpp> // Caso use glm::vec3
//  #include <glm/glm.hpp>
//  #include <string>
//  #include <vector>

struct Vertex {
    fastgltf::math::fvec3 position;
    fastgltf::math::fvec3 normal;
    fastgltf::math::fvec2 uv;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

struct Model {
    std::vector<MeshData> meshes;
};

void loadModel(const std::filesystem::path& filePath, MeshData* pMesh) {
    // 1. Create the data buffer using the modern static constructor
    auto expectedBuffer = fastgltf::GltfDataBuffer::FromPath(filePath);

    // 2. Always validate that the file read succeeded
    if (expectedBuffer.error() != fastgltf::Error::None) {
        std::cerr << "Failed to read file buffer into memory.\n";
        return;
    }

    fastgltf::Parser parser;
    constexpr auto options = fastgltf::Options::LoadExternalBuffers;

    // 3. Parse the asset using the buffer (.get() unpacks the expected value)
    auto expectedAsset = parser.loadGltf(expectedBuffer.get(), filePath.parent_path(), options);
    if (expectedAsset.error() != fastgltf::Error::None) {
        std::cerr << "Failed to parse glTF structure.\n";
        return;
    }

    fastgltf::Asset asset = std::move(expectedAsset.get());
    // Model is loaded and ready!

    for (const auto& mesh : asset.meshes) {
        for (const auto& primitive : mesh.primitives) {
            // std::vector<Vertex> vertices;
            // std::vector<uint32_t> indices;

            // --- INDEX BUFFER ---
            if (primitive.indicesAccessor.has_value()) {
                auto& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
                pMesh->indices.resize(indexAccessor.count);
                // Utiliza o fastgltf::iterateAccessor para copiar para o vetor
                fastgltf::iterateAccessorWithIndex<uint32_t>(asset, indexAccessor,
                                                             [&](uint32_t idx, size_t size) { pMesh->indices[size] = idx; });

                // (Aqui você enviaria 'indices' para um Element Array Buffer da sua API)
            }

            // uint32_t pos = 0;
            // for (auto& val : indices) {
            //     std::cout << "pos " << pos << " [ " << val << " ]\n";
            //     pos++;
            // }

            // --- VERTEX BUFFER (Posição e UVs) ---
            const auto* posAttr = primitive.findAttribute("POSITION");
            const auto* uvAttr = primitive.findAttribute("TEXCOORD_0");
            const auto* normalAttr = primitive.findAttribute("NORMAL");

            if (posAttr != primitive.attributes.end()) {
                auto& posAccessor = asset.accessors[posAttr->accessorIndex];
                pMesh->vertices.resize(posAccessor.count);

                // Define your vertex structure or destination container
                // std::vector<fastgltf::math::fvec3> positions(posAccessor.count);

                // Use fastgltf accessor tools to unpack the data directly
                // fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(
                //     asset, posAccessor, [&](fastgltf::math::fvec3 pos, std::size_t idx) { positions[idx] = pos; });

                // for (auto val3 : positions) {
                //     std::cout << "val -> x: " << val3.x() << " y: " << val3.y() << " z: " << val3.z() << '\n';
                // }

                // Carrega as posições
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, posAccessor, [&](fastgltf::math::fvec3 pos, size_t isize) {
                    pMesh->vertices[isize].position[0] = pos.x();
                    pMesh->vertices[isize].position[1] = pos.y();
                    pMesh->vertices[isize].position[2] = pos.z();
                });
            }

            if (normalAttr != primitive.attributes.end()) {
                auto& normalAccessor = asset.accessors[normalAttr->accessorIndex];
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec3>(asset, normalAccessor,
                                                                          [&](fastgltf::math::fvec3 nor, size_t isize) {
                                                                              pMesh->vertices[isize].normal[0] = nor.x();
                                                                              pMesh->vertices[isize].normal[1] = nor.y();
                                                                              pMesh->vertices[isize].normal[2] = nor.z();
                                                                          });
            }

            if (uvAttr != primitive.attributes.end()) {
                auto& uvAccessor = asset.accessors[uvAttr->accessorIndex];

                // Carrega as UVs
                fastgltf::iterateAccessorWithIndex<fastgltf::math::fvec2>(asset, uvAccessor, [&](fastgltf::math::fvec2 uvd, size_t isize) {
                    pMesh->vertices[isize].uv[0] = uvd.x();
                    pMesh->vertices[isize].uv[1] = uvd.y();
                });
            }

            // // (Aqui você enviaria 'vertices' para o seu Vertex Buffer Object da sua API)

            // --- TEXTURAS ---
            if (primitive.materialIndex.has_value()) {
                const auto& material = asset.materials[primitive.materialIndex.value()];
                // Verifica se há textura baseColor
                if (material.pbrData.baseColorTexture.has_value()) {

                    const auto& textureInfo = material.pbrData.baseColorTexture.value();
                    auto& texture = asset.textures[textureInfo.textureIndex];
                    auto& image = asset.images[texture.imageIndex.value()];

                    // Dados da imagem estão prontos para serem carregados para a GPU
                    // (Ex: usando stbi_load_from_memory ou vkCmdCopyBufferToImage)
                }
            }
        }
    }
}

int main() {

    MeshData mesh;

    loadModel("./assets/models/teste/cubo.gltf", &mesh);

    std::cout << "Tot Indices: " << mesh.indices.size() << '\n';
    for (const auto& indice : mesh.indices) {
        std::cout << "idx:" << indice << '\n';
    }

    std::cout << "Tot Vertex: " << mesh.vertices.size() << '\n';
    for (const auto& vertice : mesh.vertices) {

        std::cout << "Position: " << vertice.position.x() << ", " << vertice.position.y() << ", " << vertice.position.z();
        std::cout << "\t | Normal: " << vertice.normal.x() << ", " << vertice.normal.y() << ", " << vertice.normal.z();
        std::cout << "\t | UV: " << vertice.uv.x() << ", " << vertice.uv.y() << '\n';
    }

    std::cout << "teste OK" << '\n';
    return 0;
}
