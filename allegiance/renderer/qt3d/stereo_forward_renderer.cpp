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
#include <Qt3DRender/QNoPicking>
#include <Qt3DRender/QRasterMode>
#include <QSurfaceFormat>

all::qt3d::QStereoForwardRenderer::QStereoForwardRenderer(Qt3DCore::QNode* parent)
    : Qt3DRender::QRenderSurfaceSelector(parent)
    , m_camera(nullptr)
    , m_leftLayer(new Qt3DRender::QLayer(this))
    , m_rightLayer(new Qt3DRender::QLayer(this))
    , m_stereoImageLayer(new Qt3DRender::QLayer(this))
    , m_sceneLayer(new Qt3DRender::QLayer(this))
    , m_cursorLayer(new Qt3DRender::QLayer(this))
    , m_frustumLayer(new Qt3DRender::QLayer(this))
    , m_focusAreaLayer(new Qt3DRender::QLayer(this))
    , m_focusPlaneLayer(new Qt3DRender::QLayer(this))
{
    m_sceneLayer->setObjectName(QStringLiteral("SceneLayer"));
    m_sceneLayer->setRecursive(true);
    m_cursorLayer->setObjectName(QStringLiteral("CursorLayer"));
    m_cursorLayer->setRecursive(true);
    m_frustumLayer->setObjectName(QStringLiteral("FrustumLayer"));
    m_frustumLayer->setRecursive(true);
    m_focusAreaLayer->setObjectName(QStringLiteral("FocusAreaLayer"));

    const QSurfaceFormat f = QSurfaceFormat::defaultFormat();
    const bool supportsStereo = f.stereo();

    auto vp = new Qt3DRender::QViewport();
    auto noPicking = new Qt3DRender::QNoPicking();

    // Frame graph sub branch for the 3D scene
    auto renderStateSet = new Qt3DRender::QRenderStateSet();

    auto cullFace = new Qt3DRender::QCullFace(renderStateSet);
    cullFace->setMode(Qt3DRender::QCullFace::CullingMode::NoCulling);
    renderStateSet->addRenderState(cullFace);

    auto depthTest = new Qt3DRender::QDepthTest(renderStateSet);
    depthTest->setDepthFunction(Qt3DRender::QDepthTest::Less);
    renderStateSet->addRenderState(depthTest);

    // CenterLayerFilter is used to perform scene picking (and we want to discard everything but the sceneLayer)
    m_centerLayerFilter = new Qt3DRender::QLayerFilter();
    m_centerLayerFilter->setObjectName("CenterLayerFilter");
    m_centerLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::DiscardAnyMatchingLayers);
    m_centerLayerFilter->addLayer(m_stereoImageLayer);
    m_centerLayerFilter->addLayer(m_leftLayer);
    m_centerLayerFilter->addLayer(m_rightLayer);
    m_centerLayerFilter->addLayer(m_frustumLayer);
    m_centerLayerFilter->addLayer(m_cursorLayer);
    m_centerLayerFilter->addLayer(m_focusAreaLayer);

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

    auto makeCenterCameraPickingBranch = [&]() {
        auto* cameraSelector = new Qt3DRender::QCameraSelector();
        auto* noDraw = new Qt3DRender::QNoDraw();
        noDraw->setParent(cameraSelector);

        return cameraSelector;
    };

    auto makeRenderTarget = [&](Qt3DRender::QRenderTargetOutput::AttachmentPoint attachment) {
        auto* output = new Qt3DRender::QRenderTargetOutput;
        output->setAttachmentPoint(attachment);
        auto* renderTarget = new Qt3DRender::QRenderTarget;
        renderTarget->addOutput(output);
        renderTarget->setParent(this);
        return renderTarget;
    };

    auto makeCameraSelectorForSceneBranch = [&](Qt3DRender::QRenderTarget* rt, Qt3DRender::QRasterMode* rasterState, bool shouldClear) {
        auto* cameraSelector = new Qt3DRender::QCameraSelector();
        auto* rts = new Qt3DRender::QRenderTargetSelector();
        rts->setTarget(rt);
        rts->setParent(cameraSelector);

        if (shouldClear) {
            auto* clearBuffers = new Qt3DRender::QClearBuffers();
            clearBuffers->setBuffers(Qt3DRender::QClearBuffers::ColorDepthBuffer);
            clearBuffers->setClearColor(QColor{ "#48536A" });

            auto* noDraw = new Qt3DRender::QNoDraw();
            noDraw->setParent(clearBuffers);

            clearBuffers->setParent(rts);
        }

        auto* sceneLayerFilter = new Qt3DRender::QLayerFilter();
        sceneLayerFilter->setObjectName("SceneLayerFilter");
        sceneLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers);
        sceneLayerFilter->addLayer(m_sceneLayer);

        auto* sceneRenderState = new Qt3DRender::QRenderStateSet();
        {
            auto* depthState = new Qt3DRender::QDepthTest;
            depthState->setDepthFunction(Qt3DRender::QDepthTest::Less);

            sceneRenderState->addRenderState(depthState);
            sceneRenderState->addRenderState(rasterState);
        }

        auto* focusPlaneLayerFilter = new Qt3DRender::QLayerFilter();
        focusPlaneLayerFilter->setObjectName("FocusPlaneFilter");
        focusPlaneLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers);
        focusPlaneLayerFilter->addLayer(m_focusPlaneLayer);

        auto* cursorLayerFilter = new Qt3DRender::QLayerFilter();
        cursorLayerFilter->setObjectName("CursorLayerFilter");
        cursorLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers);
        cursorLayerFilter->addLayer(m_cursorLayer);

        auto* focusAreaLayerFilter = new Qt3DRender::QLayerFilter();
        {
            auto* focusAreaStateSet = new Qt3DRender::QRenderStateSet;
            auto* focusAreaNoDepthTest = new Qt3DRender::QDepthTest;
            focusAreaNoDepthTest->setDepthFunction(Qt3DRender::QDepthTest::Always);
            focusAreaStateSet->addRenderState(focusAreaNoDepthTest);
            focusAreaStateSet->setParent(focusAreaLayerFilter);
        }
        focusAreaLayerFilter->setObjectName("FocusAreaLayerFilter");
        focusAreaLayerFilter->setFilterMode(Qt3DRender::QLayerFilter::AcceptAnyMatchingLayers);
        focusAreaLayerFilter->addLayer(m_focusAreaLayer);

        sceneLayerFilter->setParent(rts);
        sceneRenderState->setParent(sceneLayerFilter);
        cursorLayerFilter->setParent(rts);
        focusPlaneLayerFilter->setParent(rts);
        focusAreaLayerFilter->setParent(rts);

        return cameraSelector;
    };

    Qt3DRender::QRenderTarget* leftRt = makeRenderTarget(Qt3DRender::QRenderTargetOutput::Left);
    Qt3DRender::QRenderTarget* rightRt = makeRenderTarget(supportsStereo ? Qt3DRender::QRenderTargetOutput::Right : Qt3DRender::QRenderTargetOutput::Left);

    m_leftSceneRasterMode = new Qt3DRender::QRasterMode;
    m_rightSceneRasterMode = new Qt3DRender::QRasterMode;

    auto* leftViewport = new Qt3DRender::QViewport();
    auto* rightViewport = new Qt3DRender::QViewport();

    if (!supportsStereo) {
        leftViewport->setNormalizedRect(QRectF(0.0f, 0.25f, 0.5f, 0.5f));
        rightViewport->setNormalizedRect(QRectF(0.5f, 0.25f, 0.5f, 0.5f));
    }

    m_centerCameraSelector = makeCenterCameraPickingBranch();
    m_centerCameraSelector->setObjectName("CenterCamera");
    m_leftCameraSelector = makeCameraSelectorForSceneBranch(leftRt, m_leftSceneRasterMode, true);
    m_leftCameraSelector->setObjectName("LeftCamera");
    m_rightCameraSelector = makeCameraSelectorForSceneBranch(rightRt, m_rightSceneRasterMode, supportsStereo);
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

    // Frustum
    {
        m_frustumCamera = new Qt3DRender::QCamera(this);
        m_frustumCamera->setProjectionType(Qt3DRender::QCameraLens::OrthographicProjection);

        m_leftFrustumCameraSelector = makeFrustumBranch(leftRt);
        m_leftFrustumCameraSelector->setCamera(m_frustumCamera);

        m_rightFrustumCameraSelector = makeFrustumBranch(rightRt);
        m_rightFrustumCameraSelector->setCamera(m_frustumCamera);
    }

    // Hierarchy
    vp->setParent(this);

    // Scene Picking Branch (no render)
    m_centerLayerFilter->setParent(vp);
    m_centerCameraSelector->setParent(m_centerLayerFilter);

    // Scene Render Branch (no picking)
    noPicking->setParent(vp);
    sortPolicy->setParent(noPicking);
    renderStateSet->setParent(sortPolicy);
    // Left Eye
    m_leftLayerFilter->setParent(renderStateSet);
    m_leftCameraSelector->setParent(leftViewport);
    leftViewport->setParent(m_leftLayerFilter);
    // Right Eye
    m_rightLayerFilter->setParent(renderStateSet);
    m_rightCameraSelector->setParent(rightViewport);
    rightViewport->setParent(m_rightLayerFilter);

    // Frustum Overlay
    m_leftFrustumCameraSelector->setParent(noPicking);
    m_rightFrustumCameraSelector->setParent(noPicking);

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

void all::qt3d::QStereoForwardRenderer::setDisplayMode(DisplayMode displayMode)
{
    m_displayMode = displayMode;

    if (m_camera == nullptr)
        return;

    m_centerCameraSelector->setCamera(m_camera->centerCamera());

    switch (m_displayMode) {
    case all::DisplayMode::Stereo:
        m_leftCameraSelector->setCamera(m_camera->leftCamera());
        m_rightCameraSelector->setCamera(m_camera->rightCamera());
        break;
    case all::DisplayMode::Mono:
        m_leftCameraSelector->setCamera(m_camera->centerCamera());
        m_rightCameraSelector->setCamera(m_camera->centerCamera());
        break;
    case all::DisplayMode::Left:
        m_leftCameraSelector->setCamera(m_camera->leftCamera());
        m_rightCameraSelector->setCamera(m_camera->leftCamera());
        break;
    case all::DisplayMode::Right:
        m_leftCameraSelector->setCamera(m_camera->rightCamera());
        m_rightCameraSelector->setCamera(m_camera->rightCamera());
        break;
    }
}

void all::qt3d::QStereoForwardRenderer::setCamera(QStereoProxyCamera* newCamera)
{
    if (m_camera == newCamera)
        return;
    m_camera = newCamera;
    Q_EMIT cameraChanged();

    if (m_camera) {
        setDisplayMode(m_displayMode);
    }
}

void all::qt3d::QStereoForwardRenderer::setWireframeEnabled(bool enabled)
{
    m_leftSceneRasterMode->setRasterMode(enabled ? Qt3DRender::QRasterMode::Lines : Qt3DRender::QRasterMode::Fill);
    m_rightSceneRasterMode->setRasterMode(m_leftSceneRasterMode->rasterMode());
}

