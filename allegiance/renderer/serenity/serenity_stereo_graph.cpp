#include "serenity_stereo_graph.h"
#include <Serenity/gui/render/renderer.h>
#include <Serenity/gui/render/algorithm/building_blocks/bind_groups.h>

using namespace Serenity;

using namespace KDGpu;

KDGpu::Handle<KDGpu::GpuSemaphore_t> all::serenity::StereoRenderAlgorithm::submitCommandBuffers(uint32_t frameInFlightIndex, const std::vector<KDGpu::Handle<KDGpu::GpuSemaphore_t>>& presentationCompleteSemaphores, KDGpu::Handle<KDGpu::Fence_t> frameFence)
{
    // Create a command recorder
    CommandRecorder commandRecorder = renderer()->device()->createCommandRecorder();

    // Record Memory Transform / Bind Group Updates
    updateUBOs(&commandRecorder, frameInFlightIndex);

    // Perform Buffer copies
    RenderAlgorithm::recordBufferCopiesAndIssueMemoryBarrier(&commandRecorder, frameInFlightIndex);

    const glm::vec4 clearCol = clearColor();

    // 1) Render Scene Content offscreen using multiview
    {
        const Render::RenderTargetResource* renderTargetResource = renderTargetResourceForRefIndex(offscreenMultiViewRenderTargetRefIndex());
        const RenderTarget* rt = renderTargetResource->renderTarget();

        RenderPassCommandRecorderOptions renderPassOptions{
            .colorAttachments = {
                    { .view = {},
                      .resolveView = {},
                      .clearValue = { clearCol[0], clearCol[1], clearCol[2], clearCol[3] },
                      .finalLayout = TextureLayout::ShaderReadOnlyOptimal },
            },
            .depthStencilAttachment = {
                    .view = renderTargetResource->depthTextureView(),
            },
            .samples = SampleCountFlagBits(renderTargetResource->samples()),
            .viewCount = 2,
        };

        const bool useMSAA = renderTargetResource->samples() > RenderAlgorithm::SamplesCount::Samples_1;
        if (useMSAA) {
            renderPassOptions.colorAttachments[0].view = renderTargetResource->msaaTextureView();
            renderPassOptions.colorAttachments[0].resolveView = rt->imageView(rt->currentImageIndex());
        } else {
            renderPassOptions.colorAttachments[0].view = rt->imageView(rt->currentImageIndex());
        }

        RenderPassCommandRecorder renderPass = commandRecorder.beginRenderPass(renderPassOptions);

        if (m_renderData.renderables.size() > 0) {
            const std::vector<RenderPhase>& phases = renderPhases();
            for (size_t phaseIndex = 0, m = phases.size(); phaseIndex < m; ++phaseIndex) {
                const RenderPhase& phase = phases[phaseIndex];
                switch (phase.type) {
                case RenderPhase::Type::Opaque:
                    recordCommandBuffersForOpaquePhase(&renderPass, frameInFlightIndex, phaseIndex);
                    break;
                case RenderPhase::Type::Alpha:
                    recordCommandBuffersForAlphaPhase(&renderPass, frameInFlightIndex, phaseIndex);
                    break;
                case RenderPhase::Type::ZFill:
                    recordCommandBuffersForZFillPhase(&renderPass, frameInFlightIndex, phaseIndex);
                    break;
                }
            }
        } else { // No Renderables
            auto* nullRVRenderer = renderTargetResource->nullRenderer();

            // Bind pipeline
            renderPass.setPipeline(nullRVRenderer->pipeline);

            const KDGpu::Extent2D extent = rt->identityExtent();

            // Set Viewport
            renderPass.setViewport(Viewport{
                    .x = 0.0f,
                    .y = float(extent.height),
                    .width = float(extent.width),
                    .height = -float(extent.height),
            });
        }

        // End render pass
        renderPass.end();
    }

    // 2) Render offscreen content side by side + overlays on Window
    // TODO: Render overlays for stereo
    if (this->renderMode.get() != StereoRenderMode::Stereo) {

        const Render::RenderTargetResource* offscreenRenderTargetResource = renderTargetResourceForRefIndex(offscreenMultiViewRenderTargetRefIndex());
        const RenderTarget* offscreenRt = offscreenRenderTargetResource->renderTarget();
        // Transition offscreen color texture to Shader readable layout
        // Note: we can't use OffscreenRenderTarget::transitionCurrentImageFromColorAttachmentToShaderReadOnly
        // as we target different stages (ColorAttachmentOutput vs BottomOfPipe)
        commandRecorder.textureMemoryBarrier(KDGpu::TextureMemoryBarrierOptions{
                .srcStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::ColorAttachmentOutputBit),
                .srcMask = KDGpu::AccessFlagBit::ColorAttachmentWriteBit,
                .dstStages = KDGpu::PipelineStageFlags(KDGpu::PipelineStageFlagBit::FragmentShaderBit),
                .dstMask = KDGpu::AccessFlagBit::ShaderReadBit,
                .oldLayout = KDGpu::TextureLayout::ColorAttachmentOptimal,
                .newLayout = KDGpu::TextureLayout::ShaderReadOnlyOptimal,
                .texture = offscreenRt->image(offscreenRt->currentImageIndex()),
                .range = {
                        .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                        .levelCount = 1,
                },
        });

        const Render::RenderTargetResource* presentRenderTargetResource = renderTargetResourceForRefIndex(presentRenderTargetRefIndex());
        const RenderTarget* presentRt = presentRenderTargetResource->renderTarget();

        // Prepare work for Overlays (e.g layout transition)
        prepareOverlaysForRecording(&commandRecorder, frameInFlightIndex);

        RenderPassCommandRecorderOptions renderPassOptions{
            .colorAttachments = {
                    { .view = {},
                      .resolveView = {},
                      .clearValue = { clearCol[0], clearCol[1], clearCol[2], clearCol[3] },
                      .finalLayout = TextureLayout::PresentSrc },
            },
            .depthStencilAttachment = {
                    .view = presentRenderTargetResource->depthTextureView(),
            },
            .samples = SampleCountFlagBits(presentRenderTargetResource->samples()),
        };

        const bool useMSAA = presentRenderTargetResource->samples() > RenderAlgorithm::SamplesCount::Samples_1;
        if (useMSAA) {
            renderPassOptions.colorAttachments[0].view = presentRenderTargetResource->msaaTextureView();
            renderPassOptions.colorAttachments[0].resolveView = presentRt->imageView(presentRt->currentImageIndex());
        } else {
            renderPassOptions.colorAttachments[0].view = presentRt->imageView(presentRt->currentImageIndex());
        }

        RenderPassCommandRecorder renderPass = commandRecorder.beginRenderPass(renderPassOptions);

        const KDGpu::Extent2D extent = presentRt->identityExtent();
        const float halfWidth = float(extent.width) / 2;
        const KDGpu::Handle<GraphicsPipeline_t> compositorPipeline = (renderMode() == StereoRenderMode::Anaglyph) ? m_compositor.fsqPipelineAnaglyph : m_compositor.fsqPipeline;

        // Bind Pipeline and texture bind group
        renderPass.setPipeline(compositorPipeline);
        for (const Render::BindGroupHandler* bindGroup : m_compositor.bindGroups())
            bindGroup->bind(m_compositor.fsqTextureBindGroupLayout, &renderPass, frameInFlightIndex);

        using namespace Render::BuildingBlocks;
        switch (renderMode()) {

        case StereoRenderMode::SideBySide: {
            renderPass.setViewport(Viewport{
                    .x = 0.0f,
                    .y = 0.0f,
                    .width = halfWidth,
                    .height = float(extent.height),
            });
            const int leftEyeLayer = 0;
            renderPass.pushConstant(m_compositor.fsqLayerIdxPushConstantRange, &leftEyeLayer);
            renderPass.draw(DrawCommand{ .vertexCount = 6 });

            renderPass.setViewport(Viewport{
                    .x = halfWidth,
                    .y = 0.0f,
                    .width = halfWidth,
                    .height = float(extent.height),
            });
            const int rightEyeLayer = 1;
            renderPass.pushConstant(m_compositor.fsqLayerIdxPushConstantRange, &rightEyeLayer);
            renderPass.draw(DrawCommand{ .vertexCount = 6 });
            break;
        }
        case StereoRenderMode::LeftOnly: {
            renderPass.setViewport(Viewport{
                    .x = 0.0f,
                    .y = 0.0f,
                    .width = float(extent.width),
                    .height = float(extent.height),
            });
            const int leftEyeLayer = 0;
            renderPass.pushConstant(m_compositor.fsqLayerIdxPushConstantRange, &leftEyeLayer);
            renderPass.draw(DrawCommand{ .vertexCount = 6 });
            break;
        }
        case StereoRenderMode::RightOnly: {
            renderPass.setViewport(Viewport{
                    .x = 0.0f,
                    .y = 0.0f,
                    .width = float(extent.width),
                    .height = float(extent.height),
            });
            const int rightEyeLayer = 1;
            renderPass.pushConstant(m_compositor.fsqLayerIdxPushConstantRange, &rightEyeLayer);
            renderPass.draw(DrawCommand{ .vertexCount = 6 });
            break;
        }
        case StereoRenderMode::Anaglyph: {
            renderPass.setViewport(Viewport{
                    .x = 0.0f,
                    .y = 0.0f,
                    .width = float(extent.width),
                    .height = float(extent.height),
            });
            renderPass.draw(DrawCommand{ .vertexCount = 6 });
            break;
        }
        default:
            break;
        }

        // Reset Viewport
        renderPass.setViewport(Viewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = float(extent.width),
                .height = float(extent.height),
        });

        // Overlay
        recordRenderCommandsForOverlays(&renderPass, frameInFlightIndex);

        // End render pass
        renderPass.end();

        finalizeOverlaysAfterRecording(&commandRecorder, frameInFlightIndex);
    }

    // End recording
    auto commandBuffer = commandRecorder.finish();

    // Submit command buffer to queue
    renderer()->graphicsQueue()->submit(SubmitOptions{
            .commandBuffers = { commandBuffer },
            .waitSemaphores = presentationCompleteSemaphores,
            .signalSemaphores = { m_renderFinishedSemaphores[frameInFlightIndex] },
            .signalFence = frameFence });

    // Store Command Buffer so that it remains alive
    m_commandBuffers[frameInFlightIndex] = std::move(commandBuffer);

    // Return semaphore handle that will be signaled when graphics queue operations are completed
    return m_renderFinishedSemaphores[frameInFlightIndex];
}
