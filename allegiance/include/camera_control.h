#pragma once
#include "sliders.h"
#include <QGroupBox>
#include <QPushButton>

namespace all {
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
        QPushButton* eye_disparity = new QPushButton(QIcon{ u":/a.png"_qs }, u""_qs, this);
        QPushButton* focus_distance = new QPushButton(QIcon{ u":/b.png"_qs }, u""_qs, this);


        connect(load_image, &QPushButton::clicked, this, &CameraControl::OnLoadImage);
        connect(load_model, &QPushButton::clicked, this, &CameraControl::OnLoadModel);
        connect(toggle_cursor, &QPushButton::clicked, this, &CameraControl::OnToggleCursor);
        connect(close, &QPushButton::clicked, this, &CameraControl::OnClose);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setAlignment(Qt::AlignTop);
        layout->setContentsMargins(20, 20, 20, 20);

        auto make_button = [](QPushButton* b) {
            b->setFixedSize({ 150, 100 });
            b->setIconSize({ 100, 100 });
            b->setFlat(true);
        };

        QVBoxLayout* function_layout = new QVBoxLayout(this);
        function_layout->setSpacing(50);

        make_button(load_image);
        make_button(load_model);
        make_button(toggle_cursor);

        toggle_cursor->setCheckable(true);
        toggle_cursor->setChecked(true);

        function_layout->addWidget(load_image);
        function_layout->addWidget(load_model);
        function_layout->addWidget(toggle_cursor);

        QVBoxLayout* camera_layout = new QVBoxLayout(this);

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

        setLayout(layout);
    }
signals:
    void OnEyeDisparityChanged(float value);
    void OnFocusPlaneChanged(float value);

    void OnLoadImage();
    void OnLoadModel();
    void OnToggleCursor();
    void OnClose();
};
} // namespace all
