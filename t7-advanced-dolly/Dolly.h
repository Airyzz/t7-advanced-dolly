#pragma once
#include "StdInc.h"
#include "Addresses.h"
#include <map>
#include "Math.h"
#include "Settings.h"
using namespace Addresses;

struct CustomMarkerData {
	float roll;
	double fov;
	float focalDistance;
	float aperture;
};

std::map<int, CustomMarkerData> DataMap;

int FindDollyIndexForTick(int CurrentTick)
{
	for (int i = 0; i < *NumDollyCams - 1; i++)
	{
		if (CurrentTick > DollyMarkers[i].tick && CurrentTick < DollyMarkers[i + 1].tick) {
			return i;
		}
	}
	return -1;
}

bool dataContains(int Tick)
{
	if (DataMap.find(Tick) != DataMap.end())
	{
		return true;
	}
	return false;
}

bool foremost() {
	HWND foreground = GetForegroundWindow();
	DWORD foregroundID = 0;
	GetWindowThreadProcessId(foreground, &foregroundID);

	if (foregroundID == GetCurrentProcessId())
	{
		return true;
	}
	return false;
}

namespace Dolly {

	void Loop() {

		int CurrentTick = *Tick;

		float increment_roll = 0.5;
		float increment_fov = 0.05;
		float increment_focus = 0.05;
		float increment_aperture = 0.05;

		if (GetKeyState(VK_OEM_PLUS) & 0x8000)
		{
			if (foremost())
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					*FOV += increment_fov;
				}
				else if (GetKeyState(VK_CONTROL) & 0x8000) 
				{
					*Addresses::FocalDistance += increment_focus;
				}
				else if (GetKeyState(VK_MENU) & 0x8000)
				{
					*Addresses::Aperture += increment_aperture;
				}
				else 
				{
					Settings::roll += increment_roll;
				}
			}
		}
		else if (GetKeyState(VK_OEM_MINUS) & 0x8000) {
			if (foremost())
			{
				if (GetKeyState(VK_SHIFT) & 0x8000)
				{
					if (*FOV - increment_fov >= 0.1)
					{
						*FOV -= increment_fov;;
					}
				}
				else if (GetKeyState(VK_CONTROL) & 0x8000) 
				{
					if (*Addresses::FocalDistance - increment_focus > 0)
						*Addresses::FocalDistance -= increment_focus;
				}
				else if (GetKeyState(VK_MENU) & 0x8000)
				{
					if (*Addresses::Aperture - increment_aperture > 0)
						*Addresses::Aperture -= increment_aperture;
				}
				else 
				{
					Settings::roll -= increment_roll;
				}

			}

		}

		if (*(DWORD64*)Addresses::TheaterBasePointer != 0)
		{
			if (*DollyCameraMode == 2)
			{
				int i = FindDollyIndexForTick(CurrentTick);

				if (i != -1)
				{
					int StartTick = DollyMarkers[i].tick;
					int EndTick = DollyMarkers[i + 1].tick;
					float alpha = (float)(CurrentTick - StartTick) / (float)(EndTick - StartTick);

					//Need 4 points for cubic interpolate

					//	Timeline: ('|' is current time)
					//	[-------[m0]---------[m1]-------|--------[m2]-----------[m3]----]

					DollyMarker* m0;
					DollyMarker* m1;
					DollyMarker* m2;
					DollyMarker* m3;


					if (*(DWORD64*)Addresses::TheaterBasePointer != 0)
					{
						
						//Get 2 Markers before of current marker
						if (i > 0)
						{
							m0 = &DollyMarkers[i - 1];
							m1 = &DollyMarkers[i];
						}
						else {

							//If no marker before the current marker, just use the same marker
							m0 = &DollyMarkers[i];
							m1 = &DollyMarkers[i];
						}

						//Get 2 markers after current marker
						if ((*NumDollyCams - i) > 2) {
							m2 = &DollyMarkers[i + 1];
							m3 = &DollyMarkers[i + 2];
						}
						else {
							//If no marker, duplicate the m2
							m2 = &DollyMarkers[i + 1];
							m3 = &DollyMarkers[i + 1];
						}

						if (dataContains(m0->tick) && dataContains(m1->tick) && dataContains(m2->tick) && dataContains(m3->tick))
						{
							CustomMarkerData m0d = DataMap[m0->tick];
							CustomMarkerData m1d = DataMap[m1->tick];
							CustomMarkerData m2d = DataMap[m2->tick];
							CustomMarkerData m3d = DataMap[m3->tick];

							switch (Settings::InterpolationMode) {
							case LINEAR:
								*FOV = Math::LinearInterpolate(m1d.fov, m2d.fov, alpha);
								Settings::roll = Math::LinearInterpolate(m1d.roll, m2d.roll, alpha);
								if (Settings::UseFocusDistance) {
									*FocalDistance = Math::LinearInterpolate(m1d.focalDistance, m2d.focalDistance, alpha);
									*Aperture = Math::LinearInterpolate(m1d.aperture, m2d.aperture, alpha);
								}
								break;
							case COSINE:
								*FOV = Math::CosineInterpolate(m1d.fov, m2d.fov, alpha);
								Settings::roll = Math::CosineInterpolate(m1d.roll, m2d.roll, alpha);
								if (Settings::UseFocusDistance)
								{
									*FocalDistance = Math::CosineInterpolate(m1d.focalDistance, m2d.focalDistance, alpha);
									*Aperture = Math::CosineInterpolate(m1d.aperture, m2d.aperture, alpha);
								}
								break;
							case SPLINE:
								*FOV = Math::CubicInterpolate(m0d.fov, m1d.fov, m2d.fov, m3d.fov, alpha);
								Settings::roll = Math::CubicInterpolate(m0d.roll, m1d.roll, m2d.roll, m3d.roll, alpha);
								if (Settings::UseFocusDistance)
								{
									*FocalDistance = Math::CubicInterpolate(m0d.focalDistance, m1d.focalDistance, m2d.focalDistance, m3d.focalDistance, alpha);
									*Aperture = Math::CubicInterpolate(m0d.aperture, m1d.aperture, m2d.aperture, m3d.aperture, alpha);
								}
								break;
							}
						}
					}

				}
			}
		}
	}

	void CreateDollyCam(DWORD64 MarkerAddress) {
		DollyMarker* Marker = (DollyMarker*)MarkerAddress;

		DataMap[Marker->tick].fov = *FOV;
		DataMap[Marker->tick].roll = Settings::roll;
		DataMap[Marker->tick].focalDistance = *FocalDistance;
		DataMap[Marker->tick].aperture = *Aperture;
		Log::Info("Added FOV and Roll. Fov: %f, Roll: %f, FocalDistance: %f", DataMap[Marker->tick].fov, DataMap[Marker->tick].roll, DataMap[Marker->tick].focalDistance);
	}

	void EditDollyCam(DWORD64 MarkerAddress) {
		DollyMarker* Marker = (DollyMarker*)MarkerAddress;

		DataMap[Marker->tick].fov = *FOV;
		DataMap[Marker->tick].roll = Settings::roll;
		DataMap[Marker->tick].focalDistance = *FocalDistance;
		DataMap[Marker->tick].aperture = *Aperture;
		Log::Info("Edited FOV and Roll. Fov: %f, Roll: %f, FocalDistance: %f", DataMap[Marker->tick].fov, DataMap[Marker->tick].roll, DataMap[Marker->tick].focalDistance);
	}
}