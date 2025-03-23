#pragma once
#include <fstream>
#include <sstream>

#include "Define.h"
#include "EngineLoop.h"
#include "Container/Map.h"
#include "HAL/PlatformType.h"

struct FLoaderOBJ
{
    // Obj Parsing (*.obj to FObjInfo)
    static bool ParseOBJ(const FString& ObjFilePath, FObjInfo& OutObjInfo)
    {
        std::ifstream OBJ(ObjFilePath.ToWideString());
        if (!OBJ)
        {
            return false;
        }

        OutObjInfo.PathName = ObjFilePath.ToWideString().substr(0, ObjFilePath.ToWideString().find_last_of(L"\\/") + 1);
        OutObjInfo.ObjectName = ObjFilePath.ToWideString().substr(ObjFilePath.ToWideString().find_last_of(L"\\/") + 1);

        std::string Line;

        while (std::getline(OBJ, Line))
        {
            if (Line.empty() || Line[0] == '#')
                continue;
            
            std::istringstream LineStream(Line);
            std::string Token;
            LineStream >> Token;

            if (Token == "mtllib")
            {
                LineStream >> Line;
                OutObjInfo.MatName = Line;
                continue;
            }

            if (Token == "usemtl")
            {
                LineStream >> Line;
                FString MatName(Line);

                if (!OutObjInfo.MaterialSubsets.IsEmpty())
                {
                    FMaterialSubset& LastSubset = OutObjInfo.MaterialSubsets[OutObjInfo.MaterialSubsets.Num() - 1];
                    LastSubset.IndexCount = OutObjInfo.VertexIndices.Num() - LastSubset.IndexStart;
                }
                
                FMaterialSubset MaterialSubset;
                MaterialSubset.MaterialName = MatName;
                MaterialSubset.IndexStart = OutObjInfo.VertexIndices.Num();
                MaterialSubset.IndexCount = 0;
                OutObjInfo.MaterialSubsets.Add(MaterialSubset);
            }

            if (Token == "g" || Token == "o")
            {
                LineStream >> Line;
                OutObjInfo.GroupName.Add(Line);
                OutObjInfo.NumOfGroup++;
            }

            if (Token == "v") // Vertex
            {
                float x, y, z;
                LineStream >> x >> y >> z;
                OutObjInfo.Vertices.Add(FVector(x,y,z));
                continue;
            }

            if (Token == "vn") // Normal
            {
                float nx, ny, nz;
                LineStream >> nx >> ny >> nz;
                OutObjInfo.Normals.Add(FVector(nx,ny,nz));
                continue;
            }

            if (Token == "vt") // Texture
            {
                float u, v;
                LineStream >> u >> v;
                OutObjInfo.UVs.Add(FVector2D(u, v));
                continue;
            }

            if (Token == "f")
            {
                TArray<uint32> faceVertexIndices;  // 이번 페이스의 정점 인덱스
                TArray<uint32> faceNormalIndices;  // 이번 페이스의 법선 인덱스
                TArray<uint32> faceTextureIndices; // 이번 페이스의 텍스처 인덱스
                
                while (LineStream >> Token)
                {
                    std::istringstream tokenStream(Token);
                    std::string part;
                    TArray<std::string> facePieces;

                    // '/'로 분리하여 v/vt/vn 파싱
                    while (std::getline(tokenStream, part, '/'))
                    {
                        facePieces.Add(part);
                    }

                    // OBJ 인덱스는 1부터 시작하므로 -1로 변환
                    uint32 vertexIndex = facePieces[0].empty() ? 0 : std::stoi(facePieces[0]) - 1;
                    uint32 textureIndex = (facePieces.Num() > 1 && !facePieces[1].empty()) ? std::stoi(facePieces[1]) - 1 : UINT32_MAX;
                    uint32 normalIndex = (facePieces.Num() > 2 && !facePieces[2].empty()) ? std::stoi(facePieces[2]) - 1 : UINT32_MAX;

                    faceVertexIndices.Add(vertexIndex);
                    faceTextureIndices.Add(textureIndex);
                    faceNormalIndices.Add(normalIndex);
                }

                // 삼각형화 (삼각형 팬 방식)
                for (int j = 1; j + 1 < faceVertexIndices.Num(); j++)
                {
                    OutObjInfo.VertexIndices.Add(faceVertexIndices[0]);
                    OutObjInfo.VertexIndices.Add(faceVertexIndices[j]);
                    OutObjInfo.VertexIndices.Add(faceVertexIndices[j + 1]);

                    OutObjInfo.TextureIndices.Add(faceTextureIndices[0]);
                    OutObjInfo.TextureIndices.Add(faceTextureIndices[j]);
                    OutObjInfo.TextureIndices.Add(faceTextureIndices[j + 1]);

                    OutObjInfo.NormalIndices.Add(faceNormalIndices[0]);
                    OutObjInfo.NormalIndices.Add(faceNormalIndices[j]);
                    OutObjInfo.NormalIndices.Add(faceNormalIndices[j + 1]);
                }
            }
        }
        
        return true;
    }
    
    // Material Parsing (*.obj to MaterialInfo)
    static bool ParseMaterial(FObjInfo& OutObjInfo, OBJ::FStaticMesh& OutFStaticMesh)
    {
        // Subset
        OutFStaticMesh.MaterialSubsets = OutObjInfo.MaterialSubsets;
        
        std::ifstream MtlFile(OutObjInfo.PathName + OutObjInfo.MatName.ToWideString());
        if (!MtlFile.is_open())
        {
            return false;
        }

        std::string Line;
        int32 MaterialIndex = -1;
        
        while (std::getline(MtlFile, Line))
        {
            if (Line.empty() || Line[0] == '#')
                continue;
            
            std::istringstream LineStream(Line);
            std::string Token;
            LineStream >> Token;

            // Create new material if token is 'newmtl'
            if (Token == "newmtl")
            {
                LineStream >> Line;
                MaterialIndex++;

                FObjMaterialInfo Material;
                Material.MTLName = Line;
                OutFStaticMesh.Materials.Add(Material);
            }

            if (Token == "Kd")
            {
                float x, y, z;
                LineStream >> x >> y >> z;
                OutFStaticMesh.Materials[MaterialIndex].Diffuse = FVector(x, y, z);
            }

            if (Token == "Ks")
            {
                float x, y, z;
                LineStream >> x >> y >> z;
                OutFStaticMesh.Materials[MaterialIndex].Specular = FVector(x, y, z);
            }

            if (Token == "Ka")
            {
                float x, y, z;
                LineStream >> x >> y >> z;
                OutFStaticMesh.Materials[MaterialIndex].Ambient = FVector(x, y, z);
            }

            if (Token == "Ke")
            {
                float x, y, z;
                LineStream >> x >> y >> z;
                OutFStaticMesh.Materials[MaterialIndex].Emissive = FVector(x, y, z);
            }

            if (Token == "Ns")
            {
                float x;
                LineStream >> x;
                OutFStaticMesh.Materials[MaterialIndex].SpecularScalar = x;
            }

            if (Token == "Ni")
            {
                float x;
                LineStream >> x;
                OutFStaticMesh.Materials[MaterialIndex].DensityScalar = x;
            }

            if (Token == "d" || Token == "Tr")
            {
                float x;
                LineStream >> x;
                OutFStaticMesh.Materials[MaterialIndex].TransparencyScalar = x;
                OutFStaticMesh.Materials[MaterialIndex].bTransparent = true;
            }

            if (Token == "illum")
            {
                uint32 x;
                LineStream >> x;
                OutFStaticMesh.Materials[MaterialIndex].IlluminanceModel = x;
            }

            if (Token == "map_Kd")
            {
                LineStream >> Line;
                OutFStaticMesh.Materials[MaterialIndex].DiffuseTextureName = Line;
                OutFStaticMesh.Materials[MaterialIndex].DiffuseTexturePath = OutObjInfo.PathName.append(OutFStaticMesh.Materials[MaterialIndex].DiffuseTextureName.ToWideString());
                OutFStaticMesh.Materials[MaterialIndex].bHasTexture = true;
                CreateTextureFromFile(OutFStaticMesh.Materials[MaterialIndex].DiffuseTexturePath);
            }
        }
        
        return true;
    }
    
    // Convert the Raw data to Cooked data (FStaticMesh)
    static bool ConvertToStaticMesh(const FObjInfo& RawData, OBJ::FStaticMesh& OutStaticMesh)
    {
        OutStaticMesh.ObjectName = RawData.ObjectName;
        OutStaticMesh.PathName = RawData.PathName;

        // 고유 정점을 기반으로 FVertexSimple 배열 생성
        TMap<std::string, uint32> vertexMap; // 중복 체크용

        for (int32 i = 0; i < RawData.VertexIndices.Num(); i++)
        {
            uint32 vIdx = RawData.VertexIndices[i];
            uint32 tIdx = RawData.TextureIndices[i];
            uint32 nIdx = RawData.NormalIndices[i];

            // 키 생성 (v/vt/vn 조합)
            std::string key = std::to_string(vIdx) + "/" + 
                             std::to_string(tIdx) + "/" + 
                             std::to_string(nIdx);

            uint32 index;
            if (vertexMap.Find(key) == nullptr)
            {
                FVertexSimple vertex {};
                vertex.x = RawData.Vertices[vIdx].x;
                vertex.y = RawData.Vertices[vIdx].y;
                vertex.z = RawData.Vertices[vIdx].z;

                vertex.r = 1.0f; vertex.g = 1.0f; vertex.b = 1.0f; vertex.a = 1.0f; // 기본 색상

                if (tIdx != UINT32_MAX && tIdx < RawData.UVs.Num())
                {
                    vertex.u = RawData.UVs[tIdx].x;
                    vertex.v = RawData.UVs[tIdx].y;
                }

                if (nIdx != UINT32_MAX && nIdx < RawData.Normals.Num())
                {
                    vertex.nx = RawData.Normals[nIdx].x;
                    vertex.ny = RawData.Normals[nIdx].y;
                    vertex.nz = RawData.Normals[nIdx].z;
                }

                for (int32 j = 0; j < OutStaticMesh.MaterialSubsets.Num(); j++)
                {
                    const FMaterialSubset& subset = OutStaticMesh.MaterialSubsets[j];
                    if ( i >= subset.IndexStart && i < subset.IndexStart + subset.IndexCount)
                    {
                        vertex.MaterialIndex = subset.MaterialIndex;
                        break;
                    }
                }
                
                index = OutStaticMesh.Vertices.Num();
                OutStaticMesh.Vertices.Add(vertex);
                vertexMap[key] = index;
            }
            else
            {
                index = vertexMap[key];
            }

            OutStaticMesh.Indices.Add(index);
            
        }
        
        return true;
    }

    static bool CreateTextureFromFile(const FWString& Filename)
    {
        
        if (FEngineLoop::resourceMgr.GetTexture(Filename))
        {
            return true;
        }

        HRESULT hr = FEngineLoop::resourceMgr.LoadTextureFromFile(FEngineLoop::graphicDevice.Device, FEngineLoop::graphicDevice.DeviceContext, Filename.c_str());

        if (FAILED(hr))
        {
            return false;
        }

        return true;
    }
};

struct FManagerOBJ
{
public:
    static OBJ::FStaticMesh* LoadObjStaticMeshAsset(const FString& PathFileName)
    {
        OBJ::FStaticMesh* NewStaticMesh = new OBJ::FStaticMesh();
        
        if ( const auto It = ObjStaticMeshMap.Find(PathFileName))
        {
            return *It;
        }

        // Parse OBJ
        FObjInfo NewObjInfo;
        bool Result = FLoaderOBJ::ParseOBJ(PathFileName, NewObjInfo);

        if (!Result)
        {
            return nullptr;
        }

        // Material
        if (NewObjInfo.MaterialSubsets.Num() > 0)
        {
            Result = FLoaderOBJ::ParseMaterial(NewObjInfo, *NewStaticMesh);

            if (!Result)
            {
                return nullptr;
            }

            CombineMaterialIndex(*NewStaticMesh);
        }
        
        // Convert FStaticMesh
        Result = FLoaderOBJ::ConvertToStaticMesh(NewObjInfo, *NewStaticMesh);
        if (!Result)
        {
            return nullptr;
        }

        ObjStaticMeshMap.Add(PathFileName, NewStaticMesh);
        return NewStaticMesh;
    }
    
    static void CombineMaterialIndex(OBJ::FStaticMesh& OutFStaticMesh)
    {
        for (int32 i = 0; i < OutFStaticMesh.MaterialSubsets.Num(); i++)
        {
            FString MatName = OutFStaticMesh.MaterialSubsets[i].MaterialName;
            for (int32 j = 0; j < OutFStaticMesh.Materials.Num(); j++)
            {
                if (OutFStaticMesh.Materials[j].MTLName == MatName)
                {
                    OutFStaticMesh.MaterialSubsets[i].MaterialIndex = j;
                    break;
                }
            }
        }
    }

private:
    static TMap<FString, OBJ::FStaticMesh*> ObjStaticMeshMap;
};