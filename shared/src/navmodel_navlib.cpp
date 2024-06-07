#include <shared/navmodel_navlib.h>
#include <shared/stereo_camera.h>
#include <shared/cursor.h>
#include <numbers>

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

long CNavigationModel::SetCameraMatrix(const navlib::matrix_t& matrix)
{
    auto m = toGlmMat(matrix);
    // qCDebug(spcmsView) << "setting app camera matrix to " << m;
    m_camera->SetCameraMatrix(m);
    return 0;
}

long CNavigationModel::GetCameraMatrix(navlib::matrix_t& matrix) const
{
    auto m = m_camera->GetCameraMatrix();
    // qCDebug(spcmsView) << "setting SpcMouse Camera Matrix to " << m;
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
long CNavigationModel::GetPointerPosition(navlib::point_t& position) const
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    //  return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}
long CNavigationModel::SetPointerPosition(const navlib::point_t& position)
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__ << position.x << position.y << position.z;
    return 0;
}

long CNavigationModel::GetViewExtents(navlib::box_t& extents) const
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    extents.max = { 10, 10, 1000 };
    extents.min = { -10, -10, 0.01 };
    return 0;
}

long CNavigationModel::GetViewFOV(double& fov) const
{
    fov = glm::radians(m_camera->GetFov());
    // qCDebug(spcms) << "GetViewFoV " << fov;
    return 0;
}

long CNavigationModel::GetViewFrustum(navlib::frustum_t& frustum) const
{
    float verticalFieldOfView = m_camera->GetFov(), // in degrees,
            aspectRatio = m_camera->GetAspectRatio(),
          nearPlane = m_camera->GetNearPlane(), farPlane = m_camera->GetFarPlane();

    float halfVerticalFOV = DegreesToRadians(verticalFieldOfView) / 2.0f;
    float halfHorizontalFOV = atanf(tanf(halfVerticalFOV) * aspectRatio);

    // Calculate frustum parameters
    frustum.top = nearPlane * tanf(halfVerticalFOV);
    frustum.bottom = -frustum.top;
    frustum.left = -nearPlane * tanf(halfHorizontalFOV);
    frustum.right = -frustum.left;
    frustum.nearVal = nearPlane;
    frustum.farVal = farPlane;

    // qCDebug(spcmsView) << "vertical fov " << verticalFieldOfView << " near/far: " << nearPlane << "/" << farPlane
    //                    << " left/right " << frustum.left << frustum.right
    //                    << " top/bottom " << frustum.top << frustum.bottom;

    return 0;
}

long CNavigationModel::GetIsViewPerspective(navlib::bool_t& perspective) const
{
    perspective = true;
    // qCDebug(spcms) << "get is perspective " << perspective;
    return 0;
}

long CNavigationModel::SetViewExtents(const navlib::box_t& extents)
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetViewFOV(double fov)
{
    // qCDebug(spcms) << "SetFOV";
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::SetViewFrustum(const navlib::frustum_t& frustum)
{
    // qCDebug(spcms) << "Set View Frustum";
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
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
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

long CNavigationModel::GetSelectionTransform(navlib::matrix_t& transform) const
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
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
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    return navlib::make_result_code(navlib::navlib_errc::no_data_available);
    return 0;
}

// Pivot
long CNavigationModel::IsUserPivot(navlib::bool_t& userPivot) const
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    userPivot = true;
    return 0;
}

long CNavigationModel::SetPivotPosition(const navlib::point_t& position)
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__ << position;
    m_nav_params->pivot_point = glm::vec3(position.x, position.y, position.z);
    return 0;
}

long CNavigationModel::GetPivotVisible(navlib::bool_t& visible) const
{
    visible = true;
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    return 0;
}

long CNavigationModel::SetPivotVisible(bool visible)
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__ << visible;
    return 0;
}

long CNavigationModel::GetPivotPosition(navlib::point_t& position) const
{
    auto pp = m_nav_params->pivot_point;
    position = { pp.x, pp.y, pp.z };
    // qCDebug(spcms) << __PRETTY_FUNCTION__ << position;
    return 0;
}

// Hit
long CNavigationModel::SetHitAperture(double aperture)
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    hit_aperture = aperture;
    return 0;
}

long CNavigationModel::SetHitDirection(const navlib::vector_t& direction)
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__;
    hit_direction = direction;
    return 0;
}

long CNavigationModel::SetHitLookFrom(const navlib::point_t& eye)
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__ << eye.x << eye.y << eye.z;
    hit_source = eye;
    return 0;
}

long CNavigationModel::SetHitSelectionOnly(bool onlySelection)
{
    // qCDebug(spcms) << __PRETTY_FUNCTION__ << onlySelection;
    return 0;
}

long CNavigationModel::GetHitLookAt(navlib::point_t& position) const
{
    auto res = m_nav_params->hit_test({ hit_source.x, hit_source.y, hit_source.z }, { hit_direction.x, hit_direction.y, hit_direction.z });
    // qDebug() << "resulting hit " << res;
    if (res == glm::vec3{ -1 })
        return 0;

    position = { res.x, res.y, res.z };
    // qCDebug(spcms) << __PRETTY_FUNCTION__ << position.x << position.y << position.z;
    return 0;
}

long CNavigationModel::SetActiveCommand(std::string commandId)
{
    // qCDebug(spcms) << commandId << __PRETTY_FUNCTION__;

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
