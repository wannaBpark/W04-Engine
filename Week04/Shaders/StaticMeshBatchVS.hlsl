struct FObjectData
{
    row_major float4x4 VP;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    uint isSelected;
    uint objectID;
};

StructuredBuffer<FObjectData> g_ObjectDatas : register(t0);

struct VS_INPUT
{
    float4 position : POSITION; // 버텍스 위치
    float4 color : COLOR; // 버텍스 색상
    float3 normal : NORMAL; // 버텍스 노멀
    float2 texcoord : TEXCOORD;
    uint materialID : TEXCOORD1;
    uint objectID : TEXCOORD2;
};

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
    float3 normal : NORMAL;
    bool normalFlag : TEXCOORD0;
    float2 texcoord : TEXCOORD1;
    float4 uuidColor : TEXCOORD2;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    FObjectData obj = g_ObjectDatas[input.objectID];
    
    // 위치 변환
    output.position = mul(input.position, obj.VP);
    
    output.color = input.color;
    if (obj.isSelected!=0)
        output.color *= 0.5;
    // 입력 normal 값의 길이 확인
    float normalThreshold = 0.001;
    float normalLen = length(input.normal);
    
    if (normalLen < normalThreshold)
    {
        output.normalFlag = 0.0;
    }
    else
    {
        //output.normal = normalize(input.normal);
        output.normal = mul(input.normal, obj.MInverseTranspose);
        output.normalFlag = true;
    }
    output.texcoord = input.texcoord;
    output.uuidColor = obj.UUID;
    
    return output;
}