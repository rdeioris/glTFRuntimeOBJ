// Copyright 2023, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "glTFRuntimeAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "glTFRuntimeOBJFunctionLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FglTFRuntimeOBJObjectNamesAsync, const TArray<FString>&, ObjectNames);

/**
 * 
 */
UCLASS()
class GLTFRUNTIMEOBJ_API UglTFRuntimeOBJFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "glTFRuntime|OBJ")
	static TArray<FString> GetOBJObjectNames(UglTFRuntimeAsset* Asset);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "glTFRuntime|OBJ")
	static void GetOBJObjectNamesAsync(UglTFRuntimeAsset* Asset, const FglTFRuntimeOBJObjectNamesAsync& AsyncCallback);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "MaterialsConfig", AutoCreateRefTerm = "MaterialsConfig"), Category = "glTFRuntime|OBJ")
	static bool LoadOBJAsRuntimeLOD(UglTFRuntimeAsset* Asset, const FString& ObjectName, FglTFRuntimeMeshLOD& RuntimeLOD, const FglTFRuntimeMaterialsConfig& MaterialsConfig);

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "MaterialsConfig", AutoCreateRefTerm = "MaterialsConfig"), Category = "glTFRuntime|OBJ")
	static void LoadOBJAsRuntimeLODAsync(UglTFRuntimeAsset* Asset, const FString& ObjectName, const FglTFRuntimeMeshLODAsync& AsyncCallback, const FglTFRuntimeMaterialsConfig& MaterialsConfig);
	
};
