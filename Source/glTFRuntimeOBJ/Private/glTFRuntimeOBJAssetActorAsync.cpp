// Copyright 2023, Roberto De Ioris.

#include "glTFRuntimeOBJAssetActorAsync.h"
#include "glTFRuntimeOBJFunctionLibrary.h"

// Sets default values
AglTFRuntimeOBJAssetActorAsync::AglTFRuntimeOBJAssetActorAsync()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AssetRoot = CreateDefaultSubobject<USceneComponent>(TEXT("AssetRoot"));
	RootComponent = AssetRoot;
}

// Called when the game starts or when spawned
void AglTFRuntimeOBJAssetActorAsync::BeginPlay()
{
	Super::BeginPlay();

	if (!Asset)
	{
		return;
	}

	FglTFRuntimeOBJObjectNamesAsync Delegate;
	Delegate.BindDynamic(this, &AglTFRuntimeOBJAssetActorAsync::LoadObjectsAsync);
	UglTFRuntimeOBJFunctionLibrary::GetOBJObjectNamesAsync(Asset, Delegate);
}

// Called every frame
void AglTFRuntimeOBJAssetActorAsync::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AglTFRuntimeOBJAssetActorAsync::ReceiveOnStaticMeshComponentCreated_Implementation(UStaticMeshComponent* StaticMeshComponent)
{

}

void AglTFRuntimeOBJAssetActorAsync::ReceiveOnScenesLoaded_Implementation()
{

}

void AglTFRuntimeOBJAssetActorAsync::LoadNextMeshAsync()
{
	if (MeshesToLoad.Num() == 0)
	{
		return;
	}

	auto It = MeshesToLoad.CreateIterator();
	if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(It->Key))
	{
		CurrentPrimitiveComponent = StaticMeshComponent;
		FglTFRuntimeMeshLODAsync Delegate;
		Delegate.BindDynamic(this, &AglTFRuntimeOBJAssetActorAsync::LoadStaticMeshAsync);

		UglTFRuntimeOBJFunctionLibrary::LoadOBJAsRuntimeLODAsync(Asset, It->Value, Delegate, StaticMeshConfig.MaterialsConfig);
	}
}

void AglTFRuntimeOBJAssetActorAsync::LoadObjectsAsync(const TArray<FString>& Names)
{
	for (const FString& ObjectName : Names)
	{
		UStaticMeshComponent* StaticMeshComponent = NewObject<UStaticMeshComponent>(this, MakeUniqueObjectName(this, UStaticMeshComponent::StaticClass(), *ObjectName));
		StaticMeshComponent->SetupAttachment(GetRootComponent());
		StaticMeshComponent->RegisterComponent();
		AddInstanceComponent(StaticMeshComponent);
		MeshesToLoad.Add(StaticMeshComponent, ObjectName);

		StaticMeshComponent->ComponentTags.Add(*FString::Printf(TEXT("glTFRuntime:NodeName:%s"), *ObjectName));
		StaticMeshComponent->ComponentTags.Add(TEXT("glTFRuntime:Format:OBJ"));
	}

	if (MeshesToLoad.Num() == 0)
	{
		ReceiveOnScenesLoaded();
	}
	else
	{
		LoadNextMeshAsync();
	}
}

void AglTFRuntimeOBJAssetActorAsync::LoadStaticMeshAsync(const bool bValid, const FglTFRuntimeMeshLOD& RuntimeLOD)
{
	if (bValid)
	{
		UStaticMesh* StaticMesh = Asset->LoadStaticMeshFromRuntimeLODs({ RuntimeLOD }, StaticMeshConfig);
		if (StaticMesh)
		{
			CurrentPrimitiveComponent->SetStaticMesh(StaticMesh);
		}

		ReceiveOnStaticMeshComponentCreated(CurrentPrimitiveComponent);
	}

	MeshesToLoad.Remove(CurrentPrimitiveComponent);
	if (MeshesToLoad.Num() > 0)
	{
		LoadNextMeshAsync();
	}
	// trigger event
	else
	{
		ReceiveOnScenesLoaded();
	}
}