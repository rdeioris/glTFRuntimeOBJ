// Copyright 2023, Roberto De Ioris.

#include "glTFRuntimeOBJAssetActor.h"
#include "glTFRuntimeOBJFunctionLibrary.h"

// Sets default values
AglTFRuntimeOBJAssetActor::AglTFRuntimeOBJAssetActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AssetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("AssetRoot"));
	RootComponent = AssetRoot;
}

// Called when the game starts or when spawned
void AglTFRuntimeOBJAssetActor::BeginPlay()
{
	Super::BeginPlay();

	if (!Asset)
	{
		return;
	}

	TArray<FString> ObjectNames = UglTFRuntimeOBJFunctionLibrary::GetOBJObjectNames(Asset);

	for (const FString& ObjectName : ObjectNames)
	{
		FglTFRuntimeMeshLOD LOD;
		if (UglTFRuntimeOBJFunctionLibrary::LoadOBJAsRuntimeLOD(Asset, ObjectName, LOD, StaticMeshConfig.MaterialsConfig))
		{
			UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this, MakeUniqueObjectName(this, UStaticMeshComponent::StaticClass(), *ObjectName));
			StaticMeshComponent->SetupAttachment(GetRootComponent());
			StaticMeshComponent->RegisterComponent();
			AddInstanceComponent(StaticMeshComponent);

			StaticMeshComponent->ComponentTags.Add(*FString::Printf(TEXT("glTFRuntime:NodeName:%s"), *ObjectName));
			StaticMeshComponent->ComponentTags.Add(TEXT("glTFRuntime:Format:OBJ"));

			UStaticMesh* StaticMesh = Asset->LoadStaticMeshFromRuntimeLODs({ LOD }, StaticMeshConfig);
			if (StaticMesh)
			{
				StaticMeshComponent->SetStaticMesh(StaticMesh);
			}

			ReceiveOnStaticMeshComponentCreated(StaticMeshComponent);
		}
	}

	ReceiveOnScenesLoaded();
}

// Called every frame
void AglTFRuntimeOBJAssetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AglTFRuntimeOBJAssetActor::ReceiveOnStaticMeshComponentCreated_Implementation(UStaticMeshComponent* StaticMeshComponent)
{

}

void AglTFRuntimeOBJAssetActor::ReceiveOnScenesLoaded_Implementation()
{

}