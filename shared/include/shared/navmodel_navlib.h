#pragma once
#include <stdexcept> // std::logic_error for navlib header (must be first)
#include <SpaceMouse/CNavigation3D.hpp>
#include <SpaceMouse/CCommand.hpp>
#include <unordered_map>
#include <functional>

namespace all {
class StereoCamera;
struct ModelNavParameters;

class CNavigationModel : public TDx::SpaceMouse::Navigation3D::CNavigation3D
{
    using nav3d = TDx::SpaceMouse::Navigation3D::CNavigation3D;

protected:
    long GetUnitsToMeters(double& meters) const override;
    long GetFloorPlane(navlib::plane_t& floor) const override;
    long GetFrontView(navlib::matrix_t& matrix) const override;

public:
    explicit CNavigationModel(std::shared_ptr<all::ModelNavParameters> nav_params, all::StereoCamera* camera, bool multiThreaded = false, bool rowMajor = false);

    void onModelLoaded();
    void onViewChanged();

protected:
    // Inherited via CNavigation3D
    long GetCameraMatrix(navlib::matrix_t& matrix) const override;
    long SetCameraMatrix(const navlib::matrix_t& matrix) override;
    long GetViewFOV(double& fov) const override;
    long SetViewFOV(double fov) override;
    long GetViewFrustum(navlib::frustum_t& frustum) const override;
    long SetViewFrustum(const navlib::frustum_t& frustum) override;
    long GetIsViewPerspective(navlib::bool_t& perspective) const override;
    long GetViewExtents(navlib::box_t& extents) const override;

    long SetPointerPosition(const navlib::point_t& position) override;
    long GetPointerPosition(navlib::point_t& position) const override;

    // Model
    long SetViewExtents(const navlib::box_t& extents) override;
    long GetModelExtents(navlib::box_t& extents) const override;

    long GetSelectionExtents(navlib::box_t& extents) const override;
    long GetSelectionTransform(navlib::matrix_t& transform) const override;
    long GetIsSelectionEmpty(navlib::bool_t& empty) const override;
    long SetSelectionTransform(const navlib::matrix_t& matrix) override;
    // Pivot
    long GetPivotPosition(navlib::point_t& position) const override;
    long IsUserPivot(navlib::bool_t& userPivot) const override;
    long SetPivotPosition(const navlib::point_t& position) override;
    long GetPivotVisible(navlib::bool_t& visible) const override;
    long SetPivotVisible(bool visible) override;
    // Hit
    long SetHitAperture(double aperture) override;
    long SetHitDirection(const navlib::vector_t& direction) override;
    long SetHitLookFrom(const navlib::point_t& eye) override;
    long SetHitSelectionOnly(bool onlySelection) override;
    long GetHitLookAt(navlib::point_t& position) const override;

    long SetActiveCommand(std::string commandId) override;

public:
    void AddCommand(std::string commandId, std::function<void()> command);

protected:
    all::StereoCamera* m_camera{ nullptr };
    std::shared_ptr<all::ModelNavParameters> m_nav_params;

    double hit_aperture{};
    navlib::vector_t hit_direction{};
    navlib::point_t hit_source{};

    std::unordered_map<std::string, std::function<void()>> m_commands;
};

} // namespace all