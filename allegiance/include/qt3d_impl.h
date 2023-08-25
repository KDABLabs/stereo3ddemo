#pragma once
#include "camera_control.h"

using namespace Qt3DCore;
using namespace Qt3DRender;
using namespace Qt3DInput;
using namespace Qt3DLogic;
using namespace Qt3DExtras;

class Qt3DImpl
{
public:
	Qt3DImpl()
	{

	}
public:
	void CreateScene()
	{
		root_entity = std::make_unique<QEntity>();
		view.setRootEntity(root_entity.get());
	}
	void CreateAspects(all::CameraControl*)
	{
		CreateScene();
	}
	QWindow* GetWindow() { return &view; }
private:
	Qt3DWindow view;
	std::unique_ptr<QEntity> root_entity;
};