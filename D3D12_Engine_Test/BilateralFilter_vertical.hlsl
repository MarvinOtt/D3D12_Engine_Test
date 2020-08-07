
Texture2D<float> AOtex_OUT : register(t0);
Texture2D<float4> colortex : register(t1);
Texture2D<float4> normaltex : register(t2);
Texture2D<float> depthtex : register(t3);
//RWTexture2D<float4> outputtex : register(u1);
RWTexture2D<float> AOtex_vertical : register(u2);

float gaussian(float x, float sigma) 
{
	return exp(-(pow(x, 2.f)) / (2.f * pow(sigma, 2.f))) / (2.f * 3.14159265f * pow(sigma, 2.f));
}

float distance(int x, int y, int i, int j) {
	return float(sqrt(pow(x - i, 2.f) + pow(y - j, 2.f)));
}

float getNormalSim(float3 normal1, float3 normal2)
{
	return max((dot(normal1, normal2)), 0.f);
}

float InterleavedGradientNoise(float2 uv)
{
	float3 magic = { 0.06711056, 0.00583715, 52.9829189 };
	return frac(magic.z * frac(dot(uv, magic.xy)));
}

void main(float4 pos : SV_POSITION)
{
	uint2 xy = uint2((uint)(pos.x), (uint)(pos.y));
	
	float finalAO = 0.f;

	// Consts
	float sigmaI = 1.5f;
	float sigmaS = 30.0f;
	float sigmaN = 10.f;
	float sigmaD = 0.06f;

	float wP = 0;
	int diameter = 60;
	int radius = diameter / 2;

	int i = radius;

	float AOxy = AOtex_OUT[xy].x;
	for (int j = 0; j < diameter; j++)
	{
		uint2 neighbor = uint2(xy.x, xy.y - (radius - j));
		float AOn = AOtex_OUT[neighbor].x;
		float gi = max(1.f - abs((AOn - AOxy) / sigmaI), 0.0f);
		float gs = gaussian(radius - j, sigmaS);
		float gn = pow(getNormalSim(normaltex[xy].xyz, normaltex[neighbor].xyz), sigmaN);
		float gd = max(1.f - abs((depthtex[xy].x - depthtex[neighbor].x) / (sigmaD - (1 / (1 + depthtex[xy].x * 0.7f)) * sigmaD * 0.9f)), 0.f);
		float w = (gi * gs *gn * gd);
		finalAO += AOn * w;
		wP += w;
	}

	finalAO = finalAO / wP;

	//float noise = lerp(-0.5, 0.5, InterleavedGradientNoise(xy)) * 2.0f; //or other noise method
	AOtex_vertical[xy] = float4(finalAO, 0, 0, 0);/* + noise / 255.f*/
	//outputtex[xy] = colortex[xy] * (finalAO + noise / 255.f);
	//outputtex[xy] = float4(abs(normaltex[xy].x), abs(normaltex[xy].y), abs(normaltex[xy].z), 1);

}