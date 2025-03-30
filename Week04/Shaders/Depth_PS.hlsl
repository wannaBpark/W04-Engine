struct DepthVertexOut
{
    float4 PosH : SV_POSITION;
    float Depth : ZDEPTH;
};

float4 Depth_PS(DepthVertexOut pin) : SV_Target
{
    return float4(pin.Depth, pin.Depth, pin.Depth, 1.0f);
}
