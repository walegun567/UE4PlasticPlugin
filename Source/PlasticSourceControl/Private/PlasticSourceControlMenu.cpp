// Copyright (c) 2016 Codice Software - Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "PlasticSourceControlPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"

#include "PlasticSourceControlMenu.h"
#include "PlasticSourceControlMenuStyle.h"
#include "PlasticSourceControlMenuCommands.h"

#include "PlasticSourceControlProvider.h"
#include "PlasticSourceControlOperations.h"

#include "LevelEditor.h"

static const FName PlasticSourceControlMenuTabName("PlasticSourceControlMenu");

#define LOCTEXT_NAMESPACE "PlasticSourceControl"

void FPlasticSourceControlMenu::Register()
{
	FPlasticSourceControlMenuStyle::Initialize();
	FPlasticSourceControlMenuStyle::ReloadTextures();

	FPlasticSourceControlMenuCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FPlasticSourceControlMenuCommands::Get().SyncProject,
		FExecuteAction::CreateRaw(this, &FPlasticSourceControlMenu::SyncProjectClicked),
		FCanExecuteAction());

	PluginCommands->MapAction(
		FPlasticSourceControlMenuCommands::Get().RevertUnchanged,
		FExecuteAction::CreateRaw(this, &FPlasticSourceControlMenu::RevertUnchangedClicked),
		FCanExecuteAction());

	PluginCommands->MapAction(
		FPlasticSourceControlMenuCommands::Get().RevertAll,
		FExecuteAction::CreateRaw(this, &FPlasticSourceControlMenu::RevertAllClicked),
		FCanExecuteAction());

	// Register the extension with the level editor
	{
		FLevelEditorModule::FLevelEditorMenuExtender ViewMenuExtender = FLevelEditorModule::FLevelEditorMenuExtender::CreateRaw(this, &FPlasticSourceControlMenu::OnExtendLevelEditorViewMenu);
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		auto& MenuExtenders = LevelEditorModule.GetAllLevelEditorToolbarSourceControlMenuExtenders();
		MenuExtenders.Add(ViewMenuExtender);
		ViewMenuExtenderHandle = MenuExtenders.Last().GetHandle();
	}
}

void FPlasticSourceControlMenu::Unregister()
{
	// Unregister the level editor extensions
	{
		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		LevelEditorModule.GetAllLevelEditorToolbarSourceControlMenuExtenders().RemoveAll([=](const FLevelEditorModule::FLevelEditorMenuExtender& Extender) { return Extender.GetHandle() == ViewMenuExtenderHandle; });
	}

	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	FPlasticSourceControlMenuStyle::Shutdown();

	FPlasticSourceControlMenuCommands::Unregister();
}

void FPlasticSourceControlMenu::SyncProjectClicked()
{
	if (!SyncOperation.IsValid() && !OperationInProgressNotification.IsValid())
	{
		// Launch a "Sync" Operation
		ISourceControlModule& SourceControl = FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");
		FPlasticSourceControlProvider& Provider = static_cast<FPlasticSourceControlProvider&>(SourceControl.GetProvider());
		SyncOperation = ISourceControlOperation::Create<FSync>();
		TArray<FString> Files;
		Files.Add(Provider.GetPathToWorkspaceRoot() / TEXT("")); // Sync the root of the workspace
		Provider.Execute(SyncOperation.ToSharedRef(), Files, EConcurrency::Asynchronous, FSourceControlOperationComplete::CreateRaw(this, &FPlasticSourceControlMenu::OnSourceControlOperationComplete));

		// Display an ongoing notification during the whole operation
		DisplayInProgressNotification(SyncOperation.ToSharedRef());
	}
	else
	{
		UE_LOG(LogSourceControl, Warning, TEXT("Source control operation already in progress!"));
	}
}

void FPlasticSourceControlMenu::RevertUnchangedClicked()
{
	if (!RevertUnchangedOperation.IsValid() && !OperationInProgressNotification.IsValid())
	{
		// Launch a "RevertUnchanged" Operation
		ISourceControlModule& SourceControl = FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");
		FPlasticSourceControlProvider& Provider = static_cast<FPlasticSourceControlProvider&>(SourceControl.GetProvider());
		RevertUnchangedOperation = ISourceControlOperation::Create<FPlasticRevertUnchanged>();
		TArray<FString> Files;
		Files.Add(Provider.GetPathToWorkspaceRoot() / TEXT("")); // Revert the root of the workspace
		Provider.Execute(RevertUnchangedOperation.ToSharedRef(), Files, EConcurrency::Asynchronous, FSourceControlOperationComplete::CreateRaw(this, &FPlasticSourceControlMenu::OnSourceControlOperationComplete));

		// Display an ongoing notification during the whole operation
		DisplayInProgressNotification(RevertUnchangedOperation.ToSharedRef());
	}
	else
	{
		UE_LOG(LogSourceControl, Warning, TEXT("Source control operation already in progress!"));
	}
}

void FPlasticSourceControlMenu::RevertAllClicked()
{
	if (!RevertAllOperation.IsValid() && !OperationInProgressNotification.IsValid())
	{
		// Ask the user before reverting all!
		const FText DialogText(LOCTEXT("SourceControlMenu_AskRevertAll", "Revert all modifications into the workspace?"));
		const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::OkCancel, DialogText);
		if (Choice == EAppReturnType::Ok)
		{
			// Launch a "RevertAll" Operation
			ISourceControlModule& SourceControl = FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");
			FPlasticSourceControlProvider& Provider = static_cast<FPlasticSourceControlProvider&>(SourceControl.GetProvider());
			RevertAllOperation = ISourceControlOperation::Create<FPlasticRevertAll>();
			TArray<FString> Files;
			Files.Add(Provider.GetPathToWorkspaceRoot() / TEXT("")); // Revert the root of the workspace
			Provider.Execute(RevertAllOperation.ToSharedRef(), Files, EConcurrency::Asynchronous, FSourceControlOperationComplete::CreateRaw(this, &FPlasticSourceControlMenu::OnSourceControlOperationComplete));

			// Display an ongoing notification during the whole operation
			DisplayInProgressNotification(RevertAllOperation.ToSharedRef());
		}
	}
	else
	{
		UE_LOG(LogSourceControl, Warning, TEXT("Source control operation already in progress!"));
	}
}

void FPlasticSourceControlMenu::DisplayInProgressNotification(const FSourceControlOperationRef& InOperationInProgress)
{
	// Display an ongoing notification during the whole operation
	FNotificationInfo Info(InOperationInProgress->GetInProgressString());
	Info.bFireAndForget = false;
	Info.ExpireDuration = 0.0f;
	Info.FadeOutDuration = 1.0f;
	OperationInProgressNotification = FSlateNotificationManager::Get().AddNotification(Info);
	if (OperationInProgressNotification.IsValid())
	{
		OperationInProgressNotification.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
	}
}

void FPlasticSourceControlMenu::OnSourceControlOperationComplete(const FSourceControlOperationRef& InOperation, ECommandResult::Type InResult)
{
	if (InOperation->GetName() == "Sync")
	{
		check(SyncOperation == StaticCastSharedRef<FSync>(InOperation));
		SyncOperation.Reset();
	}
	else if (InOperation->GetName() == "RevertUnchanged")
	{
		check(RevertUnchangedOperation == StaticCastSharedRef<FPlasticRevertUnchanged>(InOperation));
		RevertUnchangedOperation.Reset();
	}
	else if (InOperation->GetName() == "RevertAll")
	{
		check(RevertAllOperation == StaticCastSharedRef<FPlasticRevertAll>(InOperation));
		RevertAllOperation.Reset();
	}

	if (OperationInProgressNotification.IsValid())
	{
		OperationInProgressNotification.Pin()->ExpireAndFadeout();
		OperationInProgressNotification.Reset();
	}

	if (InResult == ECommandResult::Succeeded)
	{
		// report success with a notification
		const FText NotificationText = FText::Format(
			LOCTEXT("SourceControlMenu_Success", "{0} operation was successfull!"),
			FText::FromName(InOperation->GetName())
		);
		FNotificationInfo Info(NotificationText);
		Info.bUseSuccessFailIcons = true;
		Info.Image = FEditorStyle::GetBrush(TEXT("NotificationList.SuccessImage"));
		FSlateNotificationManager::Get().AddNotification(Info);
		UE_LOG(LogSourceControl, Log, TEXT("%s"), *NotificationText.ToString());
	}
	else
	{
		// report failure with a notification
		const FText NotificationText = FText::Format(
			LOCTEXT("SourceControlMenu_Failure", "Error: {0} operation failed!"),
			FText::FromName(InOperation->GetName())
		);
		FNotificationInfo Info(NotificationText);
		Info.ExpireDuration = 8.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		UE_LOG(LogSourceControl, Log, TEXT("%s"), *NotificationText.ToString());
	}
}

void FPlasticSourceControlMenu::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(FPlasticSourceControlMenuCommands::Get().SyncProject);
	Builder.AddMenuEntry(FPlasticSourceControlMenuCommands::Get().RevertUnchanged);
	Builder.AddMenuEntry(FPlasticSourceControlMenuCommands::Get().RevertAll);
}

TSharedRef<FExtender> FPlasticSourceControlMenu::OnExtendLevelEditorViewMenu(const TSharedRef<FUICommandList> CommandList)
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"SourceControlActions",
		EExtensionHook::After,
		PluginCommands,
		FMenuExtensionDelegate::CreateRaw(this, &FPlasticSourceControlMenu::AddMenuExtension));

	return Extender;
}

#undef LOCTEXT_NAMESPACE
