#pragma once
#include "sliders.h"
#include "vertical_label.h"
#include <QGroupBox>
#include <QPushButton>
#include <QMouseEvent>

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
        QPushButton* load_image = new QPushButton(QIcon{ u":/stereo3.png"_qs }, u""_qs, this);
        QPushButton* load_model = new QPushButton(QIcon{ u":/3D.png"_qs }, u""_qs, this);
        QPushButton* toggle_cursor = new QPushButton(QIcon{ u":/3Dcursor.png"_qs }, u""_qs, this);
        QPushButton* close = new QPushButton(QIcon{ u":/close.png"_qs }, u""_qs, this);
        ValueButton* eye_disparity = new ValueButton(QIcon{ u":/b.png"_qs }, LabelPosition::Bottom, { 0.0f, 1.0f }, 0.06f, this);
        ValueButton* focus_distance = new ValueButton(QIcon{ u":/a.png"_qs }, LabelPosition::Right, { 0.5f, 100.0f }, 5.0f, this);

        connect(load_image, &QPushButton::clicked, this, &CameraControl::OnLoadImage);
        connect(load_model, &QPushButton::clicked, this, &CameraControl::OnLoadModel);
        connect(toggle_cursor, &QPushButton::clicked, this, &CameraControl::OnToggleCursor);
        connect(close, &QPushButton::clicked, this, &CameraControl::OnClose);
        connect(eye_disparity, &ValueButton::OnValueChanged, this, &CameraControl::OnEyeDisparityChanged);
        connect(focus_distance, &ValueButton::OnValueChanged, this, &CameraControl::OnFocusPlaneChanged);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);
        layout->setContentsMargins(20, 20, 20, 20);

        auto make_button = [](QPushButton* b) {
            b->setFixedSize({ 128, 128 });
            b->setIconSize({ 128, 128 });
            b->setFlat(true);
            b->setStyleSheet(uR"(QPushButton {
                border: 2px solid #8f8f91;
                border-radius: 20px;
                min-width: 128px;
                min-height: 128px;
            }
            QPushButton:hover:!pressed {
                border: 4px solid #ff0000;
            }
            QPushButton:pressed {
                border: 4px solid #ffff00;
            }
            QPushButton:checked {
                border: 4px solid #ffff00;
            }
        )"_qs);
        };

        QVBoxLayout* function_layout = new QVBoxLayout;
        function_layout->setSpacing(50);

        make_button(load_image);
        make_button(load_model);
        make_button(toggle_cursor);

        toggle_cursor->setCheckable(true);
        toggle_cursor->setChecked(true);

        function_layout->addWidget(load_image);
        function_layout->addWidget(load_model);
        function_layout->addWidget(toggle_cursor);

        QVBoxLayout* camera_layout = new QVBoxLayout;

        make_button(eye_disparity);
        make_button(focus_distance);

        camera_layout->addWidget(eye_disparity);
        camera_layout->addWidget(focus_distance);

        layout->addLayout(function_layout);
        layout->insertSpacing(1, 50);
        layout->addLayout(camera_layout);

        make_button(close);
        layout->addStretch();
        layout->addWidget(close);

        QPixmap p{":/kdab.png"};
        p = p.scaled(128, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        auto l = new QLabel;
        l->setPixmap(p);
        l->setScaledContents(true);

        layout->addSpacing(10);
        layout->addWidget(l);

        p = QPixmap{":/schneider_white.png"};
        p = p.scaled(128, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        l = new QLabel;
        l->setPixmap(p);
        l->setScaledContents(true);

        layout->addSpacing(10);
        layout->addWidget(l);
    }
Q_SIGNALS:
    void OnEyeDisparityChanged(float value);
    void OnFocusPlaneChanged(float value);

    void OnLoadImage();
    void OnLoadModel();
    void OnToggleCursor(bool checked);
    void OnClose();
};
} // namespace all
