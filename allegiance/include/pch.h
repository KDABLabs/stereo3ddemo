#pragma once
#if ALLEGIANCE_SERENITY
#include <KDGpu/vulkan/vulkan_graphics_api.h>
#include <KDGui/gui_application.h>
#include <Serenity/core/ecs/aspect_engine.h>
#include <Serenity/core/ecs/camera.h>
#include <Serenity/core/ecs/transform.h>
#include <Serenity/gui/forward_renderer/stereo_forward_algorithm.h>
#include <Serenity/gui/light.h>
#include <Serenity/gui/mesh_generators.h>
#include <Serenity/gui/mesh_loader.h>
#include <Serenity/gui/mesh_renderer.h>
#include <Serenity/gui/render/render_aspect.h>
#include <Serenity/gui/shader_program.h>
#include <Serenity/gui/texture.h>
#include <Serenity/gui/triangle_bounding_volume.h>
#include <Serenity/logic/frame_action.h>
#include <Serenity/logic/logic_aspect.h>
#include <stdexcept>
#else
#include <Qt3DCore>
#include <Qt3DRender>
#include <Qt3DInput>
#include <Qt3DLogic>
#include <Qt3DExtras>
#endif
