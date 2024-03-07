#include <serenity_stereo_graph.h>
#include <Serenity/gui/render/renderer.h>
using namespace Serenity;

using namespace KDGpu;

KDGpu::Handle<KDGpu::GpuSemaphore_t> all::StereoRenderAlgorithm::submitCommandBuffers(uint32_t frameInFlightIndex, const std::vector<KDGpu::Handle<KDGpu::GpuSemaphore_t>>& presentationCompleteSemaphores, KDGpu::Handle<KDGpu::Fence_t> frameFence)
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
