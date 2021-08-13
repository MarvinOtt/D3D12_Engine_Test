
Texture2D<float> AOtex_OUT : register(t0);
Texture2D<float4> colortex : register(t1);
Texture2D<float4> normaltex : register(t2);
Texture2D<float> depthtex : register(t3);
RWTexture2D<float4> outputtex : register(u1);
RWTexture2D<float> AOtex_F : register(u2);

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
	const float sigmaI = 2.0f;
	const float sigmaS = 20.0f;
	const float sigmaN = 20.f;
	const float sigmaD = 0.1f * 1;
	const float sigmaDD = 50.f;

	float wP = 0;
	int diameter = 41;
	int radius = diameter / 2;

	if (length(normaltex[xy].xyz) < 0.05f)
	{
		outputtex[xy] = colortex[xy];
	}
	else
	{
		float AOxy = AOtex_OUT[xy].x;
		float minStr = 1.f;
		float curdistchange = 0.f;
		for (int i = radius - 1; i >= 0; i--)
		{
			uint2 neighbor = uint2(xy.x - (radius - i), xy.y);
			float AOn = AOtex_OUT[neighbor].x;
			float gi = gaussian(AOn - AOxy, sigmaI); // max(1.f - abs((AOn - AOxy) / sigmaI), 0.0f);
			float gs = gaussian(radius - i, sigmaS);
			float gn = pow(getNormalSim(normaltex[xy].xyz, normaltex[neighbor].xyz), sigmaN);
			float gd = max(1.f - abs((depthtex[xy].x - depthtex[neighbor].x) / (sigmaD - (1 / (1 + depthtex[xy].x * 0.7f)) * sigmaD * 0.9f)), 0.f);
			float newdistchange = (depthtex[xy].x - depthtex[neighbor].x);
			if (i == radius - 1)
				curdistchange = newdistchange;
			minStr = min(minStr, 1.f / (1.f + abs(newdistchange - curdistchange) * sigmaDD));
			curdistchange = newdistchange;
			float w = (gi * gs * gn * gd) * minStr;
			finalAO += AOn * w;
			wP += w;
		}
		minStr = 1.f;
		for (int i = radius; i < diameter; i++)
		{
			uint2 neighbor = uint2(xy.x - (radius - i), xy.y);
			float AOn = AOtex_OUT[neighbor].x;
			float gi = gaussian(AOn - AOxy, sigmaI); // max(1.f - abs((AOn - AOxy) / sigmaI), 0.0f);
			float gs = gaussian(radius - i, sigmaS);
			float gn = pow(getNormalSim(normaltex[xy].xyz, normaltex[neighbor].xyz), sigmaN);
			float gd = max(1.f - abs((depthtex[xy].x - depthtex[neighbor].x) / (sigmaD - (1 / (1 + depthtex[xy].x * 0.7f)) * sigmaD * 0.9f)), 0.f);
			float newdistchange = (depthtex[xy].x - depthtex[neighbor].x);
			if (i == radius)
				curdistchange = newdistchange;
			minStr = min(minStr, 1.f / (1.f + abs(newdistchange - curdistchange) * sigmaDD));
			curdistchange = newdistchange;
			float w = (gi * gs * gn * gd) * minStr;
			finalAO += AOn * w;
			wP += w;
		}
		finalAO = (finalAO / wP);
		finalAO = pow(finalAO, 1.005f);
		//finalAO = AOxy;
		//finalAO = AOtex_OUT[xy].x;
		float noise = lerp(-0.5, 0.5, InterleavedGradientNoise(xy)) * 2.0f; //or other noise method
		//AOtex_vertical[xy] = (finalAO/* + noise / 255.f*/);
		AOtex_F[xy] = float4(finalAO, 0, 0, 0);
		outputtex[xy] = colortex[xy] * (finalAO + noise / 255.f);
		//outputtex[xy] = float4(abs(normaltex[xy].y), 0, 0, 1);
	}
	//outputtex[xy] = float4(AOtex_OUT[xy].x, 0, 0, 1);
	//outputtex[xy] = float4(abs(normaltex[xy].x), abs(normaltex[xy].y), abs(normaltex[xy].z), 1);

}