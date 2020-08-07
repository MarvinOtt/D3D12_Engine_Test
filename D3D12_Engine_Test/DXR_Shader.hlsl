/***************************************************************************
# Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#  * Neither the name of NVIDIA CORPORATION nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
# EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
***************************************************************************/
RaytracingAccelerationStructure gRtScene : register(t0);
RWTexture2D<float4> gOutput : register(u0);
RWTexture2D<float> gAOOutput : register(u1);
RWTexture2D<float> posOutput : register(u2);
RWTexture2D<float4> normalOutput : register(u3);

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
cbuffer ObjectMatrix : register(b1)
{
    float4x4 mat;
}

struct RayPayload
{
    float3 color;
	float3 normal;
    uint counter;
    float length;
    float AOstrength;
};

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

    //float xt = randVal.x * 2.0f * 3.14159265f; //expand to 0 to 2PI
    //float yt = sqrt(1.0f - randVal.y);

    //return float3(cos(xt) * yt, sqrt(randVal.y), sin(xt) * yt);

    // Get our cosine-weighted hemisphere lobe sample direction
    return tangent * (r * cos(phi).x) + bitangent * (r * sin(phi)) + hitNorm.xyz * sqrt(1 - randVal.x);
}

float getAOstrength(uint randSeed, float3 pos, float3 normal)
{
    float AO_final = 0.0f;
    for (int i = 0; i < 2; ++i)
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
    return AO_final / 2.0f;
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

    RayDesc ray;
    ray.Origin = float4(0, 0, 0, 0) + offset;
    float3 dir = normalize(float3(d.x * aspectRatio, -d.y, 1));
    dir = mul(projection, dir);
    ray.Direction = dir;

    ray.TMin = 1e-4;
    ray.TMax = 1000;

    RayPayload payload;
    payload.counter = counter;
	payload.normal = float3(0, 0, 0);
    TraceRay(gRtScene, 0 /*rayFlags*/, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);
    float3 col = linearToSrgb(payload.color);
    gOutput[launchIndex.xy] = float4(col, 1);
    posOutput[launchIndex.xy] = payload.length;
    gAOOutput[launchIndex.xy] = float4(payload.AOstrength, payload.AOstrength, payload.AOstrength, 1);
	normalOutput[launchIndex.xy] = float4(payload.normal.x, payload.normal.y, payload.normal.z, 0);
}

[shader("miss")]
void miss(inout RayPayload payload)
{
    payload.color = float3(0.4, 0.6, 0.2);
    payload.length = 1000;
    payload.AOstrength = 1.0f;
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
    float3 rayOriginW = WorldRayOrigin();
    float3 posW = rayOriginW + hitT * rayDirW;

    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();
    uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, payload.counter, 16);
    float AO_final = getAOstrength(randSeed, posW, -normal);
    payload.color = OUT;
    payload.length = hitT;
    payload.AOstrength = AO_final;
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
    float3 rayDirW = WorldRayDirection();


    float hitT = RayTCurrent();
    float3 rayOriginW = WorldRayOrigin();

    // Find the world-space hit position
    float3 posW = rayOriginW + hitT * rayDirW;

    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();
    uint randSeed = initRand(launchIndex.x + launchIndex.y * launchDim.x, payload.counter, 16);
    float AO_final = getAOstrength(randSeed, posW, -normal);
    
    payload.color = OUT;
    payload.length = hitT;
    payload.AOstrength = AO_final;
	payload.normal = normal;
    //payload.color.rgb = worldDir;

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
