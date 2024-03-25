#include "stereo_forward_renderer.h"
#include "stereo_proxy_camera.h"


#include <Qt3DRender/QLayer>
#include <Qt3DRender/QViewport>
#include <Qt3DRender/QNoDraw>
#include <Qt3DRender/QLayerFilter>
#include <Qt3DRender/QRenderTargetOutput>
#include <Qt3DRender/QRenderTarget>
#include <Qt3DRender/QRenderStateSet>
#include <Qt3DRender/QRenderTargetSelector>
#include <Qt3DRender/QClearBuffers>
#include <Qt3DRender/QCameraSelector>
#include <Qt3DRender/QSortPolicy>
#include <Qt3DRender/QCullFace>
#include <Qt3DRender/QDepthTest>
#include <Qt3DRender/QDebugOverlay>

all::qt3d::QStereoForwardRenderer::QStereoForwardRenderer(Qt3DCore::QNode* parent)
    : Qt3DRender::QRenderSurfaceSelector(parent)
    , m_camera(nullptr)
    , m_leftLayer(new Qt3DRender::QLayer(this))
    , m_rightLayer(new Qt3DRender::QLayer(this))
    , m_stereoImageLayer(new Qt3DRender::QLayer(this))
{
    auto vp = new Qt3DRender::QViewport(this);
    auto ssel = new Qt3DRender::QRenderSurfaceSelector(vp);

    // Frame graph sub branch for the 3D scene

    m_sceneNoDraw = new Qt3DRender::QNoDraw(ssel);
    m_sceneNoDraw->setObjectName("Scene");
    m_sceneNoDraw->setEnabled(false);

    auto* stereoImageLayerFilter = new Qt3DRender::QLayerFilter(m_sceneNoDraw);
    stereoImageLayerFilter->addLayer(m_stereoImageLayer);
    stereoImageLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::FilterMode::DiscardAnyMatchingLayers);

    auto makeRenderTargetSelector = [](Qt3DRender::QFrameGraphNode* parent, Qt3DRender::QRenderTargetOutput::AttachmentPoint attachment) {
        auto* output = new Qt3DRender::QRenderTargetOutput;
        output->setAttachmentPoint(attachment);

        auto* renderTarget = new Qt3DRender::QRenderTarget;
        renderTarget->addOutput(output);

        auto* selector = new Qt3DRender::QRenderTargetSelector(parent);
        selector->setTarget(renderTarget);

        return selector;
    };

    auto makeBranch = [makeRenderTargetSelector](Qt3DRender::QFrameGraphNode* parent, Qt3DRender::QRenderTargetOutput::AttachmentPoint attachment) {
        auto renderTargetSelector = makeRenderTargetSelector(parent, attachment);

        auto renderStateSet = new Qt3DRender::QRenderStateSet(renderTargetSelector);

        auto cullFace = new Qt3DRender::QCullFace(renderStateSet);
        cullFace->setMode(Qt3DRender::QCullFace::CullingMode::NoCulling);
        renderStateSet->addRenderState(cullFace);

        auto depthTest = new Qt3DRender::QDepthTest(renderStateSet);
        depthTest->setDepthFunction(Qt3DRender::QDepthTest::Less);
        renderStateSet->addRenderState(depthTest);

        auto clearBuffers = new Qt3DRender::QClearBuffers(renderStateSet);
        clearBuffers->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
        clearBuffers->setClearColor(QColor{ "#48536A" });
        //            cb->setClearColor(attachment == Qt3DRender::QRenderTargetOutput::AttachmentPoint::Left ? QColor(Qt::blue) : QColor(Qt::red));

        auto sortPolicy = new Qt3DRender::QSortPolicy{ clearBuffers };
        sortPolicy->setSortTypes(QList<Qt3DRender::QSortPolicy::SortType>{ Qt3DRender::QSortPolicy::BackToFront });

        return new Qt3DRender::QCameraSelector(sortPolicy);
    };

    m_leftCamera = makeBranch(stereoImageLayerFilter, Qt3DRender::QRenderTargetOutput::Left);
    m_rightCamera = makeBranch(stereoImageLayerFilter, Qt3DRender::QRenderTargetOutput::Right);

    // Frame graph sub branch for the stereo image

    m_stereoImageNoDraw = new Qt3DRender::QNoDraw(ssel);
    m_stereoImageNoDraw->setObjectName("Stereo Image");
    m_stereoImageNoDraw->setEnabled(true);

    auto* imageLeftRenderTarget = makeRenderTargetSelector(m_stereoImageNoDraw, Qt3DRender::QRenderTargetOutput::Left);
    auto* leftLayerFilter = new Qt3DRender::QLayerFilter(imageLeftRenderTarget);
    leftLayerFilter->addLayer(m_leftLayer);

    auto* imageRightRenderTarget = makeRenderTargetSelector(m_stereoImageNoDraw, Qt3DRender::QRenderTargetOutput::Right);
    auto* rightLayerFilter = new Qt3DRender::QLayerFilter(imageRightRenderTarget);
    rightLayerFilter->addLayer(m_rightLayer);

#ifdef QT_DEBUG
    (void)new Qt3DRender::QDebugOverlay(m_rightCamera);
#endif
}

void all::qt3d::QStereoForwardRenderer::setMode(Mode mode)
{
    if (mode == m_mode)
        return;
    switch (mode) {
    case Mode::Scene:
        m_sceneNoDraw->setEnabled(false);
        m_stereoImageNoDraw->setEnabled(true);
        break;
    case Mode::StereoImage:
        m_sceneNoDraw->setEnabled(true);
        m_stereoImageNoDraw->setEnabled(false);
        break;
    }
    m_mode = mode;
}

void all::qt3d::QStereoForwardRenderer::setCamera(QStereoProxyCamera* newCamera)
{
    if (m_camera == newCamera)
        return;
    m_camera = newCamera;
    Q_EMIT cameraChanged();

    if (m_camera) {
        m_leftCamera->setCamera(m_camera->GetLeftCamera());
        m_rightCamera->setCamera(m_camera->GetRightCamera());
    }
}
