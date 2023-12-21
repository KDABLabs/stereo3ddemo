
#ifndef ALLEGIANCE_SPACEMOUSE_H
#define ALLEGIANCE_SPACEMOUSE_H

#include "stereo_camera.h"

#include <QAction>
#include <QBuffer>
#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(spcms, "allegiance.spcms", QtInfoMsg);
inline Q_LOGGING_CATEGORY(spcmsView, "allegiance.spcms.view", QtInfoMsg);

// attach me to a camera
class Spacemouse : public QObject
{
    Q_OBJECT
public:
    Spacemouse(all::StereoCamera* camera, QObject* parent = nullptr)
        : QObject(parent)
        , m_camera(camera)
    {
#if ALLEGIANCE_SERENITY
        camera->OnViewChanged.connect([this] { onViewChanged(); });
#else
        connect(camera, &all::StereoCamera::OnViewChanged,
                this, &Spacemouse::onViewChanged);
#endif
    }

public Q_SLOTS:
    virtual void onMouseChanged(const QVector3D& position)
    {
    }

    void onViewChanged()
    {
        update();
    }

protected:
    all::StereoCamera* getCamera()
    {
        return m_camera;
    }

    virtual void update() {
        // you don't have to tell the device aboout the new coordinates
    }

    // the device sent a new position for our camera
    void setPositionAndForward(const glm::vec3& position, const glm::vec3& forward)
    {
        m_camera->SetPosition(position);
        m_camera->SetForwardVector(forward);
    }
    void setViewMatrix(const glm::mat4x4& viewMatrix)
    {
        m_camera->SetCameraMatrix(viewMatrix);
    }

    all::StereoCamera *m_camera;
};


#ifdef WITH_NAVLIB
#include <SpaceMouse/CNavigation3D.hpp>
#include <SpaceMouse/CCommand.hpp>


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
            matrix.m30, matrix.m31, matrix.m32, matrix.m33
    );
    return gmatrix;
}
inline glm::vec3 toGlmVec3(const navlib::vector_t& point)
{
    return {point.x, point.y, point.z};
}
inline glm::vec3 toGlmVec3(const navlib::point_t& point)
{
    return {point.x, point.y, point.z};
}

inline QDebug operator<<(QDebug debug, const glm::vec3& vec)
{
    return debug << vec.x << vec.y << vec.z;
}

inline QDebug operator<<(QDebug debug, const navlib::point_t &position)
{
    return debug << position.x << position.y << position.z;
}

inline QDebug operator<<(QDebug debug, const navlib::matrix_t& matrix)
{

    std::stringstream ss;
    ss << matrix;
    debug << ss.str() << "\n";
    debug.nospace();
    debug << matrix.m00 << ", " << matrix.m01 << ", " << matrix.m02 << ", " << matrix.m03
          << matrix.m10 << ", " << matrix.m11 << ", " << matrix.m12 << ", " << matrix.m13
          << matrix.m20 << ", " << matrix.m21 << ", " << matrix.m22 << ", " << matrix.m23
          << matrix.m30 << ", " << matrix.m31 << ", " << matrix.m32 << ", " << matrix.m33;
    debug.space();
    return debug;
}


class SpacemouseNavlib;
class CNavigationModel : public TDx::SpaceMouse::Navigation3D::CNavigation3D
{
    friend SpacemouseNavlib;

    typedef TDx::SpaceMouse::Navigation3D::CNavigation3D nav3d;
    SpacemouseNavlib& lib;

protected:
    long GetUnitsToMeters(double& meters) const override;
    long GetFloorPlane(navlib::plane_t& floor) const override;
    long GetFrontView(navlib::matrix_t& matrix) const override;

public:
    explicit CNavigationModel(SpacemouseNavlib& lib, bool multiThreaded = false, bool rowMajor = false);

    // Inherited via CNavigation3D
    long GetCameraMatrix(navlib::matrix_t& matrix) const override;
    long SetCameraMatrix(const navlib::matrix_t& matrix) override;
    long GetViewFOV(double& fov) const override;
    long SetViewFOV(double fov) override;
    long GetViewFrustum(navlib::frustum_t& frustum) const override;
    long SetViewFrustum(const navlib::frustum_t& frustum) override;
    long GetIsViewPerspective(navlib::bool_t& perspective) const override;
    long GetViewExtents(navlib::box_t& extents) const override;

    long SetPointerPosition(const navlib::point_t &position) override;
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
protected:
    all::StereoCamera *m_camera{nullptr};
};


class SpacemouseNavlib : public Spacemouse
{
    Q_OBJECT
public:
    SpacemouseNavlib(all::StereoCamera* camera);

    all::StereoCamera* getCamera() { return m_camera; }
    void setCameraMatrix(glm::mat4x4 m);
    glm::mat4x4 getCameraMatrix();
    void update() override; // tell device about new coordinates

public Q_SLOTS:
    void onSetPivotPoint(const glm::vec3& position)
    {
        m_pivotPoint = position;
        m_model.Write("pivot.user", true);
    }
    void onMouseChanged(const glm::vec3& position)
    {
        m_model.SetPointerPosition({position.x, position.y, position.z});
    }

    void addActions(QVector<QAction*> actions, const QString& id, const QString& menuName);

public Q_SLOTS:
    void fireCommand(QString cmd);

protected:
    TDx::SpaceMouse::CCommandSet m_menuBar{"Default", "Ribbon"};
    QList<QAction*> m_actions;
    CNavigationModel m_model;
    glm::vec3 m_pivotPoint;
    friend CNavigationModel;
};
#endif

#ifdef WITH_SPNAV
class SpacemouseSpnav : public Spacemouse {
public:
    SpacemouseSpnav(all::StereoCamera* camera);
    ~SpacemouseSpnav();

    float m_rotFactor = 0.0001;
    float m_translFactor = 0.001;
protected:
    void poll();

    std::thread m_poller;
    std::atomic_bool stop{false};
};
#endif


#endif // ALLEGIANCE_SPACEMOUSE_H
