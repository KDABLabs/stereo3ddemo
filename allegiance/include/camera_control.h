#pragma once
#include "sliders.h"
#include <QGroupBox>

namespace all
{
    class CameraControl : public QWidget
    {
        Q_OBJECT
    public:
        CameraControl(QWidget* parent)
            : QWidget(parent)
            , eye_disparity(0.009f, "Eye Disparity", 0.001f, 0.06f)
            , focus_distance(0.5f, "Focus Distance", 0.5f, 10.0f)
            , enable_cursor(true, "Enable Stereo Cursor")
            , x(10.0f, "Radius", .5f)
            , y(0.0f, "Vertical", -0.99f, 0.99f)
            , z(0.0f, "Horizontal", 0.0f, 1.0f)
        {
            connect(&eye_disparity, &FloatSlider::OnValueChanged, this, &CameraControl::OnEyeDisparityChanged);
            connect(&focus_distance, &FloatSlider::OnValueChanged, this, &CameraControl::OnFocusPlaneChanged);
            //connect(&enable_cursor, &CheckBox::OnValueChanged, [this](bool a) { emit OnEnableCursorChanged(a); });
            QVBoxLayout* layout = new QVBoxLayout(this);
            layout->setAlignment(Qt::AlignTop);


            layout->addWidget(&eye_disparity);
            layout->addWidget(&focus_distance);
            layout->addWidget(&enable_cursor);

            QGroupBox* group = new QGroupBox("Camera Position", this);

            QVBoxLayout* group_layout = new QVBoxLayout(group);
            group_layout->setAlignment(Qt::AlignTop);
            group->setLayout(group_layout);
            group_layout->addWidget(&x);
            group_layout->addWidget(&y);
            group_layout->addWidget(&z);

            layout->addWidget(group);

            setLayout(layout);
        }
    signals:
        void OnEyeDisparityChanged(float value);
        void OnFocusPlaneChanged(float value);
        void OnEnableCursorChanged(bool value);
    private:
        FloatSlider eye_disparity;
        FloatSlider focus_distance;
        CheckBox enable_cursor;

    public:
        FloatSlider x;
        FloatSlider y;
        FloatSlider z;
    };
}