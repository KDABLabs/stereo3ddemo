#pragma once
#include <Serenity/gui/forward_renderer/stereo_forward_algorithm.h>

namespace all {
class StereoRenderAlgorithm : public Serenity::StereoForwardAlgorithm
{
public:
    StereoRenderAlgorithm() = default;

protected:
    KDGpu::Handle<KDGpu::GpuSemaphore_t> submitCommandBuffers(uint32_t frameInFlightIndex,
                                                              const std::vector<KDGpu::Handle<KDGpu::GpuSemaphore_t>>& presentationCompleteSemaphores,
                                                              KDGpu::Handle<KDGpu::Fence_t> frameFence) override;
};
} // namespace all