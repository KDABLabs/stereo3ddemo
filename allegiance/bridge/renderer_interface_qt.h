#pragma once

#ifdef ALLEGIANCE_QT3D
#include <qt3d_impl.h>
#include <QApplication>
namespace all {
constexpr bool UseShearing = false;
using Renderer = qt3d::Qt3DImpl;
using Application = QApplication;
} // namespace all
#elif defined(ALLEGIANCE_SERENITY)
#include <serenity_impl_qt.h>
namespace all {
constexpr bool UseShearing = true;
using Renderer = serenity::SerenityImplQt;
using Application = serenity::SerenityGuiApplication;
} // namespace all
#endif
