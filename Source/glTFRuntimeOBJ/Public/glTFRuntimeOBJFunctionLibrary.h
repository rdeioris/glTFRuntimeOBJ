// Copyright 2023, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "glTFRuntimeAsset.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "glTFRuntimeOBJFunctionLibrary.generated.h"

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

	UFUNCTION(BlueprintCallable, meta = (AdvancedDisplay = "MaterialsConfig", AutoCreateRefTerm = "MaterialsConfig"), Category = "glTFRuntime|OBJ")
	static bool LoadOBJAsRuntimeLOD(UglTFRuntimeAsset* Asset, const FString& ObjectName, FglTFRuntimeMeshLOD& RuntimeLOD, const FglTFRuntimeMaterialsConfig& MaterialsConfig);
	
};
