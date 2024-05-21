#include "bonesetup.h"

CBoneSetup::CBoneSetup( const CStudioHdr* studio_hdr, int bone_mask, float* pose_parameters )
    : m_pStudioHdr( studio_hdr )
    , m_boneMask( bone_mask )
    , m_flPoseParameter( pose_parameters )
    , m_pPoseDebugger( nullptr )
{ }

void CBoneSetup::InitPose( Vector pos[ ], Quaternion q[ ] ) {
    auto hdr = m_pStudioHdr->pStudioHdr;

    for ( int i = 0; i < hdr->nBones; i++ ) {
        auto bone = hdr->GetBone( i );

        if ( bone->iFlags & m_boneMask ) {
            pos[ i ] = bone->vecPosition;
            q[ i ] = bone->qWorld;// maybe wrong prob not tho
        }
    }
}

void CBoneSetup::AccumulatePose( Vector pos[ ], Quaternion q[ ], int sequence, float cycle, float weight, float time, void* IKContext ) {
    //55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? A1
    reinterpret_cast< void( __thiscall* )( void*, Vector*, Quaternion*, int, float, float, float, void* ) >
        ( Displacement::Sigs.CBoneSetup__AccumulatePose )( this, pos, q, sequence, cycle, weight, time, IKContext );
}

void CBoneSetup::CalcAutoplaySequences( Vector pos[ ], Quaternion q[ ], float real_time, void* IKContext ) {
    //55 8B EC 83 EC 10 53 56 57 8B 7D 10

    __asm
    {
        mov ecx, this
        movss xmm3, real_time
        push IKContext
        push q
        push pos
        call Displacement::Sigs.CBoneSetup__CalcAutoplaySequences
    }
}

void CBoneSetup::CalcBoneAdj( Vector pos[ ], Quaternion q[ ], const float controllers[ ] ) {
    //55 8B EC 83 E4 F8 81 EC ? ? ? ? 8B C1 89

    // go fuck yourself
    auto hdr = m_pStudioHdr;
    auto mask = m_boneMask;

    __asm
    {
        mov edx, pos
        mov ecx, hdr
        push mask
        push controllers
        push q
        call Displacement::Sigs.CBoneSetup__CalcBoneAdj
        add esp, 0xC
    }
}