// Copyright (c) 2016-2017 Codice Software - Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "PlasticSourceControlPrivatePCH.h"

#include "PlasticSourceControlMenu.h"

#include "PlasticSourceControlProvider.h"
#include "PlasticSourceControlOperations.h"

#include "LevelEditor.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Misc/MessageDialog.h"
#include "EditorStyleSet.h"


#include "Logging/MessageLog.h"

static const FName PlasticSourceControlMenuTabName("PlasticSourceControlMenu");

#define LOCTEXT_NAMESPACE "PlasticSourceControl"

void FPlasticSourceControlMenu::Register()
{
	// Register the extension with the level editor
	FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
	if (LevelEditorModule)
	{
		FLevelEditorModule::FLevelEditorMenuExtender ViewMenuExtender = FLevelEditorModule::FLevelEditorMenuExtender::CreateRaw(this, &FPlasticSourceControlMenu::OnExtendLevelEditorViewMenu);
		auto& MenuExtenders = LevelEditorModule->GetAllLevelEditorToolbarSourceControlMenuExtenders();
		MenuExtenders.Add(ViewMenuExtender);
		ViewMenuExtenderHandle = MenuExtenders.Last().GetHandle();
	}
}

void FPlasticSourceControlMenu::Unregister()
{
	// Unregister the level editor extensions
	FLevelEditorModule* LevelEditorModule = FModuleManager::GetModulePtr<FLevelEditorModule>("LevelEditor");
	if (LevelEditorModule)
	{
		LevelEditorModule->GetAllLevelEditorToolbarSourceControlMenuExtenders().RemoveAll([=](const FLevelEditorModule::FLevelEditorMenuExtender& Extender) { return Extender.GetHandle() == ViewMenuExtenderHandle; });
	}
}

bool FPlasticSourceControlMenu::IsSourceControlConnected() const
{
	const ISourceControlProvider& Provider = ISourceControlModule::Get().GetProvider();
	return Provider.IsEnabled() && Provider.IsAvailable();
}

void FPlasticSourceControlMenu::SyncProjectClicked()
{
	if (!SyncOperation.IsValid())
	{
		// Launch a "Sync" Operation
		ISourceControlModule& SourceControl = FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");
		FPlasticSourceControlProvider& Provider = static_cast<FPlasticSourceControlProvider&>(SourceControl.GetProvider());
		SyncOperation = ISourceControlOperation::Create<FSync>();
		TArray<FString> Files;
		Files.Add(Provider.GetPathToWorkspaceRoot()); // Sync the root of the workspace (needs an absolute path)
		ECommandResult::Type Result = Provider.Execute(SyncOperation.ToSharedRef(), Files, EConcurrency::Asynchronous, FSourceControlOperationComplete::CreateRaw(this, &FPlasticSourceControlMenu::OnSourceControlOperationComplete));
		if (Result == ECommandResult::Succeeded)
		{
			// Display an ongoing notification during the whole operation
			DisplayInProgressNotification(SyncOperation.ToSharedRef());
		}
		else
		{
			// Report failure with a notification
			DisplayFailureNotification(SyncOperation.ToSharedRef());
			SyncOperation.Reset();
		}
	}
	else
	{
		FMessageLog("LogSourceControl").Warning(LOCTEXT("SourceControlMenu_InProgress", "Source control operation already in progress"));
	}
}

void FPlasticSourceControlMenu::RevertUnchangedClicked()
{
	if (!RevertUnchangedOperation.IsValid())
	{
		// Launch a "RevertUnchanged" Operation
		ISourceControlModule& SourceControl = FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");
		FPlasticSourceControlProvider& Provider = static_cast<FPlasticSourceControlProvider&>(SourceControl.GetProvider());
		RevertUnchangedOperation = ISourceControlOperation::Create<FPlasticRevertUnchanged>();
		TArray<FString> Files;
		Files.Add(Provider.GetPathToWorkspaceRoot()); // Revert the root of the workspace (needs an absolute path)
		ECommandResult::Type Result = Provider.Execute(RevertUnchangedOperation.ToSharedRef(), Files, EConcurrency::Asynchronous, FSourceControlOperationComplete::CreateRaw(this, &FPlasticSourceControlMenu::OnSourceControlOperationComplete));
		if (Result == ECommandResult::Succeeded)
		{
			// Display an ongoing notification during the whole operation
			DisplayInProgressNotification(RevertUnchangedOperation.ToSharedRef());
		}
		else
		{
			// Report failure with a notification
			DisplayFailureNotification(RevertUnchangedOperation.ToSharedRef());
			RevertUnchangedOperation.Reset();
		}
	}
	else
	{
		FMessageLog("LogSourceControl").Warning(LOCTEXT("SourceControlMenu_InProgress", "Source control operation already in progress"));
	}
}

void FPlasticSourceControlMenu::RevertAllClicked()
{
	if (!RevertAllOperation.IsValid())
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
			Files.Add(Provider.GetPathToWorkspaceRoot()); // Revert the root of the workspace (needs an absolute path)
			ECommandResult::Type Result = Provider.Execute(RevertAllOperation.ToSharedRef(), Files, EConcurrency::Asynchronous, FSourceControlOperationComplete::CreateRaw(this, &FPlasticSourceControlMenu::OnSourceControlOperationComplete));
			if (Result == ECommandResult::Succeeded)
			{
				// Display an ongoing notification during the whole operation
				DisplayInProgressNotification(RevertAllOperation.ToSharedRef());
			}
			else
			{
				// Report failure with a notification
				DisplayFailureNotification(RevertAllOperation.ToSharedRef());
				RevertAllOperation.Reset();
			}
		}
	}
	else
	{
		FMessageLog("LogSourceControl").Warning(LOCTEXT("SourceControlMenu_InProgress", "Source control operation already in progress"));
	}
}

void FPlasticSourceControlMenu::RefreshClicked()
{
	if (!RefreshOperation.IsValid())
	{
		// Launch an "UpdateStatus" Operation
		ISourceControlModule& SourceControl = FModuleManager::LoadModuleChecked<ISourceControlModule>("SourceControl");
		FPlasticSourceControlProvider& Provider = static_cast<FPlasticSourceControlProvider&>(SourceControl.GetProvider());
		RefreshOperation = ISourceControlOperation::Create<FUpdateStatus>();
		RefreshOperation->SetCheckingAllFiles(true);
		RefreshOperation->SetGetOpenedOnly(true);
		ECommandResult::Type Result = Provider.Execute(RefreshOperation.ToSharedRef(), TArray<FString>(), EConcurrency::Asynchronous, FSourceControlOperationComplete::CreateRaw(this, &FPlasticSourceControlMenu::OnSourceControlOperationComplete));
		if (Result == ECommandResult::Succeeded)
		{
			// Display an ongoing notification during the whole operation
			DisplayInProgressNotification(RefreshOperation.ToSharedRef());
		}
		else
		{
			// Report failure with a notification
			DisplayFailureNotification(RefreshOperation.ToSharedRef());
			RefreshOperation.Reset();
		}
	}
	else
	{
		FMessageLog("LogSourceControl").Warning(LOCTEXT("SourceControlMenu_InProgress", "Source control operation already in progress"));
	}
}

// Display an ongoing notification during the whole operation
void FPlasticSourceControlMenu::DisplayInProgressNotification(const FSourceControlOperationRef& InOperation)
{
	if (!OperationInProgressNotification.IsValid())
	{
		FNotificationInfo Info(InOperation->GetInProgressString());
		Info.bFireAndForget = false;
		Info.ExpireDuration = 0.0f;
		Info.FadeOutDuration = 1.0f;
		OperationInProgressNotification = FSlateNotificationManager::Get().AddNotification(Info);
		if (OperationInProgressNotification.IsValid())
		{
			OperationInProgressNotification.Pin()->SetCompletionState(SNotificationItem::CS_Pending);
		}
	}
}

// Remove the ongoing notification at the end of the operation
void FPlasticSourceControlMenu::RemoveInProgressNotification()
{
	if (OperationInProgressNotification.IsValid())
	{
		OperationInProgressNotification.Pin()->ExpireAndFadeout();
		OperationInProgressNotification.Reset();
	}
}

// Display a temporary success notification at the end of the operation
void FPlasticSourceControlMenu::DisplaySucessNotification(const FSourceControlOperationRef& InOperation)
{
	const FText NotificationText = FText::Format(
		LOCTEXT("SourceControlMenu_Success", "{0} operation was successful!"),
		FText::FromName(InOperation->GetName())
	);
	FNotificationInfo Info(NotificationText);
	Info.bUseSuccessFailIcons = true;
	Info.Image = FEditorStyle::GetBrush(TEXT("NotificationList.SuccessImage"));
	FSlateNotificationManager::Get().AddNotification(Info);
	FMessageLog("LogSourceControl").Info(NotificationText);
}

// Display a temporary failure notification at the end of the operation
void FPlasticSourceControlMenu::DisplayFailureNotification(const FSourceControlOperationRef& InOperation)
{
	const FText NotificationText = FText::Format(
		LOCTEXT("SourceControlMenu_Failure", "Error: {0} operation failed!"),
		FText::FromName(InOperation->GetName())
	);
	FNotificationInfo Info(NotificationText);
	Info.ExpireDuration = 8.0f;
	FSlateNotificationManager::Get().AddNotification(Info);
	FMessageLog("LogSourceControl").Info(NotificationText);
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
	else if (InOperation->GetName() == "UpdateStatus")
	{
		check(RefreshOperation == StaticCastSharedRef<FUpdateStatus>(InOperation));
		RefreshOperation.Reset();
	}
	else
	{
		UE_LOG(LogSourceControl, Error, TEXT("Unknown operation '%s'"), *InOperation->GetName().ToString());
	}

	RemoveInProgressNotification();

	// Report result with a notification
	if (InResult == ECommandResult::Succeeded)
	{
		DisplaySucessNotification(InOperation);
	}
	else
	{
		DisplayFailureNotification(InOperation);
	}
}

void FPlasticSourceControlMenu::AddMenuExtension(FMenuBuilder& Builder)
{
	Builder.AddMenuEntry(
		LOCTEXT("PlasticSync",			"Sync/Update Workspace"),
		LOCTEXT("PlasticSyncTooltip",	"Update all files in the workspace to the latest version."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "SourceControl.Actions.Sync"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FPlasticSourceControlMenu::SyncProjectClicked),
			FCanExecuteAction::CreateRaw(this, &FPlasticSourceControlMenu::IsSourceControlConnected)
		)
	);

	Builder.AddMenuEntry(
		LOCTEXT("PlasticRevertUnchanged",			"Revert Unchanged"),
		LOCTEXT("PlasticRevertUnchangedTooltip",	"Revert checked-out but unchanged files in the workspace."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "SourceControl.Actions.Revert"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FPlasticSourceControlMenu::RevertUnchangedClicked),
			FCanExecuteAction()
		)
	);

	Builder.AddMenuEntry(
		LOCTEXT("PlasticRevertAll",			"Revert All"),
		LOCTEXT("PlasticRevertAllTooltip",	"Revert all files in the workspace to their controlled/unchanged state."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "SourceControl.Actions.Revert"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FPlasticSourceControlMenu::RevertAllClicked),
			FCanExecuteAction()
		)
	);

	Builder.AddMenuEntry(
		LOCTEXT("PlasticRefresh",			"Refresh All"),
		LOCTEXT("PlasticRefreshTooltip",	"Update the source control status of all files in the workspace."),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "SourceControl.Actions.Refresh"),
		FUIAction(
			FExecuteAction::CreateRaw(this, &FPlasticSourceControlMenu::RefreshClicked),
			FCanExecuteAction()
		)
	);
}

TSharedRef<FExtender> FPlasticSourceControlMenu::OnExtendLevelEditorViewMenu(const TSharedRef<FUICommandList> CommandList)
{
	TSharedRef<FExtender> Extender(new FExtender());

	Extender->AddMenuExtension(
		"SourceControlActions",
		EExtensionHook::After,
		nullptr,
		FMenuExtensionDelegate::CreateRaw(this, &FPlasticSourceControlMenu::AddMenuExtension));

	return Extender;
}

#undef LOCTEXT_NAMESPACE
