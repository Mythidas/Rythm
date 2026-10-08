struct VSInput {
    float3 Position : POSITION;
    float4 Color    : COLOR0;
    float2 UV       : TEXCOORD0;
    float Texture   : TEXCOORD1;
};

struct CameraData {
    float4x4 projection;
    float4x4 view;
};

struct VSOutput {
    float4 Position : SV_POSITION;
    float4 Color    : COLOR0;
    float2 UV       : TEXCOORD0;
    float Texture   : TEXCOORD1;
};

[shader("vertex")]
VSOutput main(
    VSInput input,
    uniform CameraData* camera
) {
    VSOutput output;

    float4 worldPosition = float4(input.Position, 1.0f);

    output.Position = mul(
        camera->projection,
        mul(camera->view, worldPosition)
    );

    output.Color = input.Color;
    output.UV = input.UV;
    output.Texture = input.Texture;

    return output;
}

[shader("fragment")]
float4 main(VSOutput input) : SV_Target {
    return input.Color;
}
