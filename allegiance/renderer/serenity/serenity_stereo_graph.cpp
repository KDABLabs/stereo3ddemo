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
    const bool isStereo = this->renderMode.get() == StereoRenderMode::Stereo;

    // 1) Render Scene Content using multiview
    {
        const Render::RenderTargetResource* renderTargetResource = renderTargetResourceForRefIndex(offscreenMultiViewRenderTargetRefIndex());
        const RenderTarget* rt = renderTargetResource->renderTarget();

        RenderPassCommandRecorderOptions renderPassOptions{
            .colorAttachments = {
                    {
                            .view = {},
                            .resolveView = {},
                            .clearValue = { clearCol[0], clearCol[1], clearCol[2], clearCol[3] },
                            .finalLayout = TextureLayout::ColorAttachmentOptimal,
                    },
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
    if (!isStereo) {
        compositeAndOverlayNonStereo(commandRecorder, frameInFlightIndex);
    } else {
        overlayForStereo(commandRecorder, frameInFlightIndex);
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

void all::serenity::StereoRenderAlgorithm::compositeAndOverlayNonStereo(KDGpu::CommandRecorder& commandRecorder, uint32_t frameInFlightIndex)
{
    const glm::vec4 clearCol = clearColor();
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

void all::serenity::StereoRenderAlgorithm::overlayForStereo(KDGpu::CommandRecorder& commandRecorder, uint32_t frameInFlightIndex)
{
    const Render::RenderTargetResource* presentRenderTargetResource = renderTargetResourceForRefIndex(presentRenderTargetRefIndex());
    const RenderTarget* presentRt = presentRenderTargetResource->renderTarget();
    const glm::vec4 clearCol = clearColor();
    const KDGpu::Extent2D extent = presentRt->identityExtent();

    // Prepare work for Overlays (e.g layout transition)
    prepareOverlaysForRecording(&commandRecorder, frameInFlightIndex);

    const bool useMSAA = presentRenderTargetResource->samples() > RenderAlgorithm::SamplesCount::Samples_1;
    const uint32_t currentImageIndex = presentRt->currentImageIndex();

    // Create TextureViews for Swapchain/RenderTargetResource that only target a single array layer
    {
        const bool rebuildRTTextureViews = m_lastDepthTextureH != presentRenderTargetResource->depthTexture().handle() ||
                m_lastMSAATextureH != presentRenderTargetResource->msaaTexture().handle();

        // Note: we only recreate the views when we know that they have changed due to swapchain being recreated
        if (rebuildRTTextureViews) {
            const KDGpu::Texture& depthTexture = presentRenderTargetResource->depthTexture();
            m_singleArrayDepthView = depthTexture.createView(KDGpu::TextureViewOptions{
                    .label = "DepthView",
                    .viewType = KDGpu::ViewType::ViewType2D,
                    .format = presentRenderTargetResource->depthFormat(),
                    .range = {
                            .aspectMask = KDGpu::TextureAspectFlagBits::DepthBit,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                    },
            });
            m_lastDepthTextureH = depthTexture.handle();

            if (useMSAA) {
                const KDGpu::Texture& msaaTexture = presentRenderTargetResource->msaaTexture();
                m_singleArrayMSAAOutputView = msaaTexture.createView(KDGpu::TextureViewOptions{
                        .label = "MSAAView",
                        .viewType = KDGpu::ViewType::ViewType2D,
                        .format = presentRt->imageFormat(),
                        .range = {
                                .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                                .baseArrayLayer = 0,
                                .layerCount = 1,
                        },
                });
                m_lastMSAATextureH = msaaTexture;
            }
        }

        const KDGpu::Texture& swapchainTexture = presentRt->image(currentImageIndex);
        if (m_lastSwapchainImageHandles[currentImageIndex] != swapchainTexture.handle()) {
            m_singleArrayResolvedOutputViews[currentImageIndex] = swapchainTexture.createView(KDGpu::TextureViewOptions{
                    .label = "ResolveView",
                    .viewType = KDGpu::ViewType::ViewType2D,
                    .format = presentRt->imageFormat(),
                    .range = {
                            .aspectMask = KDGpu::TextureAspectFlagBits::ColorBit,
                            .baseArrayLayer = 0,
                            .layerCount = 1,
                    },
            });
            m_lastSwapchainImageHandles[currentImageIndex] = swapchainTexture.handle();
        }
    }

    RenderPassCommandRecorderOptions renderPassOptions{
        .colorAttachments = {
                { .view = {},
                  .resolveView = {},
                  .loadOperation = KDGpu::AttachmentLoadOperation::Load,
                  .clearValue = { clearCol[0], clearCol[1], clearCol[2], clearCol[3] },
                  .initialLayout = TextureLayout::ColorAttachmentOptimal,
                  .finalLayout = TextureLayout::PresentSrc },
        },
        .depthStencilAttachment = {
                .view = m_singleArrayDepthView.handle(),
                .depthLoadOperation{ KDGpu::AttachmentLoadOperation::Load },
                .initialLayout = TextureLayout::DepthStencilAttachmentOptimal,
        },
        .samples = SampleCountFlagBits(presentRenderTargetResource->samples()),
        .framebufferArrayLayers = 1,
    };

    if (useMSAA) {
        renderPassOptions.colorAttachments[0].view = m_singleArrayMSAAOutputView.handle();
        renderPassOptions.colorAttachments[0].resolveView = m_singleArrayResolvedOutputViews[currentImageIndex].handle();
    } else {
        renderPassOptions.colorAttachments[0].view = m_singleArrayResolvedOutputViews[currentImageIndex].handle();
    }

    RenderPassCommandRecorder renderPass = commandRecorder.beginRenderPass(renderPassOptions);

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
