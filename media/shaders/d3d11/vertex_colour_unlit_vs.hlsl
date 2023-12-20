cbuffer MatrixBuffer
{
	matrix wvp;
};

struct VertexInput
{
    float4 position : POSITION;
    float4 colour : COLOR;

};

struct PixelInput
{
    float4 position : SV_POSITION;
    float4 colour : COLOR0;
};

void VS( in VertexInput input,
         out PixelInput output )
{
    output.position = mul(input.position, wvp);
    output.colour = input.colour;
}
