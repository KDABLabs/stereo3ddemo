#pragma once
#include <QWidget>
#include <QVBoxLayout>
#include <QDoubleValidator>
#include <QLineEdit>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>

namespace all {
	class FloatSlider : public QWidget
	{
		Q_OBJECT
	private:
		constexpr static auto precision = 1000;
		constexpr static auto fprecision = float(precision);
	public:
		inline FloatSlider(float value, const QString& name, float min = -20, float max = 20);
	public:
		inline void SetMin(float min);
		inline void SetMax(float max);
	signals:
		void OnValueChanged(float val);
	private:
		QVBoxLayout vl;
		QHBoxLayout lay;
		QSlider slider;
		QLineEdit text;
		QDoubleValidator valid;
		float val;
		float dpi;
		float max;
		float min;
	};

	FloatSlider::FloatSlider(float value, const QString& name, float xmin, float xmax)
		:valid(xmin, xmax, 4), min(xmin), max(xmax), slider(Qt::Horizontal), val(value), dpi(fabs(xmax - xmin) / fprecision)
	{
		vl.setContentsMargins(0, 0, 0, 0);
		lay.setContentsMargins(0, 0, 0, 0);
		if (!name.isEmpty())vl.addWidget(new QLabel(name));


		slider.setRange(0, precision);
		text.setValidator(&valid);
		connect(&text, &QLineEdit::textEdited,
			[this](const QString& v) {
				val = v.toFloat();
				slider.setValue(int(roundf((val - min) / dpi)));
				emit OnValueChanged(val);
			});
		connect(&slider, &QSlider::sliderMoved,
			[this](int v) {
				val = v * dpi + min;
				text.setText(QString::number(val));
				emit OnValueChanged(val);
			});

		text.setText(QString::number(value));
		slider.setValue(int(roundf((value - min) / dpi)));

		// Set text size policy
		QSizePolicy spText(QSizePolicy::Preferred, QSizePolicy::Fixed);
		spText.setHorizontalStretch(1);
		text.setSizePolicy(spText);

		// Set slider size policy
		QSizePolicy spSlider(QSizePolicy::Preferred, QSizePolicy::Fixed);
		spSlider.setHorizontalStretch(3);
		slider.setSizePolicy(spSlider);

		lay.addWidget(&slider);
		lay.addWidget(&text);
		vl.addLayout(&lay);
		setLayout(&vl);
	}
	void FloatSlider::SetMin(float xmin)
	{
		valid.setBottom(min = xmin);
		dpi = fabs(max - min) / fprecision;
		slider.setValue(int(roundf((val - min) / dpi)));
	}
	void FloatSlider::SetMax(float xmax)
	{
		valid.setTop(max = xmax);
		dpi = fabs(max - min) / fprecision;
		slider.setValue(int(roundf((val - min) / dpi)));
	}

	class CheckBox :public QWidget
	{
		Q_OBJECT
	public:
		inline CheckBox(bool value, const QString& name);
	signals:
		void OnValueChanged(bool v);
	private:
		QVBoxLayout lay;
		QCheckBox box;
	};

	CheckBox::CheckBox(bool value, const QString& name)
		: box(name)
	{
		box.setChecked(value);
		lay.addWidget(&box);
		setLayout(&lay);
		connect(&box, &QCheckBox::stateChanged, [this](int a) { emit OnValueChanged(bool(a)); });
	}
}