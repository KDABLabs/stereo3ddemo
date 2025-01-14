#include "frustum_rect.h"

namespace all::serenity {

FrustumRect::FrustumRect()
{
    enabled.valueChanged().connect([this](bool isEnabled) {
                              component<Serenity::MeshRenderer>()->mesh = (isEnabled) ? m_mesh.get() : nullptr;
                          })
            .release();
    backgroundColor.valueChanged().connect([this] { updateUBO(); }).release();
    outlineColor.valueChanged().connect([this] { updateUBO(); }).release();
    outlineWidth.valueChanged().connect([this] { updateUBO(); }).release();

    m_materialUBO = createChild<Serenity::StaticUniformBuffer>();
    m_materialUBO->size = sizeof(glm::vec4) * 3;

    {
        auto *vertexBuffer = createChild<Serenity::StaticVertexBuffer>();
        vertexBuffer->size = 6 * sizeof(glm::vec3);

        std::vector<uint8_t> rawVertexData;
        rawVertexData.resize(6 * sizeof(glm::vec3));
        glm::vec3* vertices = reinterpret_cast<glm::vec3*>(rawVertexData.data());
        vertices[0] = glm::vec3(-1.0f, 1.0f, 0.0f);
        vertices[1] = glm::vec3(-1.0f, -1.0f, 0.0f);
        vertices[2] = glm::vec3(1.0f, -1.0f, 0.0f);
        vertices[3] = glm::vec3(1.0f, -1.0f, 0.0f);
        vertices[4] = glm::vec3(1.0f, 1.0f, 0.0f);
        vertices[5] = glm::vec3(-1.0f, 1.0f, 0.0f);
        vertexBuffer->data = rawVertexData;

        m_mesh = std::make_unique<Serenity::Mesh>();
        m_mesh->setObjectName("FrustumRect");
        m_mesh->vertexBuffers = { vertexBuffer };
        m_mesh->vertexFormat = Serenity::VertexFormat{
            .buffers = {
                    KDGpu::VertexBufferLayout{
                            .binding = 0,
                            .stride = sizeof(glm::vec3),
                    },
            },
            .attributes = {
                    KDGpu::VertexAttribute{
                            .location = 0,
                            .binding = 0,
                            .format = KDGpu::Format::R32G32B32_SFLOAT,
                            .offset = 0,
                    },
            },
        };

        auto* frustumRectShader = createChild<Serenity::SpirVShaderProgram>();
        frustumRectShader->vertexShader = SHADER_DIR "frustum_rect.vert.spv";
        frustumRectShader->fragmentShader = SHADER_DIR "frustum_rect.frag.spv";

        auto* material = createChild<Serenity::Material>();
        material->shaderProgram = frustumRectShader;
        material->setUniformBuffer(3, 0, m_materialUBO);

        Serenity::ColorBlendState blendState;
        Serenity::ColorBlendState::AttachmentBlendState attachmentBlendState;

        attachmentBlendState.format = KDGpu::Format::UNDEFINED;
        attachmentBlendState.blending.blendingEnabled = true;
        attachmentBlendState.blending.alpha.operation = KDGpu::BlendOperation::Add;
        attachmentBlendState.blending.color.operation = KDGpu::BlendOperation::Add;
        attachmentBlendState.blending.alpha.srcFactor = KDGpu::BlendFactor::SrcAlpha;
        attachmentBlendState.blending.color.srcFactor = KDGpu::BlendFactor::SrcAlpha;
        attachmentBlendState.blending.alpha.dstFactor = KDGpu::BlendFactor::OneMinusSrcAlpha;
        attachmentBlendState.blending.color.dstFactor = KDGpu::BlendFactor::OneMinusSrcAlpha;
        blendState.attachmentBlendStates = { attachmentBlendState };

        Serenity::RenderStateSet rectRenderState;
        rectRenderState.setColorBlendState(blendState);

        auto* meshRenderer = createComponent<Serenity::MeshRenderer>();
        meshRenderer->renderState = rectRenderState;
        meshRenderer->mesh = m_mesh.get();
        meshRenderer->material = material;
        meshRenderer->computeTriangleInfo = false;

    }

    updateUBO();
}

void FrustumRect::updateUBO()
{
    std::vector<uint8_t> rawData;
    rawData.resize(sizeof(glm::vec4) * 3);

    const glm::vec4 background = backgroundColor();
    const glm::vec4 outline = outlineColor();
    const float width = outlineWidth();

    std::memcpy(rawData.data(), &background, sizeof(glm::vec4));
    std::memcpy(rawData.data() + sizeof(glm::vec4), &outline, sizeof(glm::vec4));
    std::memcpy(rawData.data() + 2 * sizeof(glm::vec4), &width, sizeof(float));

    m_materialUBO->data = rawData;
}

} //
