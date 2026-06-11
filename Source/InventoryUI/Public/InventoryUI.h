// 背包 UI 模块
// InventorySystem Plugin

#pragma once

#include "Modules/ModuleManager.h"

class FInventoryUIModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
