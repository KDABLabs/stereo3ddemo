#include <shared/navmodel_navlib.h>
#include <shared/stereo_camera.h>
#include <shared/cursor.h>
#include <numbers>
#include <glm/ext/matrix_transform.hpp>

using namespace all;

inline navlib::matrix_t toNavlibMatrix(const glm::mat4x4& mat)
{
    return {
        mat[0][0], mat[0][1], mat[0][2], mat[0][3],
        mat[1][0], mat[1][1], mat[1][2], mat[1][3],
        mat[2][0], mat[2][1], mat[2][2], mat[2][3],
        mat[3][0], mat[3][1], mat[3][2], mat[3][3]
    };
}

inline glm::mat4x4 toGlmMat(const navlib::matrix_t& matrix)
{
    glm::mat4x4 gmatrix(
            matrix.m00, matrix.m01, matrix.m02, matrix.m03,
            matrix.m10, matrix.m11, matrix.m12, matrix.m13,
            matrix.m20, matrix.m21, matrix.m22, matrix.m23,
            matrix.m30, matrix.m31, matrix.m32, matrix.m33);
    return gmatrix;
}
inline glm::vec3 toGlmVec3(const navlib::vector_t& point)
{
    return { point.x, point.y, point.z };
}
inline glm::vec3 toGlmVec3(const navlib::point_t& point)
{
    return { point.x, point.y, point.z };
}

template<std::floating_point T>
inline T DegreesToRadians(T degrees)
{
    return degrees * std::numbers::pi_v<T> / T(180.0);
}

long CNavigationModel::SetCameraMatrix(const navlib::matrix_t& inverseViewMatrix)
{
    const auto m = toGlmMat(inverseViewMatrix);
    glm::vec4 pos { 0., 0., 0., 1. };
    pos = m * pos;
    glm::vec4 dir { 0., 0., -1., 0. };
    dir = m * dir;
    glm::vec4 up { 0., 1., 0., 0. };
    up = m * up;

    m_camera->position = pos;
    m_camera->setForwardVector(dir);
    m_camera->setUpVector(up);

    return 0;
}

// Eye Space -> World Space (inverseViewMatrix)
long CNavigationModel::GetCameraMatrix(navlib::matrix_t& inverseViewMatrix) const
{
    inverseViewMatrix = toNavlibMatrix(glm::inverse(glm::lookAt(m_camera->position(),
                                                                m_camera->position() + m_camera->forwardVector(),
                                                                m_camera->upVector())));
    return 0;
}

long CNavigationModel::GetUnitsToMeters(double& meters) const
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
    meters = 0.5;
    return 0;
}
long CNavigationModel::GetFloorPlane(navlib::plane_t& floor) const
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
    floor.d = 0;
    floor.n = { 0, -1, 0 };

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
CNavigationModel::CNavigationModel(std::shared_ptr<all::ModelNavParameters> nav_params, all::StereoCamera* camera, bool multiThreaded, bool rowMajor)
    : CNavigation3D(multiThreaded, rowMajor), m_nav_params(std::move(nav_params)), m_camera(camera)
{
    nav3d::Profile = "Allegiance";
    // Enable input from / output to the Navigation3D controller.
    nav3d::Enable = false;
    nav3d::FrameTiming = TimingSource::SpaceMouse;
}

void CNavigationModel::onModelLoaded()
{
    nav3d::Write(navlib::selection_empty_k, true);
    navlib::matrix_t coordinateSystem;
    if (GetCoordinateSystem(coordinateSystem) == 0) {
        nav3d::Write(navlib::coordinate_system_k, coordinateSystem);
    }

    navlib::box_t extents;
    if (GetModelExtents(extents) == 0) {
        nav3d::Write(navlib::model_extents_k, extents);
    }

    navlib::point_t pivotPosition;
    if (GetPivotPosition(pivotPosition) == 0) {
        nav3d::Write(navlib::pivot_position_k, pivotPosition);
    }

    navlib::matrix_t affine;
    if (GetFrontView(affine) == 0) {
        nav3d::Write(navlib::views_front_k, affine);
    }

    onViewChanged();
}

void CNavigationModel::onViewChanged()
{
    navlib::matrix_t inverseViewMatrix;

    if (GetCameraMatrix(inverseViewMatrix) == 0) {
        Write(navlib::view_affine_k, inverseViewMatrix);
    }
}

long CNavigationModel::GetPointerPosition(navlib::point_t& position) const
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}
long CNavigationModel::SetPointerPosition(const navlib::point_t& position)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::GetViewExtents(navlib::box_t& extents) const
{
    extents.max = { 10, 10, 1000 };
    extents.min = { -10, -10, 0.01 };
    return 0;
}

long CNavigationModel::GetViewFOV(double& fov) const
{
    fov = DegreesToRadians(m_camera->fov());
    return 0;
}

long CNavigationModel::GetViewFrustum(navlib::frustum_t& frustum) const
{
    const float verticalFieldOfView = m_camera->fov(); // in degrees,
    const float nearPlane = m_camera->nearPlane();
    const float farPlane = m_camera->farPlane();

    const float halfVerticalFOV = DegreesToRadians(verticalFieldOfView) * 0.5f;
    const float tanHalfVFoc = std::tan(halfVerticalFOV);
    const float halfHeight = halfVerticalFOV * tanHalfVFoc;
    const float halfWidth = m_camera->aspectRatio() * halfHeight;

    // Calculate frustum parameters
    frustum.top = halfHeight;
    frustum.bottom = -halfHeight;
    frustum.left = -halfWidth;
    frustum.right = -halfWidth;
    frustum.nearVal = nearPlane;
    frustum.farVal = farPlane;

    return 0;
}

long CNavigationModel::GetIsViewPerspective(navlib::bool_t& perspective) const
{
    perspective = true;
    return 0;
}

long CNavigationModel::SetViewExtents(const navlib::box_t& extents)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::SetViewFOV(double fov)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::SetViewFrustum(const navlib::frustum_t& frustum)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::GetModelExtents(navlib::box_t& extents) const
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    auto& min_ext = m_nav_params->min_extent;
    auto& max_ext = m_nav_params->max_extent;
    extents.min = { min_ext.x, min_ext.y, min_ext.z };
    extents.max = { max_ext.x, max_ext.y, max_ext.z };
    return 0;
}

long CNavigationModel::GetSelectionExtents(navlib::box_t& extents) const
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::GetSelectionTransform(navlib::matrix_t& transform) const
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::GetIsSelectionEmpty(navlib::bool_t& empty) const
{
    empty = true;
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::SetSelectionTransform(const navlib::matrix_t& matrix)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

// Pivot
long CNavigationModel::IsUserPivot(navlib::bool_t& userPivot) const
{
    userPivot = true;
    return 0;
}

long CNavigationModel::SetPivotPosition(const navlib::point_t& position)
{
    m_nav_params->pivot_point = glm::vec3(position.x, position.y, position.z);
    return 0;
}

long CNavigationModel::GetPivotVisible(navlib::bool_t& visible) const
{
    visible = true;
    return 0;
}

long CNavigationModel::SetPivotVisible(bool visible)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::GetPivotPosition(navlib::point_t& position) const
{
    auto pp = m_nav_params->pivot_point;
    position = { pp.x, pp.y, pp.z };
    return 0;
}

// Hit
long CNavigationModel::SetHitAperture(double aperture)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::SetHitDirection(const navlib::vector_t& direction)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::SetHitLookFrom(const navlib::point_t& eye)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::SetHitSelectionOnly(bool onlySelection)
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::GetHitLookAt(navlib::point_t& position) const
{
    return navlib::make_result_code(navlib::navlib_errc::invalid_operation);
}

long CNavigationModel::SetActiveCommand(std::string commandId)
{
    if (commandId.empty())
        return 0;

    if (auto a = m_commands.find(commandId); a != m_commands.end()) {
        a->second();
    }
    return 0;
}

void CNavigationModel::AddCommand(std::string commandId, std::function<void()> command)
{
    m_commands[commandId] = command;
}
