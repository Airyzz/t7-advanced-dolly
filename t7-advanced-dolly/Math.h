#pragma once

namespace Math {
    #define PI 3.141592653589

	float LinearInterpolate(float y1, float y2, float alpha)
	{
		return(y1 * (1 - alpha) + y2 * alpha);
	}

    float CubicInterpolate(float y0, float y1, float y2, float y3, float alpha)
    {
        float a0, a1, a2, a3, mu2;

        mu2 = alpha * alpha;
        a0 = y3 - y2 - y0 + y1;
        a1 = y0 - y1 - a0;
        a2 = y2 - y0;
        a3 = y1;

        return(a0 * alpha * mu2 + a1 * mu2 + a2 * alpha + a3);
    }

    float CosineInterpolate(float y1, float y2, float alpha)
    {
        float mu2 = (1 - cos(alpha * PI)) / 2;
        return(y1 * (1 - mu2) + y2 * mu2);
    }
}