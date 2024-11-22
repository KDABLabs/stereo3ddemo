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
#include <Qt3DRender/QCamera>

all::qt3d::QStereoForwardRenderer::QStereoForwardRenderer(Qt3DCore::QNode* parent)
    : Qt3DRender::QRenderSurfaceSelector(parent)
    , m_camera(nullptr)
    , m_leftLayer(new Qt3DRender::QLayer(this))
    , m_rightLayer(new Qt3DRender::QLayer(this))
    , m_stereoImageLayer(new Qt3DRender::QLayer(this))
    , m_sceneLayer(new Qt3DRender::QLayer(this))
    , m_cursorLayer(new Qt3DRender::QLayer(this))
    , m_frustumLayer(new Qt3DRender::QLayer(this))
{
    m_sceneLayer->setObjectName(QStringLiteral("SceneLayer"));
    m_sceneLayer->setRecursive(true);
    m_cursorLayer->setObjectName(QStringLiteral("CursorLayer"));
    m_cursorLayer->setRecursive(true);
    m_frustumLayer->setObjectName(QStringLiteral("FrustumLayer"));

    auto vp = new Qt3DRender::QViewport();

    // Frame graph sub branch for the 3D scene
    auto renderStateSet = new Qt3DRender::QRenderStateSet();

    auto cullFace = new Qt3DRender::QCullFace(renderStateSet);
    cullFace->setMode(Qt3DRender::QCullFace::CullingMode::NoCulling);
    renderStateSet->addRenderState(cullFace);

    auto depthTest = new Qt3DRender::QDepthTest(renderStateSet);
    depthTest->setDepthFunction(Qt3DRender::QDepthTest::Less);
    renderStateSet->addRenderState(depthTest);

    m_leftLayerFilter = new Qt3DRender::QLayerFilter();
    m_leftLayerFilter->setObjectName("LeftLayerFilter");
    m_leftLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
    m_leftLayerFilter->addLayer(m_stereoImageLayer);
    m_leftLayerFilter->addLayer(m_leftLayer);

    m_rightLayerFilter = new Qt3DRender::QLayerFilter();
    m_rightLayerFilter->setObjectName("RightLayerFilter");
    m_rightLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
    m_rightLayerFilter->addLayer(m_stereoImageLayer);
    m_rightLayerFilter->addLayer(m_rightLayer);

    auto* sortPolicy = new Qt3DRender::QSortPolicy();
    sortPolicy->setSortTypes(QList<Qt3DRender::QSortPolicy::SortType>{ Qt3DRender::QSortPolicy::BackToFront });

    auto makeRenderTarget = [&](Qt3DRender::QRenderTargetOutput::AttachmentPoint attachment) {
        auto* output = new Qt3DRender::QRenderTargetOutput;
        output->setAttachmentPoint(attachment);
        auto* renderTarget = new Qt3DRender::QRenderTarget;
        renderTarget->addOutput(output);
        renderTarget->setParent(this);
        return renderTarget;
    };

    auto makeCameraSelectorForSceneBranch = [&](Qt3DRender::QRenderTarget* rt) {
        auto* cameraSelector = new Qt3DRender::QCameraSelector();
        auto* rts = new Qt3DRender::QRenderTargetSelector();
        rts->setTarget(rt);
        rts->setParent(cameraSelector);

        auto* clearBuffers = new Qt3DRender::QClearBuffers();
        clearBuffers->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
        clearBuffers->setClearColor(QColor{ "#48536A" });

        auto* noDraw = new Qt3DRender::QNoDraw();
        noDraw->setParent(clearBuffers);

        auto* sceneLayerFilter = new Qt3DRender::QLayerFilter();
        sceneLayerFilter->setObjectName("SceneLayerFilter");
        sceneLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers);
        sceneLayerFilter->addLayer(m_sceneLayer);

        auto* cursorLayerFilter = new Qt3DRender::QLayerFilter();
        cursorLayerFilter->setObjectName("CursorLayerFilter");
        cursorLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers);
        cursorLayerFilter->addLayer(m_cursorLayer);

        clearBuffers->setParent(rts);
        sceneLayerFilter->setParent(rts);
        cursorLayerFilter->setParent(rts);

        return cameraSelector;
    };

    Qt3DRender::QRenderTarget* leftRt = makeRenderTarget(Qt3DRender::QRenderTargetOutput::Left);
    Qt3DRender::QRenderTarget* rightRt = makeRenderTarget(Qt3DRender::QRenderTargetOutput::Right);

    m_leftCameraSelector = makeCameraSelectorForSceneBranch(leftRt);
    m_leftCameraSelector->setObjectName("LeftCamera");
    m_rightCameraSelector = makeCameraSelectorForSceneBranch(rightRt);
    m_rightCameraSelector->setObjectName("RightCamera");

    auto makeFrustumBranch = [&](Qt3DRender::QRenderTarget* rt) {
        auto* cameraSelector = new Qt3DRender::QCameraSelector();

        auto* vp = new Qt3DRender::QViewport();
        vp->setNormalizedRect(QRectF(0.0f, 0.6f, 0.4f, 0.4f));

        auto* rts = new Qt3DRender::QRenderTargetSelector();
        rts->setTarget(rt);

        auto* clearBuffers = new Qt3DRender::QClearBuffers();
        clearBuffers->setBuffers(Qt3DRender::QClearBuffers::DepthBuffer);

        auto* noDraw = new Qt3DRender::QNoDraw();
        noDraw->setParent(clearBuffers);

        auto* frustumLayerFilter = new Qt3DRender::QLayerFilter();
        frustumLayerFilter->setObjectName("FrustumLayerFilter");
        frustumLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAllMatchingLayers);
        frustumLayerFilter->addLayer(m_frustumLayer);

        frustumLayerFilter->setParent(rts);
        clearBuffers->setParent(rts);
        rts->setParent(vp);
        vp->setParent(cameraSelector);

        return cameraSelector;
    };

    m_frustumCamera = new Qt3DRender::QCamera;
    m_frustumCamera->setProjectionType(Qt3DRender::QCameraLens::OrthographicProjection);

    m_frustumCameraSelector = makeFrustumBranch(leftRt);
    m_frustumCameraSelector->setCamera(m_frustumCamera);

    // Hierarchy
    vp->setParent(this);

    // Scene Render Branch
    sortPolicy->setParent(vp);
    renderStateSet->setParent(sortPolicy);
    // Left Eye
    m_leftLayerFilter->setParent(renderStateSet);
    m_leftCameraSelector->setParent(m_leftLayerFilter);
    // Right Eye
    m_rightLayerFilter->setParent(renderStateSet);
    m_rightCameraSelector->setParent(m_rightLayerFilter);

    // Frustum Overlay on Left Target
    m_frustumCameraSelector->setParent(vp);

#ifdef QT_DEBUG
    auto* debugOverlay = new Qt3DRender::QDebugOverlay();
    auto* noDraw = new Qt3DRender::QNoDraw();
    noDraw->setParent(debugOverlay);
    debugOverlay->setParent(m_rightCameraSelector);

// To dump out FrameGraph tree to console:
// CMakeLists.txt:    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::3DExtras Qt6::3DRenderPrivate)
// Add    #include <Qt3DRender/private/qframegraphnode_p.h>
//        qDebug() << qPrintable(Qt3DRender::QFrameGraphNodePrivate::get(this)->dumpFrameGraph());
#endif
}

void all::qt3d::QStereoForwardRenderer::setMode(Mode mode)
{
    if (mode == m_mode)
        return;
    switch (mode) {
    case Mode::Scene:
        m_leftLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
        m_rightLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
        break;
    case Mode::StereoImage:
        m_leftLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAllMatchingLayers);
        m_rightLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAllMatchingLayers);
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
        m_leftCameraSelector->setCamera(m_camera->leftCamera());
        m_rightCameraSelector->setCamera(m_camera->rightCamera());
    }
}
