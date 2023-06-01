// Pulled directly from the "VulkanDescriptorSets" sample.
// Removing the arrays & using HLSL StructuredBuffer<> would be better.
//#define MAX_INSTANCE_PER_DRAW 240

#pragma pack_matrix(row_major)

StructuredBuffer<matrix> transforms : register(t0);

struct ATTRIBUTES
{
    float3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    float3 Ks; // specular reflectivity
    float Ns; // specular exponent
    float3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    float3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    float3 Ke; // emissive reflectivity
    uint illum; // illumination model
};
// an ultra simple hlsl vertex shader
cbuffer SHADER_VARS : register(b0)
{
    float4x4 world_Matrix;
    float4x4 view_Matrix;
    float4x4 projection_Matrix;
    float4 lDir;
    float4 lColor;
    float4 camPos;
    float4 ambTerm;
    ATTRIBUTES mat;
    float4 transformStart;
}

struct VS_IN
{
    float3 pos : POSITION;
    float3 uvw : UV;
    float3 nrm : NRM;
};

struct VS_OUT
{
    float4 pos : SV_POSITION;
    float2 uvw : UV;
    float3 nrm : NRM;

    float3 worldSPos : World_Space_Surface;
};

VS_OUT main(VS_IN input, uint id : SV_InstanceID)
{
    VS_OUT output;
    output.pos = float4(input.pos, 1);
    output.uvw = float2(input.uvw.x, input.uvw.y);
    output.nrm = input.nrm;
    
    output.pos = mul(transforms[transformStart.x + id], output.pos);
    output.worldSPos = output.pos.xyz;
    output.nrm = mul(transforms[transformStart.x + id], float4(output.nrm, 0)).xyz;
    output.pos = mul(output.pos, view_Matrix);
    output.pos = mul(output.pos, projection_Matrix);
    return output;
}

//cbuffer INSTANCE_UNIFORMS
//{
//	matrix instance_transforms[MAX_INSTANCE_PER_DRAW];
//	vector instance_colors[MAX_INSTANCE_PER_DRAW];
//};
//struct V_OUT { 
//	float4 hpos : SV_POSITION;
//	nointerpolation uint pixelInstanceID : INSTANCE;
//}; 
//V_OUT main(	float2 inputVertex : POSITION, 
//			uint vertexInstanceID : SV_INSTANCEID)
//{
//	V_OUT send = (V_OUT)0;
//	send.hpos = mul(instance_transforms[vertexInstanceID], 
//					float4(inputVertex,0,1));
//	send.pixelInstanceID = vertexInstanceID;
//	return send;
//}