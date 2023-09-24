#pragma once
#include <QLabel>
#include <QPainter>

namespace all {
class VerticalLabel : public QLabel
{
public:
    explicit inline VerticalLabel(QWidget* parent = 0);
    explicit inline VerticalLabel(const QString& text, QWidget* parent = 0);

protected:
    inline void paintEvent(QPaintEvent*);
    inline QSize sizeHint() const;
    inline QSize minimumSizeHint() const;
};

VerticalLabel::VerticalLabel(QWidget* parent)
    : QLabel(parent)
{
}

VerticalLabel::VerticalLabel(const QString& text, QWidget* parent)
    : QLabel(text, parent)
{
}

void VerticalLabel::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.setBrush(Qt::Dense1Pattern);

    painter.rotate(90);

    painter.drawText(0, 0, text());
}

QSize VerticalLabel::minimumSizeHint() const
{
    QSize s = QLabel::minimumSizeHint();
    return QSize(s.height(), s.width());
}

QSize VerticalLabel::sizeHint() const
{
    QSize s = QLabel::sizeHint();
    return QSize(s.height(), s.width());
}
} // namespace all

// int main(int argc, char* argv[])
//{
//     QApplication app(argc, argv);
//
//     // Create a QMainWindow
//     QMainWindow window;
//     window.setWindowTitle("Button with Text on Icon");
//
//     // Create a central widget
//     QWidget* centralWidget = new QWidget(&window);
//     window.setCentralWidget(centralWidget);
//
//     // Create a QVBoxLayout for the central widget
//     QVBoxLayout* layout = new QVBoxLayout(centralWidget);
//
//     // Create a custom widget for the button
//     QWidget* customButtonWidget = new QWidget;
//     QVBoxLayout* buttonLayout = new QVBoxLayout(customButtonWidget);
//
//     //// Create an icon (You can also load an icon from a file)
//     // QIcon icon(":/a.png"); // Replace with your icon path
//     // QLabel* iconLabel = new QLabel;
//     // iconLabel->setPixmap(icon.pixmap(QSize(128, 128))); // Adjust the size as needed
//
//     // Create a QLabel for the text
//     VerticalLabel* textLabel = new VerticalLabel("Click Me");
//     textLabel->setAttribute(Qt::WA_TranslucentBackground);
//
//     //// Add the icon and text labels to the button's layout
//     // buttonLayout->addWidget(iconLabel);
//     buttonLayout->addWidget(textLabel);
//     buttonLayout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
//
//     // Create a QPushButton and set the custom widget as its widget
//     QPushButton* button = new QPushButton{ QIcon(":/a.png"), "" };
//     button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
//     button->setFixedSize({ 128, 128 }); // Adjust the height as needed
//     button->setIconSize({ 128, 128 });
//
//     button->setFocusPolicy(Qt::NoFocus);
//     button->setLayout(buttonLayout);
//
//     // Add the button to the central widget's layout
//     layout->addWidget(button);
//     layout->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
//
//     // Connect a slot to handle button click
//     QObject::connect(button, &QPushButton::clicked, [&]() {
//         QMessageBox::information(&window, "Button Clicked", "Button was clicked!");
//     });
//
//     // Show the main window
//     window.show();
//
//     return app.exec();
// }