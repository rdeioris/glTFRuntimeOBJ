// Copyright 2023 Roberto De Ioris.


#include "glTFRuntimeOBJFunctionLibrary.h"


struct FglTFRuntimeOBJCacheData : FglTFRuntimePluginCacheData
{
	TArray<TArray<FString>> GeometryLines;
	TArray<TArray<FString>> MaterialLines;
};

namespace glTFRuntimeOBJ
{
	FString GetRemainingString(const TArray<FString>& Line, const int32 Index)
	{
		FString NewString;
		if (!Line.IsValidIndex(Index))
		{
			return NewString;
		}

		for (int32 CurrentIndex = Index; CurrentIndex < Line.Num(); CurrentIndex++)
		{
			if (CurrentIndex == Index)
			{
				NewString += Line[CurrentIndex];
			}
			else
			{
				NewString += " " + Line[CurrentIndex];
			}
		}
		return NewString;
	}

	void FillLinesFromBlob(const TArray64<uint8>& Blob, TArray<TArray<FString>>& Lines)
	{
		TArray<FString> CurrentLine;
		FString CurrentString;

		for (int64 Index = 0; Index < Blob.Num(); Index++)
		{
			const char Char = static_cast<char>(Blob[Index]);
			if (Char == ' ' || Char == '\t' || Char == '\r' || Char == '\n')
			{
				if (!CurrentString.IsEmpty())
				{
					CurrentLine.Add(CurrentString);
				}
				CurrentString = "";
				if (Char == '\r' || Char == '\n')
				{
					if (CurrentLine.Num() > 0)
					{
						Lines.Add(CurrentLine);
					}
					CurrentLine.Empty();
				}
			}
			else
			{
				CurrentString += Char;
			}
		}
	}

	TSharedPtr<FglTFRuntimeOBJCacheData> GetCacheData(UglTFRuntimeAsset* Asset)
	{
		if (Asset->GetParser()->PluginsCacheData.Contains("OBJ"))
		{
			if (Asset->GetParser()->PluginsCacheData["OBJ"] && Asset->GetParser()->PluginsCacheData["OBJ"]->bValid)
			{
				return StaticCastSharedPtr<FglTFRuntimeOBJCacheData>(Asset->GetParser()->PluginsCacheData["OBJ"]);
			}
		}

		if (!Asset->GetParser()->PluginsCacheData.Contains("OBJ"))
		{
			Asset->GetParser()->PluginsCacheData.Add("OBJ", MakeShared<FglTFRuntimeOBJCacheData>());
		}

		TArray64<uint8> ArchiveBlob;

		const TArray64<uint8>* Blob = nullptr;

		if (Asset->IsArchive())
		{
			for (const FString& Name : Asset->GetArchiveItems())
			{
				if (Name.EndsWith(".obj"))
				{
					if (!Asset->GetParser()->GetBlobByName(Name, ArchiveBlob))
					{
						return nullptr;
					}

					Blob = &ArchiveBlob;
					break;
				}
			}
		}
		else
		{
			Blob = &(Asset->GetParser()->GetBlob());
		}

		if (!Blob)
		{
			return nullptr;
		}

		TSharedPtr<FglTFRuntimeOBJCacheData> RuntimeOBJCacheData = StaticCastSharedPtr<FglTFRuntimeOBJCacheData>(Asset->GetParser()->PluginsCacheData["OBJ"]);

		FillLinesFromBlob(*Blob, RuntimeOBJCacheData->GeometryLines);

		// build materials
		for (int32 LineIndex = 0; LineIndex < RuntimeOBJCacheData->GeometryLines.Num(); LineIndex++)
		{
			const TArray<FString>& Line = RuntimeOBJCacheData->GeometryLines[LineIndex];

			// mtllib
			if (Line[0] == "mtllib")
			{
				const FString MaterialFilename = GetRemainingString(Line, 1);
				TArray64<uint8> MaterialBlob;
				if (Asset->GetParser()->LoadPathToBlob(MaterialFilename, MaterialBlob))
				{
					FillLinesFromBlob(MaterialBlob, RuntimeOBJCacheData->MaterialLines);
				}
			}
		}

		RuntimeOBJCacheData->bValid = true;

		return RuntimeOBJCacheData;
	}

	void FixPrimitive(FglTFRuntimePrimitive& Primitive, const TArray<TStaticArray<TPair<uint32, bool>, 3>> Indices, const TArray<FVector>& Vertices, const TArray<FVector2D>& UVs, const TArray<FVector>& Normals)
	{
		if (UVs.Num() > 0)
		{
			Primitive.UVs.AddDefaulted();
		}

		for (int32 Index = 0; Index < Indices.Num(); Index++)
		{
			uint32 VertexIndex = Indices[Index][0].Key;
			bool bHasVertex = Indices[Index][0].Value;
			if (!bHasVertex)
			{
				continue;
			}

			const int32 PositionIndex = Primitive.Positions.Add(Vertices[VertexIndex]);

			if (UVs.Num() > 0)
			{
				uint32 UVIndex = Indices[Index][1].Key;
				bool bHasUV = Indices[Index][1].Value;

				if (bHasUV)
				{
					Primitive.UVs[0].Add(UVs[UVIndex]);
				}
				else
				{
					Primitive.UVs[0].Add(FVector2D::Zero());
				}
			}

			if (Normals.Num() > 0)
			{
				uint32 NormalIndex = Indices[Index][2].Key;
				bool bHasNormal = Indices[Index][2].Value;

				if (bHasNormal)
				{
					Primitive.Normals.Add(Normals[NormalIndex]);
				}
				else
				{
					Primitive.Normals.Add(FVector::ZAxisVector);
				}
			}

			Primitive.Indices.Add(PositionIndex);
		}
	}

	void FillMaterial(UglTFRuntimeAsset* Asset, const FString& MaterialName, FglTFRuntimeMaterial& Material, const FglTFRuntimeMaterialsConfig& MaterialsConfig)
	{
		TSharedPtr<FglTFRuntimeOBJCacheData> RuntimeOBJCacheData = GetCacheData(Asset);

		int32 StartingLine = -1;

		// search for material
		for (int32 LineIndex = 0; LineIndex < RuntimeOBJCacheData->MaterialLines.Num(); LineIndex++)
		{
			const TArray<FString>& Line = RuntimeOBJCacheData->MaterialLines[LineIndex];

			if (Line[0] == "newmtl")
			{
				if (GetRemainingString(Line, 1) == MaterialName)
				{
					StartingLine = LineIndex + 1;
					break;
				}
			}
		}

		if (StartingLine < 0)
		{
			return;
		}

		// fill material
		for (int32 LineIndex = StartingLine; LineIndex < RuntimeOBJCacheData->MaterialLines.Num(); LineIndex++)
		{
			const TArray<FString>& Line = RuntimeOBJCacheData->MaterialLines[LineIndex];
			if (Line[0] == "newmtl")
			{
				break;
			}

			if (Line[0] == "Kd")
			{
				if (Line.Num() >= 4)
				{
					Material.bHasBaseColorFactor = true;
					Material.BaseColorFactor.R = FCString::Atod(*(Line[1]));
					Material.BaseColorFactor.G = FCString::Atod(*(Line[2]));
					Material.BaseColorFactor.B = FCString::Atod(*(Line[3]));
				}
				continue;
			}

			if (Line[0] == "d")
			{
				if (Line.Num() >= 2)
				{
					Material.BaseColorFactor.A = FCString::Atod(*(Line[1]));
					if (Material.BaseColorFactor.A < 1)
					{
						Material.bTranslucent = true;
						Material.MaterialType = EglTFRuntimeMaterialType::Translucent;
					}
				}
				continue;
			}

			if (Line[0] == "Tr")
			{
				if (Line.Num() >= 2)
				{
					Material.BaseColorFactor.A = 1 - FCString::Atod(*(Line[1]));
					if (Material.BaseColorFactor.A < 1)
					{
						Material.bTranslucent = true;
						Material.MaterialType = EglTFRuntimeMaterialType::Translucent;
					}
				}
				continue;
			}

			if (Line[0] == "Ns")
			{
				if (Line.Num() >= 2)
				{
					Material.BaseSpecularFactor = FCString::Atod(*(Line[1])) / 1000;
				}
				continue;
			}

			if (Line[0] == "map_Kd")
			{
				const FString Filename = GetRemainingString(Line, 1);
				TArray64<uint8> ImageData;
				if (Asset->GetParser()->LoadPathToBlob(Filename, ImageData))
				{
					Asset->GetParser()->LoadBlobToMips(ImageData, Material.BaseColorTextureMips, true, MaterialsConfig);
				}
				continue;
			}

			if (Line[0] == "map_Bump")
			{
				const FString Filename = GetRemainingString(Line, 1);
				TArray64<uint8> ImageData;
				if (Asset->GetParser()->LoadPathToBlob(Filename, ImageData))
				{
					Asset->GetParser()->LoadBlobToMips(ImageData, Material.NormalTextureMips, false, MaterialsConfig);
				}
				continue;
			}
		}
	}
}

TArray<FString> UglTFRuntimeOBJFunctionLibrary::GetOBJObjectNames(UglTFRuntimeAsset* Asset)
{
	TArray<FString> Names;

	if (!Asset)
	{
		return Names;
	}

	TSharedPtr<FglTFRuntimeOBJCacheData> RuntimeOBJCacheData = glTFRuntimeOBJ::GetCacheData(Asset);
	if (!RuntimeOBJCacheData)
	{
		return Names;
	}

	for (const TArray<FString>& Line : RuntimeOBJCacheData->GeometryLines)
	{
		if (Line[0] == "o")
		{
			Names.Add(glTFRuntimeOBJ::GetRemainingString(Line, 1));
		}
	}

	return Names;
}

bool UglTFRuntimeOBJFunctionLibrary::LoadOBJAsRuntimeLOD(UglTFRuntimeAsset* Asset, const FString& ObjectName, FglTFRuntimeMeshLOD& RuntimeLOD, const FglTFRuntimeMaterialsConfig& MaterialsConfig)
{
	if (!Asset)
	{
		return false;
	}

	TSharedPtr<FglTFRuntimeOBJCacheData> RuntimeOBJCacheData = glTFRuntimeOBJ::GetCacheData(Asset);
	if (!RuntimeOBJCacheData)
	{
		return false;
	}

	int32 StartingLine = -1;

	if (!ObjectName.IsEmpty())
	{
		for (int32 LineIndex = 0; LineIndex < RuntimeOBJCacheData->GeometryLines.Num(); LineIndex++)
		{
			const TArray<FString>& Line = RuntimeOBJCacheData->GeometryLines[LineIndex];

			if (Line[0] == "o")
			{
				if (glTFRuntimeOBJ::GetRemainingString(Line, 1) == ObjectName)
				{
					StartingLine = LineIndex + 1;
					break;
				}
			}
		}
	}

	if (StartingLine < 0)
	{
		return false;
	}

	TArray<FVector> Vertices;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;

	// step 1, gather vertices, normals and uvs
	for (int32 LineIndex = 0; LineIndex < RuntimeOBJCacheData->GeometryLines.Num(); LineIndex++)
	{
		const TArray<FString>& Line = RuntimeOBJCacheData->GeometryLines[LineIndex];

		// vertex
		if (Line[0] == "v")
		{
			if (Line.Num() < 4)
			{
				return false;
			}

			FVector Vertex = FVector(FCString::Atod(*(Line[1])), FCString::Atod(*(Line[2])), FCString::Atod(*(Line[3])));
			Vertices.Add(Asset->GetParser()->TransformPosition(Vertex));
			continue;
		}

		// uv
		if (Line[0] == "vt")
		{
			if (Line.Num() < 3)
			{
				return false;
			}

			FVector2D UV = FVector2D(FCString::Atod(*(Line[1])), 1 - FCString::Atod(*(Line[2])));
			UVs.Add(UV);
			continue;
		}

		// normal
		if (Line[0] == "vn")
		{
			if (Line.Num() < 4)
			{
				return false;
			}

			FVector Normal = FVector(FCString::Atod(*(Line[1])), FCString::Atod(*(Line[2])), FCString::Atod(*(Line[3])));
			Normals.Add(Asset->GetParser()->TransformVector(Normal));
			continue;
		}
	}

	TArray<TStaticArray<TPair<uint32, bool>, 3>> Indices;

	FglTFRuntimePrimitive Primitive;
	Primitive.Material = UMaterial::GetDefaultMaterial(MD_Surface);

	// step 2, build primitives
	for (int32 LineIndex = StartingLine; LineIndex < RuntimeOBJCacheData->GeometryLines.Num(); LineIndex++)
	{
		const TArray<FString>& Line = RuntimeOBJCacheData->GeometryLines[LineIndex];

		// end of object
		if (Line[0] == "o")
		{
			break;
		}

		// group
		if (Line[0] == "g")
		{
			if (Indices.Num() > 0)
			{
				glTFRuntimeOBJ::FixPrimitive(Primitive, Indices, Vertices, UVs, Normals);
				RuntimeLOD.Primitives.Add(MoveTemp(Primitive));
			}
			Indices.Empty();
			Primitive = FglTFRuntimePrimitive();
			Primitive.Material = UMaterial::GetDefaultMaterial(MD_Surface);
			Primitive.MaterialName = glTFRuntimeOBJ::GetRemainingString(Line, 1);
			continue;
		}

		// face
		if (Line[0] == "f")
		{
			if (Line.Num() < 4)
			{
				return false;
			}

			for (int32 FaceVertexIndex = 0; FaceVertexIndex < 3; FaceVertexIndex++)
			{
				TArray<FString> FaceVertexParts;
				Line[FaceVertexIndex + 1].ParseIntoArray(FaceVertexParts, TEXT("/"));

				TStaticArray<TPair<uint32, bool>, 3> Index;

				Index[0] = TPair<uint32, bool>(FCString::Atoi(*(FaceVertexParts[0])) - 1, true);
				Index[1] = TPair<uint32, bool>(0, false);
				Index[2] = TPair<uint32, bool>(0, false);

				if (FaceVertexParts.Num() > 1)
				{
					Index[1] = TPair<uint32, bool>(FCString::Atoi(*(FaceVertexParts[1])) - 1, true);
				}

				if (FaceVertexParts.Num() > 2)
				{
					Index[2] = TPair<uint32, bool>(FCString::Atoi(*(FaceVertexParts[2])) - 1, true);
				}

				Indices.Add(Index);
			}


			// quad ?
			if (Line.Num() > 4)
			{
				TArray<FString> FaceVertexParts;
				Line[4].ParseIntoArray(FaceVertexParts, TEXT("/"));

				TStaticArray<TPair<uint32, bool>, 3> FourthIndex;

				FourthIndex[0] = TPair<uint32, bool>(FCString::Atoi(*(FaceVertexParts[0])) - 1, true);
				FourthIndex[1] = TPair<uint32, bool>(0, false);
				FourthIndex[2] = TPair<uint32, bool>(0, false);

				if (FaceVertexParts.Num() > 1)
				{
					FourthIndex[1] = TPair<uint32, bool>(FCString::Atoi(*(FaceVertexParts[1])) - 1, true);
				}

				if (FaceVertexParts.Num() > 2)
				{
					FourthIndex[2] = TPair<uint32, bool>(FCString::Atoi(*(FaceVertexParts[2])) - 1, true);
				}

				const TStaticArray<TPair<uint32, bool>, 3> ThirdIndex = Indices.Last();
				const TStaticArray<TPair<uint32, bool>, 3> FirstIndex = Indices.Last(2);
				Indices.Add(ThirdIndex);
				Indices.Add(FourthIndex);
				Indices.Add(FirstIndex);
			}
			continue;
		}

		// material
		if (Line[0] == "usemtl")
		{
			Primitive.MaterialName = glTFRuntimeOBJ::GetRemainingString(Line, 1);
			FglTFRuntimeMaterial Material;
			glTFRuntimeOBJ::FillMaterial(Asset, Primitive.MaterialName, Material, MaterialsConfig);
			Primitive.Material = Asset->GetParser()->BuildMaterial(-1, Primitive.MaterialName, Material, MaterialsConfig, false);
			continue;
		}
	}

	if (Indices.Num() > 0)
	{
		glTFRuntimeOBJ::FixPrimitive(Primitive, Indices, Vertices, UVs, Normals);
		RuntimeLOD.Primitives.Add(MoveTemp(Primitive));
	}

	return true;
}