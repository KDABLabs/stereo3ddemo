#pragma once
#include <QObject>
#include <QColor>
#include <QPalette>
#include <QFont>
#include <QtQml>

class AppStyle : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QColor background READ backgroundColor CONSTANT)
    Q_PROPERTY(QColor base READ baseColor CONSTANT)
    Q_PROPERTY(QColor alternateBase READ alternateBaseColor CONSTANT)
    Q_PROPERTY(QColor toolTipBase READ toolTipBaseColor CONSTANT)
    Q_PROPERTY(QColor toolTipText READ toolTipTextColor CONSTANT)
    Q_PROPERTY(QColor text READ textColor CONSTANT)
    Q_PROPERTY(QColor button READ buttonColor CONSTANT)
    Q_PROPERTY(QColor buttonHover READ buttonHover CONSTANT)
    Q_PROPERTY(QColor buttonDisabled READ buttonDisabled CONSTANT)
    Q_PROPERTY(QColor accent READ accent CONSTANT)
    Q_PROPERTY(QColor buttonText READ buttonText CONSTANT)
    Q_PROPERTY(QColor brightText READ brightText CONSTANT)
    Q_PROPERTY(QColor link READ linkColor CONSTANT)
    Q_PROPERTY(QColor highlight READ highlightColor CONSTANT)
    Q_PROPERTY(QColor highlightedText READ highlightedTextColor CONSTANT)
    Q_PROPERTY(QPalette palette READ palette CONSTANT)

    Q_PROPERTY(QFont fontH1 READ fontH1 CONSTANT)
    Q_PROPERTY(QFont fontH2 READ fontH2 CONSTANT)
    Q_PROPERTY(QFont fontDefault READ fontDefault CONSTANT)
    Q_PROPERTY(QFont fontButton READ fontButton CONSTANT)

    QML_SINGLETON
    QML_NAMED_ELEMENT(Style)

public:
    static constexpr uint32_t kDefaultFontSize = 14;
    static constexpr uint32_t kButtonFontSize = 20;
    static constexpr uint32_t kH1FontSize = 40;
    static constexpr uint32_t kH2FontSize = 30;

public:
    explicit AppStyle(QObject* parent = nullptr)
        : QObject(parent)
        , m_fontH1(u"Segoe UI"_qs, kH1FontSize)
        , m_fontH2(u"Segoe UI"_qs, kH2FontSize)
        , m_fontDefault(u"Segoe UI"_qs, kDefaultFontSize)
        , m_fontButton(u"Segoe UI"_qs, kButtonFontSize)
    {
        m_palette.setColor(QPalette::Window, QColor(53, 53, 53));
        m_palette.setColor(QPalette::WindowText, Qt::white);
        m_palette.setColor(QPalette::Base, QColor(25, 25, 25));
        m_palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        m_palette.setColor(QPalette::ToolTipBase, Qt::white);
        m_palette.setColor(QPalette::ToolTipText, Qt::white);
        m_palette.setColor(QPalette::Text, Qt::white);
        m_palette.setColor(QPalette::Button, QColor(40, 40, 40));
        m_palette.setColor(QPalette::ButtonText, Qt::white);
        m_palette.setColor(QPalette::BrightText, Qt::red);
        m_palette.setColor(QPalette::Link, QColor(42, 130, 218));
        m_palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        m_palette.setColor(QPalette::HighlightedText, Qt::black);
    }

public:
    QColor backgroundColor() const noexcept { return m_palette.color(QPalette::Window); }
    QColor baseColor() const noexcept { return m_palette.color(QPalette::Base); }
    QColor alternateBaseColor() const noexcept { return m_palette.color(QPalette::AlternateBase); }
    QColor toolTipBaseColor() const noexcept { return m_palette.color(QPalette::ToolTipBase); }
    QColor toolTipTextColor() const noexcept { return m_palette.color(QPalette::ToolTipText); }
    QColor textColor() const noexcept { return m_palette.color(QPalette::Text); }
    QColor buttonColor() const noexcept { return m_palette.color(QPalette::Button); }
    QColor buttonHover() const noexcept { return QColor(0x35, 0x35, 0x35); }
    QColor buttonDisabled() const noexcept { return QColor(0x3A, 0x3A, 0x3A); }
    QColor accent() const noexcept { return QColor("#783DBC"); }
    QColor buttonText() const noexcept { return m_palette.color(QPalette::ButtonText); }
    QColor brightText() const noexcept { return m_palette.color(QPalette::BrightText); }
    QColor linkColor() const noexcept { return m_palette.color(QPalette::Link); }
    QColor highlightColor() const noexcept { return m_palette.color(QPalette::Highlight); }
    QColor highlightedTextColor() const noexcept { return m_palette.color(QPalette::HighlightedText); }
    QPalette palette() const noexcept { return m_palette; }

    QFont fontH1() const noexcept { return m_fontH1; }
    QFont fontH2() const noexcept { return m_fontH2; }
    QFont fontDefault() const noexcept { return m_fontDefault; }
    QFont fontButton() const noexcept { return m_fontButton; }

private:
    QPalette m_palette;
    QFont m_fontH1;
    QFont m_fontH2;
    QFont m_fontDefault;
    QFont m_fontButton;
};
