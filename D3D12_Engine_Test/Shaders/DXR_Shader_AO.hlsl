RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float> gAOOutput : register(u1);
Texture2D<float> posInput : register(t4);
Texture2D<float4> normalInput : register(t5);
Texture2D<float> TS_strength : register(t3);

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

cbuffer CameraWVP : register(b0)
{
    float4 offset;
    float4x4 projection;
    uint counter;
}

struct ShadowPayLoad
{
    float length;
};

// Generates a seed for a random number generator from 2 inputs plus a backoff
uint initRand(uint val0, uint val1, uint backoff = 16)
{
    uint v0 = val0, v1 = val1, s0 = 0;

    [unroll]
    for (uint n = 0; n < backoff; n++)
    {
        s0 += 0x9e3779b9;
        v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
        v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
    }
    return v0;
}

// Takes our seed, updates it, and returns a pseudorandom float in [0..1]
float nextRand(inout uint s)
{
    //s = 10;// 214013u * s + 2531011u;
    s = (1664525u * s + 1013904223u);
    return float((s) & 0x00FFFFFF) / float(0x01000000);
}

float3 getPerpendicularVector(float3 u)
{
    float3 a = abs(u);
    uint xm = ((a.x - a.y) < 0 && (a.x - a.z) < 0) ? 1 : 0;
    uint ym = (a.y - a.z) < 0 ? (1 ^ xm) : 0;
    uint zm = 1 ^ (xm | ym);
    return cross(u, float3(xm, ym, zm));
}

// Get a cosine-weighted random vector centered around a specified normal direction.
float3 getCosHemisphereSample(inout uint randSeed, float3 hitNorm)
{
    // Get 2 random numbers to select our sample with
    float2 randVal = float2(nextRand(randSeed), nextRand(randSeed));

    // Cosine weighted hemisphere sample from RNG
    float3 bitangent = getPerpendicularVector(hitNorm);
    float3 tangent = cross(bitangent, hitNorm);
    float r = sqrt(randVal.x);
    float phi = 2.0f * 3.14159265f * randVal.y;

    // Get our cosine-weighted hemisphere lobe sample direction
    return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) + hitNorm.xyz * sqrt(1 - randVal.x);
}

float getAOstrength(uint randSeed, float3 pos, float3 normal)
{
	uint3 launchIndex = DispatchRaysIndex();

	float TSS = TS_strength[launchIndex.xy];
	TSS = clamp(TSS * 4.0f, 0, 1);
	int ray_count = round(max(1, (1.f - TSS) * 3));

	
    float AO_final = 0.0f;
    for (int i = 0; i < ray_count; ++i)
    {
        float3 worldDir = getCosHemisphereSample(randSeed, normal);

        RayDesc ray;
        ray.Origin = pos;
        ray.Direction = normalize(worldDir);

        ray.TMin = 0.0001f;
        ray.TMax = 1.2f;

        ShadowPayLoad payload2;
        TraceRay(gRtScene, 0 /*rayFlags*/, 0xFF, 1 /* ray index*/, 0, 1, ray, payload2);

        float factor = payload2.length / 1.2f;
        factor = 0.1f + factor * 0.9f;
        AO_final += pow(factor, 2.f);
    }
    return AO_final / ((float)ray_count);
}

[shader("raygeneration")]
void rayGen()
{
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();

    float2 crd = float2(launchIndex.xy);
    float2 dims = float2(launchDim.xy);

    float2 d = ((crd / dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

	float3 dir = normalize(float3(d.x * aspectRatio, -d.y, 1));
	dir = mul(projection, dir);

	uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, counter, 16);
	float AO_final = getAOstrength(randSeed, offset.xyz + dir * posInput[launchIndex.xy], -normalInput[launchIndex.xy]);
	
	gAOOutput[launchIndex.xy] = AO_final;
}

[shader("closesthit")]
void shadowChs(inout ShadowPayLoad payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.length = RayTCurrent();
}

[shader("miss")]
void shadowMiss(inout ShadowPayLoad payload)
{
    payload.length = 1.2f;
}
