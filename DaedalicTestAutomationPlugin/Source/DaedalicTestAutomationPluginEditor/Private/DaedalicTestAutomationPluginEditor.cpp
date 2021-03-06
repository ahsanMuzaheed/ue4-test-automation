// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "AutomationTestFramework/DaeTestAutomationPluginAutomationTestFrameworkIntegration.h"
#include "DaeTestAutomationPluginEditorSettings.h"
#include "DaedalicTestAutomationPluginEditorClasses.h"
#include "IDaedalicTestAutomationPluginEditor.h"
#include <AssetToolsModule.h>
#include <CoreMinimal.h>
#include <IAssetTypeActions.h>
#include <ISettingsModule.h>
#include <ISettingsSection.h>
#include <Modules/ModuleManager.h>

#define LOCTEXT_NAMESPACE "DaedalicTestAutomationPlugin"

class FDaedalicTestAutomationPluginEditor : public IDaedalicTestAutomationPluginEditor
{
public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    /** Asset catetory for test automation assets. */
    EAssetTypeCategories::Type DaedalicTestAutomationAssetCategory;

    /** Asset type actions registered by this module. */
    TArray<TSharedPtr<IAssetTypeActions>> AssetTypeActions;

    /** Integration with the Unreal Automation Test Framework. */
    FDaeTestAutomationPluginAutomationTestFrameworkIntegration AutomationTestFrameworkIntegration;

    void RegisterAssetTypeAction(class IAssetTools& AssetTools,
                                 TSharedRef<IAssetTypeActions> Action);

    void OnTestMapPathChanged(const FString& NewTestMapPath);
};

IMPLEMENT_MODULE(FDaedalicTestAutomationPluginEditor, DaedalicTestAutomationPluginEditor)

void FDaedalicTestAutomationPluginEditor::StartupModule()
{
    // Register asset types.
    IAssetTools& AssetTools =
        FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

    DaedalicTestAutomationAssetCategory =
        AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("DaedalicTestAutomationPlugin")),
                                                 NSLOCTEXT("DaedalicTestAutomationPlugin",
                                                           "DaedalicTestAutomationAssetCategory",
                                                           "Test Automation"));

    TSharedRef<IAssetTypeActions> TestActorBlueprintAction = MakeShareable(
        new FAssetTypeActions_DaeGauntletTestActorBlueprint(DaedalicTestAutomationAssetCategory));
    RegisterAssetTypeAction(AssetTools, TestActorBlueprintAction);

    // Register settings.
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        UDaeTestAutomationPluginEditorSettings* TestAutomationPluginEditorSettings =
            GetMutableDefault<UDaeTestAutomationPluginEditorSettings>();

        ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings(
            "Editor", "Plugins", "DaedalicTestAutomationPlugin",
            NSLOCTEXT("DaedalicTestAutomationPlugin",
                      "DaeTestAutomationPluginEditorSettings.DisplayName",
                      "Daedalic Test Automation Plugin"),
            NSLOCTEXT("DaedalicTestAutomationPlugin",
                      "AssetTypeActions_DaeGauntletTestActorBlueprint.Description",
                      "Configure the discovery of automated tests."),
            TestAutomationPluginEditorSettings);

        TestAutomationPluginEditorSettings->OnTestMapPathChanged.AddRaw(
            this, &FDaedalicTestAutomationPluginEditor::OnTestMapPathChanged);

        OnTestMapPathChanged(TestAutomationPluginEditorSettings->TestMapPath);
    }
}

void FDaedalicTestAutomationPluginEditor::ShutdownModule()
{
    // Unregister asset types.
    if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
    {
        IAssetTools& AssetToolsModule =
            FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
        for (auto& AssetTypeAction : AssetTypeActions)
        {
            if (AssetTypeAction.IsValid())
            {
                AssetToolsModule.UnregisterAssetTypeActions(AssetTypeAction.ToSharedRef());
            }
        }
    }

    AssetTypeActions.Empty();

    // Unregister settings.
    if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Editor", "Plugins", "DaedalicTestAutomationPlugin");
    }
}

void FDaedalicTestAutomationPluginEditor::RegisterAssetTypeAction(
    class IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
{
    AssetTools.RegisterAssetTypeActions(Action);
    AssetTypeActions.Add(Action);
}

void FDaedalicTestAutomationPluginEditor::OnTestMapPathChanged(const FString& NewTestMapPath)
{
    // Discover tests.
    AutomationTestFrameworkIntegration.SetTestMapPath(NewTestMapPath);
}

#undef LOCTEXT_NAMESPACE
