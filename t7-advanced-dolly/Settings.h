#pragma once

enum Interpolation {
	LINEAR,
	COSINE,
	SPLINE
};

namespace Settings {
	Interpolation InterpolationMode = SPLINE;
	bool UseFocusDistance = true;
	float roll;
}

extern "C" __declspec(dllexport) int __cdecl SetInterpolationMode(Interpolation mode) {

	Settings::InterpolationMode = mode;

	switch (mode)
	{
	case LINEAR:
		Log::Info("Set interpolation mode to linear");
		break;
	case COSINE:
		Log::Info("Set interpolation mode to cosine");
		break;
	case SPLINE:
		Log::Info("Set interpolation mode to spline");
		break;
	}

	return 0;
}


extern "C" __declspec(dllexport) int __cdecl ToggleFocalDistance(int value) {
	if (value == 1)
	{
		Settings::UseFocusDistance = true;
		Log::Info("Enabled Focus Distance");
	}
	else {
		Settings::UseFocusDistance = false;
		Log::Info("Disabled Focus Distance");
	}
	return 0;
}