// Copyright (c) 2016-2018 Codice Software - Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"

class FPlasticSourceControlSettings
{
public:
	/** Get the Plastic Binary Path */
	const FString GetBinaryPath() const;

	/** Set the Plastic Binary Path */
	bool SetBinaryPath(const FString& InString);

	/** Load settings from ini file */
	void LoadSettings();

	/** Save settings to ini file */
	void SaveSettings() const;

private:
	/** A critical section for settings access */
	mutable FCriticalSection CriticalSection;

	/** Plastic binary path */
	FString BinaryPath;
};