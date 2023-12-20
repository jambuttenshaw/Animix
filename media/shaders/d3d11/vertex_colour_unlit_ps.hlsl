struct PixelInput
{
    float4 position : SV_POSITION;
    float4 colour: COLOR0;
};

float4 PS( PixelInput input ) : SV_Target
{
    return input.colour;
}
