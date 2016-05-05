// Copyright (c) 2016 Codice Software - Sebastien Rombauts (sebastien.rombauts@gmail.com)

#include "PlasticSourceControlPrivatePCH.h"
#include "SPlasticSourceControlSettings.h"
#include "PlasticSourceControlModule.h"
#include "PlasticSourceControlUtils.h"

#define LOCTEXT_NAMESPACE "SPlasticSourceControlSettings"

void SPlasticSourceControlSettings::Construct(const FArguments& InArgs)
{
	FSlateFontInfo Font = FEditorStyle::GetFontStyle(TEXT("SourceControl.LoginWindow.Font"));

	bAutoCreateIgnoreFile = true;
	bAutoInitialCommit = true;

	InitialCommitMessage = LOCTEXT("InitialCommitMessage", "Initial chekin");
	ServerUrl = LOCTEXT("DefaultServerUrl", "localhost:8087");

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage( FEditorStyle::GetBrush("DetailsView.CategoryBottom"))
		.Padding(FMargin(0.0f, 3.0f, 0.0f, 0.0f))
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(1.5f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("BinaryPathLabel", "Plastic SCM Path"))
						.ToolTipText(LOCTEXT("BinaryPathLabel_Tooltip", "Path to Plastic SCM binary"))
						.Font(Font)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					[
						SNew(SEditableTextBox)
						.Text(this, &SPlasticSourceControlSettings::GetBinaryPathText)
						.ToolTipText(LOCTEXT("BinaryPathLabel_Tooltip", "Path to Plastic SCM binary"))
						.OnTextCommitted(this, &SPlasticSourceControlSettings::OnBinaryPathTextCommited)
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WorkspaceRootLabel", "Root of the workspace"))
						.ToolTipText(LOCTEXT("WorkspaceRootLabel_Tooltip", "Path to the root of the Plastic SCM workspace"))
						.Font(Font)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					[
						SNew(STextBlock)
						.Text(this, &SPlasticSourceControlSettings::GetPathToWorkspaceRoot)
						.ToolTipText(LOCTEXT("WorkspaceRootLabel_Tooltip", "Path to the root of the Plastic SCM workspace"))
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("PlasticUserName", "User Name"))
						.ToolTipText(LOCTEXT("PlasticUserName_Tooltip", "User name configured for the Plastic SCM workspace"))
						.Font(Font)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					[
						SNew(STextBlock)
						.Text(this, &SPlasticSourceControlSettings::GetUserName)
						.ToolTipText(LOCTEXT("PlasticUserName_Tooltip", "User name configured for the Plastic SCM workspace"))
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.5f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SPlasticSourceControlSettings::CanInitializePlasticWorkspace)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("WorkspaceName", "Workspace Name"))
						.ToolTipText(LOCTEXT("WorkspaceName_Tooltip", "Set the Workspace Name"))
						.Font(Font)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.5f)
					.Padding(2.0f)
					[
						SNew(SEditableTextBox)
						.Text(this, &SPlasticSourceControlSettings::GetWorkspaceName)
						.ToolTipText(LOCTEXT("WorkspaceName_Tooltip", "Set the Workspace Name"))
						.HintText(LOCTEXT("WorkspaceName", "Workspace Name"))
						.OnTextCommitted(this, &SPlasticSourceControlSettings::OnWorkspaceNameCommited)
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.5f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SPlasticSourceControlSettings::CanInitializePlasticWorkspace)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("RepositoryName", "Repository Name"))
						.ToolTipText(LOCTEXT("RepositoryName_Tooltip", "Set the Repository Name"))
						.Font(Font)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					[
						SNew(SEditableTextBox)
						.Text(this, &SPlasticSourceControlSettings::GetRepositoryName)
						.ToolTipText(LOCTEXT("RepositoryName_Tooltip", "Set the Repository Name"))
						.HintText(LOCTEXT("RepositoryName", "Repository Name"))
						.OnTextCommitted(this, &SPlasticSourceControlSettings::OnRepositoryNameCommited)
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.5f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SPlasticSourceControlSettings::CanInitializePlasticWorkspace)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ServerUrl", "Server URL address:port"))
						.ToolTipText(LOCTEXT("ServerUrl_Tooltip", "Set the Server URL in the form address:port (localhost:8084)"))
						.Font(Font)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					[
						SNew(SEditableTextBox)
						.Text(this, &SPlasticSourceControlSettings::GetServerUrl)
						.ToolTipText(LOCTEXT("ServerUrl_Tooltip", "Set the Server URL in the form address:port (localhost:8084)"))
						.HintText(LOCTEXT("ServerUrl", "Set the Workspace Name"))
						.OnTextCommitted(this, &SPlasticSourceControlSettings::OnServerUrlCommited)
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.5f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SPlasticSourceControlSettings::CanInitializePlasticWorkspace)
				+SHorizontalBox::Slot()
				.FillWidth(0.1f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SCheckBox)
						.ToolTipText(LOCTEXT("CreateIgnoreFile_Tooltip", "Create and add a standard 'ignore.conf' file"))
						.IsChecked(ECheckBoxState::Checked)
						.OnCheckStateChanged(this, &SPlasticSourceControlSettings::OnCheckedCreateIgnoreFile)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.9f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CreateIgnoreFile", "Add a ignore.conf file"))
						.ToolTipText(LOCTEXT("CreateIgnoreFile_Tooltip", "Create and add a standard 'ignore.conf' file"))
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(1.5f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SPlasticSourceControlSettings::CanInitializePlasticWorkspace)
				+SHorizontalBox::Slot()
				.FillWidth(0.1f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SCheckBox)
						.ToolTipText(LOCTEXT("InitialCommit_Tooltip", "Make the initial Plastic SCM commit"))
						.IsChecked(ECheckBoxState::Checked)
						.OnCheckStateChanged(this, &SPlasticSourceControlSettings::OnCheckedInitialCommit)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(0.9f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("InitialCommit", "Make the initial Plastic SCM Commit"))
						.ToolTipText(LOCTEXT("InitialCommit_Tooltip", "Make the initial Plastic SCM commit"))
						.Font(Font)
					]
				]
				+SHorizontalBox::Slot()
				.FillWidth(2.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(1.5f)
					.Padding(2.0f)
					[
						SNew(SMultiLineEditableTextBox)
						.Text(this, &SPlasticSourceControlSettings::GetInitialCommitMessage)
						.ToolTipText(LOCTEXT("InitialCommitMessage_Tooltip", "Message of initial commit"))
						.HintText(LOCTEXT("InitialCommitMessage_Tooltip", "Message of initial commit"))
						.OnTextCommitted(this, &SPlasticSourceControlSettings::OnInitialCommitMessageCommited)
						.Font(Font)
					]
				]
			]
			+SVerticalBox::Slot()
			.FillHeight(2.0f)
			.Padding(2.0f)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				.Visibility(this, &SPlasticSourceControlSettings::CanInitializePlasticWorkspace)
				+SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.FillHeight(2.0f)
					.Padding(2.0f)
					.VAlign(VAlign_Center)
					.AutoHeight()
					[
						SNew(SButton)
						.IsEnabled(this, &SPlasticSourceControlSettings::IsReadyToInitializePlasticWorkspace)
						.Text(LOCTEXT("PlasticInitWorkspace", "Initialize project with Plastic SCM"))
						.ToolTipText(LOCTEXT("PlasticInitWorkspace_Tooltip", "Initialize current project as a new Plastic SCM workspace"))
						.OnClicked(this, &SPlasticSourceControlSettings::OnClickedInitializePlasticWorkspace)
						.HAlign(HAlign_Center)
					]
				]
			]
		]
	];
}

FText SPlasticSourceControlSettings::GetBinaryPathText() const
{
	FPlasticSourceControlModule& PlasticSourceControl = FModuleManager::LoadModuleChecked<FPlasticSourceControlModule>("PlasticSourceControl");
	return FText::FromString(PlasticSourceControl.AccessSettings().GetBinaryPath());
}

void SPlasticSourceControlSettings::OnBinaryPathTextCommited(const FText& InText, ETextCommit::Type InCommitType) const
{
	FPlasticSourceControlModule& PlasticSourceControl = FModuleManager::LoadModuleChecked<FPlasticSourceControlModule>("PlasticSourceControl");
	PlasticSourceControl.AccessSettings().SetBinaryPath(InText.ToString());
	PlasticSourceControl.SaveSettings();
}

FText SPlasticSourceControlSettings::GetPathToWorkspaceRoot() const
{
	FPlasticSourceControlModule& PlasticSourceControl = FModuleManager::LoadModuleChecked<FPlasticSourceControlModule>("PlasticSourceControl");
	return FText::FromString(PlasticSourceControl.GetProvider().GetPathToWorkspaceRoot());
}

FText SPlasticSourceControlSettings::GetUserName() const
{
	FPlasticSourceControlModule& PlasticSourceControl = FModuleManager::LoadModuleChecked<FPlasticSourceControlModule>("PlasticSourceControl");
	return FText::FromString(PlasticSourceControl.GetProvider().GetUserName());
}


EVisibility SPlasticSourceControlSettings::CanInitializePlasticWorkspace() const
{
	FPlasticSourceControlModule& PlasticSourceControl = FModuleManager::LoadModuleChecked<FPlasticSourceControlModule>("PlasticSourceControl");
	const bool bPlasticAvailable = PlasticSourceControl.GetProvider().IsEnabled();
	const bool bPlasticWorkspaceFound = PlasticSourceControl.GetProvider().IsWorkspaceFound();
	return (bPlasticAvailable && !bPlasticWorkspaceFound) ? EVisibility::Visible : EVisibility::Collapsed;
}

bool SPlasticSourceControlSettings::IsReadyToInitializePlasticWorkspace() const
{
	// Workspace Name cannot be left empty
	const bool bWorkspaceNameOk = !WorkspaceName.IsEmpty();
	// Either RepositoryName should be empty, or ServerUrl should also be filled
	const bool bRepositoryNameOk = RepositoryName.IsEmpty() || !ServerUrl.IsEmpty();
	// If Initial Commit is requested, commit message cannot be empty
	const bool bInitialCommitOk = (!bAutoInitialCommit || !InitialCommitMessage.IsEmpty());
	return bWorkspaceNameOk && bRepositoryNameOk && bInitialCommitOk;
}


void SPlasticSourceControlSettings::OnWorkspaceNameCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	WorkspaceName = InText;
}
FText SPlasticSourceControlSettings::GetWorkspaceName() const
{
	return WorkspaceName;
}

void SPlasticSourceControlSettings::OnRepositoryNameCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	RepositoryName = InText;
}
FText SPlasticSourceControlSettings::GetRepositoryName() const
{
	return RepositoryName;
}

void SPlasticSourceControlSettings::OnServerUrlCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	ServerUrl = InText;
}
FText SPlasticSourceControlSettings::GetServerUrl() const
{
	return ServerUrl;
}

void SPlasticSourceControlSettings::OnCheckedCreateIgnoreFile(ECheckBoxState NewCheckedState)
{
	bAutoCreateIgnoreFile = (NewCheckedState == ECheckBoxState::Checked);
}

void SPlasticSourceControlSettings::OnCheckedInitialCommit(ECheckBoxState NewCheckedState)
{
	bAutoInitialCommit = (NewCheckedState == ECheckBoxState::Checked);
}

void SPlasticSourceControlSettings::OnInitialCommitMessageCommited(const FText& InText, ETextCommit::Type InCommitType)
{
	InitialCommitMessage = InText;
}

FText SPlasticSourceControlSettings::GetInitialCommitMessage() const
{
	return InitialCommitMessage;
}


FReply SPlasticSourceControlSettings::OnClickedInitializePlasticWorkspace()
{
	FPlasticSourceControlModule& PlasticSourceControl = FModuleManager::LoadModuleChecked<FPlasticSourceControlModule>("PlasticSourceControl");
	const FString& PathToPlasticBinary = PlasticSourceControl.AccessSettings().GetBinaryPath();
	const FString PathToGameDir = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
	TArray<FString> InfoMessages;
	TArray<FString> ErrorMessages;
	bool bResult;
	if (!RepositoryName.IsEmpty())
	{
		TArray<FString> Parameters;
		Parameters.Add(ServerUrl.ToString());
		Parameters.Add(RepositoryName.ToString());
		PlasticSourceControlUtils::RunCommand(TEXT("makerepository"), Parameters, TArray<FString>(), InfoMessages, ErrorMessages);
	}
	{
		TArray<FString> Parameters;
		Parameters.Add(WorkspaceName.ToString());
		TArray<FString> Files;
		Files.Add(PathToGameDir);
		if (!RepositoryName.IsEmpty())
		{
			// working only if repository already exists
			Parameters.Add(FString::Printf(TEXT("--repository=rep:%s@repserver:%s"), *RepositoryName.ToString(), *ServerUrl.ToString()));
		}
		bResult = PlasticSourceControlUtils::RunCommand(TEXT("makeworkspace"), Parameters, Files, InfoMessages, ErrorMessages);
	}
	if (bResult)
	{
		// Check the new workspace status to enable connection
		PlasticSourceControl.GetProvider().CheckPlasticAvailability();
	}
	if (PlasticSourceControl.GetProvider().IsWorkspaceFound())
	{
		TArray<FString> ProjectFiles;
		ProjectFiles.Add(FPaths::ConvertRelativePathToFull(FPaths::GetProjectFilePath()));
		ProjectFiles.Add(FPaths::ConvertRelativePathToFull(FPaths::GameConfigDir()));
		ProjectFiles.Add(FPaths::ConvertRelativePathToFull(FPaths::GameContentDir()));
		if (FPaths::DirectoryExists(FPaths::GameSourceDir()))
		{
			ProjectFiles.Add(FPaths::ConvertRelativePathToFull(FPaths::GameSourceDir()));
		}
		if (bAutoCreateIgnoreFile)
		{
			// Create a standard "ignore.conf" file with common patterns for a typical Blueprint & C++ project
			const FString Filename = FPaths::Combine(*PathToGameDir, TEXT("ignore.conf"));
			const FString IgnoreFileContent = TEXT("Binaries\nBuild\nDerivedDataCache\nIntermediate\nSaved\n*.VC.db\n*.opensdf\n*.opendb\n*.sdf\n*.sln\n*.suo\n*.xcodeproj\n*.xcworkspace");
			if (FFileHelper::SaveStringToFile(IgnoreFileContent, *Filename, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
			{
				ProjectFiles.Add(Filename);
			}
		}
		// Add .uproject, Config/, Content/ and Source/ files (and ignore.conf if any)
		TArray<FString> Parameters;
		Parameters.Add(TEXT("-R"));
		bResult = PlasticSourceControlUtils::RunCommand(TEXT("add"), Parameters, ProjectFiles, InfoMessages, ErrorMessages);
		if (bAutoInitialCommit && bResult)
		{
			// optionnal initial commit with custom message
			TArray<FString> Parameters;
			FString ParamCommitMsg = TEXT("-c=\"");
			ParamCommitMsg += InitialCommitMessage.ToString();
			ParamCommitMsg += TEXT("\"");
			Parameters.Add(ParamCommitMsg);
			TArray<FString> Files;
			Files.Add(PathToGameDir);
			PlasticSourceControlUtils::RunCommand(TEXT("checkin"), Parameters, Files, InfoMessages, ErrorMessages);
		}
	}
	return FReply::Handled();
}


// TODO Delegate to add a Plastic ignore.conf file to an existing Plastic SCM workspace
EVisibility SPlasticSourceControlSettings::CanAddIgnoreFile() const
{
	FPlasticSourceControlModule& PlasticSourceControl = FModuleManager::LoadModuleChecked<FPlasticSourceControlModule>("PlasticSourceControl");
	const bool bPlasticWorkspaceFound = PlasticSourceControl.GetProvider().IsEnabled();
	const bool bIgnoreFileFound = false; // TODO
	return (bPlasticWorkspaceFound && !bIgnoreFileFound) ? EVisibility::Visible : EVisibility::Collapsed;
}
FReply SPlasticSourceControlSettings::OnClickedAddIgnoreFile()
{
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE
