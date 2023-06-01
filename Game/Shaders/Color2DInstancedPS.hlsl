// Pulled directly from the "VulkanDescriptorSets" sample.
// Removing the arrays & using HLSL StructuredBuffer<> would be better.
//#define MAX_INSTANCE_PER_DRAW 240

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

struct PS_IN
{
    float4 pos : SV_POSITION;
    float2 uvw : UV;
    float3 nrm : NRM;
    float3 worldSPos : World_Space_Surface;
};

// an ultra simple hlsl pixel shader
float4 main(PS_IN input) : SV_TARGET
{
    float4 diffuse = float4(mat.Kd, mat.d);
    float4 emissive = float4(mat.Ke, mat.illum);
    
    float lRatio = clamp(dot(normalize(-lDir.xyz), normalize(input.nrm)), 0, 1);
    float4 totalDirect = lRatio * lColor;

    float3 viewDir = normalize(camPos.xyz - input.worldSPos.xyz);
    float3 halfVector = normalize((normalize(-lDir.xyz)) + viewDir);
    float intensity = max(pow(clamp(dot(normalize(input.nrm.xyz), halfVector), 0, 1), mat.Ns), 0);
    float4 totalReflected = lColor * normalize(float4(mat.Ks, 1)) * (intensity * .1);

    return float4(saturate(totalDirect.xyz + (ambTerm.xyz * mat.Ka)) * diffuse.xyz + totalReflected.xyz + emissive.xyz, diffuse.w);
}

//cbuffer INSTANCE_UNIFORMS
//{
//	matrix instance_transforms[MAX_INSTANCE_PER_DRAW];
//	vector instance_colors[MAX_INSTANCE_PER_DRAW];
//};
//float4 main(uint pixelInstanceID : INSTANCE) : SV_TARGET 
//{	
//	return instance_colors[pixelInstanceID]; 
//}