#include <fastgltf/core.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/types.hpp>
#include <filesystem>
#include <glm/glm.hpp>
#include <iostream>
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::string textureName;
};

struct Model {
    std::vector<MeshData> meshes;
};

void carregarMateriais(const fastgltf::Asset& asset) {

    // 3. Itera pelos materiais do modelo
    // const auto& loadedAsset = asset.get();
    for (size_t i = 0; i < asset.materials.size(); ++i) {
        const auto& material = asset.materials[i];

        std::cout << "Material [" << i << "]: " << material.name.c_str() << '\n';

        // Acessa o atalho PBR da nova API
        const auto& pbr = material.pbrData;

        // 5. Extrai o 'baseColorFactor', que contem o vetor RGBA (canais de 0.0 a 1.0)
        auto color = pbr.baseColorFactor;
        // auto metallic = pbr.metallicFactor;
        // auto roughness = pbr.roughnessFactor;

        // Acesso ao índice da textura base (se houver)
        if (pbr.baseColorTexture.has_value()) {
            auto textureIndex = pbr.baseColorTexture->textureIndex;
            std::cout << "Texture index: " << textureIndex << '\n';
        }

        // Pega os 3 primeiros canais (R, G, B) e converte para valor de 0 a 255
        float red = color[0];
        float green = color[1];
        float blue = color[2];

        std::cout << "  Cor RGB: R(" << red * 255.0F << ") G(" << green * 255.0F << ") B(" << blue * 255.0F << ")" << '\n';
    }
}

void loadModel(const std::filesystem::path& filePath, MeshData* pMesh) { // NOLINT
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

    carregarMateriais(asset);

    for (const auto& mesh : asset.meshes) {
        for (const auto& primitive : mesh.primitives) {

            // --- INDEX BUFFER ---
            if (primitive.indicesAccessor.has_value()) {
                auto& indexAccessor = asset.accessors[primitive.indicesAccessor.value()];
                pMesh->indices.resize(indexAccessor.count);

                fastgltf::iterateAccessorWithIndex<uint32_t>(asset, indexAccessor,
                                                             [&](uint32_t idx, size_t size) { pMesh->indices[size] = idx; });
            }

            // --- VERTEX BUFFER ---
            const auto* posAttr = primitive.findAttribute("POSITION");
            const auto* uvAttr = primitive.findAttribute("TEXCOORD_0");
            const auto* normalAttr = primitive.findAttribute("NORMAL");

            if (posAttr != primitive.attributes.end()) {
                auto& posAccessor = asset.accessors[posAttr->accessorIndex];
                pMesh->vertices.resize(posAccessor.count);

                // Carrega as posições
                fastgltf::iterateAccessorWithIndex<glm::vec3>(
                    asset, posAccessor, [&](glm::vec3 vertexPosition, size_t index) { pMesh->vertices[index].position = vertexPosition; });
            }

            if (normalAttr != primitive.attributes.end()) {
                auto& normalAccessor = asset.accessors[normalAttr->accessorIndex];

                // Carrega normal
                fastgltf::iterateAccessorWithIndex<glm::vec3>(asset, normalAccessor,
                                                              [&](glm::vec3 nor, size_t index) { pMesh->vertices[index].normal = nor; });
            }

            if (uvAttr != primitive.attributes.end()) {
                auto& uvAccessor = asset.accessors[uvAttr->accessorIndex];

                // Carrega as UVs
                fastgltf::iterateAccessorWithIndex<glm::vec2>(asset, uvAccessor,
                                                              [&](glm::vec2 uvd, size_t index) { pMesh->vertices[index].uv = uvd; });
            }

            // --- TEXTURAS ---
            if (primitive.materialIndex.has_value()) {
                const auto& material = asset.materials[primitive.materialIndex.value()];
                // Verifica se há textura baseColor
                if (material.pbrData.baseColorTexture.has_value()) {

                    const auto& textureInfo = material.pbrData.baseColorTexture.value();
                    auto& texture = asset.textures[textureInfo.textureIndex];

                    // auto& image = asset.images[texture.imageIndex.value()];

                    if (texture.imageIndex.has_value()) {
                        const auto& image = asset.images[texture.imageIndex.value()];

                        // Caminho da imagem em disco ou arquivo embutido
                        if (std::holds_alternative<fastgltf::sources::URI>(image.data)) {
                            pMesh->textureName = std::get<fastgltf::sources::URI>(image.data).uri.c_str();
                        }
                    }
                }
            }
        }
    }
}

int main() {

    MeshData mesh;

    loadModel("./assets/models/teste/cubo.gltf", &mesh);

    // Dados da imagem estão prontos para serem carregados para a GPU
    // (Ex: usando stbi_load_from_memory ou vkCmdCopyBufferToImage)

    std::cout << "Tot Indices: " << mesh.indices.size() << '\n';
    for (const auto& indice : mesh.indices) {
        std::cout << "idx:" << indice << '\n';
    }

    std::cout << "Tot Vertex: " << mesh.vertices.size() << '\n';
    for (const auto& vertice : mesh.vertices) {

        std::cout << "Position: " << vertice.position.x << ", " << vertice.position.y << ", " << vertice.position.z;
        std::cout << "\t | Normal: " << vertice.normal.x << ", " << vertice.normal.y << ", " << vertice.normal.z;
        std::cout << "\t | UV: " << vertice.uv.x << ", " << vertice.uv.y << '\n';
    }
    std::cout << "Texture URI: " << mesh.textureName << '\n';

    std::cout << "teste OK" << '\n';
    return 0;
}
