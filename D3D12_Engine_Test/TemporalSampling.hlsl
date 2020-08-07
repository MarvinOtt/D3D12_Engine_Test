
//RWTexture2D<float4> colortex : register(u4);
RWTexture2D<float> AOtex_new : register(u1);
RWTexture2D<float> postex_new : register(u2);
//RWTexture2D<float4> outputtex : register(u3);
Texture2D<float> postex_old : register(t0);
Texture2D<float> AOtex_old : register(t1);
RWTexture2D<float> AOtex_OUT : register(u6);

SamplerState oldpos_sampler : register(s0);

cbuffer camera : register(b0)
{
    float4 offset;
    float4x4 rot;
    float4 oldoffset;
    float4x4 oldrot_inv;
    float4x4 oldrot;
}

void main(float4 pos : SV_POSITION)
{
	const float WX = 1920.f / 1;
	const float WY = 1080.f / 1;

	uint2 xy = uint2((uint)(pos.x), (uint)(pos.y));
	float newlength = postex_new[xy];
	//outputtex[xy] = colortex[xy] * (AOtex_new[xy].x) * 1.0f + postex_new[xy] * 0.0004f;

	float2 d = ((xy / float2(WX, WY)) * 2.f - 1.f);
	float aspectRatio = WX / WY;
	float3 dir = normalize(float3(d.x * aspectRatio, -d.y, 1));
	dir = mul(rot, dir);
	float3 finalpos = offset + dir * newlength;

	float3 olddir = finalpos - oldoffset.xyz;
	float3 oldorigdir = mul(oldrot_inv, olddir);
	float2 projectedpos_float = float2((oldorigdir.x / oldorigdir.z) / aspectRatio, -oldorigdir.y / oldorigdir.z);
	float2 projectedpos_float0011 = (projectedpos_float + float2(1.000f, 1.000f)) * 0.5f;
	projectedpos_float0011 += float2(1.f / WX, 1.f / WY) * 0.5f;
	//int2 projectedpos = int2((int)(projectedpos_float0011.x * 1910), (int)(projectedpos_float0011.y * 1030));

	//outputtex[xy] = float4(projectedpos_float.y, 0, 0, 1);

	float factor = 0;
	float lengthdif = 0;
	if (projectedpos_float0011.x >= 0.f && projectedpos_float0011.x < 1.f && projectedpos_float0011.y >= 0.f && projectedpos_float0011.y < 1.f)
	{
		float oldlength = postex_old.SampleLevel(oldpos_sampler, projectedpos_float0011, 0).x;     //postex_old[projectedpos];

		float2 d2 = projectedpos_float;//   ((projectedpos / float2(1910.f, 1030.f)) * 2.f - 1.f);
		float3 dir2 = normalize(float3(d2.x * aspectRatio, -d2.y, 1));
		dir2 = mul(oldrot, dir2);

		//outputtex[xy] = float4(oldlength * 0.1f, 0, 0, 1);

		lengthdif = length(finalpos - (oldoffset.xyz + dir2 * oldlength));
		//outputtex[xy] = float4(oldlength * 0.1f, 0, 0, 1);
		//if (lengthdif > -0.03f * postex_new[xy] && lengthdif < 0.03f * postex_new[xy])
		//    IsValid = true;
		factor = clamp(0.94f - (lengthdif / (1.f + newlength)) * 200.f, 0, 1);
	}
	//else
	//    outputtex[xy] = float4(0, 1, 0, 1);
	//if (factor > 0.1f)
	//	outputtex[xy] = float4(0, 1, 0, 1);
	//else
	//	outputtex[xy] = float4(1, 0, 0, 1);



	AOtex_OUT[xy] = (AOtex_old.SampleLevel(oldpos_sampler, projectedpos_float0011, 0)) * (factor) + (AOtex_new[xy]) * (1.0f - factor);

	//outputtex[xy] = colortex[xy] * AOtex_OUT[xy];
}
