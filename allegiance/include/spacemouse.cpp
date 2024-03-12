

#include "spacemouse.h"

#include "util_qt.h"
#include "cursor.h"

#include <exception>

#ifdef WITH_NAVLIB
#include <SpaceMouse/CCategory.hpp>
#include <SpaceMouse/CImage.hpp>


#define __PRETTY_FUNCTION__ __FUNCSIG__
SpacemouseNavlib::SpacemouseNavlib(all::StereoCamera* camera)
    : Spacemouse(camera), m_model(*this)
{
    try {
        m_model.EnableNavigation(true);
    } catch (const std::system_error& e) {
        qDebug() << "Could not enable Spacemouse, are the drivers installed? :" << e.what();
    }
    auto * c = &all::Controller::getInstance();
    connect(c, &all::Controller::modelExtentChanged,
        [this](const all::extent_t e) {
            navlib::box_t b{e.min_x, e.min_y, e.min_z, e.max_x, e.max_y, e.max_z};
            m_model.Write("model.extents", b);
        });
    update();
}

void SpacemouseNavlib::setCameraMatrix(glm::mat4x4 m)
{
    setViewMatrix(m);
}

glm::mat4x4 SpacemouseNavlib::getCameraMatrix()
{
    return m_camera->GetCameraMatrix();
}

void SpacemouseNavlib::update()
{
    auto gm = m_camera->GetCameraMatrix();
    auto m = toNavlibMatrix(gm);
    m_model.GetCameraMatrix(m);
}
void SpacemouseNavlib::addActions(QVector<QAction*> actions, const QString& id, const QString& menuName)
{
    using TDx::CImage;
    using TDx::SpaceMouse::CCategory;
    using TDx::SpaceMouse::CCommand;
    using TDx::SpaceMouse::CCommandSet;

    CCategory menu(id.toStdString(), menuName.toStdString());
    std::vector<CImage> images;
    for (auto action : actions) {
        m_actions.push_back(action);

        auto icon = action->icon();
        if (!icon.isNull()) {
            auto p = action->icon().pixmap(30, 30);
            QImage image = p.toImage();
            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG"); // Change format as needed

            std::string data(byteArray.constData(), byteArray.length());

            images.push_back(CImage::FromData(data, 0, action->text().toStdString().c_str()));
        };

        menu.push_back(CCommand{ action->text().toStdString(), action->toolTip().toStdString(), action->whatsThis().toStdString() });
    }
    m_menuBar.push_back(std::move(menu));
    m_model.AddImages(images);

    qDebug() << m_menuBar.Id;
    m_model.AddCommandSet(m_menuBar);
    // m_model.ActiveCommands = m_menuBar.Id;
    m_model.PutActiveCommands(m_menuBar.Id);
}

void SpacemouseNavlib::fireCommand(QString cmd)
{
    if (cmd.isEmpty())
        return;
    auto a = std::find_if(m_actions.begin(), m_actions.end(), [cmd](auto e) { return e->text() == cmd; });
    if (a) {
        auto b = *a;
        b->trigger();
    }
}

long CNavigationModel::SetCameraMatrix(const navlib::matrix_t& matrix)
{
    auto m = toGlmMat(matrix);
    qCDebug(spcmsView) << "setting app camera matrix to " << m;
    lib.setCameraMatrix(m);
    return 0;
}

long CNavigationModel::GetCameraMatrix(navlib::matrix_t& matrix) const
{
    auto m = (lib.getCameraMatrix());
    qCDebug(spcmsView) << "setting SpcMouse Camera Matrix to " << m;
    matrix = toNavlibMatrix(m);
    return 0;
}

long CNavigationModel::GetUnitsToMeters(double& meters) const
{
    meters = 0.5;
    return 0;
}
long CNavigationModel::GetFloorPlane(navlib::plane_t& floor) const
{
    floor.d = 0;
    floor.n = {0, -1, 0};

    return 0;
}
long CNavigationModel::GetFrontView(navlib::matrix_t& matrix) const
{
    matrix = {
        -1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, -1, 0,
        0, 0, 0, 1
    };
    return 0;
}
CNavigationModel::CNavigationModel(SpacemouseNavlib& lib, bool multiThreaded, bool rowMajor)
    : CNavigation3D(multiThreaded, rowMajor), lib(lib)
{
    m_camera = lib.getCamera();

    nav3d::Profile = "Allegiance";
    // Enable input from / output to the Navigation3D controller.
    nav3d::Enable = false;
    nav3d::FrameTiming = TimingSource::SpaceMouse;
}
long CNavigationModel::GetPointerPosition(navlib::point_t& position) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    //return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}
long CNavigationModel::SetPointerPosition(const navlib::point_t& position)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__ << position.x << position.y << position.z;
    return 0;
}


long CNavigationModel::GetViewExtents(navlib::box_t& extents) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    extents.max = { 10, 10, 1000 };
    extents.min = { -10, -10, 0.01 };
    return 0;
}

long CNavigationModel::GetViewFOV(double& fov) const
{
    auto m_camera = lib.getCamera();
    fov = glm::radians(lib.getCamera()->GetFov());
    qCDebug(spcms) << "GetViewFoV " << fov;
    return 0;
}

long CNavigationModel::GetViewFrustum(navlib::frustum_t& frustum) const
{
    auto m_camera = lib.getCamera();
    float verticalFieldOfView = m_camera->GetFov(), // in degrees,
            aspectRatio = m_camera->GetAspectRatio(),
          nearPlane = m_camera->GetNearPlane(), farPlane = m_camera->GetFarPlane();

    float halfVerticalFOV = qDegreesToRadians(verticalFieldOfView) / 2.0f;
    float halfHorizontalFOV = atanf(tanf(halfVerticalFOV) * aspectRatio);

    // Calculate frustum parameters
    frustum.top = nearPlane * tanf(halfVerticalFOV);
    frustum.bottom = -frustum.top;
    frustum.left = -nearPlane * tanf(halfHorizontalFOV);
    frustum.right = -frustum.left;
    frustum.nearVal = nearPlane;
    frustum.farVal = farPlane;

    qCDebug(spcmsView) << "vertical fov " << verticalFieldOfView << " near/far: " << nearPlane << "/" << farPlane
             << " left/right " << frustum.left << frustum.right
             << " top/bottom " << frustum.top << frustum.bottom;

    return 0;
}

long CNavigationModel::GetIsViewPerspective(navlib::bool_t& perspective) const
{
    perspective = true;
    qCDebug(spcms) << "get is perspective " << perspective;
    return 0;
}

long CNavigationModel::SetViewExtents(const navlib::box_t& extents)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetViewFOV(double fov)
{
    qCDebug(spcms) << "SetFOV";
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetViewFrustum(const navlib::frustum_t& frustum)
{
    qCDebug(spcms) << "Set View Frustum";
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::GetModelExtents(navlib::box_t& extents) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;

    auto e = all::Controller::getInstance().modelExtent;
    extents = {e.min_x, e.min_y, e.min_z, e.max_x, e.max_y, e.max_z};
    return 0;
}

long CNavigationModel::GetSelectionExtents(navlib::box_t& extents) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::GetSelectionTransform(navlib::matrix_t& transform) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::GetIsSelectionEmpty(navlib::bool_t& empty) const
{
    empty = true;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetSelectionTransform(const navlib::matrix_t& matrix)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

// Pivot
long CNavigationModel::IsUserPivot(navlib::bool_t& userPivot) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    userPivot = true;
    return 0;
}

long CNavigationModel::SetPivotPosition(const navlib::point_t& position)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__ << position;
    return 0;
}

long CNavigationModel::GetPivotVisible(navlib::bool_t& visible) const
{
    visible = true;
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return 0;
}

long CNavigationModel::SetPivotVisible(bool visible)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__ << visible;
    return 0;
}

long CNavigationModel::GetPivotPosition(navlib::point_t& position) const
{
    const auto &p = lib.m_pivotPoint;
    position = navlib::point_t{p.x, p.y, p.z};
    qCDebug(spcms) << __PRETTY_FUNCTION__ << position;
    return 0;
}

// Hit
long CNavigationModel::SetHitAperture(double aperture)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    auto &c = all::Controller::getInstance();
    c.hitAperture = aperture;
    return 0;
}

long CNavigationModel::SetHitDirection(const navlib::vector_t& direction)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    auto &c = all::Controller::getInstance();
    c.hitDirection = toGlmVec3(direction);
    return 0;
}

long CNavigationModel::SetHitLookFrom(const navlib::point_t& eye)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__ << eye.x << eye.y << eye.z;
    auto &c = all::Controller::getInstance();
    c.hitFrom = toGlmVec3(eye);
    return 0;
}

long CNavigationModel::SetHitSelectionOnly(bool onlySelection)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__ << onlySelection;
    return 0;
}

long CNavigationModel::GetHitLookAt(navlib::point_t& position) const
{
    auto &a= all::Controller::getInstance();
    auto res = a.hitTest(a.hitFrom, a.hitDirection, a.hitAperture);
    qDebug() << "resulting hit " << res << " - " << (res - a.hitFrom);
    if (res == glm::vec3{-1}) return 0;

    position.x = res.x; position.y = res.y; position.z = res.z;

    qCDebug(spcms) << __PRETTY_FUNCTION__<< position.x << position.y << position.z;
    return 0;
}



long CNavigationModel::SetActiveCommand(std::string commandId)
{
    qCDebug(spcms) << commandId << __PRETTY_FUNCTION__;
    lib.fireCommand(QLatin1String{ commandId });
    return 0;
}


#endif // WITH_NAVLIB

/* for megalan
#include <X11/Xlib.h>
Display *dpy;
Window win;
*/
#ifdef WITH_SPNAV

#include <spnav.h>

SpacemouseSpnav::SpacemouseSpnav(all::StereoCamera* camera)
    : Spacemouse(camera)
{
    if (spnav_open() == -1) {
        qCDebug(spcms) << "could not connect to spacenavd";
    } else {
        m_poller = std::thread{ [this]() {
            while (!stop) {
                qCDebug(spcms) << "polling";
                poll();
            }
            qCDebug(spcms) << "STOP!";
        } };
        m_poller.detach();
    }
}

SpacemouseSpnav::~SpacemouseSpnav()
{
    stop = true;
    if (m_poller.joinable())
        m_poller.join();
    spnav_close();
}

void SpacemouseSpnav::poll()
{
    qCDebug(spcms) << "poll";
    spnav_event sev;
    if (spnav_wait_event(&sev)) {
        const glm::mat4x4 viewCenter = m_camera->GetCameraMatrix();

        const glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f), sev.motion.rx * m_rotFactor, glm::vec3(1.0f, 0.0f, 0.0f));
        const glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f), sev.motion.ry * m_rotFactor, glm::vec3(0.0f, 1.0f, 0.0f));
        const glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f), sev.motion.rz * m_rotFactor, glm::vec3(0.0f, 0.0f, 1.0f));

        // Combine the rotation matrices
        const glm::mat4 combinedRotation = rotateX * rotateY * rotateZ;

        // Apply the combined rotation to the original view matrix
        glm::mat4 rotatedViewMatrix = combinedRotation * viewCenter;

        qCDebug(spcms) << "spcms event" << viewCenter;
        switch (sev.type) {
        case SPNAV_EVENT_MOTION:
            qCDebug(spcms) << sev.motion.rx << sev.motion.ry << sev.motion.rz;
            rotatedViewMatrix[3] = {m_camera->GetPosition() + glm::vec3{ sev.motion.x, sev.motion.y, sev.motion.z } * m_translFactor, 1};
            m_camera->SetCameraMatrix(rotatedViewMatrix);

            break;
        case SPNAV_EVENT_BUTTON:
            /* 0-based button number in sev.button.bnum.
             * button state in sev.button.press (non-zero means pressed).
             */
            break;
        default:;
        }
    }
}
#endif
