#pragma once
#include "sliders.h"
#include "vertical_label.h"
#include <QGroupBox>
#include <QPushButton>
#include <QMouseEvent>
#include <QQuickWidget>
#include <QQuickStyle>
#include <QQmlEngine>

namespace all {
struct MouseTracker {
    QPoint last_pos = {};
    bool is_pressed = false;
    bool skip_first = false;
};

enum class LabelPosition {
    Right,
    Bottom
};
class ValueButton : public QPushButton
{
    Q_OBJECT
public:
    ValueButton(const QIcon& icon, LabelPosition pos, std::pair<float, float> boundaries, float initial, QWidget* parent = nullptr)
        : QPushButton(icon, {}, parent)
        , label(pos == LabelPosition::Right ? new VerticalLabel(this) : new QLabel(this))
        , value(initial)
        , boundaries(boundaries)
        , label_position(pos)
    {
        label->setFont(QFont{ u"Arial"_qs, 15 });
        label->setAlignment(Qt::AlignCenter);
        label->setText(QString::number(value, 'f', 2));

        QBoxLayout* layout = pos == LabelPosition::Right
                ? (QBoxLayout*)new QHBoxLayout(this)
                : (QBoxLayout*)new QVBoxLayout(this);

        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(label);
        layout->setAlignment(pos == LabelPosition::Right
                                     ? Qt::AlignVCenter | Qt::AlignRight
                                     : Qt::AlignHCenter | Qt::AlignBottom);
        setLayout(layout);
    }

public:
    void mouseMoveEvent(QMouseEvent* e) override
    {
        if (e->buttons() & Qt::LeftButton) {
            auto pos = e->pos();

            if (tracker.skip_first) {
                tracker.skip_first = false;
                tracker.last_pos = pos;
                return QPushButton::mouseMoveEvent(e);
            }
            value += (label_position == LabelPosition::Right
                              ? (tracker.last_pos.y() - pos.y())
                              : (pos.x() - tracker.last_pos.x())) *
                    (boundaries.second - boundaries.first) * 0.005f;

            value = std::clamp(value, boundaries.first, boundaries.second);
            tracker.last_pos = pos;
            label->setText(QString::number(value, 'f', 2));
            Q_EMIT OnValueChanged(value);
        }
        return QPushButton::mouseMoveEvent(e);
    }
    void mousePressEvent(QMouseEvent* e) override
    {
        if (e->button() == Qt::LeftButton) {
            tracker.is_pressed = true;
            tracker.skip_first = true;
        }
        return QPushButton::mousePressEvent(e);
    }
    void mouseReleaseEvent(QMouseEvent* e) override
    {
        if (e->button() == Qt::LeftButton) {
            tracker.is_pressed = false;
        }
        return QPushButton::mouseReleaseEvent(e);
    }
Q_SIGNALS:
    void OnValueChanged(float value);

private:
    QLabel* label;
    std::pair<float, float> boundaries;
    float value = 0.0f;
    MouseTracker tracker;
    LabelPosition label_position;
};

class CameraControl : public QWidget
{
    Q_OBJECT
public:
    CameraControl(QWidget* parent)
        : QWidget(parent)
    {
        QQuickStyle::setStyle("Fusion");

        layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);
        layout->setContentsMargins(20, 20, 20, 20);

        engine = new QQmlEngine(this);
        qw = new QQuickWidget(engine, this);
        qw->setSource(QUrl(u"qrc:/resources/camera_control.qml"_qs));
        qw->setResizeMode(QQuickWidget::SizeRootObjectToView);
        qw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        layout->addWidget(qw);
        // layout->addWidget(new QQuickWidget(QUrl{ u"qrc:/camera_control.qml"_qs }, this));

        // QPixmap p{ ":/kdab.png" };
        // p = p.scaled(128, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // auto l = new QLabel;
        // l->setPixmap(p);
        // l->setScaledContents(true);

        //// layout->addSpacing(10);
        //// layout->addWidget(l);

        // p = QPixmap{ ":/schneider_white.png" };
        // p = p.scaled(128, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        // l = new QLabel;
        // l->setPixmap(p);
        // l->setScaledContents(true);
    }
    void Reload()
    {
        //engine->clearComponentCache();
        //qw->setSource(QUrl::fromLocalFile(u"C:/Users/Agrae/source/repos/qt3d/allegiance/resources/camera_control.qml"_qs));
        //return;

        //auto state = qw->status();
        //if (state != QQuickWidget::Ready && state != QQuickWidget::Error)
        //    return;

        //auto qw1 = new QQuickWidget(engine, this);
        //qw1->setSource(QUrl::fromLocalFile(u"C:/Users/Agrae/source/repos/qt3d/allegiance/resources/camera_control.qml"_qs));
        //qw1->setResizeMode(QQuickWidget::SizeRootObjectToView);
        //qw1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        //layout->replaceWidget(qw, qw1);
        //delete qw;
        //qw = qw1;
    }
Q_SIGNALS:
    void OnEyeDisparityChanged(float value);
    void OnFocusPlaneChanged(float value);

    void OnLoadImage();
    void OnLoadModel();
    void OnToggleCursor(bool checked);
    void OnClose();

private:
    QQuickWidget* qw;
    QVBoxLayout* layout;
    QQmlEngine* engine;
};
} // namespace all
