// Copyright 2023, Roberto De Ioris.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "glTFRuntimeAsset.h"
#include "glTFRuntimeOBJAssetActor.generated.h"

UCLASS()
class GLTFRUNTIMEOBJ_API AglTFRuntimeOBJAssetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AglTFRuntimeOBJAssetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|OBJ")
	UglTFRuntimeAsset* Asset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Meta = (ExposeOnSpawn = true), Category = "glTFRuntime|OBJ")
	FglTFRuntimeStaticMeshConfig StaticMeshConfig;

	UFUNCTION(BlueprintNativeEvent, Category = "glTFRuntime|OBJ", meta = (DisplayName = "On StaticMeshComponent Created"))
	void ReceiveOnStaticMeshComponentCreated(UStaticMeshComponent* StaticMeshComponent);

	UFUNCTION(BlueprintNativeEvent, Category = "glTFRuntime|OBJ", meta = (DisplayName = "On Scenes Loaded"))
		void ReceiveOnScenesLoaded();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category = "glTFRuntime|OBJ")
	USceneComponent* AssetRoot;

};
