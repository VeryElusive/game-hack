#pragma once
#include "../animation.h"

struct CBoneSetup {
    CBoneSetup( const CStudioHdr* studio_hdr, int bone_mask, float* pose_parameters );

    void InitPose( Vector pos[ ], Quaternion q[ ] );
    void AccumulatePose( Vector pos[ ], Quaternion q[ ], int sequence, float cycle, float weight, float time, void* IKContext );
    void CalcAutoplaySequences( Vector pos[ ], Quaternion q[ ], float real_time, void* IKContext );
    void CalcBoneAdj( Vector pos[ ], Quaternion q[ ], const float controllers[ ] );

    const CStudioHdr* m_pStudioHdr;
    int m_boneMask;
    float* m_flPoseParameter;
    void* m_pPoseDebugger;
};

struct IBoneSetup {
    CBoneSetup* bone_setup;
};