// 背包 UI 模块 —— 依赖 UMG/Slate，不包含运行时逻辑
// InventorySystem Plugin

using UnrealBuildTool;

public class InventoryUI : ModuleRules
{
	public InventoryUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"UMG",
				"Slate",
				"SlateCore",
				"InventorySystem",
				"InputCore",
				"GameplayTags", "EnhancedInput",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine", "EnhancedInput",
			}
		);
	}
}
