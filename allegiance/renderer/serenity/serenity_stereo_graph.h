#pragma once
#include <Serenity/gui/forward_renderer/stereo_forward_algorithm.h>
#include <Serenity/gui/render/algorithm/building_blocks/capture_recorder.h>

namespace all::serenity {
class StereoRenderAlgorithm : public Serenity::StereoForwardAlgorithm
{
public:
    StereoRenderAlgorithm();

public:
    void Screenshot(const std::function<void(const uint8_t* data, uint32_t width, uint32_t height)>& in);

protected:
    KDGpu::Handle<KDGpu::GpuSemaphore_t> submitCommandBuffers(uint32_t frameInFlightIndex,
                                                              const std::vector<KDGpu::Handle<KDGpu::GpuSemaphore_t>>& presentationCompleteSemaphores,
                                                              KDGpu::Handle<KDGpu::Fence_t> frameFence) override;

private:
    void compositeAndOverlayNonStereo(KDGpu::CommandRecorder& commandRecorder, uint32_t frameInFlightIndex);
    void overlayForStereo(KDGpu::CommandRecorder& commandRecorder, uint32_t frameInFlightIndex);

    static constexpr uint32_t MAX_IMAGE_COUNT = 8;
    std::array<KDGpu::TextureView, MAX_IMAGE_COUNT> m_singleArrayResolvedOutputViews;
    KDGpu::TextureView m_singleArrayMSAAOutputView;
    KDGpu::TextureView m_singleArrayDepthView;

    KDGpu::Handle<KDGpu::Texture_t> m_lastDepthTextureH;
    KDGpu::Handle<KDGpu::Texture_t> m_lastMSAATextureH;
    std::array<KDGpu::Handle<KDGpu::Texture_t>, MAX_IMAGE_COUNT> m_lastSwapchainImageHandles;
    Serenity::Render::BuildingBlocks::Algorithm::CaptureRecorder m_captureRecorder{ this };
};

} // namespace all::serenity
