#pragma once
#include "sliders.h"

namespace all
{
	class CameraControl : public QWidget
	{
		Q_OBJECT
	public:
		CameraControl(QWidget* parent) : QWidget(parent),
			eye_disparity(x1, "Eye Disparity", 0.001f, 0.06f),
			focus_distance(x2, "Focus Distance", 0.5f, 10.0f),
			enable_cursor(true, "Enable Stereo Cursor")
		{
			connect(&eye_disparity, &FloatSlider::OnValueChanged, this, &CameraControl::OnEyeDisparityChanged);
			connect(&focus_distance, &FloatSlider::OnValueChanged, this, &CameraControl::OnFocusPlaneChanged);
			//connect(&enable_cursor, &CheckBox::OnValueChanged, [this](bool a) { emit OnEnableCursorChanged(a); });
			QVBoxLayout* layout = new QVBoxLayout(this);
			layout->setAlignment(Qt::AlignTop);


			layout->addWidget(&eye_disparity);
			layout->addWidget(&focus_distance);
			layout->addWidget(&enable_cursor);
			setLayout(layout);
		}
	signals:
		void OnEyeDisparityChanged(float value);
		void OnFocusPlaneChanged(float value);
		void OnEnableCursorChanged(bool value);
	private:
		float x1 = 0.009f;
		float x2 = 0.5f;
		FloatSlider eye_disparity;
		FloatSlider focus_distance;
		CheckBox enable_cursor;
	};
}