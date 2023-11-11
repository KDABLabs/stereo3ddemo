

#include "spacemouse.h"
#include <exception>

#ifdef WITH_NAVLIB
#include <SpaceMouse/CCategory.hpp>
#include <SpaceMouse/CImage.hpp>


#define __PRETTY_FUNCTION__ __FUNCSIG__
SpacemouseNavlib::SpacemouseNavlib(all::StereoCamera* camera)
    : Spacemouse(camera), m_model(*this)
{
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

long CNavigationModel::SetCameraMatrix(const navlib::matrix_t& matrix)
{
    auto m = toGlmMat(matrix);
    qCDebug(spcms) << "setting app camera matrix to " << m;
    lib.setCameraMatrix(m);
    return 0;
}

long CNavigationModel::GetCameraMatrix(navlib::matrix_t& matrix) const
{
    auto m = (lib.getCameraMatrix());
    qCDebug(spcms) << "setting SpcMouse Camera Matrix to " << m;
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
    nav3d::Enable = true;
    nav3d::FrameTiming = TimingSource::SpaceMouse;
}
long CNavigationModel::GetPointerPosition(navlib::point_t& position) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::GetViewExtents(navlib::box_t& extents) const
{
    // return navlib::make_result_code(navlib::navlib_errc::no_data_available);
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
    qCDebug(spcms) << frustum.left;
    frustum.right = -frustum.left;
    frustum.nearVal = nearPlane;
    frustum.farVal = farPlane;

    qCDebug(spcms) << "vertical fov " << verticalFieldOfView << " near/far: " << nearPlane << "/" << farPlane
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

long CNavigationModel::IsUserPivot(navlib::bool_t& userPivot) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    userPivot = true;
    // return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetPivotPosition(const navlib::point_t& position)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::GetPivotVisible(navlib::bool_t& visible) const
{
    visible = true;
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetPivotVisible(bool visible)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetHitAperture(double aperture)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetHitDirection(const navlib::vector_t& direction)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetHitLookFrom(const navlib::point_t& eye)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__ << eye.x << eye.y << eye.z;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetHitSelectionOnly(bool onlySelection)
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::GetModelExtents(navlib::box_t& extents) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    extents.max = { 2, 1.5, 6 };
    extents.min = { -2, 0, -6 };
    return 0;
}

long CNavigationModel::GetPivotPosition(navlib::point_t& position) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::GetHitLookAt(navlib::point_t& position) const
{
    qCDebug(spcms) << __PRETTY_FUNCTION__;
    auto p = m_camera->GetPosition() + m_camera->GetForwardVector();
    position = { p.x, p.y, p.z };

    qCDebug(spcms) << "GetHitLookat " << p.x << p.y << p.z;
    return 0;
}

long CNavigationModel::SetActiveCommand(std::string commandId)
{
    qCDebug(spcms) << commandId << __PRETTY_FUNCTION__;
    lib.fireCommand(QLatin1String{commandId});
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
