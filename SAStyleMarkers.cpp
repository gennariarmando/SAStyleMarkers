#include "plugin.h"
#include "C3dMarkers.h"
#include "CTimer.h"
#include "CWorld.h"

using namespace plugin;

C3dMarker* PlaceMarker(unsigned int id, unsigned short type, CVector& pos, float size, unsigned char r, unsigned char g, unsigned char b, unsigned char a, unsigned short pulsePeriod, float pulseFraction, short rotateRate) {
	C3dMarker* marker = NULL;

#ifndef GTAVC
	size *= 0.7f;
#endif

	if (type != 1 && type != 4)
		return NULL;

	for (int i = 0; i < 32; i++) {
		if (!C3dMarkers::m_aMarkerArray[i].m_bIsUsed && C3dMarkers::m_aMarkerArray[i].m_nIdentifier == id) {
			marker = &C3dMarkers::m_aMarkerArray[i];
			break;
		}
	}

	if (marker == NULL) {
		for (int i = 0; i < 32; i++) {
			if (C3dMarkers::m_aMarkerArray[i].m_nType == 257) {
				marker = &C3dMarkers::m_aMarkerArray[i];
				break;
			}
		}
	}

	float dist = sqrtf((pos.x - FindPlayerCentreOfWorld(0).x) * (pos.x - FindPlayerCentreOfWorld(0).x) + (pos.y - FindPlayerCentreOfWorld(0).y) * (pos.y - FindPlayerCentreOfWorld(0).y));

	if (marker == NULL && type == 1) {
		for (int i = 0; i < 32; i++) {
			if (dist < C3dMarkers::m_aMarkerArray[i].m_fCameraRange && C3dMarkers::m_aMarkerArray[i].m_nType == 1 && (marker == NULL || C3dMarkers::m_aMarkerArray[i].m_fCameraRange > marker->m_fCameraRange)) {
				marker = &C3dMarkers::m_aMarkerArray[i];
				break;
			}
		}

		if (marker != NULL)
			marker->m_nType = 257;
	}

	if (marker == NULL)
		return marker;

	marker->m_fCameraRange = dist;
	marker->m_colour.a = a;

	if (marker->m_nIdentifier == id && marker->m_nType == type) {
		float s = sinf((3.1415f * 2.0f) * (float)((marker->m_nPulsePeriod - 1) & (CTimer::m_snTimeInMilliseconds - marker->m_nStartTime)) / (float)marker->m_nPulsePeriod);

		if (type == 1) {
			pos.z += 0.25f * (size - 0.3f * size) * s;
		}

		if (marker->m_nRotateRate && type != 1) {
			CVector pos = marker->m_mMat.pos;
			marker->m_mMat.RotateZ((marker->m_nRotateRate * CTimer::ms_fTimeStep * 3.1415f / 180.0f));
			marker->m_mMat.pos = pos;
		}

		if (type == 1)
			marker->m_mMat.pos = pos;

		marker->m_bIsUsed = true;
		return marker;
	}

	if (marker->m_nIdentifier != 0) {
		marker->m_nIdentifier = 0;
		marker->m_nStartTime = 0;
		marker->m_bIsUsed = false;
		marker->m_nType = 257;

		if (marker->m_pAtomic) {
			RpAtomic* atomic = ((RpAtomic*)(marker->m_pAtomic));
			RwFrame* frame = (RwFrame*)atomic->object.object.parent;
			RpAtomicDestroy(atomic);
			RwFrameDestroy(frame);
			atomic = NULL;
		}
	}

	marker->AddMarker(id, type, size, r, g, b, a, pulsePeriod, pulseFraction, rotateRate);

	if (type == 4 || type == 0 || type == 2) {
		float z = CWorld::FindGroundZFor3DCoord(pos.x, pos.y, pos.z + 1.0f, NULL);
		if (z != 0.0f)
			pos.z = z - 0.05f * size;
	}

	marker->m_mMat.SetTranslate(pos.x, pos.y, pos.z);

	if (type == 2) {
		marker->m_mMat.RotateX(3.1415f);
		marker->m_mMat.pos = pos;
	}

	marker->m_mMat.UpdateRW();
	marker->m_bIsUsed = true;
	return marker;
}

class SAStyleMarkers {
public:
    SAStyleMarkers() {
#ifdef GTAVC
		plugin::patch::RedirectJump(0x570340, PlaceMarker);
#elif GTA3
        plugin::patch::RedirectJump(0x51B480, PlaceMarker);
#endif
    }
} sAStyleMarkers;
