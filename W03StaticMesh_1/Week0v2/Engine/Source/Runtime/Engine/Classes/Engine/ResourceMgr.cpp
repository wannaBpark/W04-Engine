#include "ResourceMgr.h"
#include "D3D11RHI/GraphicDevice.h"
#include <fstream>
#include <sstream>
#include "Renderer/Renderer.h"
#include "Define.h"
#include "Components/Quad.h"
#include <wincodec.h>
#include "DirectXTK/Include/DDSTextureLoader.h"
void FResourceMgr::Initialize(FRenderer* renderer, FGraphicsDevice* device)
{
	//GenerateSphere();
	//RegisterMesh(renderer, "Sphere", sphere_vertices, sizeof(sphere_vertices) / sizeof(FVertexSimple), nullptr, 0);
    RegisterMesh(renderer, "Quad", quadVertices, sizeof(quadVertices) / sizeof(FVertexSimple), quadInices, sizeof(quadInices)/sizeof(uint32));

    LoadObjAsset(renderer, "ArrowX", L"Assets/AxisArrowX.obj");
    LoadObjAsset(renderer, "ArrowY", L"Assets/AxisArrowY.obj");
    LoadObjAsset(renderer, "ArrowZ", L"Assets/AxisArrowZ.obj");
	LoadObjAsset(renderer, "ScaleX", L"Assets/AxisScaleArrowX.obj");
	LoadObjAsset(renderer, "ScaleY", L"Assets/AxisScaleArrowY.obj");
	LoadObjAsset(renderer, "ScaleZ", L"Assets/AxisScaleArrowZ.obj");
	LoadObjAsset(renderer, "CircleX", L"Assets/AxisCircleX.obj");
	LoadObjAsset(renderer, "CircleY", L"Assets/AxisCircleY.obj");
	LoadObjAsset(renderer, "CircleZ", L"Assets/AxisCircleZ.obj");



	LoadObjNormalAsset(renderer, "Cube", L"Assets/Cube.obj");
	LoadObjNormalAsset(renderer, "Sphere", L"Assets/Sphere.obj");
	LoadObjNormalTextureAsset(renderer, "SkySphere", L"Assets/skySphere.obj");

	
	
	LoadTextureFromFile(device->Device, device->DeviceContext, L"Assets/Texture/ocean_sky.jpg");
	LoadTextureFromFile(device->Device, device->DeviceContext, L"Assets/Texture/font.png");
	LoadTextureFromDDS(device->Device, device->DeviceContext, L"Assets/Texture/font.dds");
	LoadTextureFromFile(device->Device, device->DeviceContext, L"Assets/Texture/emart.png");
	LoadTextureFromFile(device->Device, device->DeviceContext, L"Assets/Texture/T_Explosion_SubUV.png");
	LoadTextureFromFile(device->Device, device->DeviceContext, L"Assets/Texture/UUID_Font.png");
	LoadTextureFromDDS(device->Device, device->DeviceContext, L"Assets/Texture/UUID_Font.dds");
	LoadTextureFromFile(device->Device, device->DeviceContext, L"Assets/Texture/Wooden Crate_Crate_BaseColor.png");
	LoadTextureFromFile(device->Device, device->DeviceContext, L"Assets/Texture/spotLight.png");
}

void FResourceMgr::Release(FRenderer* renderer) {
    for (auto& pair : meshMap) {
        FStaticMesh* mesh = pair.second.get();
        renderer->ReleaseBuffer(mesh->vertexBuffer);
        renderer->ReleaseBuffer(mesh->indexBuffer);
    }
	for (auto& pair : textureMap) {
		FTexture* texture =	pair.second.get();
		texture->Release();
	}
}

#include <unordered_map>

struct PairHash {
	template <typename T1, typename T2>
	std::size_t operator()(const std::pair<T1, T2>& pair) const {
		return std::hash<T1>()(pair.first) ^ (std::hash<T2>()(pair.second) << 1);
	}
};
struct TupleHash {
	template <typename T1, typename T2, typename T3>
	std::size_t operator()(const std::tuple<T1, T2, T3>& tuple) const {
		std::size_t h1 = std::hash<T1>()(std::get<0>(tuple));
		std::size_t h2 = std::hash<T2>()(std::get<1>(tuple));
		std::size_t h3 = std::hash<T3>()(std::get<2>(tuple));

		return h1 ^ (h2 << 1) ^ (h3 << 2);  // 해시 값 섞기
	}
};
void FResourceMgr::LoadObjNormalAsset(FRenderer* renderer, const FString& meshName, const FWString& filepath)
{
	std::ifstream objFile(filepath.c_str());

	TArray<FVector> positions;
	TArray<FVector> normals;
	TArray<FVertexSimple> vertices;
	TArray<uint32> indices;
	TArray<FVector4> Colors;
	if (objFile)
	{
		FString line;
		while (std::getline(objFile, line))
		{
			std::istringstream lineStream(line);
			std::string type;
			lineStream >> type;

			if (type == "v")
			{
				FVector vertex;
				FVector color;
				lineStream >> vertex.x >> vertex.y >> vertex.z >> color.x >> color.y >> color.z;

				positions.Add(vertex);
				Colors.Add(FVector4(color.x,color.y,color.z, 1.0f));
			}
			else if (type == "vn")
			{
				FVector normal;
				lineStream >> normal.x >> normal.y >> normal.z;
				normals.Add(normal);
			}
			else if (type == "f")
			{
				TArray<uint32> faceIndices;
				TArray<uint32> normalIndices;
				TArray<uint32> uvIndices;
				std::string vertexInfo;

				while (lineStream >> vertexInfo)
				{
					size_t firstSlash = vertexInfo.find("/");
					if (firstSlash != std::string::npos)
					{
						int vIdx = std::stoi(vertexInfo.substr(0, firstSlash)) - 1;
						int nIdx = std::stoi(vertexInfo.substr(firstSlash + 2)) - 1;

						faceIndices.Add(vIdx);
						normalIndices.Add(nIdx);
					}
				}

				std::unordered_map<std::pair<int, int>, uint32_t, PairHash> vertexCache;

				for (size_t i = 1; i + 1 < faceIndices.Num(); ++i)
				{
					uint32 triangleIndices[3];
					for (size_t j = 0; j < 3; j++)
					{
						int vIdx = faceIndices[j];
						int nIdx = normalIndices[j];

						std::pair<int, int> key = { vIdx, nIdx };
						auto it = vertexCache.find(key);

						if (it != vertexCache.end())
						{
							triangleIndices[j] = it->second;
						}
						else
						{
							FVector position = positions[vIdx];
							FVector normal = normals[nIdx];
							FVector4 color = Colors[vIdx];

							FVertexSimple vertexSimple{
								position.x, position.y, position.z, color.x, color.y, color.z, color.a,
								normal.x, normal.y, normal.z
							};

							uint32 newIndex = static_cast<uint32>(vertices.Num());
							vertices.Add(vertexSimple);
							vertexCache[key] = newIndex;
							triangleIndices[j] = newIndex;
						}
					}

					indices.Add(triangleIndices[0]);
					indices.Add(triangleIndices[1]);
					indices.Add(triangleIndices[2]);
				}
			}
		}
	}


	if (vertices.IsEmpty()) {
		UE_LOG(LogLevel::Error, "Error: OBJ file is empty or failed to parse!");
		return;
	}

	FVertexSimple* vertexArray = new FVertexSimple[vertices.Num()];
	std::memcpy(vertexArray, vertices.GetData(), vertices.Num() * sizeof(FVertexSimple));

	UINT* indexArray = nullptr;
	if (!indices.IsEmpty()) {
		indexArray = new UINT[indices.Num()];
		std::memcpy(indexArray, indices.GetData(), indices.Num() * sizeof(UINT));
	}

	UE_LOG(LogLevel::Error, "Arrow Vertex Size : %d", vertices.Num());
	ID3D11Buffer* vertexBuffer = renderer->CreateVertexBuffer(vertexArray, static_cast<UINT>(vertices.Num() * sizeof(FVertexSimple)));
	ID3D11Buffer* indexBuffer = (indexArray) ? renderer->CreateIndexBuffer(indexArray, static_cast<UINT>(indices.Num() * sizeof(UINT))) : nullptr;

	if (!vertexBuffer || !indexBuffer) {
		UE_LOG(LogLevel::Error, "Error: Failed to create buffers for OBJ: %s", filepath.c_str());
		delete[] vertexArray;
		delete[] indexArray;
		return;
	}

	meshMap[meshName] = std::make_shared<FStaticMesh>(vertexBuffer, static_cast<UINT>(vertices.Num()), vertexArray, indexBuffer, static_cast<UINT>(indices.Num()), indexArray);

	delete[] vertexArray;
	delete[] indexArray;

	UE_LOG(LogLevel::Error, "OBJ Loaded: %s - %d vertices, %d indices", meshName.c_str(), vertices.Num(), indices.Num());
}

void FResourceMgr::LoadObjNormalTextureAsset(FRenderer* renderer, const FString& meshName, const FWString& filepath)
{
	std::ifstream objFile(filepath.c_str());

	TArray<FVector> positions;
	TArray<FVector> normals;
	TArray<FVertexSimple> vertices;
	TArray<uint32> indices;
	TArray<FVector4> Colors;
	TArray<std::pair<float, float>> UVs;

	if (objFile)
	{
		FString line;
		while (std::getline(objFile, line))
		{
			std::istringstream lineStream(line);
			std::string type;
			lineStream >> type;

			if (type == "v")
			{
				FVector vertex;
				FVector color(1.0f, 1.0f, 1.0f); // 기본값 추가
				lineStream >> vertex.x >> vertex.y >> vertex.z;

				if (!(lineStream >> color.x >> color.y >> color.z)) {
					color = FVector(1.0f, 1.0f, 1.0f); // 색상이 없으면 기본값 유지
				}

				positions.Add(vertex);
				Colors.Add(FVector4(color.x, color.y, color.z, 1.0f));
			}
			else if (type == "vn")
			{
				FVector normal;
				lineStream >> normal.x >> normal.y >> normal.z;
				normals.Add(normal);
			}
			else if (type == "vt")
			{
				std::pair<float, float> texcoord;
				lineStream >> texcoord.first >> texcoord.second;
				UVs.Add(texcoord);
			}
			else if (type == "f")
			{
				TArray<uint32> faceIndices;
				TArray<uint32> normalIndices;
				TArray<uint32> uvIndices;
				std::string vertexInfo;

				while (lineStream >> vertexInfo)
				{
					size_t firstSlash = vertexInfo.find("/");
					size_t secondSlash = vertexInfo.find("/", firstSlash + 1);

					if (firstSlash != std::string::npos )
					{
						int vIdx = std::stoi(vertexInfo.substr(0, firstSlash)) - 1;
						int uvIdx = std::stoi(vertexInfo.substr(firstSlash + 1, secondSlash - firstSlash - 1)) - 1;

						faceIndices.Add(vIdx);
						uvIndices.Add(uvIdx);
					}
					if (secondSlash != std::string::npos)
					{
						int nIdx = std::stoi(vertexInfo.substr(secondSlash + 1)) - 1;
						normalIndices.Add(nIdx);
					}
					else
					{
						normalIndices.Add(0);
					}
				}

				if (faceIndices.Num() < 3) continue; // 유효한 삼각형이 아닐 경우 무시

				std::unordered_map<std::tuple<int, int, int>, uint32_t, TupleHash> vertexCache;

				// 삼각형을 여러 개로 나누어 저장
				for (size_t i = 1; i + 1 < faceIndices.Num(); ++i)
				{
					uint32 triangleIndices[3] = { faceIndices[0], faceIndices[i], faceIndices[i + 1] };

					for (size_t j = 0; j < 3; j++)
					{
						int vIdx = triangleIndices[j];
						int uvIdx = uvIndices[j];
						int nIdx = normalIndices[j];

						std::tuple<int, int, int> key = { vIdx, nIdx, uvIdx };
						auto it = vertexCache.find(key);

						if (it != vertexCache.end())
						{
							triangleIndices[j] = it->second;
						}
						else
						{
							FVector position = positions[vIdx];
							FVector normal = (nIdx >= 0 && nIdx < normals.Num()) ? normals[nIdx] : FVector(0, 0, 0);
							FVector4 color = (vIdx >= 0 && vIdx < Colors.Num()) ? Colors[vIdx] : FVector4(1, 1, 1, 1);
							std::pair<float, float> uv = (uvIdx >= 0 && uvIdx < UVs.Num()) ? UVs[uvIdx] : std::pair<float, float>(0.f, 0.f);

							FVertexSimple vertexSimple{
								position.x, position.y, position.z,
								color.x, color.y, color.z, color.a,
								normal.x, normal.y, normal.z,
								uv.first, uv.second
							};

							uint32 newIndex = static_cast<uint32>(vertices.Num());
							vertices.Add(vertexSimple);
							vertexCache[key] = newIndex;
							triangleIndices[j] = newIndex;
						}
					}

					// 삼각형 추가
					indices.Add(triangleIndices[0]);
					indices.Add(triangleIndices[1]);
					indices.Add(triangleIndices[2]);
				}
			}
		}
	}


	if (vertices.IsEmpty()) {
		UE_LOG(LogLevel::Error, "Error: OBJ file is empty or failed to parse!");
		return;
	}

	FVertexSimple* vertexArray = new FVertexSimple[vertices.Num()];
	std::memcpy(vertexArray, vertices.GetData(), vertices.Num() * sizeof(FVertexSimple));

	UINT* indexArray = nullptr;
	if (!indices.IsEmpty()) {
		indexArray = new UINT[indices.Num()];
		std::memcpy(indexArray, indices.GetData(), indices.Num() * sizeof(UINT));
	}

	UE_LOG(LogLevel::Error, "Arrow Vertex Size : %d", vertices.Num());
	ID3D11Buffer* vertexBuffer = renderer->CreateVertexBuffer(vertexArray, static_cast<UINT>(vertices.Num() * sizeof(FVertexSimple)));
	ID3D11Buffer* indexBuffer = (indexArray) ? renderer->CreateIndexBuffer(indexArray, static_cast<UINT>(indices.Num() * sizeof(UINT))) : nullptr;

	if (!vertexBuffer || !indexBuffer) {
		UE_LOG(LogLevel::Error, "Error: Failed to create buffers for OBJ: %s", filepath.c_str());
		delete[] vertexArray;
		delete[] indexArray;
		return;
	}

	meshMap[meshName] = std::make_shared<FStaticMesh>(vertexBuffer, static_cast<UINT>(vertices.Num()), vertexArray, indexBuffer, static_cast<UINT>(indices.Num()), indexArray);


	delete[] vertexArray;
	delete[] indexArray;

	UE_LOG(LogLevel::Error, "OBJ Loaded: %s - %d vertices, %d indices", meshName.c_str(), vertices.Num(), indices.Num());
}

void FResourceMgr::LoadObjAsset(FRenderer* renderer, const FString& meshName, const FWString& filepath)
{
	std::ifstream objFile(filepath.c_str());
	TArray<FVector> positions;
	TArray<FVertexSimple> vertices;
	TArray<uint32> indices;

	if (objFile) {

		FString line;

		int normalcount = 0;
		while (std::getline(objFile, line)) {
			std::istringstream lineStream(line);
			std::string type;
			lineStream >> type;
			if (type == "v") // Vertex position
			{
				FVector vertex;
				FVector color;
				lineStream >> vertex.x >> vertex.y >> vertex.z >> color.x >> color.y >> color.z;

				FVertexSimple vertexSimple{ vertex.x, vertex.y, vertex.z, color.x, color.y , color.z, 0.0f,0.0f,0.0f };
				vertices.Add(vertexSimple);
				positions.Add(vertex);
			}

			else if (type == "f") // Face
			{
				TArray<uint32_t> faceIndices;
				uint32_t index;

				while (lineStream >> index) {
					faceIndices.Add(index - 1);
				}

				for (size_t i = 1; i + 1 < faceIndices.Num(); ++i) {
					indices.Add(faceIndices[0]);    // ù ��° ����
					indices.Add(faceIndices[i]);    // �� ��° ����
					indices.Add(faceIndices[i + 1]); // �� ��° ����
				}
			}

		}
	}
	if (vertices.IsEmpty()) {
		UE_LOG(LogLevel::Error, "Error: OBJ file is empty or failed to parse!");
		return;
	}

	// **버텍스 및 인덱스 데이터를 배열로 변환**
	FVertexSimple* vertexArray = new FVertexSimple[vertices.Num()];
	std::memcpy(vertexArray, vertices.GetData(), vertices.Num() * sizeof(FVertexSimple));

	UINT* indexArray = nullptr;
	if (!indices.IsEmpty()) {
		indexArray = new UINT[indices.Num()];
		std::memcpy(indexArray, indices.GetData(), indices.Num() * sizeof(UINT));
	}


	UE_LOG(LogLevel::Error, "Arrow Vertex Size : %d", vertices.Num());
	ID3D11Buffer* vertexBuffer = renderer->CreateVertexBuffer(vertexArray, static_cast<UINT>(vertices.Num() * sizeof(FVertexSimple)));
	ID3D11Buffer* indexBuffer = (indexArray) ? renderer->CreateIndexBuffer(indexArray, static_cast<UINT>(indices.Num() * sizeof(UINT))) : nullptr;
	if (!vertexBuffer) {
		UE_LOG(LogLevel::Error, "Error: Failed to create buffers for OBJ: %s", filepath.c_str());
		delete[] vertexArray;
		delete[] indexArray;
		return;
	}
	meshMap[meshName] = std::make_shared<FStaticMesh>(vertexBuffer, static_cast<UINT>(vertices.Num()), vertexArray, indexBuffer, static_cast<UINT>(indices.Num()), indexArray);

	delete[] vertexArray;
	delete[] indexArray;

	UE_LOG(LogLevel::Error, "OBJ Loaded: %s - %d vertices, %d indices", meshName.c_str(), vertices.Num(), indices.Num());
}


void FResourceMgr::RegisterMesh(FRenderer* renderer, const std::string& name, FVertexSimple* vertices, uint32 vCount, uint32* indices, uint32 iCount)
{
    INT numVertices = vCount;
    UINT numIndices = iCount;

    ID3D11Buffer* vertexBuffer = renderer->CreateVertexBuffer(vertices, vCount * sizeof(FVertexSimple));
    ID3D11Buffer* indexBuffer = (indices && iCount > 0) ? renderer->CreateIndexBuffer(indices, iCount * sizeof(UINT)) : nullptr;
    meshMap[name] = std::make_shared<FStaticMesh>(vertexBuffer, vCount, vertices, indexBuffer, iCount, indices);
}

void FResourceMgr::RegisterMesh(FRenderer* renderer, const std::string& name, TArray<FVertexSimple>& vertices, uint32 vCount, TArray<uint32>& indices, uint32 iCount)
{
	INT numVertices = vCount;
	UINT numIndices = iCount;

	FVertexSimple* vertexArray = new FVertexSimple[vertices.Num()];
	std::memcpy(vertexArray, vertices.GetData(), vertices.Num() * sizeof(FVertexSimple));

	UINT* indexArray = nullptr;
	if (!indices.IsEmpty()) {
		indexArray = new UINT[indices.Num()];
		std::memcpy(indexArray, indices.GetData(), indices.Num() * sizeof(UINT));
	}

	ID3D11Buffer* vertexBuffer = renderer->CreateVertexBuffer(vertices, vCount * sizeof(FVertexSimple));
	ID3D11Buffer* indexBuffer = (!indices.IsEmpty() && iCount > 0) ? renderer->CreateIndexBuffer(indices, iCount * sizeof(UINT)) : nullptr;
	meshMap[name] = std::make_shared<FStaticMesh>(vertexBuffer, vCount, vertexArray, indexBuffer, iCount, indexArray);
}

std::shared_ptr<FStaticMesh> FResourceMgr::GetMesh(const FString& name) const
{
    auto it = meshMap.find(name);
    return (it != meshMap.end()) ? it->second : nullptr;
}

std::shared_ptr<FTexture> FResourceMgr::GetTexture(const FWString& name) const
{
	auto it = textureMap.find(name);
	return (it != textureMap.end()) ? it->second : nullptr;
}

void FResourceMgr::RegisterMesh(FRenderer* renderer, const std::string& name, TArray<FVertexTexture>& vertices, uint32 vCount, TArray<uint32>& indices, uint32 iCount)
{
	INT numVertices = vCount;
	UINT numIndices = iCount;

	FVertexSimple* vertexArray = new FVertexSimple[vertices.Num()];
	std::memcpy(vertexArray, vertices.GetData(), vertices.Num() * sizeof(FVertexSimple));

	UINT* indexArray = nullptr;
	if (!indices.IsEmpty()) {
		indexArray = new UINT[indices.Num()];
		std::memcpy(indexArray, indices.GetData(), indices.Num() * sizeof(UINT));
	}

	ID3D11Buffer* vertexBuffer = renderer->CreateVertexTextureBuffer(vertices.GetData(), vCount * sizeof(FVertexTexture));
	ID3D11Buffer* indexBuffer = (!indices.IsEmpty() && iCount > 0) ? renderer->CreateIndexBuffer(indices, iCount * sizeof(UINT)) : nullptr;
	meshMap[name] = std::make_shared<FStaticMesh>(vertexBuffer, vCount, vertexArray, indexBuffer, iCount, indexArray);
}

HRESULT FResourceMgr::LoadTextureFromFile(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename)
{
	IWICImagingFactory* wicFactory = nullptr;
	IWICBitmapDecoder* decoder = nullptr;
	IWICBitmapFrameDecode* frame = nullptr;
	IWICFormatConverter* converter = nullptr;

	// WIC 팩토리 생성
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) return hr;

	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
	if (FAILED(hr)) return hr;


	// 이미지 파일 디코딩
	hr = wicFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(hr)) return hr;


	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr)) return hr;

	// WIC 포맷 변환기 생성 (픽셀 포맷 변환)
	hr = wicFactory->CreateFormatConverter(&converter);
	if (FAILED(hr)) return hr;

	hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
	if (FAILED(hr)) return hr;

	// 이미지 크기 가져오기
	UINT width, height;
	frame->GetSize(&width, &height);
	
	// 픽셀 데이터 로드
	BYTE* imageData = new BYTE[width * height * 4];
	hr = converter->CopyPixels(nullptr, width * 4, width * height * 4, imageData);
	if (FAILED(hr)) {
		delete[] imageData;
		return hr;
	}

	// DirectX 11 텍스처 생성
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = imageData;
	initData.SysMemPitch = width * 4;
	ID3D11Texture2D* Texture2D;
	hr = device->CreateTexture2D(&textureDesc, &initData, &Texture2D);
	delete[] imageData;
	if (FAILED(hr)) return hr;

	// Shader Resource View 생성
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	ID3D11ShaderResourceView* TextureSRV;
	hr = device->CreateShaderResourceView(Texture2D, &srvDesc, &TextureSRV);

	// 리소스 해제
	wicFactory->Release();
	decoder->Release();
	frame->Release();
	converter->Release();

	//샘플러 스테이트 생성
	ID3D11SamplerState* SamplerState;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &SamplerState);
	FWString name = FWString(filename);

	textureMap[name] = std::make_shared<FTexture>(TextureSRV, Texture2D, SamplerState, width, height);

	Console::GetInstance().AddLog(LogLevel::Warning, "Texture File Load Successs");
	return hr;
}

HRESULT FResourceMgr::LoadTextureFromDDS(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename)
{

	ID3D11Resource* texture = nullptr;
	ID3D11ShaderResourceView* textureView = nullptr;

	HRESULT hr = DirectX::CreateDDSTextureFromFile(
		device, context,
		filename,
		&texture,
		&textureView
	);
	if (FAILED(hr) || texture == nullptr) abort();

#pragma region WidthHeight

	ID3D11Texture2D* texture2D = nullptr;
	hr = texture->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture2D);
	if (FAILED(hr) || texture2D == nullptr) {
		std::wcerr << L"Failed to query ID3D11Texture2D interface!" << std::endl;
		texture->Release();
		abort();
		return hr;
	}

	// 🔹 텍스처 크기 가져오기
	D3D11_TEXTURE2D_DESC texDesc;
	texture2D->GetDesc(&texDesc);
	uint32 width = static_cast<uint32>(texDesc.Width);
	uint32 height = static_cast<uint32>(texDesc.Height);

#pragma endregion WidthHeight

#pragma region Sampler
	ID3D11SamplerState* SamplerState;
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	device->CreateSamplerState(&samplerDesc, &SamplerState);
#pragma endregion Sampler

	FWString name = FWString(filename);

	textureMap[name] = std::make_shared<FTexture>(textureView, texture2D, SamplerState, width, height);
	
	//texture2D->Release();
	//texture->Release();
	Console::GetInstance().AddLog(LogLevel::Warning, "Texture File Load Successs");

	return hr;
}
