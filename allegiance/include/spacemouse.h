
#ifndef ALLEGIANCE_SPACEMOUSE_H
#define ALLEGIANCE_SPACEMOUSE_H

#include "stereo_camera.h"

#include <QAction>
#include <QBuffer>
#include <QLoggingCategory>

inline Q_LOGGING_CATEGORY(spcms, "allegiance.spcms", QtInfoMsg);

// attach me to a camera
class Spacemouse : public QObject
{
    Q_OBJECT
public:
    Spacemouse(all::StereoCamera* camera, QObject* parent = nullptr)
        : QObject(parent)
        , m_camera(camera)
    {
        m_orbital = static_cast<all::OrbitalStereoCamera*>(m_camera);
#if ALLEGIANCE_SERENITY
        camera->OnViewChanged.connect([this] { onViewChanged(); });
#else
        connect(camera, &all::StereoCamera::OnViewChanged,
                this, &Spacemouse::onViewChanged);
#endif
    }

    public Q_SLOTS:

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
    all::OrbitalStereoCamera *m_orbital;
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
    typedef TDx::SpaceMouse::Navigation3D::CNavigation3D base_type;
    typedef TDx::SpaceMouse::Navigation3D::CNavigation3D nav3d;
    SpacemouseNavlib& lib;

protected:
    long GetUnitsToMeters(double& meters) const override;
    long GetFloorPlane(navlib::plane_t& floor) const override;
    long GetFrontView(navlib::matrix_t& matrix) const override;

public:
    explicit CNavigationModel(SpacemouseNavlib& lib, bool multiThreaded = false, bool rowMajor = false);

    // Inherited via CNavigation3D
    long GetPointerPosition(navlib::point_t& position) const override;
    long GetViewExtents(navlib::box_t& extents) const override;
    long GetViewFOV(double& fov) const override;
    long GetViewFrustum(navlib::frustum_t& frustum) const override;
    long GetIsViewPerspective(navlib::bool_t& perspective) const override;
    long SetCameraMatrix(const navlib::matrix_t& matrix) override;
    long SetViewExtents(const navlib::box_t& extents) override;
    long SetViewFOV(double fov) override;
    long SetViewFrustum(const navlib::frustum_t& frustum) override;
    long GetSelectionExtents(navlib::box_t& extents) const override;
    long GetSelectionTransform(navlib::matrix_t& transform) const override;
    long GetIsSelectionEmpty(navlib::bool_t& empty) const override;
    long SetSelectionTransform(const navlib::matrix_t& matrix) override;
    long IsUserPivot(navlib::bool_t& userPivot) const override;
    long SetPivotPosition(const navlib::point_t& position) override;
    long GetPivotVisible(navlib::bool_t& visible) const override;
    long SetPivotVisible(bool visible) override;
    long SetHitAperture(double aperture) override;
    long SetHitDirection(const navlib::vector_t& direction) override;
    long SetHitLookFrom(const navlib::point_t& eye) override;
    long SetHitSelectionOnly(bool onlySelection) override;
    long GetCameraMatrix(navlib::matrix_t& matrix) const override;
    long GetModelExtents(navlib::box_t& extents) const override;
    long GetPivotPosition(navlib::point_t& position) const override;
    long GetHitLookAt(navlib::point_t& position) const override;
    long SetActiveCommand(std::string commandId) override;

    navlib::matrix_t* m_viewMatrix{nullptr};
    all::StereoCamera *m_camera{nullptr};
};

inline std::string convertPixmapToString(const QPixmap& pixmap) {
    QImage image = pixmap.toImage();

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");

    return std::string(byteArray.constData(), byteArray.length());
}

class SpacemouseNavlib : public Spacemouse
{
    Q_OBJECT
public:
    SpacemouseNavlib(all::StereoCamera* camera);

    all::StereoCamera* getCamera() { return m_camera; }
    void setCameraMatrix(glm::mat4x4 m);
    glm::mat4x4 getCameraMatrix();
    void update() override; // tell device about new coordinates

    void addAction(QAction* action, int slot = 0)
    {
        using TDx::SpaceMouse::CCategory;
        using TDx::SpaceMouse::CCommand;
        using TDx::SpaceMouse::CCommandSet;
        using TDx::CImage;


        auto icon = action->icon();
        if(!icon.isNull()) {
            qDebug() << "Adding icon";
            auto p = action->icon().pixmap(30, 30);
            QImage image = p.toImage();

            QByteArray byteArray;
            QBuffer buffer(&byteArray);
            buffer.open(QIODevice::WriteOnly);
            image.save(&buffer, "PNG");  // Change format as needed
            std::string data(byteArray.constData(), byteArray.length());
            std::vector<CImage> images = {
                CImage::FromData(data, 0, action->text().toStdString().c_str())
            };
            m_model.AddImages(images);
        }

        CCategory menu("FileMenu", "File");
        menu.push_back(CCommand{action->text().toStdString(), action->toolTip().toStdString(), action->whatsThis().toStdString()});
        m_menuBar.push_back(std::move(menu));

        qDebug() << m_menuBar.Id;
        m_model.AddCommandSet(m_menuBar);
        //m_model.ActiveCommands = m_menuBar.Id;
        m_model.PutActiveCommands(m_menuBar.Id);


        m_actions.push_back(action);
    }

public Q_SLOTS:
    void fireCommand(QString cmd)
    {
        if (cmd.isEmpty()) return;
        auto a = std::find_if(m_actions.begin(), m_actions.end(), [cmd](auto e) { return e->text() == cmd; });
        if (a) {
            auto b = *a;
            b->trigger();
        }
    }

protected:
    TDx::SpaceMouse::CCommandSet m_menuBar{"Default", "Ribbon"};
    QList<QAction*> m_actions;
    CNavigationModel m_model;
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
