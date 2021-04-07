RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);
RWTexture2D<float> posOutput : register(u2);
RWTexture2D<float4> normalOutput : register(u3);
RWTexture2D<float> TS_strength : register(u4);
RWTexture2D<float4> AOout : register(u5);
Texture2D<float> postex_old : register(t3);
Texture2D<float> AOtex_old : register(t4);

float3 linearToSrgb(float3 c)
{
    // Based on http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
    float3 sq1 = sqrt(c);
    float3 sq2 = sqrt(sq1);
    float3 sq3 = sqrt(sq2);
    float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;
    return srgb;
}

struct Vertex
{
    float3 pos;
    float3 color;
};

StructuredBuffer<Vertex> vertexbuffer : register(t1);
StructuredBuffer<int> indexbuffer : register(t2);

SamplerState oldpos_sampler : register(s0);

cbuffer camera : register(b0)
{
	float4 offset;
	float4x4 rot;
	float4 oldoffset;
	float4x4 oldrot_inv;
	float4x4 oldrot;
}

cbuffer ObjectMatrix : register(b1)
{
    float4x4 mat;
}

struct RayPayload
{
    float3 color;
	float3 normal;
    float length;
};

[shader("raygeneration")]
void rayGen()
{
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();
	const float WX = 2560.f / 1;
	const float WY = 1440.f / 1;
	uint2 xy = launchIndex.xy;

    float2 crd = float2(launchIndex.xy);
    float2 dims = float2(launchDim.xy);

    float2 d = ((crd / dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

    RayDesc ray;
    ray.Origin = float4(0, 0, 0, 0) + offset;
    float3 dir = normalize(float3(d.x * aspectRatio, -d.y, 1));
    dir = mul(rot, dir);
    ray.Direction = dir;

    ray.TMin = 1e-4;
    ray.TMax = 1000;

    RayPayload payload;
	payload.normal = float3(0, 0, 0);
    TraceRay(gRtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);



	float newlength = payload.length;
	//outputtex[xy] = colortex[xy] * (AOtex_new[xy].x) * 1.0f + postex_new[xy] * 0.0004f;

	float3 finalpos = offset + dir * newlength;

	float3 olddir = finalpos - oldoffset.xyz;
	float3 oldorigdir = mul(oldrot_inv, olddir);
	float2 projectedpos_float = float2((oldorigdir.x / oldorigdir.z) / aspectRatio, -oldorigdir.y / oldorigdir.z);
	float2 projectedpos_float0011 = (projectedpos_float + float2(1.000f, 1.000f)) * 0.5f;
	projectedpos_float0011 += float2(1.f / WX, 1.f / WY) * 0.5f;

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
		float lengthfac = (lengthdif / (1.f + newlength)) * 15000.f;
		//if (lengthfac > 0.025f)
		//	lengthfac *= 200;
		TS_strength[xy] = clamp(TS_strength[xy] + (1.f - lengthfac) * 0.2f * 0.075f, 0, 1);
		AOout[xy] = AOtex_old.SampleLevel(oldpos_sampler, projectedpos_float0011, 0);
	}
	else
	{
		TS_strength[xy] = 0.f;
		AOout[xy] = float4(0, 0, 0, 0);
	}

	
	
    float3 col = linearToSrgb(payload.color);
    gOutput[launchIndex.xy] = float4(col, 1);
    posOutput[launchIndex.xy] = payload.length;
    // gAOOutput[launchIndex.xy] = float4(payload.AOstrength, payload.AOstrength, payload.AOstrength, 1);
	normalOutput[launchIndex.xy] = float4(payload.normal.x, payload.normal.y, payload.normal.z, 0);
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = float3(0.4, 0.6, 0.2);
    payload.length = 1000;
}

[shader("closesthit")]
void chs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);

    uint vertId = 3 * PrimitiveIndex();

    float3 OUT = vertexbuffer[indexbuffer[vertId + 0]].color * barycentrics.x +
        vertexbuffer[indexbuffer[vertId + 1]].color * barycentrics.y +
        vertexbuffer[indexbuffer[vertId + 2]].color * barycentrics.z;

    float3 pos0 = mul(mat, vertexbuffer[indexbuffer[vertId + 0]].pos);
    float3 pos1 = mul(mat, vertexbuffer[indexbuffer[vertId + 1]].pos);
    float3 pos2 = mul(mat, vertexbuffer[indexbuffer[vertId + 2]].pos);
    float3 normal = normalize(cross(pos0 - pos1, pos2 - pos1));
    float3 rayDirW = WorldRayDirection();
	if (dot(normal, rayDirW) < 0)
		normal = -normal;
    float hitT = RayTCurrent();
    payload.color = OUT;
    payload.length = hitT;
	payload.normal = normal;
}

[shader("closesthit")]
void planeChs(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(1.0 - attribs.barycentrics.x - attribs.barycentrics.y, attribs.barycentrics.x, attribs.barycentrics.y);
    uint vertId = 3 * PrimitiveIndex();

    float3 OUT = vertexbuffer[vertId + 0].color * barycentrics.x +
        vertexbuffer[vertId + 1].color * barycentrics.y +
        vertexbuffer[vertId + 2].color * barycentrics.z;

    float3 pos0 = mul(mat, vertexbuffer[vertId + 0].pos);
    float3 pos1 = mul(mat, vertexbuffer[vertId + 1].pos);
    float3 pos2 = mul(mat, vertexbuffer[vertId + 2].pos);
    float3 normal = normalize(cross(pos0 - pos1, pos2 - pos1));

    float hitT = RayTCurrent();
    
    payload.color = OUT;
    payload.length = hitT;
	payload.normal = normal;
}