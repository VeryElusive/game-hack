#include "../animation.h"
#include "bonesetup.h"

class CBoneMergeCache
{
public:
    void* operator new( size_t size )
    {
        CBoneMergeCache* ptr = ( CBoneMergeCache* )Interfaces::MemAlloc->Alloc( sizeof( CBoneMergeCache ) );
        Construct( ptr );

        return ptr;
    }

    void operator delete( void* ptr ) {
        Interfaces::MemAlloc->Free( ptr );
    }

    void Init( CBasePlayer* pOwner ) {
        // sig is massively long and not unique
        reinterpret_cast< void( __thiscall* )( void*, CBasePlayer* ) > ( Displacement::Sigs.CBoneMergeCache__Init )( this, pOwner );
    }     
    
    static void Construct( CBoneMergeCache* bmc ) {
        // 56 8B F1 0F 57 C0 C7 86 ? ? ? ? ? ? ? ? C7 86
        reinterpret_cast< void( __thiscall* )( void* ) > ( Displacement::Sigs.CBoneMergeCache__Construct )( bmc );
    }        
    
    void MergeMatchingPoseParams( ) {
        // 55 8B EC 83 EC 0C 53 56 8B F1 57 89 75 F8 E8
        reinterpret_cast< void( __thiscall* )( void* ) > ( Displacement::Sigs.CBoneMergeCache__MergeMatchingPoseParams )( this );
    }    
    
    void CopyFromFollow( Vector* followPos, Quaternion* followQ, int boneMask, Vector* myPos, Quaternion* myQ ) {
        // 55 8B EC 83 EC 08 53 56 57 8B F9 89 7D F8 E8 ? ? ? ? 83 7F 10 00 0F 84
        reinterpret_cast< void( __thiscall* )( void*, Vector*, Quaternion*, int, Vector*, Quaternion* ) > ( Displacement::Sigs.CBoneMergeCache__CopyFromFollow )( this, followPos, followQ, boneMask, myPos, myQ );
    }

    void CopyToFollow( Vector* myPos, Quaternion* myQ, int boneMask, Vector* followPos, Quaternion* followQ ) {
        // 55 8B EC 83 EC 08 53 56 57 8B F9 89 7D F8 E8 ? ? ? ? 83 7F 10 00 0F 84
        reinterpret_cast< void( __thiscall* )( void*, Vector*, Quaternion*, int, Vector*, Quaternion* ) > ( Displacement::Sigs.CBoneMergeCache__CopyToFollow )( this, myPos, myQ, boneMask, followPos, followQ );
    }

    uint8_t pad1[ 0xA0 ];
    unsigned short m_iRawIndexMapping[256];
    char pad2[ 0x4 ];
};

int GetNumSeq( CStudioHdr* hdr ) {
    if ( !hdr->pVirtualModel )
        return hdr->pStudioHdr->nLocalSequences;
    else
        return hdr->pVirtualModel->vecSequence.Count( );
}

void CAnimationSys::GetSkeleton( CBasePlayer* player, CStudioHdr* hdr, Vector* pos, Quaternion* q, int boneMask, CIKContext* ik ) {
    CBoneSetup boneSetup{ hdr, boneMask, player->m_flPoseParameter( ).data( ) };
    boneSetup.InitPose( pos, q );
    boneSetup.AccumulatePose( pos, q, player->m_nSequence( ), player->m_flCycle( ), 1.f, Interfaces::Globals->flCurTime, ik );

    // sort the layers
    constexpr auto MAX_OVERLAYS{ 15 };
    int layer[ MAX_OVERLAYS ] = { };
    for ( int i = 0; i < player->m_iAnimationLayersCount( ); i++ )
        layer[ i ] = MAX_OVERLAYS;

    for ( int i = 0; i < player->m_iAnimationLayersCount( ); i++ ) {
        CAnimationLayer& pLayer{ player->m_AnimationLayers( )[ i ] };
        if ( ( pLayer.flWeight > 0 ) && pLayer.IsActive( ) && pLayer.iOrder >= 0 && pLayer.iOrder < player->m_iAnimationLayersCount( ) )
            layer[ pLayer.iOrder ] = i;
    }

    // check if this is a player with a valid weapon
    // look for weapon, pull layer animations from it if/when they exist
    CWeaponCSBase* weapon{ };
    CBasePlayer* weaponWorldModel{ };

    bool doWeaponSetup{ };

    if ( player->m_bUseNewAnimstate( ) ) {
        weapon = player->GetWeapon( );
        if ( weapon ) {
            weaponWorldModel = static_cast< CBasePlayer* >( Interfaces::ClientEntityList->GetClientEntityFromHandle( weapon->m_hWeaponWorldModel( ) ) );
            if ( weaponWorldModel &&
                weaponWorldModel->m_pStudioHdr( ) /*&&
                weaponWorldModel->HoldsPlayerAnimations( )*/ ) {

                if ( !weaponWorldModel->m_pBoneMergeCache( ) ) {
                    weaponWorldModel->m_pBoneMergeCache( ) = new CBoneMergeCache( );
                    weaponWorldModel->m_pBoneMergeCache( )->Init( weaponWorldModel );
                }

                if ( weaponWorldModel->m_pBoneMergeCache( ) )
                    doWeaponSetup = true;
            }
        }
    }

    if ( doWeaponSetup ) {
        CStudioHdr* weaponStudioHdr{ weaponWorldModel->m_pStudioHdr( ) };

        // copy matching player pose params to weapon pose params
        weaponWorldModel->m_pBoneMergeCache( )->MergeMatchingPoseParams( );

        // build a temporary setup for the weapon
        const auto weaponIK{ new CIKContext };
        weaponIK->Init( weaponStudioHdr, player->GetAbsAngles( ), player->GetAbsOrigin( ), Interfaces::Globals->flCurTime, 0, BONE_USED_BY_BONE_MERGE );

        CBoneSetup weaponSetup( weaponStudioHdr, BONE_USED_BY_BONE_MERGE, weaponWorldModel->m_flPoseParameter( ).data( ) );
        alignas( 16 ) Vector weaponPos[ 256 ];
        alignas( 16 ) Quaternion weaponQ[ 256 ];

        weaponSetup.InitPose( weaponPos, weaponQ );

        for ( int i = 0; i < player->m_iAnimationLayersCount( ); i++ ) {
            auto pLayer{ &player->m_AnimationLayers( )[ i ] };

            if ( pLayer->nSequence <= 1 || pLayer->flWeight <= 0.f )
                continue;

               player->UpdateDispatchLayer( pLayer, weaponStudioHdr, pLayer->nSequence );

            if ( pLayer->nDispatchedDst > 0 && pLayer->nDispatchedDst < GetNumSeq( weaponStudioHdr ) ) {
                // copy player bones to weapon setup bones
                weaponWorldModel->m_pBoneMergeCache( )->CopyFromFollow( pos, q, BONE_USED_BY_BONE_MERGE, weaponPos, weaponQ );

                // respect ik rules on archetypal sequence, even if we're not playing it
                mstudioseqdesc_t& seqdesc{ hdr->pStudioHdr->GetSequenceDescription( pLayer->nSequence ) };
                ik->AddDependencies( seqdesc, pLayer->nSequence, pLayer->flCycle, player->m_flPoseParameter( ).data( ), pLayer->flWeight );

                weaponSetup.AccumulatePose( weaponPos, weaponQ, pLayer->nDispatchedDst, pLayer->flCycle, pLayer->flWeight, Interfaces::Globals->flCurTime, weaponIK );
                weaponWorldModel->m_pBoneMergeCache( )->CopyToFollow( weaponPos, weaponQ, BONE_USED_BY_BONE_MERGE, pos, q );

                weaponIK->CopyTo( ik, weaponWorldModel->m_pBoneMergeCache( )->m_iRawIndexMapping );
            }
            else
                boneSetup.AccumulatePose( pos, q, pLayer->nSequence, pLayer->flCycle, pLayer->flWeight, Interfaces::Globals->flCurTime, player->m_pIk( ) );
        }

        delete weaponIK;
    }
    else {
        for ( int i = 0; i < player->m_iAnimationLayersCount( ); i++ ) {
            if ( layer[ i ] >= 0 && layer[ i ] < player->m_iAnimationLayersCount( ) ) {
                CAnimationLayer& pLayer = player->m_AnimationLayers( )[ layer[ i ] ];
                boneSetup.AccumulatePose( pos, q, pLayer.nSequence, pLayer.flCycle, pLayer.flWeight, Interfaces::Globals->flCurTime, player->m_pIk( ) );
            }
        }
    }

    const auto autoIk{ new CIKContext };
    autoIk->Init( hdr, player->GetAbsAngles( ), player->GetAbsOrigin( ), Interfaces::Globals->flCurTime, 0, boneMask );
    boneSetup.CalcAutoplaySequences( pos, q, Interfaces::Globals->flCurTime, autoIk );

    boneSetup.CalcBoneAdj( pos, q, player->m_flEncodedController( ) );

    delete autoIk;
}

void QuaternionMatrix( const Quaternion& q, matrix3x4_t& matrix )
{
    // Original code
    // This should produce the same code as below with optimization, but looking at the assmebly,
    // it doesn't.  There are 7 extra multiplies in the release build of this, go figure.
    matrix[ 0 ][ 0 ] = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z;
    matrix[ 1 ][ 0 ] = 2.0 * q.x * q.y + 2.0 * q.w * q.z;
    matrix[ 2 ][ 0 ] = 2.0 * q.x * q.z - 2.0 * q.w * q.y;

    matrix[ 0 ][ 1 ] = 2.0f * q.x * q.y - 2.0f * q.w * q.z;
    matrix[ 1 ][ 1 ] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
    matrix[ 2 ][ 1 ] = 2.0f * q.y * q.z + 2.0f * q.w * q.x;

    matrix[ 0 ][ 2 ] = 2.0f * q.x * q.z + 2.0f * q.w * q.y;
    matrix[ 1 ][ 2 ] = 2.0f * q.y * q.z - 2.0f * q.w * q.x;
    matrix[ 2 ][ 2 ] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;

    matrix[ 0 ][ 3 ] = 0.0f;
    matrix[ 1 ][ 3 ] = 0.0f;
    matrix[ 2 ][ 3 ] = 0.0f;
}

void QuaternionMatrix( const Quaternion& q, const Vector& pos, matrix3x4_t& matrix )
{
    QuaternionMatrix( q, matrix );

    matrix[ 0 ][ 3 ] = pos.x;
    matrix[ 1 ][ 3 ] = pos.y;
    matrix[ 2 ][ 3 ] = pos.z;
}


void concat_transforms( const matrix3x4_t& m0, const matrix3x4_t& m1, matrix3x4_t& out )
{
    for ( int i = 0; i < 3; i++ ) {
        // Normally, you can't just multiply 2 3x4 matrices together, so translation is done separately
        out[ i ][ 3 ] = m1[ 0 ][ 3 ] * m0[ i ][ 0 ] + m1[ 1 ][ 3 ] * m0[ i ][ 1 ] + m1[ 2 ][ 3 ] * m0[ i ][ 2 ] + m0[ i ][ 3 ]; // translation

        for ( int j = 0; j < 3; j++ ) // rotation/scale
        {
            out[ i ][ j ] = m0[ i ][ 0 ] * m1[ 0 ][ j ] + m0[ i ][ 1 ] * m1[ 1 ][ j ] + m0[ i ][ 2 ] * m1[ 2 ][ j ];
        }
    }
}

void CAnimationSys::BuildMatrices( CBasePlayer* player, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4a_t* bones, int boneMask, uint8_t boneComputed[ ] ) {
    int i, j;

    int chain[ 256 ] = { };
    int chain_length = hdr->pStudioHdr->nBones;

    for ( i = 0; i < hdr->pStudioHdr->nBones; i++ )
        chain[ chain_length - i - 1 ] = i;

    matrix3x4a_t rotation_matrix{ };
    rotation_matrix.SetAngles( player->GetAbsAngles( ).y );
    rotation_matrix.SetOrigin( player->GetAbsOrigin( ) );

    for ( j = chain_length - 1; j >= 0; j-- ) {
        i = chain[ j ];

        //if ( ( ( 1 << ( i & 0x1F ) ) & boneComputed[ i >> 5 ] ) )
        //    continue;

        if ( hdr->vecBoneFlags[ i ] & boneMask ) {
            matrix3x4_t bone_matrix{ };

            QuaternionMatrix( q[ i ], pos[ i ], bone_matrix );

            if ( hdr->vecBoneParent[ i ] == -1 )
                concat_transforms( rotation_matrix, bone_matrix, bones[ i ] );
            else
                concat_transforms( bones[ hdr->vecBoneParent[ i ] ], bone_matrix, bones[ i ] );
        }
    }
}