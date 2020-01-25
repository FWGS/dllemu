#include "engine/eiface.h"
#include "loader.h"

enginefuncs_t svengfuncs;
#include "pm_shared/pm_defs.h"
pmtrace_t (*orig_playerTrace)( float *start, float *end, int i1, int i2);
pmtrace_t *playerTrace(pmtrace_t *tr, float *start, float *end, int i1, int i2)
{
    *tr = orig_playerTrace(start,end,i1,i2);
    return tr;
}


#if X86EMU
#include "bridge.h"
#include "wrapper.h"
#include "emu/x86emu_private.h"
#include "emu/x87emu_private.h"

bridge_t *serverbridge;
#define WRAP_ENGFUNC( x, y ) out->x = AddBridge( serverbridge, y, in->x, 0 );
x86emu_t *emu;

static void HL_FillEngFuncs( enginefuncs_t *out, enginefuncs_t *in )
{
    WRAP_ENGFUNC( pfnPrecacheModel, iFp )
    WRAP_ENGFUNC( pfnPrecacheSound, iFp )
    WRAP_ENGFUNC( pfnSetModel, vFpp )
    WRAP_ENGFUNC( pfnModelIndex, iFp )
    WRAP_ENGFUNC( pfnModelFrames, iFi )
    WRAP_ENGFUNC( pfnSetSize, vFppp )
    WRAP_ENGFUNC( pfnChangeLevel, vFpp )
    WRAP_ENGFUNC( pfnGetSpawnParms, vFp )
    WRAP_ENGFUNC( pfnSaveSpawnParms, vFp )
    WRAP_ENGFUNC( pfnVecToYaw, fFp )
    WRAP_ENGFUNC( pfnVecToAngles, vFpp )
    WRAP_ENGFUNC( pfnMoveToOrigin, vFppfi )
    WRAP_ENGFUNC( pfnChangeYaw, vFp )
    WRAP_ENGFUNC( pfnChangePitch, vFp )
    WRAP_ENGFUNC( pfnFindEntityByString, pFppp )
    WRAP_ENGFUNC( pfnGetEntityIllum, iFp )
    WRAP_ENGFUNC( pfnFindEntityInSphere, pFppf )
    WRAP_ENGFUNC( pfnFindClientInPVS, pFp )
    WRAP_ENGFUNC( pfnEntitiesInPVS, pFp )
    WRAP_ENGFUNC( pfnMakeVectors, vFp )
    WRAP_ENGFUNC( pfnAngleVectors, vFpppp )
    WRAP_ENGFUNC( pfnCreateEntity, pFv )
    WRAP_ENGFUNC( pfnRemoveEntity, vFp )
    WRAP_ENGFUNC( pfnCreateNamedEntity, pFi )
    WRAP_ENGFUNC( pfnMakeStatic, vFp )
    WRAP_ENGFUNC( pfnEntIsOnFloor, iFp )
    WRAP_ENGFUNC( pfnDropToFloor, iFp )
    WRAP_ENGFUNC( pfnWalkMove, iFpffi )
    WRAP_ENGFUNC( pfnSetOrigin, vFpp )
    WRAP_ENGFUNC( pfnEmitSound, vFpippfii )
    WRAP_ENGFUNC( pfnEmitAmbientSound, vFpppffii )
    WRAP_ENGFUNC( pfnTraceLine, vFppipp )
    WRAP_ENGFUNC( pfnTraceToss, vFppp )
    WRAP_ENGFUNC( pfnTraceMonsterHull, iFpppipp )
    WRAP_ENGFUNC( pfnTraceHull, vFppiipp )
    WRAP_ENGFUNC( pfnTraceModel, vFppipp )
    WRAP_ENGFUNC( pfnTraceTexture, pFppp )
    WRAP_ENGFUNC( pfnTraceSphere, vFppifpp )
    WRAP_ENGFUNC( pfnGetAimVector, vFpfp )
    WRAP_ENGFUNC( pfnServerCommand, vFp )
    WRAP_ENGFUNC( pfnServerExecute, vFv )
    WRAP_ENGFUNC( pfnClientCommand, vFppV )
    WRAP_ENGFUNC( pfnParticleEffect, vFppff )
    WRAP_ENGFUNC( pfnLightStyle, vFip )
    WRAP_ENGFUNC( pfnDecalIndex, iFp )
    WRAP_ENGFUNC( pfnPointContents, iFp )
    WRAP_ENGFUNC( pfnMessageBegin, vFiipp )
    WRAP_ENGFUNC( pfnMessageEnd, vFv )
    WRAP_ENGFUNC( pfnWriteByte, vFi )
    WRAP_ENGFUNC( pfnWriteChar, vFi )
    WRAP_ENGFUNC( pfnWriteShort, vFi )
    WRAP_ENGFUNC( pfnWriteLong, vFi )
    WRAP_ENGFUNC( pfnWriteAngle, vFf )
    WRAP_ENGFUNC( pfnWriteCoord, vFf )
    WRAP_ENGFUNC( pfnWriteString, vFp )
    WRAP_ENGFUNC( pfnWriteEntity, vFi )
    WRAP_ENGFUNC( pfnCVarRegister, vFp )
    WRAP_ENGFUNC( pfnCVarGetFloat, fFp )
    WRAP_ENGFUNC( pfnCVarGetString, pFp )
    WRAP_ENGFUNC( pfnCVarSetFloat, vFpf )
    WRAP_ENGFUNC( pfnCVarSetString, vFpp )
    WRAP_ENGFUNC( pfnAlertMessage, vFppppp )
    WRAP_ENGFUNC( pfnEngineFprintf, vFppV )
    WRAP_ENGFUNC( pfnPvAllocEntPrivateData, pFpi )
    WRAP_ENGFUNC( pfnPvEntPrivateData, pFp )
    WRAP_ENGFUNC( pfnFreeEntPrivateData, vFp )
    WRAP_ENGFUNC( pfnSzFromIndex, pFi )
    WRAP_ENGFUNC( pfnAllocString, iFp )
    WRAP_ENGFUNC( pfnGetVarsOfEnt, pFp )
    WRAP_ENGFUNC( pfnPEntityOfEntOffset, pFi )
    WRAP_ENGFUNC( pfnEntOffsetOfPEntity, iFp )
    WRAP_ENGFUNC( pfnIndexOfEdict, iFp )
    WRAP_ENGFUNC( pfnPEntityOfEntIndex, pFi )
    WRAP_ENGFUNC( pfnFindEntityByVars, pFp )
    WRAP_ENGFUNC( pfnGetModelPtr, pFp )
    WRAP_ENGFUNC( pfnRegUserMsg, iFpi )
    WRAP_ENGFUNC( pfnAnimationAutomove, vFpf )
    WRAP_ENGFUNC( pfnGetBonePosition, vFpipp )
    WRAP_ENGFUNC( pfnFunctionFromName, uFp )
    WRAP_ENGFUNC( pfnNameForFunction, pFu )
    WRAP_ENGFUNC( pfnClientPrintf, vFpup )
    WRAP_ENGFUNC( pfnServerPrint, vFp )
    WRAP_ENGFUNC( pfnCmd_Args, pFv )
    WRAP_ENGFUNC( pfnCmd_Argv, pFi )
    WRAP_ENGFUNC( pfnCmd_Argc, iFv )
    WRAP_ENGFUNC( pfnGetAttachment, vFpipp )
    WRAP_ENGFUNC( pfnCRC32_Init, vFp )
    WRAP_ENGFUNC( pfnCRC32_ProcessBuffer, vFppi )
    WRAP_ENGFUNC( pfnCRC32_ProcessByte, vFpc )
    WRAP_ENGFUNC( pfnCRC32_Final, uFu )
    WRAP_ENGFUNC( pfnRandomLong, iFii )
    WRAP_ENGFUNC( pfnRandomFloat, fFff )
    WRAP_ENGFUNC( pfnSetView, vFpp )
    WRAP_ENGFUNC( pfnTime, fFv )
    WRAP_ENGFUNC( pfnCrosshairAngle, vFpff )
    WRAP_ENGFUNC( pfnLoadFileForMe, pFpp )
    WRAP_ENGFUNC( pfnFreeFile, vFp )
    WRAP_ENGFUNC( pfnEndSection, vFp )
    WRAP_ENGFUNC( pfnCompareFileTime, iFppp )
    WRAP_ENGFUNC( pfnGetGameDir, vFp )
    WRAP_ENGFUNC( pfnCvar_RegisterVariable, vFp )
    WRAP_ENGFUNC( pfnFadeClientVolume, vFpiiii )
    WRAP_ENGFUNC( pfnSetClientMaxspeed, vFpf )
    WRAP_ENGFUNC( pfnCreateFakeClient, pFp )
    WRAP_ENGFUNC( pfnRunPlayerMove, vFppfffwcc )
    WRAP_ENGFUNC( pfnNumberOfEntities, iFv )
    WRAP_ENGFUNC( pfnGetInfoKeyBuffer, pFp )
    WRAP_ENGFUNC( pfnInfoKeyValue, pFpp )
    WRAP_ENGFUNC( pfnSetKeyValue, vFppp )
    WRAP_ENGFUNC( pfnSetClientKeyValue, vFippp )
    WRAP_ENGFUNC( pfnIsMapValid, iFp )
    WRAP_ENGFUNC( pfnStaticDecal, vFpiii )
    WRAP_ENGFUNC( pfnPrecacheGeneric, iFp )
    WRAP_ENGFUNC( pfnGetPlayerUserId, iFp )
    WRAP_ENGFUNC( pfnBuildSoundMsg, vFpippfiiiipp )
    WRAP_ENGFUNC( pfnIsDedicatedServer, iFv )
    WRAP_ENGFUNC( pfnCVarGetPointer, pFp )
    WRAP_ENGFUNC( pfnGetPlayerWONId, uFp )
    WRAP_ENGFUNC( pfnInfo_RemoveKey, vFpp )
    WRAP_ENGFUNC( pfnGetPhysicsKeyValue, pFpp )
    WRAP_ENGFUNC( pfnSetPhysicsKeyValue, vFppp )
    WRAP_ENGFUNC( pfnGetPhysicsInfoString, pFp )
    WRAP_ENGFUNC( pfnPrecacheEvent, wFip )
    WRAP_ENGFUNC( pfnPlaybackEvent, vFipwfppffiiii )
    WRAP_ENGFUNC( pfnSetFatPVS, pFp )
    WRAP_ENGFUNC( pfnSetFatPAS, pFp )
    WRAP_ENGFUNC( pfnCheckVisibility, iFpp )
    WRAP_ENGFUNC( pfnDeltaSetField, vFpp )
    WRAP_ENGFUNC( pfnDeltaUnsetField, vFpp )
    WRAP_ENGFUNC( pfnDeltaAddEncoder, vFpp )
    WRAP_ENGFUNC( pfnGetCurrentPlayer, iFv )
    WRAP_ENGFUNC( pfnCanSkipPlayer, iFp )
    WRAP_ENGFUNC( pfnDeltaFindField, iFpp )
    WRAP_ENGFUNC( pfnDeltaSetFieldByIndex, vFpi )
    WRAP_ENGFUNC( pfnDeltaUnsetFieldByIndex, vFpi )
    WRAP_ENGFUNC( pfnSetGroupMask, vFii )
    WRAP_ENGFUNC( pfnCreateInstancedBaseline, iFip )
    WRAP_ENGFUNC( pfnCvar_DirectSet, vFpp )
    WRAP_ENGFUNC( pfnForceUnmodified, vFuppp )
    WRAP_ENGFUNC( pfnGetPlayerStats, vFppp )
    WRAP_ENGFUNC( pfnAddServerCommand, vFpp )
    WRAP_ENGFUNC( pfnVoice_GetClientListening, iFii )
    WRAP_ENGFUNC( pfnVoice_SetClientListening, iFiii )
    WRAP_ENGFUNC( pfnGetPlayerAuthId, pFp )
    WRAP_ENGFUNC( pfnSequenceGet, pFpp )
    WRAP_ENGFUNC( pfnSequencePickSentence, pFpip )
    WRAP_ENGFUNC( pfnGetFileSize, iFp )
    WRAP_ENGFUNC( pfnGetApproxWavePlayLen, uFp )
    WRAP_ENGFUNC( pfnIsCareerMatch, iFv )
    WRAP_ENGFUNC( pfnGetLocalizedStringLength, iFp )
    WRAP_ENGFUNC( pfnRegisterTutorMessageShown, vFi )
    WRAP_ENGFUNC( pfnGetTimesTutorMessageShown, iFi )
    WRAP_ENGFUNC( pfnProcessTutorMessageDecayBuffer, vFpi )
    WRAP_ENGFUNC( pfnConstructTutorMessageDecayBuffer, vFpi )
    WRAP_ENGFUNC( pfnResetTutorMessageDecayData, vFv )
    WRAP_ENGFUNC( pfnQueryClientCvarValue, vFpp )
    WRAP_ENGFUNC( pfnQueryClientCvarValue2, vFppi )
    WRAP_ENGFUNC( pfnCheckParm, iFpp )
}

#define WRAP_FUNC( x, y ) out->x = AddCheckBridge( serverbridge, y, in->x, 0 );
DLL_FUNCTIONS *api;


void WrapPmove( struct playermove_s *pm )
{
    struct playermove_s *out = pm, *in = pm;
    WRAP_FUNC( PM_Info_ValueForKey, pFpp )
    WRAP_FUNC( PM_Particle, vFppppp )
    WRAP_FUNC( PM_TestPlayerPosition, iFpp )
    WRAP_FUNC( Con_NPrintf, vFipV )
    WRAP_FUNC( Con_DPrintf, iFp )
    WRAP_FUNC( Con_Printf, vFp )
    WRAP_FUNC( Sys_FloatTime, dFv )
    WRAP_FUNC( PM_StuckTouch, vFip )
    WRAP_FUNC( PM_PointContents, iFpp )
    WRAP_FUNC( PM_TruePointContents, iFp )
    WRAP_FUNC( PM_HullPointContents, iFpip )
#if 0 // used by wrapper generator, do not remove
    WRAP_FUNC( PM_PlayerTrace, pFpppii )
#endif
    orig_playerTrace = pm->PM_PlayerTrace;
    pm->PM_PlayerTrace = AddCheckBridge( serverbridge, pFpppii, playerTrace, 0 );
    WRAP_FUNC( PM_TraceLine, pFppiii )
    WRAP_FUNC( RandomLong, iFii )
    WRAP_FUNC( RandomFloat, fFff )
    WRAP_FUNC( PM_GetModelType, iFp )
    WRAP_FUNC( PM_GetModelBounds, vFppp )
    WRAP_FUNC( PM_HullForBsp, pFpp )
    WRAP_FUNC( PM_TraceModel, fFpppp )
    WRAP_FUNC( COM_FileSize, iFp )
    WRAP_FUNC( COM_LoadFile, pFpip )
    WRAP_FUNC( COM_FreeFile, vFp )
    WRAP_FUNC( memfgets, pFppppp )
    WRAP_FUNC( PM_PlaySound, vFpppppp )
    WRAP_FUNC( PM_TraceTexture, pFipp )
    WRAP_FUNC( PM_PlaybackEventFull, vFpppppppppppp )
    WRAP_FUNC( PM_PlayerTraceEx, uFppip )
    WRAP_FUNC( PM_TestPlayerPositionEx, iFppp )
    WRAP_FUNC( PM_TraceLineEx, pFppiip )
    WRAP_FUNC( PM_TraceSurface, pFipp )

}

void wrap_pfnGameInit( void ){
;
R_ESP -= 0;
DynaCall( emu, api->pfnGameInit );
R_ESP += 0;

}
int32_t wrap_pfnSpawn( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnSpawn );
R_ESP += 4;
return *((int32_t*)&R_EAX);
}
void wrap_pfnThink( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnThink );
R_ESP += 4;

}
void wrap_pfnUse( void* a0, void* a1 ){
memcpy(R_ESP - 4, &a1,4);
memcpy(R_ESP - 8, &a0,4);
R_ESP -= 8;
DynaCall( emu, api->pfnUse );
R_ESP += 8;

}
void wrap_pfnTouch( void* a0, void* a1 ){
memcpy(R_ESP - 4, &a1,4);
memcpy(R_ESP - 8, &a0,4);
R_ESP -= 8;
DynaCall( emu, api->pfnTouch );
R_ESP += 8;

}
void wrap_pfnBlocked( void* a0, void* a1 ){
memcpy(R_ESP - 4, &a1,4);
memcpy(R_ESP - 8, &a0,4);
R_ESP -= 8;
DynaCall( emu, api->pfnBlocked );
R_ESP += 8;

}
void wrap_pfnKeyValue( void* a0, void* a1 ){
memcpy(R_ESP - 4, &a1,4);
memcpy(R_ESP - 8, &a0,4);
R_ESP -= 8;
DynaCall( emu, api->pfnKeyValue );
R_ESP += 8;

}
void wrap_pfnSave( void* a0, void* a1 ){
memcpy(R_ESP - 4, &a1,4);
memcpy(R_ESP - 8, &a0,4);
R_ESP -= 8;
DynaCall( emu, api->pfnSave );
R_ESP += 8;

}
int32_t wrap_pfnRestore( void* a0, void* a1, int32_t a2 ){
memcpy(R_ESP - 4, &a2,4);
memcpy(R_ESP - 8, &a1,4);
memcpy(R_ESP - 12, &a0,4);
R_ESP -= 12;
DynaCall( emu, api->pfnRestore );
R_ESP += 12;
return *((int32_t*)&R_EAX);
}
void wrap_pfnSetAbsBox( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnSetAbsBox );
R_ESP += 4;

}
void wrap_pfnSaveWriteFields( void* a0, void* a1, void* a2, void* a3, int32_t a4 ){
memcpy(R_ESP - 4, &a4,4);
memcpy(R_ESP - 8, &a3,4);
memcpy(R_ESP - 12, &a2,4);
memcpy(R_ESP - 16, &a1,4);
memcpy(R_ESP - 20, &a0,4);
R_ESP -= 20;
DynaCall( emu, api->pfnSaveWriteFields );
R_ESP += 20;

}
void wrap_pfnSaveReadFields( void* a0, void* a1, void* a2, void* a3, int32_t a4 ){
memcpy(R_ESP - 4, &a4,4);
memcpy(R_ESP - 8, &a3,4);
memcpy(R_ESP - 12, &a2,4);
memcpy(R_ESP - 16, &a1,4);
memcpy(R_ESP - 20, &a0,4);
R_ESP -= 20;
DynaCall( emu, api->pfnSaveReadFields );
R_ESP += 20;

}
void wrap_pfnSaveGlobalState( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnSaveGlobalState );
R_ESP += 4;

}
void wrap_pfnRestoreGlobalState( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnRestoreGlobalState );
R_ESP += 4;

}
void wrap_pfnResetGlobalState( void ){
;
R_ESP -= 0;
DynaCall( emu, api->pfnResetGlobalState );
R_ESP += 0;

}
int32_t wrap_pfnClientConnect( void* a0, void* a1, void* a2, int8_t a3 ){
memcpy(R_ESP - 4, &a3,4);
memcpy(R_ESP - 8, &a2,4);
memcpy(R_ESP - 12, &a1,4);
memcpy(R_ESP - 16, &a0,4);
R_ESP -= 16;
DynaCall( emu, api->pfnClientConnect );
R_ESP += 16;
return *((int32_t*)&R_EAX);
}
void wrap_pfnClientDisconnect( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnClientDisconnect );
R_ESP += 4;

}
void wrap_pfnClientKill( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnClientKill );
R_ESP += 4;

}
void wrap_pfnClientPutInServer( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnClientPutInServer );
R_ESP += 4;

}
void wrap_pfnClientCommand( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnClientCommand );
R_ESP += 4;

}
void wrap_pfnClientUserInfoChanged( void* a0, void* a1 ){
memcpy(R_ESP - 4, &a1,4);
memcpy(R_ESP - 8, &a0,4);
R_ESP -= 8;
DynaCall( emu, api->pfnClientUserInfoChanged );
R_ESP += 8;

}
void wrap_pfnServerActivate( void* a0, int32_t a1, int32_t a2 ){
memcpy(R_ESP - 4, &a2,4);
memcpy(R_ESP - 8, &a1,4);
memcpy(R_ESP - 12, &a0,4);
R_ESP -= 12;
DynaCall( emu, api->pfnServerActivate );
R_ESP += 12;

}
void wrap_pfnServerDeactivate( void ){
;
R_ESP -= 0;
DynaCall( emu, api->pfnServerDeactivate );
R_ESP += 0;

}
void wrap_pfnPlayerPreThink( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnPlayerPreThink );
R_ESP += 4;

}
void wrap_pfnPlayerPostThink( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnPlayerPostThink );
R_ESP += 4;

}
void wrap_pfnStartFrame( void ){
;
R_ESP -= 0;
DynaCall( emu, api->pfnStartFrame );
R_ESP += 0;

}
void wrap_pfnParmsNewLevel( void ){
;
R_ESP -= 0;
DynaCall( emu, api->pfnParmsNewLevel );
R_ESP += 0;

}
void wrap_pfnParmsChangeLevel( void ){
;
R_ESP -= 0;
DynaCall( emu, api->pfnParmsChangeLevel );
R_ESP += 0;

}
void* wrap_pfnGetGameDescription( void ){
;
R_ESP -= 0;
DynaCall( emu, api->pfnGetGameDescription );
R_ESP += 0;
return *((void**)&R_EAX);
}
void wrap_pfnPlayerCustomization( void* a0, void* a1 ){
memcpy(R_ESP - 4, &a1,4);
memcpy(R_ESP - 8, &a0,4);
R_ESP -= 8;
DynaCall( emu, api->pfnPlayerCustomization );
R_ESP += 8;

}
void wrap_pfnSpectatorConnect( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnSpectatorConnect );
R_ESP += 4;

}
void wrap_pfnSpectatorDisconnect( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnSpectatorDisconnect );
R_ESP += 4;

}
void wrap_pfnSpectatorThink( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnSpectatorThink );
R_ESP += 4;

}
void wrap_pfnSys_Error( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnSys_Error );
R_ESP += 4;

}
void wrap_pfnPM_Move( void* a0, int32_t a1 ){
memcpy(R_ESP - 4, &a1,4);
memcpy(R_ESP - 8, &a0,4);
R_ESP -= 8;
DynaCall( emu, api->pfnPM_Move );
R_ESP += 8;

}
void wrap_pfnPM_Init( void* a0 ){
    WrapPmove(a0);
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnPM_Init );
R_ESP += 4;

}
int8_t wrap_pfnPM_FindTextureType( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnPM_FindTextureType );
R_ESP += 4;
return *((int8_t*)&R_EAX);
}
void wrap_pfnSetupVisibility( void* a0, void* a1, void* a2, void* a3 ){
memcpy(R_ESP - 4, &a3,4);
memcpy(R_ESP - 8, &a2,4);
memcpy(R_ESP - 12, &a1,4);
memcpy(R_ESP - 16, &a0,4);
R_ESP -= 16;
DynaCall( emu, api->pfnSetupVisibility );
R_ESP += 16;

}
void wrap_pfnUpdateClientData( void* a0, int32_t a1, void* a2 ){
memcpy(R_ESP - 4, &a2,4);
memcpy(R_ESP - 8, &a1,4);
memcpy(R_ESP - 12, &a0,4);
R_ESP -= 12;
DynaCall( emu, api->pfnUpdateClientData );
R_ESP += 12;

}
int32_t wrap_pfnAddToFullPack( void* a0, int32_t a1, void* a2, void* a3, int32_t a4, int32_t a5, void* a6 ){
memcpy(R_ESP - 4, &a6,4);
memcpy(R_ESP - 8, &a5,4);
memcpy(R_ESP - 12, &a4,4);
memcpy(R_ESP - 16, &a3,4);
memcpy(R_ESP - 20, &a2,4);
memcpy(R_ESP - 24, &a1,4);
memcpy(R_ESP - 28, &a0,4);
R_ESP -= 28;
DynaCall( emu, api->pfnAddToFullPack );
R_ESP += 28;
return *((int32_t*)&R_EAX);
}
void wrap_pfnCreateBaseline( int32_t a0, int32_t a1, void* a2, void* a3, int32_t a4, uint32_t a5, uint32_t a6 ){
memcpy(R_ESP - 4, &a6,4);
memcpy(R_ESP - 8, &a5,4);
memcpy(R_ESP - 12, &a4,4);
memcpy(R_ESP - 16, &a3,4);
memcpy(R_ESP - 20, &a2,4);
memcpy(R_ESP - 24, &a1,4);
memcpy(R_ESP - 28, &a0,4);
R_ESP -= 28;
DynaCall( emu, api->pfnCreateBaseline );
R_ESP += 28;

}
void wrap_pfnRegisterEncoders( void ){
;
R_ESP -= 0;
DynaCall( emu, api->pfnRegisterEncoders );
R_ESP += 0;

}
int32_t wrap_pfnGetWeaponData( void* a0, void* a1 ){
memcpy(R_ESP - 4, &a1,4);
memcpy(R_ESP - 8, &a0,4);
R_ESP -= 8;
DynaCall( emu, api->pfnGetWeaponData );
R_ESP += 8;
return *((int32_t*)&R_EAX);
}
void wrap_pfnCmdStart( void* a0, void* a1, uint32_t a2 ){
memcpy(R_ESP - 4, &a2,4);
memcpy(R_ESP - 8, &a1,4);
memcpy(R_ESP - 12, &a0,4);
R_ESP -= 12;
DynaCall( emu, api->pfnCmdStart );
R_ESP += 12;

}
void wrap_pfnCmdEnd( void* a0 ){
memcpy(R_ESP - 4, &a0,4);
R_ESP -= 4;
DynaCall( emu, api->pfnCmdEnd );
R_ESP += 4;

}
int32_t wrap_pfnConnectionlessPacket( void* a0, void* a1, void* a2, void* a3 ){
memcpy(R_ESP - 4, &a3,4);
memcpy(R_ESP - 8, &a2,4);
memcpy(R_ESP - 12, &a1,4);
memcpy(R_ESP - 16, &a0,4);
R_ESP -= 16;
DynaCall( emu, api->pfnConnectionlessPacket );
R_ESP += 16;
return *((int32_t*)&R_EAX);
}
int32_t wrap_pfnGetHullBounds( int32_t a0, void* a1, void* a2 ){
memcpy(R_ESP - 4, &a2,4);
memcpy(R_ESP - 8, &a1,4);
memcpy(R_ESP - 12, &a0,4);
R_ESP -= 12;
DynaCall( emu, api->pfnGetHullBounds );
R_ESP += 12;
return *((int32_t*)&R_EAX);
}
void wrap_pfnCreateInstancedBaselines( void ){
;
R_ESP -= 0;
//DynaCall( emu, api->pfnCreateInstancedBaselines );
R_ESP += 0;

}
int32_t wrap_pfnInconsistentFile( void* a0, void* a1, void* a2 ){
memcpy(R_ESP - 4, &a2,4);
memcpy(R_ESP - 8, &a1,4);
memcpy(R_ESP - 12, &a0,4);
R_ESP -= 12;
DynaCall( emu, api->pfnInconsistentFile );
R_ESP += 12;
return *((int32_t*)&R_EAX);
}
int32_t wrap_pfnAllowLagCompensation( void ){
;
R_ESP -= 0;
DynaCall( emu, api->pfnAllowLagCompensation );
R_ESP += 0;
return *((int32_t*)&R_EAX);
}


#define WRAP_GAMEFUNC(x,y)\
    out->x = wrap_##x;

static void HL_FillGameFuncs( DLL_FUNCTIONS *out, DLL_FUNCTIONS *in )
{
    api = in;

    WRAP_GAMEFUNC( pfnGameInit, vFv )
    WRAP_GAMEFUNC( pfnSpawn, iFp )
    WRAP_GAMEFUNC( pfnThink, vFp )
    WRAP_GAMEFUNC( pfnUse, vFpp )
    WRAP_GAMEFUNC( pfnTouch, vFpp )
    WRAP_GAMEFUNC( pfnBlocked, vFpp )
    WRAP_GAMEFUNC( pfnKeyValue, vFpp )
    WRAP_GAMEFUNC( pfnSave, vFpp )
    WRAP_GAMEFUNC( pfnRestore, iFppi )
    WRAP_GAMEFUNC( pfnSetAbsBox, vFp )
    WRAP_GAMEFUNC( pfnSaveWriteFields, vFppppi )
    WRAP_GAMEFUNC( pfnSaveReadFields, vFppppi )
    WRAP_GAMEFUNC( pfnSaveGlobalState, vFp )
    WRAP_GAMEFUNC( pfnRestoreGlobalState, vFp )
    WRAP_GAMEFUNC( pfnResetGlobalState, vFv )
    WRAP_GAMEFUNC( pfnClientConnect, iFpppc )
    WRAP_GAMEFUNC( pfnClientDisconnect, vFp )
    WRAP_GAMEFUNC( pfnClientKill, vFp )
    WRAP_GAMEFUNC( pfnClientPutInServer, vFp )
    WRAP_GAMEFUNC( pfnClientCommand, vFp )
    WRAP_GAMEFUNC( pfnClientUserInfoChanged, vFpp )
    WRAP_GAMEFUNC( pfnServerActivate, vFpii )
    WRAP_GAMEFUNC( pfnServerDeactivate, vFv )
    WRAP_GAMEFUNC( pfnPlayerPreThink, vFp )
    WRAP_GAMEFUNC( pfnPlayerPostThink, vFp )
    WRAP_GAMEFUNC( pfnStartFrame, vFv )
    WRAP_GAMEFUNC( pfnParmsNewLevel, vFv )
    WRAP_GAMEFUNC( pfnParmsChangeLevel, vFv )
    WRAP_GAMEFUNC( pfnGetGameDescription, pFv )
    WRAP_GAMEFUNC( pfnPlayerCustomization, vFpp )
    WRAP_GAMEFUNC( pfnSpectatorConnect, vFp )
    WRAP_GAMEFUNC( pfnSpectatorDisconnect, vFp )
    WRAP_GAMEFUNC( pfnSpectatorThink, vFp )
    WRAP_GAMEFUNC( pfnSys_Error, vFp )
    WRAP_GAMEFUNC( pfnPM_Move, vFpi )
    WRAP_GAMEFUNC( pfnPM_Init, vFp )
    WRAP_GAMEFUNC( pfnPM_FindTextureType, cFp )
    WRAP_GAMEFUNC( pfnSetupVisibility, vFpppp )
    WRAP_GAMEFUNC( pfnUpdateClientData, vFpip )
    WRAP_GAMEFUNC( pfnAddToFullPack, iFpippiip )
    WRAP_GAMEFUNC( pfnCreateBaseline, vFiippiuu )
    WRAP_GAMEFUNC( pfnRegisterEncoders, vFv )
    WRAP_GAMEFUNC( pfnGetWeaponData, iFpp )
    WRAP_GAMEFUNC( pfnCmdStart, vFppu )
    WRAP_GAMEFUNC( pfnCmdEnd, vFp )
    WRAP_GAMEFUNC( pfnConnectionlessPacket, iFpppp )
    WRAP_GAMEFUNC( pfnGetHullBounds, iFipp )
    WRAP_GAMEFUNC( pfnCreateInstancedBaselines, vFv )
    WRAP_GAMEFUNC( pfnInconsistentFile, iFppp )

}

void nullstub()
{

}

void *lastProcAddress;
void *give, *get2;
void HL_GiveFnPtrToDll( enginefuncs_t *funcs, void *globals )
{
    funcs->pfnDeltaAddEncoder = nullstub;
    HL_FillEngFuncs( &svengfuncs, funcs );
    // callback needed
    Push32(emu,globals);
    Push32(emu, &svengfuncs);
    DynaCall(emu, give);
    // stdcall, no stack restore
}

void HL_GetEntityAPI2( void *ftable, int *version )
{
    static DLL_FUNCTIONS funcs;
    Push32(emu,version);
    Push32(emu, &funcs);
    DynaCall(emu, get2);
    HL_FillGameFuncs( ftable, &funcs );
    R_ESP += 8;
}

void HL_SpawnFunc(void *arg)
{
    Push32(emu,arg);
    DynaCall(emu, lastProcAddress);
    R_ESP += 4;
}



void HL_Init()
{
    InitX86Emu();
    serverbridge = NewBridge();
}

void *HL_GetProcAddress( void *lib, const char *proc )
{
    void *addr = GetProcAddress(lib, proc);
    lastProcAddress = NULL;
    if( !strcmp( proc, "GiveFnptrsToDll") )
    {
        give = addr;
        return HL_GiveFnPtrToDll;
    }
    else if( !strcmp( proc, "GetEntityAPI2") )
    {
        get2 = addr;
        return HL_GetEntityAPI2;
    }
    else if( !strcmp( proc, "Server_GetPhysicsInterface") || !strcmp( proc, "GetEntityAPI") || !strcmp( proc, "Server_GetBlendingInterface" ) )
        return NULL;
    else if( addr )
    {
        printf("spawnfunc %s\n", proc);
        lastProcAddress = addr;
        return HL_SpawnFunc;
    }
    else return NULL;
}

#else
static void WINAPI (*GiveFnPtrsToDll)( void*, void* );

void HL_Init()
{
}

void (*pfnPM_Init_orig)( struct playermove_s *pm );

void pfnPM_Init( struct playermove_s *pm )
{
    orig_playerTrace = pm->PM_PlayerTrace;
    pm->PM_PlayerTrace = playerTrace;
    pfnPM_Init_orig( pm );
}
static void (*GetEntityAPI2)( DLL_FUNCTIONS *ftable, int *version );
void HL_GetEntityAPI2( DLL_FUNCTIONS *ftable, int *version )
{
    GetEntityAPI2(ftable,version);
    pfnPM_Init_orig = ftable->pfnPM_Init;
    ftable->pfnPM_Init = pfnPM_Init;
}

static void GiveFnPtrsToDll_wrap( void* f, void* g )
{
	GiveFnPtrsToDll( f, g );
}
void *HL_GetProcAddress( void *lib, const char *proc )
{
    void *addr = GetProcAddress(lib, proc);

    // stdcall
    if( !strcmp( proc, "GiveFnptrsToDll") )
    {
        GiveFnPtrsToDll = addr;
        return GiveFnPtrsToDll_wrap;
    }
    // PlayerTrace
    else if( !strcmp( proc, "GetEntityAPI2") )
    {
        GetEntityAPI2 = addr;
        return HL_GetEntityAPI2;
    }
    return addr;
}
#endif

