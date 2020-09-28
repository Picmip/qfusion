/*
Copyright (C) 2008 German Garcia

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

#include "g_local.h"
#include "g_as_local.h"

//=======================================================================

static const gs_asEnumVal_t asSpawnSystemEnumVals[] =
{
	ASLIB_ENUM_VAL( SPAWNSYSTEM_INSTANT ),
	ASLIB_ENUM_VAL( SPAWNSYSTEM_WAVES ),
	ASLIB_ENUM_VAL( SPAWNSYSTEM_HOLD ),

	ASLIB_ENUM_VAL_NULL
};

static const gs_asEnumVal_t asMovetypeEnumVals[] =
{
	ASLIB_ENUM_VAL( MOVETYPE_NONE ),
	ASLIB_ENUM_VAL( MOVETYPE_PLAYER ),
	ASLIB_ENUM_VAL( MOVETYPE_NOCLIP ),
	ASLIB_ENUM_VAL( MOVETYPE_PUSH ),
	ASLIB_ENUM_VAL( MOVETYPE_STOP ),
	ASLIB_ENUM_VAL( MOVETYPE_FLY ),
	ASLIB_ENUM_VAL( MOVETYPE_TOSS ),
	ASLIB_ENUM_VAL( MOVETYPE_LINEARPROJECTILE ),
	ASLIB_ENUM_VAL( MOVETYPE_BOUNCE ),
	ASLIB_ENUM_VAL( MOVETYPE_BOUNCEGRENADE ),
	ASLIB_ENUM_VAL( MOVETYPE_TOSSSLIDE ),

	ASLIB_ENUM_VAL_NULL
};

static const gs_asEnumVal_t asInstagibNegItemMaskEnumVals[] =
{
	ASLIB_ENUM_VAL( G_INSTAGIB_NEGATE_ITEMMASK ),

	ASLIB_ENUM_VAL_NULL
};

static const gs_asEnumVal_t asDamageEnumVals[] =
{
	ASLIB_ENUM_VAL( DAMAGE_NO ),
	ASLIB_ENUM_VAL( DAMAGE_YES ),
	ASLIB_ENUM_VAL( DAMAGE_AIM ),

	ASLIB_ENUM_VAL_NULL
};

static const gs_asEnumVal_t asMiscelaneaEnumVals[] =
{
	ASLIB_ENUM_VAL_NULL
};

//=======================================================================

static const gs_asEnum_t asGameEnums[] =
{
	{ "spawnsystem_e", asSpawnSystemEnumVals },
	{ "movetype_e", asMovetypeEnumVals },

	{ "G_INSTAGIB_NEGATE_ITEMMASK_e", asInstagibNegItemMaskEnumVals },

	{ "takedamage_e", asDamageEnumVals },
	{ "miscelanea_e", asMiscelaneaEnumVals },

	ASLIB_ENUM_VAL_NULL
};

//=======================================================================

static asITypeInfo *asEntityArrayType() {
	asIScriptContext *ctx = game.asExport->asGetActiveContext();
	asIScriptEngine *engine = ctx->GetEngine();
	asITypeInfo *ot = engine->GetTypeInfoById( engine->GetTypeIdByDecl( "array<Entity @>" ) );
	return ot;
}

//=======================================================================

// CLASS: Match

static void objectMatch_launchState( int state, match_t *self ) {
	if( state >= MATCH_STATE_NONE && state < MATCH_STATE_TOTAL ) {
		G_Match_LaunchState( state );
	}
}

static void objectMatch_startAutorecord( match_t *self ) {
	G_Match_Autorecord_Start();
}

static void objectMatch_stopAutorecord( match_t *self ) {
	G_Match_Autorecord_Stop();
}

static bool objectMatch_scoreLimitHit( match_t *self ) {
	return G_Match_ScorelimitHit();
}

static bool objectMatch_timeLimitHit( match_t *self ) {
	return G_Match_TimelimitHit();
}

static bool objectMatch_isTied( match_t *self ) {
	return G_Match_Tied();
}

static bool objectMatch_checkExtendPlayTime( match_t *self ) {
	return G_Match_CheckExtendPlayTime();
}

static bool objectMatch_suddenDeathFinished( match_t *self ) {
	return G_Match_SuddenDeathFinished();
}

static bool objectMatch_isPaused( match_t *self ) {
	return GS_MatchPaused();
}

static bool objectMatch_isWaiting( match_t *self ) {
	return GS_MatchWaiting();
}

static bool objectMatch_isExtended( match_t *self ) {
	return GS_MatchExtended();
}

static unsigned int objectMatch_duration( match_t *self ) {
	return GS_MatchDuration();
}

static int64_t objectMatch_startTime( match_t *self ) {
	return GS_MatchStartTime();
}

static int64_t objectMatch_endTime( match_t *self ) {
	return GS_MatchEndTime();
}

static int objectMatch_getState( match_t *self ) {
	return GS_MatchState();
}

static asstring_t *objectMatch_getName( match_t *self ) {
	const char *s = trap_GetConfigString( CS_MATCHNAME );

	return game.asExport->asStringFactoryBuffer( s, strlen( s ) );
}

static asstring_t *objectMatch_getScore( match_t *self ) {
	const char *s = trap_GetConfigString( CS_MATCHSCORE );

	return game.asExport->asStringFactoryBuffer( s, strlen( s ) );
}

static void objectMatch_setName( asstring_t *name, match_t *self ) {
	char buf[MAX_CONFIGSTRING_CHARS];

	COM_SanitizeColorString( name->buffer, buf, sizeof( buf ), -1, COLOR_WHITE );

	trap_ConfigString( CS_MATCHNAME, buf );
}

static void objectMatch_setScore( asstring_t *name, match_t *self ) {
	char buf[MAX_CONFIGSTRING_CHARS];

	COM_SanitizeColorString( name->buffer, buf, sizeof( buf ), -1, COLOR_WHITE );

	trap_ConfigString( CS_MATCHSCORE, buf );
}

static void objectMatch_setClockOverride( int64_t time, match_t *self ) {
	gs.gameState.stats[GAMESTAT_CLOCKOVERRIDE] = time;
}

static const gs_asMethod_t match_Methods[] =
{
	{ ASLIB_FUNCTION_DECL( void, launchState, (int state) const ), asFUNCTION( objectMatch_launchState ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, startAutorecord, ( ) const ), asFUNCTION( objectMatch_startAutorecord ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, stopAutorecord, ( ) const ), asFUNCTION( objectMatch_stopAutorecord ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, scoreLimitHit, ( ) const ), asFUNCTION( objectMatch_scoreLimitHit ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, timeLimitHit, ( ) const ), asFUNCTION( objectMatch_timeLimitHit ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, isTied, ( ) const ), asFUNCTION( objectMatch_isTied ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, checkExtendPlayTime, ( ) const ), asFUNCTION( objectMatch_checkExtendPlayTime ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, suddenDeathFinished, ( ) const ), asFUNCTION( objectMatch_suddenDeathFinished ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, isPaused, ( ) const ), asFUNCTION( objectMatch_isPaused ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, isWaiting, ( ) const ), asFUNCTION( objectMatch_isWaiting ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, isExtended, ( ) const ), asFUNCTION( objectMatch_isExtended ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( uint, duration, ( ) const ), asFUNCTION( objectMatch_duration ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int64, startTime, ( ) const ), asFUNCTION( objectMatch_startTime ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int64, endTime, ( ) const ), asFUNCTION( objectMatch_endTime ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, getState, ( ) const ), asFUNCTION( objectMatch_getState ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_name, ( ) const ), asFUNCTION( objectMatch_getName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, getScore, ( ) const ), asFUNCTION( objectMatch_getScore ),  asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_name, ( String & in ) ), asFUNCTION( objectMatch_setName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setScore, ( String & in ) ), asFUNCTION( objectMatch_setScore ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setClockOverride, ( int64 milliseconds ) ), asFUNCTION( objectMatch_setClockOverride ), asCALL_CDECL_OBJLAST },

	ASLIB_METHOD_NULL
};

static const gs_asClassDescriptor_t asMatchClassDescriptor =
{
	"Match",													/* name */
	static_cast<asEObjTypeFlags>( asOBJ_REF | asOBJ_NOHANDLE ), /* object type flags */
	sizeof( match_t ),											/* size */
	NULL,														/* funcdefs */
	NULL,														/* object behaviors */
	match_Methods,												/* methods */
	NULL,														/* properties */
	NULL, NULL,													/* string factory hack */
};

//=======================================================================

// CLASS: GametypeDesc

static asstring_t *objectGametypeDescriptor_getTitle( gametype_descriptor_t *self ) {
	const char *s = trap_GetConfigString( CS_GAMETYPETITLE );

	return game.asExport->asStringFactoryBuffer( s, strlen( s ) );
}

static void objectGametypeDescriptor_setTitle( asstring_t *other, gametype_descriptor_t *self ) {
	if( !other || !other->buffer ) {
		return;
	}

	trap_ConfigString( CS_GAMETYPETITLE, other->buffer );
}

static asstring_t *objectGametypeDescriptor_getName( gametype_descriptor_t *self ) {
	return game.asExport->asStringFactoryBuffer( gs.gametypeName, strlen( gs.gametypeName ) );
}

static asstring_t *objectGametypeDescriptor_getVersion( gametype_descriptor_t *self ) {
	const char *s = trap_GetConfigString( CS_GAMETYPEVERSION );

	return game.asExport->asStringFactoryBuffer( s, strlen( s ) );
}

static void objectGametypeDescriptor_setVersion( asstring_t *other, gametype_descriptor_t *self ) {
	if( !other || !other->buffer ) {
		return;
	}

	trap_ConfigString( CS_GAMETYPEVERSION, other->buffer );
}

static asstring_t *objectGametypeDescriptor_getAuthor( gametype_descriptor_t *self ) {
	const char *s = trap_GetConfigString( CS_GAMETYPEAUTHOR );

	return game.asExport->asStringFactoryBuffer( s, strlen( s ) );
}

static void objectGametypeDescriptor_setAuthor( asstring_t *other, gametype_descriptor_t *self ) {
	if( !other || !other->buffer ) {
		return;
	}

	trap_ConfigString( CS_GAMETYPEAUTHOR, other->buffer );
}

static asstring_t *objectGametypeDescriptor_getManifest( gametype_descriptor_t *self ) {
	const char *s = trap_GetConfigString( CS_MODMANIFEST );

	return game.asExport->asStringFactoryBuffer( s, strlen( s ) );
}

static void objectGametypeDescriptor_SetTeamSpawnsystem( int team, int spawnsystem, int wave_time, int wave_maxcount, bool spectate_team, gametype_descriptor_t *self ) {
	G_SpawnQueue_SetTeamSpawnsystem( team, spawnsystem, wave_time, wave_maxcount, spectate_team );
}

static bool objectGametypeDescriptor_isInstagib( gametype_descriptor_t *self ) {
	return GS_Instagib();
}

static bool objectGametypeDescriptor_hasFallDamage( gametype_descriptor_t *self ) {
	return GS_FallDamage();
}

static bool objectGametypeDescriptor_hasSelfDamage( gametype_descriptor_t *self ) {
	return GS_SelfDamage();
}

static bool objectGametypeDescriptor_isInvidualGameType( gametype_descriptor_t *self ) {
	return GS_InvidualGameType();
}

static const gs_asMethod_t gametypedescr_Methods[] =
{
	{ ASLIB_FUNCTION_DECL( const String @, get_name, ( ) const ), asFUNCTION( objectGametypeDescriptor_getName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_title, ( ) const ), asFUNCTION( objectGametypeDescriptor_getTitle ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_title, ( String & ) ), asFUNCTION( objectGametypeDescriptor_setTitle ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_version, ( ) const ), asFUNCTION( objectGametypeDescriptor_getVersion ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_version, ( String & ) ), asFUNCTION( objectGametypeDescriptor_setVersion ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_author, ( ) const ), asFUNCTION( objectGametypeDescriptor_getAuthor ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_author, ( String & ) ), asFUNCTION( objectGametypeDescriptor_setAuthor ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_manifest, ( ) const ), asFUNCTION( objectGametypeDescriptor_getManifest ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setTeamSpawnsystem, ( int team, int spawnsystem, int wave_time, int wave_maxcount, bool deadcam ) ), asFUNCTION( objectGametypeDescriptor_SetTeamSpawnsystem ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, get_isInstagib, ( ) const ), asFUNCTION( objectGametypeDescriptor_isInstagib ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, get_hasFallDamage, ( ) const ), asFUNCTION( objectGametypeDescriptor_hasFallDamage ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, get_hasSelfDamage, ( ) const ), asFUNCTION( objectGametypeDescriptor_hasSelfDamage ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, get_isInvidualGameType, ( ) const ), asFUNCTION( objectGametypeDescriptor_isInvidualGameType ), asCALL_CDECL_OBJLAST },

	ASLIB_METHOD_NULL,
};

static const gs_asProperty_t gametypedescr_Properties[] =
{
	{ ASLIB_PROPERTY_DECL( uint, spawnableItemsMask ), ASLIB_FOFFSET( gametype_descriptor_t, spawnableItemsMask ) },
	{ ASLIB_PROPERTY_DECL( uint, respawnableItemsMask ), ASLIB_FOFFSET( gametype_descriptor_t, respawnableItemsMask ) },
	{ ASLIB_PROPERTY_DECL( uint, dropableItemsMask ), ASLIB_FOFFSET( gametype_descriptor_t, dropableItemsMask ) },
	{ ASLIB_PROPERTY_DECL( uint, pickableItemsMask ), ASLIB_FOFFSET( gametype_descriptor_t, pickableItemsMask ) },
	{ ASLIB_PROPERTY_DECL( bool, isTeamBased ), ASLIB_FOFFSET( gametype_descriptor_t, isTeamBased ) },
	{ ASLIB_PROPERTY_DECL( bool, isRace ), ASLIB_FOFFSET( gametype_descriptor_t, isRace ) },
	{ ASLIB_PROPERTY_DECL( bool, isTutorial ), ASLIB_FOFFSET( gametype_descriptor_t, isTutorial ) },
	{ ASLIB_PROPERTY_DECL( bool, inverseScore ), ASLIB_FOFFSET( gametype_descriptor_t, inverseScore ) },
	{ ASLIB_PROPERTY_DECL( bool, hasChallengersQueue ), ASLIB_FOFFSET( gametype_descriptor_t, hasChallengersQueue ) },
	{ ASLIB_PROPERTY_DECL( bool, hasChallengersRoulette ), ASLIB_FOFFSET( gametype_descriptor_t, hasChallengersRoulette ) },
	{ ASLIB_PROPERTY_DECL( int, maxPlayersPerTeam ), ASLIB_FOFFSET( gametype_descriptor_t, maxPlayersPerTeam ) },
	{ ASLIB_PROPERTY_DECL( int, ammoRespawn ), ASLIB_FOFFSET( gametype_descriptor_t, ammo_respawn ) },
	{ ASLIB_PROPERTY_DECL( int, armorRespawn ), ASLIB_FOFFSET( gametype_descriptor_t, armor_respawn ) },
	{ ASLIB_PROPERTY_DECL( int, weaponRespawn ), ASLIB_FOFFSET( gametype_descriptor_t, weapon_respawn ) },
	{ ASLIB_PROPERTY_DECL( int, healthRespawn ), ASLIB_FOFFSET( gametype_descriptor_t, health_respawn ) },
	{ ASLIB_PROPERTY_DECL( int, powerupRespawn ), ASLIB_FOFFSET( gametype_descriptor_t, powerup_respawn ) },
	{ ASLIB_PROPERTY_DECL( int, megahealthRespawn ), ASLIB_FOFFSET( gametype_descriptor_t, megahealth_respawn ) },
	{ ASLIB_PROPERTY_DECL( int, ultrahealthRespawn ), ASLIB_FOFFSET( gametype_descriptor_t, ultrahealth_respawn ) },
	{ ASLIB_PROPERTY_DECL( bool, readyAnnouncementEnabled ), ASLIB_FOFFSET( gametype_descriptor_t, readyAnnouncementEnabled ) },
	{ ASLIB_PROPERTY_DECL( bool, scoreAnnouncementEnabled ), ASLIB_FOFFSET( gametype_descriptor_t, scoreAnnouncementEnabled ) },
	{ ASLIB_PROPERTY_DECL( bool, countdownEnabled ), ASLIB_FOFFSET( gametype_descriptor_t, countdownEnabled ) },
	{ ASLIB_PROPERTY_DECL( bool, mathAbortDisabled ), ASLIB_FOFFSET( gametype_descriptor_t, matchAbortDisabled ) },
	{ ASLIB_PROPERTY_DECL( bool, matchAbortDisabled ), ASLIB_FOFFSET( gametype_descriptor_t, matchAbortDisabled ) },
	{ ASLIB_PROPERTY_DECL( bool, shootingDisabled ), ASLIB_FOFFSET( gametype_descriptor_t, shootingDisabled ) },
	{ ASLIB_PROPERTY_DECL( bool, infiniteAmmo ), ASLIB_FOFFSET( gametype_descriptor_t, infiniteAmmo ) },
	{ ASLIB_PROPERTY_DECL( bool, canForceModels ), ASLIB_FOFFSET( gametype_descriptor_t, canForceModels ) },
	{ ASLIB_PROPERTY_DECL( bool, canShowMinimap ), ASLIB_FOFFSET( gametype_descriptor_t, canShowMinimap ) },
	{ ASLIB_PROPERTY_DECL( bool, teamOnlyMinimap ), ASLIB_FOFFSET( gametype_descriptor_t, teamOnlyMinimap ) },
	{ ASLIB_PROPERTY_DECL( int, spawnpointRadius ), ASLIB_FOFFSET( gametype_descriptor_t, spawnpointRadius ) },
	{ ASLIB_PROPERTY_DECL( bool, customDeadBodyCam ), ASLIB_FOFFSET( gametype_descriptor_t, customDeadBodyCam ) },
	{ ASLIB_PROPERTY_DECL( bool, removeInactivePlayers ), ASLIB_FOFFSET( gametype_descriptor_t, removeInactivePlayers ) },
	{ ASLIB_PROPERTY_DECL( bool, mmCompatible ), ASLIB_FOFFSET( gametype_descriptor_t, mmCompatible ) },
	{ ASLIB_PROPERTY_DECL( uint, numBots ), ASLIB_FOFFSET( gametype_descriptor_t, numBots ) },
	{ ASLIB_PROPERTY_DECL( bool, dummyBots ), ASLIB_FOFFSET( gametype_descriptor_t, dummyBots ) },
	{ ASLIB_PROPERTY_DECL( uint, forceTeamHumans ), ASLIB_FOFFSET( gametype_descriptor_t, forceTeamHumans ) },
	{ ASLIB_PROPERTY_DECL( uint, forceTeamBots ), ASLIB_FOFFSET( gametype_descriptor_t, forceTeamBots ) },
	{ ASLIB_PROPERTY_DECL( bool, disableObituaries ), ASLIB_FOFFSET( gametype_descriptor_t, disableObituaries ) },

	ASLIB_PROPERTY_NULL,
};

static const gs_asClassDescriptor_t asGametypeClassDescriptor = {
	"GametypeDesc",					 /* name */
	asOBJ_REF | asOBJ_NOHANDLE,		 /* object type flags */
	sizeof( gametype_descriptor_t ), /* size */
	NULL,							 /* funcdefs */
	NULL,							 /* object behaviors */
	gametypedescr_Methods,			 /* methods */
	gametypedescr_Properties,		 /* properties */
	NULL, NULL						 /* string factory hack */
};

//=======================================================================

// CLASS: Team
static edict_t *objectTeamlist_GetPlayerEntity( int index, g_teamlist_t *obj ) {
	if( index < 0 || index >= obj->numplayers ) {
		return NULL;
	}

	if( obj->playerIndices[index] < 1 || obj->playerIndices[index] > gs.maxclients ) {
		return NULL;
	}

	return &game.edicts[ obj->playerIndices[index] ];
}

static asstring_t *objectTeamlist_getName( g_teamlist_t *obj ) {
	const char *name = GS_TeamName( obj - teamlist );

	return game.asExport->asStringFactoryBuffer( name, name ? strlen( name ) : 0 );
}

static asstring_t *objectTeamlist_getDefaultName( g_teamlist_t *obj ) {
	const char *name = GS_DefaultTeamName( obj - teamlist );

	return game.asExport->asStringFactoryBuffer( name, name ? strlen( name ) : 0 );
}

static void objectTeamlist_setName( asstring_t *str, g_teamlist_t *obj ) {
	int team;
	char buf[MAX_CONFIGSTRING_CHARS];

	team = obj - teamlist;
	if( team < TEAM_ALPHA || team > TEAM_BETA ) {
		return;
	}

	COM_SanitizeColorString( str->buffer, buf, sizeof( buf ), -1, COLOR_WHITE );

	trap_ConfigString( CS_TEAM_ALPHA_NAME + team - TEAM_ALPHA, buf );
}

static bool objectTeamlist_IsLocked( g_teamlist_t *obj ) {
	return G_Teams_TeamIsLocked( obj - teamlist );
}

static bool objectTeamlist_Lock( g_teamlist_t *obj ) {
	return ( obj ? G_Teams_LockTeam( obj - teamlist ) : false );
}

static bool objectTeamlist_Unlock( g_teamlist_t *obj ) {
	return ( obj ? G_Teams_UnLockTeam( obj - teamlist ) : false );
}

static void objectTeamlist_ClearInvites( g_teamlist_t *obj ) {
	obj->invited[0] = 0;
}

static int objectTeamlist_getTeamIndex( g_teamlist_t *obj ) {
	int index = ( obj - teamlist );

	if( index < 0 || index >= GS_MAX_TEAMS ) {
		return -1;
	}

	return index;
}

static const gs_asMethod_t teamlist_Methods[] =
{
	{ ASLIB_FUNCTION_DECL( Entity @, ent, ( int index ) ), asFUNCTION( objectTeamlist_GetPlayerEntity ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_name, ( ) const ), asFUNCTION( objectTeamlist_getName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_defaultName, ( ) const ), asFUNCTION( objectTeamlist_getDefaultName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_name, ( String & in ) ), asFUNCTION( objectTeamlist_setName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, isLocked, ( ) const ), asFUNCTION( objectTeamlist_IsLocked ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, lock, ( ) const ), asFUNCTION( objectTeamlist_Lock ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, unlock, ( ) const ), asFUNCTION( objectTeamlist_Unlock ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, clearInvites, ( ) ), asFUNCTION( objectTeamlist_ClearInvites ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, team, ( ) const ), asFUNCTION( objectTeamlist_getTeamIndex ), asCALL_CDECL_OBJLAST },

	ASLIB_METHOD_NULL
};

static const gs_asProperty_t teamlist_Properties[] =
{
	{ ASLIB_PROPERTY_DECL( Stats, stats ), ASLIB_FOFFSET( g_teamlist_t, stats ) },
	{ ASLIB_PROPERTY_DECL( const int, numPlayers ), ASLIB_FOFFSET( g_teamlist_t, numplayers ) },
	{ ASLIB_PROPERTY_DECL( const int, ping ), ASLIB_FOFFSET( g_teamlist_t, ping ) },
	{ ASLIB_PROPERTY_DECL( const bool, hasCoach ), ASLIB_FOFFSET( g_teamlist_t, has_coach ) },

	ASLIB_PROPERTY_NULL
};

static const gs_asClassDescriptor_t asTeamListClassDescriptor =
{
	"Team",					   /* name */
	asOBJ_REF | asOBJ_NOCOUNT, /* object type flags */
	sizeof( g_teamlist_t ),	   /* size */
	NULL,					   /* funcdefs */
	NULL,					   /* object behaviors */
	teamlist_Methods,		   /* methods */
	teamlist_Properties,	   /* properties */
	NULL, NULL,				   /* string factory hack */
};

//=======================================================================

// CLASS: Stats
static void objectScoreStats_Clear( score_stats_t *obj ) {
	memset( obj, 0, sizeof( *obj ) );
}

static int objectScoreStats_AccShots( int ammo, score_stats_t *obj ) {
	if( ammo < AMMO_GUNBLADE || ammo >= AMMO_TOTAL ) {
		return 0;
	}

	return obj->accuracy_shots[ ammo - AMMO_GUNBLADE ];
}

static int objectScoreStats_AccHits( int ammo, score_stats_t *obj ) {
	if( ammo < AMMO_GUNBLADE || ammo >= AMMO_TOTAL ) {
		return 0;
	}

	return obj->accuracy_hits[ ammo - AMMO_GUNBLADE ];
}

static int objectScoreStats_AccHitsDirect( int ammo, score_stats_t *obj ) {
	if( ammo < AMMO_GUNBLADE || ammo >= AMMO_TOTAL ) {
		return 0;
	}

	return obj->accuracy_hits_direct[ ammo - AMMO_GUNBLADE ];
}

static int objectScoreStats_AccHitsAir( int ammo, score_stats_t *obj ) {
	if( ammo < AMMO_GUNBLADE || ammo >= AMMO_TOTAL ) {
		return 0;
	}

	return obj->accuracy_hits_air[ ammo - AMMO_GUNBLADE ];
}

static int objectScoreStats_AccDamage( int ammo, score_stats_t *obj ) {
	if( ammo < AMMO_GUNBLADE || ammo >= AMMO_TOTAL ) {
		return 0;
	}

	return obj->accuracy_damage[ ammo - AMMO_GUNBLADE ];
}

static void objectScoreStats_ScoreSet( int newscore, score_stats_t *obj ) {
	obj->score = newscore;
}

static void objectScoreStats_ScoreAdd( int score, score_stats_t *obj ) {
	obj->score += score;
}

static void objectScoreStats_RoundAdd( score_stats_t *obj ) {
	obj->numrounds++;
}

static const gs_asMethod_t scorestats_Methods[] =
{
	{ ASLIB_FUNCTION_DECL( void, setScore, ( int i ) ), asFUNCTION( objectScoreStats_ScoreSet ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, addScore, ( int i ) ), asFUNCTION( objectScoreStats_ScoreAdd ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, addRound, ( ) ), asFUNCTION( objectScoreStats_RoundAdd ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, clear, ( ) ), asFUNCTION( objectScoreStats_Clear ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, accuracyShots, ( int ammo ) const ), asFUNCTION( objectScoreStats_AccShots ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, accuracyHits, ( int ammo ) const ), asFUNCTION( objectScoreStats_AccHits ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, accuracyHitsDirect, ( int ammo ) const ), asFUNCTION( objectScoreStats_AccHitsDirect ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, accuracyHitsAir, ( int ammo ) const ), asFUNCTION( objectScoreStats_AccHitsAir ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, accuracyDamage, ( int ammo ) const ), asFUNCTION( objectScoreStats_AccDamage ), asCALL_CDECL_OBJLAST },

	ASLIB_METHOD_NULL
};

static const gs_asProperty_t scorestats_Properties[] =
{
	{ ASLIB_PROPERTY_DECL( const int, score ), ASLIB_FOFFSET( score_stats_t, score ) },
	{ ASLIB_PROPERTY_DECL( const int, deaths ), ASLIB_FOFFSET( score_stats_t, deaths ) },
	{ ASLIB_PROPERTY_DECL( const int, frags ), ASLIB_FOFFSET( score_stats_t, frags ) },
	{ ASLIB_PROPERTY_DECL( const int, suicides ), ASLIB_FOFFSET( score_stats_t, suicides ) },
	{ ASLIB_PROPERTY_DECL( const int, teamFrags ), ASLIB_FOFFSET( score_stats_t, teamfrags ) },
	{ ASLIB_PROPERTY_DECL( const int, awards ), ASLIB_FOFFSET( score_stats_t, awards ) },
	{ ASLIB_PROPERTY_DECL( const int, totalDamageGiven ), ASLIB_FOFFSET( score_stats_t, total_damage_given ) },
	{ ASLIB_PROPERTY_DECL( const int, totalDamageReceived ), ASLIB_FOFFSET( score_stats_t, total_damage_received ) },
	{ ASLIB_PROPERTY_DECL( const int, totalTeamDamageGiven ), ASLIB_FOFFSET( score_stats_t, total_teamdamage_given ) },
	{ ASLIB_PROPERTY_DECL( const int, totalTeamDamageReceived ), ASLIB_FOFFSET( score_stats_t, total_teamdamage_received ) },
	{ ASLIB_PROPERTY_DECL( const int, healthTaken ), ASLIB_FOFFSET( score_stats_t, health_taken ) },
	{ ASLIB_PROPERTY_DECL( const int, armorTaken ), ASLIB_FOFFSET( score_stats_t, armor_taken ) },

	ASLIB_PROPERTY_NULL
};

static const gs_asClassDescriptor_t asScoreStatsClassDescriptor =
{
	"Stats",				   /* name */
	asOBJ_REF | asOBJ_NOCOUNT, /* object type flags */
	sizeof( score_stats_t ),   /* size */
	NULL,					   /* funcdefs */
	NULL,					   /* object behaviors */
	scorestats_Methods,		   /* methods */
	scorestats_Properties,	   /* properties */
	NULL, NULL,				   /* string factory hack */
};

//=======================================================================

// CLASS: Bot

static const gs_asFuncdef_t asbot_Funcdefs[] =
{
	ASLIB_FUNCDEF_NULL
};

static const gs_asBehavior_t asbot_ObjectBehaviors[] =
{
	ASLIB_BEHAVIOR_NULL
};

static const gs_asMethod_t asbot_Methods[] =
{
	{ ASLIB_FUNCTION_DECL(void, clearGoalWeights, ()), asFUNCTION(AI_ClearWeights), asCALL_CDECL_OBJFIRST },
	{ ASLIB_FUNCTION_DECL(void, resetGoalWeights, ()), asFUNCTION(AI_ResetWeights), asCALL_CDECL_OBJFIRST },
	{ ASLIB_FUNCTION_DECL(void, setGoalWeight, ( int i, float weight )), asFUNCTION(AI_SetGoalWeight), asCALL_CDECL_OBJFIRST },
	{ ASLIB_FUNCTION_DECL(float, getItemWeight, ( const Item @item ) const), asFUNCTION(AI_GetItemWeight), asCALL_CDECL_OBJFIRST },

	// character
	{ ASLIB_FUNCTION_DECL(float, get_reactionTime, () const), asFUNCTION(AI_GetCharacterReactionTime), asCALL_CDECL_OBJFIRST },
	{ ASLIB_FUNCTION_DECL(float, get_offensiveness, () const), asFUNCTION(AI_GetCharacterOffensiveness), asCALL_CDECL_OBJFIRST },
	{ ASLIB_FUNCTION_DECL(float, get_campiness, () const), asFUNCTION(AI_GetCharacterCampiness), asCALL_CDECL_OBJFIRST },
	{ ASLIB_FUNCTION_DECL(float, get_firerate, () const), asFUNCTION(AI_GetCharacterFirerate), asCALL_CDECL_OBJFIRST },

	ASLIB_METHOD_NULL
};

static const gs_asProperty_t asbot_Properties[] =
{
	ASLIB_PROPERTY_NULL
};

static const gs_asClassDescriptor_t asBotClassDescriptor =
{
	"Bot",						/* name */
	asOBJ_REF|asOBJ_NOCOUNT,	/* object type flags */
	ai_handle_size,				/* size */
	asbot_Funcdefs,				/* funcdefs */
	asbot_ObjectBehaviors,		/* object behaviors */
	asbot_Methods,				/* methods */
	asbot_Properties,			/* properties */

	NULL, NULL					/* string factory hack */
};

//=======================================================================

// CLASS: Client
static int objectGameClient_PlayerNum( gclient_t *self ) {
	if( self->asFactored ) {
		return -1;
	}
	return PLAYERNUM( self );
}

static bool objectGameClient_isReady( gclient_t *self ) {
	if( self->asFactored ) {
		return false;
	}

	return ( level.ready[self - game.clients] || GS_MatchState() == MATCH_STATE_PLAYTIME ) ? true : false;
}

static bool objectGameClient_isBot( gclient_t *self ) {
	int playerNum;
	const edict_t *ent;

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 && playerNum >= gs.maxclients ) {
		return false;
	}

	ent = PLAYERENT( playerNum );
	return ( ( ent->r.svflags & SVF_FAKECLIENT ) && AI_GetType( ent->ai ) == AI_ISBOT );
}

static ai_handle_t *objectGameClient_getBot( gclient_t *self )
{
	int			   playerNum;
	const edict_t *ent;

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 && playerNum >= gs.maxclients )
		return NULL;

	ent = PLAYERENT( playerNum );
	if( !( ent->r.svflags & SVF_FAKECLIENT ) || AI_GetType( ent->ai ) != AI_ISBOT )
		return NULL;

	return ent->ai;
}

static int objectGameClient_ClientState( gclient_t *self ) {
	if( self->asFactored ) {
		return CS_FREE;
	}

	return trap_GetClientState( (int)( self - game.clients ) );
}

static void objectGameClient_ClearPlayerStateEvents( gclient_t *self ) {
	G_ClearPlayerStateEvents( self );
}

static asstring_t *objectGameClient_getName( gclient_t *self ) {
	char temp[MAX_NAME_BYTES + 2];

	Q_strncpyz( temp, self->netname, sizeof( temp ) );
	Q_strncatz( temp, S_COLOR_WHITE, sizeof( temp ) );

	return game.asExport->asStringFactoryBuffer( temp, strlen( temp ) );
}

static asstring_t *objectGameClient_getClanName( gclient_t *self ) {
	char temp[MAX_CLANNAME_BYTES + 2];

	Q_strncpyz( temp, self->clanname, sizeof( temp ) );
	Q_strncatz( temp, S_COLOR_WHITE, sizeof( temp ) );

	return game.asExport->asStringFactoryBuffer( temp, strlen( temp ) );
}

static asstring_t *objectGameClient_getMMLogin( gclient_t *self ) {
	const char *login = NULL;

	if( self->mm_session > 0 ) {
		login = Info_ValueForKey( self->userinfo, "cl_mm_login" );
	}
	if( !login ) {
		login = "";
	}

	return game.asExport->asStringFactoryBuffer( login, strlen( login ) );
}

static void objectGameClient_Respawn( bool ghost, gclient_t *self ) {
	int playerNum;

	if( self->asFactored ) {
		return;
	}

	playerNum = objectGameClient_PlayerNum( self );

	if( playerNum >= 0 && playerNum < gs.maxclients ) {
		G_ClientRespawn( &game.edicts[playerNum + 1], ghost );
	}
}

static edict_t *objectGameClient_GetEntity( gclient_t *self ) {
	int playerNum;

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return NULL;
	}

	return PLAYERENT( playerNum );
}

static int objectGameClient_InventoryCount( int index, gclient_t *self ) {
	if( index < 0 || index >= MAX_ITEMS ) {
		return 0;
	}

	return self->ps.inventory[ index ];
}

static void objectGameClient_InventorySetCount( int index, int newcount, gclient_t *self ) {
	gsitem_t *it;

	if( index < 0 || index >= MAX_ITEMS ) {
		return;
	}

	it = GS_FindItemByTag( index );
	if( !it ) {
		return;
	}

	if( newcount == 0 && ( it->type & IT_WEAPON ) ) {
		if( index == self->ps.stats[STAT_PENDING_WEAPON] ) {
			self->ps.stats[STAT_PENDING_WEAPON] = self->ps.stats[STAT_WEAPON];
		} else if( index == self->ps.stats[STAT_WEAPON] ) {
			self->ps.stats[STAT_WEAPON] = self->ps.stats[STAT_PENDING_WEAPON] = WEAP_NONE;
			self->ps.weaponState = WEAPON_STATE_READY;
		}
	}

	self->ps.inventory[ index ] = newcount;
}

static void objectGameClient_InventoryGiveItemExt( int index, int count, gclient_t *self ) {
	const gsitem_t *it;
	int playerNum;

	if( index < 0 || index >= MAX_ITEMS ) {
		return;
	}

	it = GS_FindItemByTag( index );
	if( !it ) {
		return;
	}

	if( !( it->flags & ITFLAG_PICKABLE ) ) {
		return;
	}

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	G_PickupItem( PLAYERENT( playerNum ), it, 0, count < 0 ? it->quantity : count, NULL );
}

static void objectGameClient_InventoryGiveItem( int index, gclient_t *self ) {
	objectGameClient_InventoryGiveItemExt( index, -1, self );
}

static void objectGameClient_InventoryClear( gclient_t *self ) {
	memset( self->ps.inventory, 0, sizeof( self->ps.inventory ) );

	self->ps.stats[STAT_WEAPON] = self->ps.stats[STAT_PENDING_WEAPON] = WEAP_NONE;
	self->ps.weaponState = WEAPON_STATE_READY;
}

static bool objectGameClient_CanSelectWeapon( int index, gclient_t *self ) {
	if( index < WEAP_NONE || index >= WEAP_TOTAL ) {
		return false;
	}

	return ( GS_CheckAmmoInWeapon( &self->ps, index ) ) == true;
}

static void objectGameClient_SelectWeapon( int index, gclient_t *self ) {
	if( index < WEAP_NONE || index >= WEAP_TOTAL ) {
		self->ps.stats[STAT_PENDING_WEAPON] = GS_SelectBestWeapon( &self->ps );
		return;
	}

	if( GS_CheckAmmoInWeapon( &self->ps, index ) ) {
		self->ps.stats[STAT_PENDING_WEAPON] = index;
	}
}

static void objectGameClient_addAward( asstring_t *msg, gclient_t *self ) {
	int playerNum;

	if( !msg ) {
		return;
	}

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	G_PlayerAward( PLAYERENT( playerNum ), msg->buffer );
}

static void objectGameClient_addMetaAward( asstring_t *msg, gclient_t *self ) {
	int playerNum;

	if( !msg ) {
		return;
	}

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	G_PlayerMetaAward( PLAYERENT( playerNum ), msg->buffer );
}

static void objectGameClient_execGameCommand( asstring_t *msg, gclient_t *self ) {
	int playerNum;

	if( !msg ) {
		return;
	}

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	trap_GameCmd( PLAYERENT( playerNum ), msg->buffer );
}

static void objectGameClient_setHUDStat( int stat, int value, gclient_t *self ) {
	if( !ISGAMETYPESTAT( stat ) ) {
		if( stat > 0 && stat < GS_GAMETYPE_STATS_START ) {
			G_Printf( "* WARNING: stat %i is write protected\n", stat );
		} else {
			G_Printf( "* WARNING: %i is not a valid stat\n", stat );
		}
		return;
	}

	self->ps.stats[ stat ] = ( (short)value & 0xFFFF );
}

static int objectGameClient_getHUDStat( int stat, gclient_t *self ) {
	if( stat < 0 && stat >= PS_MAX_STATS ) {
		G_Printf( "* WARNING: stat %i is out of range\n", stat );
		return 0;
	}
	return self->ps.stats[ stat ];
}

static void objectGameClient_setPMoveFeatures( unsigned int bitmask, gclient_t *self ) {
	self->ps.pmove.stats[PM_STAT_FEATURES] = ( bitmask & PMFEAT_ALL );
}

static unsigned int objectGameClient_getPMoveFeatures( gclient_t *self ) {
	return self->ps.pmove.stats[PM_STAT_FEATURES];
}

static unsigned int objectGameClient_getPressedKeys( gclient_t *self ) {
	return self->ps.plrkeys;
}

static void objectGameClient_setPMoveMaxSpeed( float speed, gclient_t *self ) {
	if( speed < 0.0f ) {
		self->ps.pmove.stats[PM_STAT_MAXSPEED] = (short)DEFAULT_PLAYERSPEED;
	} else {
		self->ps.pmove.stats[PM_STAT_MAXSPEED] = ( (int)speed & 0xFFFF );
	}
}

static float objectGameClient_getPMoveMaxSpeed( gclient_t *self ) {
	return self->ps.pmove.stats[PM_STAT_MAXSPEED];
}

static void objectGameClient_setPMoveJumpSpeed( float speed, gclient_t *self ) {
	if( speed < 0.0f ) {
		self->ps.pmove.stats[PM_STAT_JUMPSPEED] = (short)DEFAULT_JUMPSPEED;
	} else {
		self->ps.pmove.stats[PM_STAT_JUMPSPEED] = ( (int)speed & 0xFFFF );
	}
}

static float objectGameClient_getPMoveJumpSpeed( gclient_t *self ) {
	return self->ps.pmove.stats[PM_STAT_JUMPSPEED];
}

static void objectGameClient_setPMoveDashSpeed( float speed, gclient_t *self ) {
	if( speed < 0.0f ) {
		self->ps.pmove.stats[PM_STAT_DASHSPEED] = (short)DEFAULT_DASHSPEED;
	} else {
		self->ps.pmove.stats[PM_STAT_DASHSPEED] = ( (int)speed & 0xFFFF );
	}
}

static float objectGameClient_getPMoveDashSpeed( gclient_t *self ) {
	return self->ps.pmove.stats[PM_STAT_DASHSPEED];
}

static asstring_t *objectGameClient_getUserInfoKey( asstring_t *key, gclient_t *self ) {
	char *s;

	if( !key || !key->buffer || !key->buffer[0] ) {
		return game.asExport->asStringFactoryBuffer( NULL, 0 );
	}

	s = Info_ValueForKey( self->userinfo, key->buffer );
	if( !s || !*s ) {
		return game.asExport->asStringFactoryBuffer( NULL, 0 );
	}

	return game.asExport->asStringFactoryBuffer( s, strlen( s ) );
}

static void objectGameClient_printMessage( asstring_t *str, gclient_t *self ) {
	int playerNum;

	if( !str || !str->buffer ) {
		return;
	}

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	G_PrintMsg( PLAYERENT( playerNum ), "%s", str->buffer );
}

static void objectGameClient_ChaseCam( asstring_t *str, bool teamonly, gclient_t *self ) {
	int playerNum;

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	G_ChasePlayer( PLAYERENT( playerNum ), str ? str->buffer : NULL, teamonly, 0 );
}

static void objectGameClient_SetChaseActive( bool active, gclient_t *self ) {
	int playerNum;

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	self->resp.chase.active = active;
	G_UpdatePlayerMatchMsg( PLAYERENT( playerNum ) );
}

static bool objectGameClient_GetChaseActive( gclient_t *self ) {
	return self->resp.chase.active;
}

static void objectGameClient_NewRaceRun( int numSectors, gclient_t *self ) {
	int playerNum;

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	G_NewRaceRun( PLAYERENT( playerNum ), numSectors );
}

static void objectGameClient_SetRaceTime( int sector, int64_t time, gclient_t *self ) {
	int playerNum;

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	G_SetRaceTime( PLAYERENT( playerNum ), sector, time );
}

static void objectGameClient_SetHelpMessage( unsigned int index, gclient_t *self ) {
	int playerNum;

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	G_SetPlayerHelpMessage( PLAYERENT( playerNum ), index );
}

static void objectGameClient_SetOverlayMenuItems( asstring_t *str, gclient_t *self ) {
	int playerNum;

	if( !str || !str->buffer ) {
		return;
	}

	playerNum = objectGameClient_PlayerNum( self );
	if( playerNum < 0 || playerNum >= gs.maxclients ) {
		return;
	}

	if( objectGameClient_isBot( self ) ) {
		return;
	}

	Q_strncpyz( self->level.overlayMenuItems, str->buffer, sizeof( self->level.overlayMenuItems ) );
	trap_GameCmd( PLAYERENT( playerNum ), va( "qm %s", str->buffer ) );
}

static const gs_asMethod_t gameclient_Methods[] =
{
	{ ASLIB_FUNCTION_DECL( int, get_playerNum, ( ) const ), asFUNCTION( objectGameClient_PlayerNum ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, isReady, ( ) const ), asFUNCTION( objectGameClient_isReady ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, isBot, () const ), asFUNCTION( objectGameClient_isBot ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( Bot @, getBot, () const ), asFUNCTION( objectGameClient_getBot ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, state, ( ) const ), asFUNCTION( objectGameClient_ClientState ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, respawn, ( bool ghost ) ), asFUNCTION( objectGameClient_Respawn ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, clearPlayerStateEvents, ( ) ), asFUNCTION( objectGameClient_ClearPlayerStateEvents ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_name, ( ) const ), asFUNCTION( objectGameClient_getName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_clanName, ( ) const ), asFUNCTION( objectGameClient_getClanName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, getMMLogin, ( ) const ), asFUNCTION( objectGameClient_getMMLogin ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( Entity @, getEnt, ( ) const ), asFUNCTION( objectGameClient_GetEntity ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, inventoryCount, ( int tag ) const ), asFUNCTION( objectGameClient_InventoryCount ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, inventorySetCount, ( int tag, int count ) ), asFUNCTION( objectGameClient_InventorySetCount ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, inventoryGiveItem, ( int tag, int count ) ), asFUNCTION( objectGameClient_InventoryGiveItemExt ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, inventoryGiveItem, ( int tag ) ), asFUNCTION( objectGameClient_InventoryGiveItem ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, inventoryClear, ( ) ), asFUNCTION( objectGameClient_InventoryClear ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, canSelectWeapon, ( int tag ) const ), asFUNCTION( objectGameClient_CanSelectWeapon ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, selectWeapon, ( int tag ) ), asFUNCTION( objectGameClient_SelectWeapon ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, addAward, ( const String &in ) ), asFUNCTION( objectGameClient_addAward ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, addMetaAward, ( const String &in ) ), asFUNCTION( objectGameClient_addMetaAward ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, execGameCommand, ( const String &in ) ), asFUNCTION( objectGameClient_execGameCommand ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setHUDStat, ( int stat, int value ) ), asFUNCTION( objectGameClient_setHUDStat ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, getHUDStat, ( int stat ) const ), asFUNCTION( objectGameClient_getHUDStat ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_pmoveFeatures, ( uint bitmask ) ), asFUNCTION( objectGameClient_setPMoveFeatures ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_pmoveMaxSpeed, ( float speed ) ), asFUNCTION( objectGameClient_setPMoveMaxSpeed ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_pmoveJumpSpeed, ( float speed ) ), asFUNCTION( objectGameClient_setPMoveJumpSpeed ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_pmoveDashSpeed, ( float speed ) ), asFUNCTION( objectGameClient_setPMoveDashSpeed ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( uint, get_pmoveFeatures, ( ) const ), asFUNCTION( objectGameClient_getPMoveFeatures ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( uint, get_pressedKeys, ( ) const ), asFUNCTION( objectGameClient_getPressedKeys ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( float, get_pmoveMaxSpeed, ( ) const ), asFUNCTION( objectGameClient_getPMoveMaxSpeed ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( float, get_pmoveJumpSpeed, ( ) const ), asFUNCTION( objectGameClient_getPMoveJumpSpeed ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( float, get_pmoveDashSpeed, ( ) const ), asFUNCTION( objectGameClient_getPMoveDashSpeed ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, getUserInfoKey, ( const String &in ) const ), asFUNCTION( objectGameClient_getUserInfoKey ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, printMessage, ( const String &in ) ), asFUNCTION( objectGameClient_printMessage ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, chaseCam, ( const String @+, bool teamOnly ) ), asFUNCTION( objectGameClient_ChaseCam ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_chaseActive, ( const bool active ) ), asFUNCTION( objectGameClient_SetChaseActive ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, get_chaseActive, ( ) const ), asFUNCTION( objectGameClient_GetChaseActive ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, newRaceRun, ( int numSectors ) ), asFUNCTION( objectGameClient_NewRaceRun ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setRaceTime, ( int sector, int64 time ) ), asFUNCTION( objectGameClient_SetRaceTime ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setHelpMessage, ( uint msg ) ), asFUNCTION( objectGameClient_SetHelpMessage ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setOverlayMenuItems, ( const String &in ) ), asFUNCTION( objectGameClient_SetOverlayMenuItems ), asCALL_CDECL_OBJLAST },

	ASLIB_METHOD_NULL
};

static const gs_asProperty_t gameclient_Properties[] =
{
	{ ASLIB_PROPERTY_DECL( Stats, stats ), ASLIB_FOFFSET( gclient_t, level.stats ) },
	{ ASLIB_PROPERTY_DECL( const bool, connecting ), ASLIB_FOFFSET( gclient_t, connecting ) },
	{ ASLIB_PROPERTY_DECL( int, team ), ASLIB_FOFFSET( gclient_t, team ) },
	{ ASLIB_PROPERTY_DECL( const int, hand ), ASLIB_FOFFSET( gclient_t, hand ) },
	{ ASLIB_PROPERTY_DECL( const bool, isOperator ), ASLIB_FOFFSET( gclient_t, isoperator ) },
	{ ASLIB_PROPERTY_DECL( const int64, queueTimeStamp ), ASLIB_FOFFSET( gclient_t, queueTimeStamp ) },
	{ ASLIB_PROPERTY_DECL( const int, muted ), ASLIB_FOFFSET( gclient_t, muted ) },
	{ ASLIB_PROPERTY_DECL( float, armor ), ASLIB_FOFFSET( gclient_t, resp.armor ) },
	{ ASLIB_PROPERTY_DECL( const bool, chaseActive ), ASLIB_FOFFSET( gclient_t, resp.chase.active ) },
	{ ASLIB_PROPERTY_DECL( int, chaseTarget ), ASLIB_FOFFSET( gclient_t, resp.chase.target ) },
	{ ASLIB_PROPERTY_DECL( bool, chaseTeamonly ), ASLIB_FOFFSET( gclient_t, resp.chase.teamonly ) },
	{ ASLIB_PROPERTY_DECL( int, chaseFollowMode ), ASLIB_FOFFSET( gclient_t, resp.chase.followmode ) },
	{ ASLIB_PROPERTY_DECL( const bool, coach ), ASLIB_FOFFSET( gclient_t, teamstate.is_coach ) },
	{ ASLIB_PROPERTY_DECL( const int, ping ), ASLIB_FOFFSET( gclient_t, r.ping ) },
	{ ASLIB_PROPERTY_DECL( const int16, weapon ), ASLIB_FOFFSET( gclient_t, ps.stats[STAT_WEAPON] ) },
	{ ASLIB_PROPERTY_DECL( const int16, pendingWeapon ), ASLIB_FOFFSET( gclient_t, ps.stats[STAT_PENDING_WEAPON] ) },
	{ ASLIB_PROPERTY_DECL( bool, takeStun ), ASLIB_FOFFSET( gclient_t, resp.takeStun ) },
	{ ASLIB_PROPERTY_DECL( int64, lastActivity ), ASLIB_FOFFSET( gclient_t, level.last_activity ) },
	{ ASLIB_PROPERTY_DECL( const int64, uCmdTimeStamp ), ASLIB_FOFFSET( gclient_t, ucmd.serverTimeStamp ) },

	ASLIB_PROPERTY_NULL
};

static const gs_asClassDescriptor_t asGameClientDescriptor =
{
	"Client",				   /* name */
	asOBJ_REF | asOBJ_NOCOUNT, /* object type flags */
	sizeof( gclient_t ),	   /* size */
	NULL,					   /* funcdefs */
	NULL,					   /* object behaviors */
	gameclient_Methods,		   /* methods */
	gameclient_Properties,	   /* properties */
	NULL, NULL,				   /* string factory hack */
};

//=======================================================================

// CLASS: Entity
static void *asEntityCallThinkFuncPtr = NULL;
static void *asEntityCallTouchFuncPtr = NULL;
static void *asEntityCallUseFuncPtr = NULL;
static void *asEntityCallStopFuncPtr = NULL;
static void *asEntityCallPainFuncPtr = NULL;
static void *asEntityCallDieFuncPtr = NULL;

static asvec3_t objectGameEntity_GetVelocity( edict_t *obj ) {
	asvec3_t velocity;

	VectorCopy( obj->velocity, velocity.v );

	return velocity;
}

static void objectGameEntity_SetVelocity( asvec3_t *vel, edict_t *self ) {
	VectorCopy( vel->v, self->velocity );

	if( self->r.client && trap_GetClientState( PLAYERNUM( self ) ) >= CS_SPAWNED ) {
		VectorCopy( vel->v, self->r.client->ps.pmove.velocity );
	}
}

static asvec3_t objectGameEntity_GetAVelocity( edict_t *obj ) {
	asvec3_t avelocity;

	VectorCopy( obj->avelocity, avelocity.v );

	return avelocity;
}

static void objectGameEntity_SetAVelocity( asvec3_t *vel, edict_t *self ) {
	VectorCopy( vel->v, self->avelocity );
}

static asvec3_t objectGameEntity_GetOrigin( edict_t *obj ) {
	asvec3_t origin;

	VectorCopy( obj->s.origin, origin.v );
	return origin;
}

static void objectGameEntity_SetOrigin( asvec3_t *vec, edict_t *self ) {
	if( self->r.client && trap_GetClientState( PLAYERNUM( self ) ) >= CS_SPAWNED ) {
		VectorCopy( vec->v, self->r.client->ps.pmove.origin );
	}
	VectorCopy( vec->v, self->s.origin );
}

static asvec3_t objectGameEntity_GetOrigin2( edict_t *obj ) {
	asvec3_t origin;

	VectorCopy( obj->s.origin2, origin.v );
	return origin;
}

static void objectGameEntity_SetOrigin2( asvec3_t *vec, edict_t *self ) {
	VectorCopy( vec->v, self->s.origin2 );
}

static asvec3_t objectGameEntity_GetAngles( edict_t *obj ) {
	asvec3_t angles;

	VectorCopy( obj->s.angles, angles.v );
	return angles;
}

static void objectGameEntity_SetAngles( asvec3_t *vec, edict_t *self ) {
	VectorCopy( vec->v, self->s.angles );

	if( self->r.client && trap_GetClientState( PLAYERNUM( self ) ) >= CS_SPAWNED ) {
		int i;

		VectorCopy( vec->v, self->r.client->ps.viewangles );

		// update the delta angle
		for( i = 0; i < 3; i++ )
			self->r.client->ps.pmove.delta_angles[i] = ANGLE2SHORT( self->r.client->ps.viewangles[i] ) - self->r.client->ucmd.angles[i];
	}
}

static void objectGameEntity_GetSize( asvec3_t *mins, asvec3_t *maxs, edict_t *self ) {
	VectorCopy( self->r.maxs, maxs->v );
	VectorCopy( self->r.mins, mins->v );
}

static void objectGameEntity_SetSize( asvec3_t *mins, asvec3_t *maxs, edict_t *self ) {
	VectorCopy( mins->v, self->r.mins );
	VectorCopy( maxs->v, self->r.maxs );
}

static asvec3_t objectGameEntity_GetMovedir( edict_t *self ) {
	asvec3_t movedir;

	VectorCopy( self->moveinfo.movedir, movedir.v );
	return movedir;
}

static void objectGameEntity_SetMovedir( edict_t *self ) {
	G_SetMovedir( self->s.angles, self->moveinfo.movedir );
}

static bool objectGameEntity_isBrushModel( edict_t *self ) {
	return ISBRUSHMODEL( self->s.modelindex );
}

static bool objectGameEntity_IsGhosting( edict_t *self ) {
	if( self->r.client && trap_GetClientState( PLAYERNUM( self ) ) < CS_SPAWNED ) {
		return true;
	}

	return G_ISGHOSTING( self ) ? true : false;
}

static int objectGameEntity_EntNum( edict_t *self ) {
	return ( ENTNUM( self ) );
}

static int objectGameEntity_PlayerNum( edict_t *self ) {
	return ( PLAYERNUM( self ) );
}

static asstring_t *objectGameEntity_getModelName( edict_t *self ) {
	return game.asExport->asStringFactoryBuffer( self->model, self->model ? strlen( self->model ) : 0 );
}

static asstring_t *objectGameEntity_getModel2Name( edict_t *self ) {
	return game.asExport->asStringFactoryBuffer( self->model2, self->model2 ? strlen( self->model2 ) : 0 );
}

static asstring_t *objectGameEntity_getClassname( edict_t *self ) {
	return game.asExport->asStringFactoryBuffer( self->classname, self->classname ? strlen( self->classname ) : 0 );
}

/*
static asstring_t *objectGameEntity_getSpawnKey( asstring_t *key, edict_t *self )
{
const char *val;

if( !key )
return game.asExport->asStringFactoryBuffer( NULL, 0 );

val = G_GetEntitySpawnKey( key->buffer, self );

return game.asExport->asStringFactoryBuffer( val, strlen( val ) );
}
*/

static asstring_t *objectGameEntity_getTargetname( edict_t *self ) {
	return game.asExport->asStringFactoryBuffer( self->targetname, self->targetname ? strlen( self->targetname ) : 0 );
}

static void objectGameEntity_setTargetname( asstring_t *targetname, edict_t *self ) {
	self->targetname = G_RegisterLevelString( targetname->buffer );
}

static asstring_t *objectGameEntity_getTarget( edict_t *self ) {
	return game.asExport->asStringFactoryBuffer( self->target, self->target ? strlen( self->target ) : 0 );
}

static void objectGameEntity_setTarget( asstring_t *target, edict_t *self ) {
	self->target = G_RegisterLevelString( target->buffer );
}

static asstring_t *objectGameEntity_getMap( edict_t *self ) {
	return game.asExport->asStringFactoryBuffer( self->map, self->map ? strlen( self->map ) : 0 );
}

static asstring_t *objectGameEntity_getSoundName( edict_t *self ) {
	return game.asExport->asStringFactoryBuffer( self->sounds, self->sounds ? strlen( self->sounds ) : 0 );
}

static void objectGameEntity_setClassname( asstring_t *classname, edict_t *self ) {
	self->classname = G_RegisterLevelString( classname->buffer );
}

static void objectGameEntity_setMap( asstring_t *map, edict_t *self ) {
	self->map = G_RegisterLevelString( map->buffer );
}

static void objectGameEntity_GhostClient( edict_t *self ) {
	if( self->r.client ) {
		G_GhostClient( self );
	}
}

static void objectGameEntity_SetupModelExt( asstring_t *modelstr, asstring_t *skinstr, edict_t *self ) {
	char *path;
	const char *s;

	if( !modelstr ) {
		self->s.modelindex = 0;
		return;
	}

	path = modelstr->buffer;
	while( path[0] == '$' )
		path++;
	s = strstr( path, "models/players/" );

	// if it's a player model
	if( s == path ) {
		char skin[MAX_QPATH], model[MAX_QPATH];

		s += strlen( "models/players/" );

		Q_snprintfz( model, sizeof( model ), "$%s", path );
		Q_snprintfz( skin, sizeof( skin ), "models/players/%s/%s", s, skinstr && skinstr->buffer[0] ? skinstr->buffer : DEFAULT_PLAYERSKIN );

		self->s.modelindex = trap_ModelIndex( model );
		self->s.skinnum = trap_SkinIndex( skin );
		return;
	}

	GClip_SetBrushModel( self, path );
}

static void objectGameEntity_SetupModel( asstring_t *modelstr, edict_t *self ) {
	objectGameEntity_SetupModelExt( modelstr, NULL, self );
}

static void objectGameEntity_UseTargets( edict_t *activator, edict_t *self ) {
	G_UseTargets( self, activator );
}

static edict_t *objectGameEntity_DropItemByTag( int tag, edict_t *self ) {
	gsitem_t *item = GS_FindItemByTag( tag );

	if( !item ) {
		return NULL;
	}

	return Drop_Item( self, item );
}

static edict_t *objectGameEntity_DropItem( gsitem_t *item, edict_t *self ) {
	if( !item ) {
		return NULL;
	}
	return Drop_Item( self, item );
}

static CScriptArrayInterface *objectGameEntity_findTargets( edict_t *self ) {
	asITypeInfo *ot = asEntityArrayType();
	CScriptArrayInterface *arr = game.asExport->asCreateArrayCpp( 0, ot );

	if( self->target && self->target[0] != '\0' ) {
		int count = 0;
		edict_t *ent = NULL;
		while( ( ent = G_Find( ent, FOFS( targetname ), self->target ) ) != NULL ) {
			arr->Resize( count + 1 );
			*( (edict_t **)arr->At( count ) ) = ent;
			count++;
		}
	}

	return arr;
}

static CScriptArrayInterface *objectGameEntity_findTargeting( edict_t *self ) {
	asITypeInfo *ot = asEntityArrayType();
	CScriptArrayInterface *arr = game.asExport->asCreateArrayCpp( 0, ot );

	if( self->targetname && self->targetname[0] != '\0' ) {
		int count = 0;
		edict_t *ent = NULL;
		while( ( ent = G_Find( ent, FOFS( target ), self->targetname ) ) != NULL ) {
			arr->Resize( count + 1 );
			*( (edict_t **)arr->At( count ) ) = ent;
			count++;
		}
	}

	return arr;
}

static void objectGameEntity_TeleportEffect( bool in, edict_t *self ) {
	G_TeleportEffect( self, in );
}

static void objectGameEntity_sustainDamage( edict_t *inflictor, edict_t *attacker, asvec3_t *dir, float damage, float knockback, float stun, int mod, edict_t *self ) {
	G_Damage( self, inflictor, attacker,
			  dir ? dir->v : NULL, dir ? dir->v : NULL,
			  inflictor ? inflictor->s.origin : self->s.origin,
			  damage, knockback, stun, 0, mod >= 0 ? mod : 0 );
}

static void objectGameEntity_splashDamage( edict_t *attacker, int radius, float damage, float knockback, float stun, int mod, edict_t *self ) {
	if( radius < 1 ) {
		return;
	}

	self->projectileInfo.maxDamage = damage;
	self->projectileInfo.minDamage = 1;
	self->projectileInfo.maxKnockback = knockback;
	self->projectileInfo.minKnockback = 1;
	self->projectileInfo.stun = stun;
	self->projectileInfo.radius = radius;

	G_RadiusDamage( self, attacker, NULL, self, mod >= 0 ? mod : 0 );
}

static void objectGameEntity_explosionEffect( int radius, edict_t *self ) {
	int i, eventType, eventRadius;
	vec3_t center;

	if( radius < 8 ) {
		return;
	}

	// turn entity into event
	if( radius > 255 * 8 ) {
		eventType = EV_EXPLOSION2;
		eventRadius = (int)( radius * 1 / 16 ) & 0xFF;
	} else {
		eventType = EV_EXPLOSION1;
		eventRadius = (int)( radius * 1 / 8 ) & 0xFF;
	}

	if( eventRadius < 1 ) {
		eventRadius = 1;
	}

	for( i = 0; i < 3; i++ )
		center[i] = self->s.origin[i] + ( 0.5f * ( self->r.maxs[i] + self->r.mins[i] ) );

	G_SpawnEvent( eventType, eventRadius, center );
}

static entity_state_t *objectGameEntity_getEntityState( edict_t *self ) {
	return &self->s;
}

static const gs_asFuncdef_t gedict_Funcdefs[] =
{
	{ ASLIB_FUNCTION_DECL( void, entThink, ( Entity @ent ) ) },
	{ ASLIB_FUNCTION_DECL( void, entTouch, ( Entity @ent, Entity @other, const Vec3 planeNormal, int surfFlags ) ) },
	{ ASLIB_FUNCTION_DECL( void, entUse, ( Entity @ent, Entity @other, Entity @activator ) ) },
	{ ASLIB_FUNCTION_DECL( void, entPain, ( Entity @ent, Entity @other, float kick, float damage ) ) },
	{ ASLIB_FUNCTION_DECL( void, entDie, ( Entity @ent, Entity @inflicter, Entity @attacker ) ) },
	{ ASLIB_FUNCTION_DECL( void, entStop, ( Entity @ent ) ) },

	ASLIB_FUNCDEF_NULL
};

static const gs_asMethod_t gedict_Methods[] =
{
	{ ASLIB_FUNCTION_DECL( Vec3, get_velocity, ( ) const ), asFUNCTION( objectGameEntity_GetVelocity ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_velocity, ( const Vec3 &in ) ), asFUNCTION( objectGameEntity_SetVelocity ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( Vec3, get_avelocity, ( ) const ), asFUNCTION( objectGameEntity_GetAVelocity ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_avelocity, ( const Vec3 &in ) ), asFUNCTION( objectGameEntity_SetAVelocity ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( Vec3, get_origin, ( ) const ), asFUNCTION( objectGameEntity_GetOrigin ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_origin, ( const Vec3 &in ) ), asFUNCTION( objectGameEntity_SetOrigin ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( Vec3, get_origin2, ( ) const ), asFUNCTION( objectGameEntity_GetOrigin2 ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_origin2, ( const Vec3 &in ) ), asFUNCTION( objectGameEntity_SetOrigin2 ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( Vec3, get_angles, ( ) const ), asFUNCTION( objectGameEntity_GetAngles ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_angles, ( const Vec3 &in ) ), asFUNCTION( objectGameEntity_SetAngles ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, getSize, ( Vec3 & out, Vec3 & out ) ), asFUNCTION( objectGameEntity_GetSize ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setSize, ( const Vec3 &in, const Vec3 &in ) ), asFUNCTION( objectGameEntity_SetSize ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( Vec3, get_movedir, ( ) const ), asFUNCTION( objectGameEntity_GetMovedir ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_movedir, ( ) ), asFUNCTION( objectGameEntity_SetMovedir ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, isBrushModel, ( ) const ), asFUNCTION( objectGameEntity_isBrushModel ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, freeEntity, ( ) ), asFUNCTION( G_FreeEdict ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, linkEntity, ( ) ), asFUNCTION( GClip_LinkEntity ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, unlinkEntity, ( ) ), asFUNCTION( GClip_UnlinkEntity ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( bool, isGhosting, ( ) const ), asFUNCTION( objectGameEntity_IsGhosting ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, get_entNum, ( ) const ), asFUNCTION( objectGameEntity_EntNum ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( int, get_playerNum, ( ) const ), asFUNCTION( objectGameEntity_PlayerNum ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_model, ( ) const ), asFUNCTION( objectGameEntity_getModelName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_model2, ( ) const ), asFUNCTION( objectGameEntity_getModel2Name ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_sounds, ( ) const ), asFUNCTION( objectGameEntity_getSoundName ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_classname, ( ) const ), asFUNCTION( objectGameEntity_getClassname ), asCALL_CDECL_OBJLAST },
	//{ ASLIB_FUNCTION_DECL(const String @, getSpawnKey, ( String &in )), asFUNCTION(objectGameEntity_getSpawnKey), NULL, asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_targetname, ( ) const ), asFUNCTION( objectGameEntity_getTargetname ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_target, ( ) const ), asFUNCTION( objectGameEntity_getTarget ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( const String @, get_map, ( ) const ), asFUNCTION( objectGameEntity_getMap ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_target, ( const String &in ) ), asFUNCTION( objectGameEntity_setTarget ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_targetname, ( const String &in ) ), asFUNCTION( objectGameEntity_setTargetname ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_classname, ( const String &in ) ), asFUNCTION( objectGameEntity_setClassname ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, set_map, ( const String &in ) ), asFUNCTION( objectGameEntity_setMap ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, ghost, ( ) ), asFUNCTION( objectGameEntity_GhostClient ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, spawnqueueAdd, ( ) ), asFUNCTION( G_SpawnQueue_AddClient ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, teleportEffect, ( bool ) ), asFUNCTION( objectGameEntity_TeleportEffect ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, respawnEffect, ( ) ), asFUNCTION( G_RespawnEffect ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setupModel, ( const String &in ) ), asFUNCTION( objectGameEntity_SetupModel ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, setupModel, ( const String &in, const String &in ) ), asFUNCTION( objectGameEntity_SetupModelExt ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( array<Entity @> @, findTargets, ( ) const ), asFUNCTION( objectGameEntity_findTargets ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( array<Entity @> @, findTargeting, ( ) const ), asFUNCTION( objectGameEntity_findTargeting ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, useTargets, ( const Entity @activator ) ), asFUNCTION( objectGameEntity_UseTargets ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( Entity @, dropItem, ( int tag ) const ), asFUNCTION( objectGameEntity_DropItemByTag ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( Entity @, dropItem, ( Item @ ) const ), asFUNCTION( objectGameEntity_DropItem ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, sustainDamage, ( Entity @inflicter, Entity @attacker, const Vec3 &in dir, float damage, float knockback, float stun, int mod ) ), asFUNCTION( objectGameEntity_sustainDamage ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, splashDamage, ( Entity @attacker, int radius, float damage, float knockback, float stun, int mod ) ), asFUNCTION( objectGameEntity_splashDamage ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( void, explosionEffect, ( int radius ) ), asFUNCTION( objectGameEntity_explosionEffect ), asCALL_CDECL_OBJLAST },
	{ ASLIB_FUNCTION_DECL( EntityState @, getState, () const ), asFUNCTION( objectGameEntity_getEntityState ), asCALL_CDECL_OBJLAST },

	ASLIB_METHOD_NULL
};

static const gs_asProperty_t gedict_Properties[] =
{
	{ ASLIB_PROPERTY_DECL( Client @, client ), ASLIB_FOFFSET( edict_t, r.client ) },
	{ ASLIB_PROPERTY_DECL( Item @, item ), ASLIB_FOFFSET( edict_t, item ) },
	{ ASLIB_PROPERTY_DECL( Entity @, groundEntity ), ASLIB_FOFFSET( edict_t, groundentity ) },
	{ ASLIB_PROPERTY_DECL( Entity @, owner ), ASLIB_FOFFSET( edict_t, r.owner ) },
	{ ASLIB_PROPERTY_DECL( Entity @, enemy ), ASLIB_FOFFSET( edict_t, enemy ) },
	{ ASLIB_PROPERTY_DECL( Entity @, activator ), ASLIB_FOFFSET( edict_t, activator ) },
	{ ASLIB_PROPERTY_DECL( int, type ), ASLIB_FOFFSET( edict_t, s.type ) },
	{ ASLIB_PROPERTY_DECL( int, modelindex ), ASLIB_FOFFSET( edict_t, s.modelindex ) },
	{ ASLIB_PROPERTY_DECL( int, modelindex2 ), ASLIB_FOFFSET( edict_t, s.modelindex2 ) },
	{ ASLIB_PROPERTY_DECL( int, frame ), ASLIB_FOFFSET( edict_t, s.frame ) },
	{ ASLIB_PROPERTY_DECL( int, ownerNum ), ASLIB_FOFFSET( edict_t, s.ownerNum ) },
	{ ASLIB_PROPERTY_DECL( int, counterNum ), ASLIB_FOFFSET( edict_t, s.counterNum ) },
	{ ASLIB_PROPERTY_DECL( int, skinNum ), ASLIB_FOFFSET( edict_t, s.skinnum ) },
	{ ASLIB_PROPERTY_DECL( int, colorRGBA ), ASLIB_FOFFSET( edict_t, s.colorRGBA ) },
	{ ASLIB_PROPERTY_DECL( int, weapon ), ASLIB_FOFFSET( edict_t, s.weapon ) },
	{ ASLIB_PROPERTY_DECL( bool, teleported ), ASLIB_FOFFSET( edict_t, s.teleported ) },
	{ ASLIB_PROPERTY_DECL( uint, effects ), ASLIB_FOFFSET( edict_t, s.effects ) },
	{ ASLIB_PROPERTY_DECL( int, sound ), ASLIB_FOFFSET( edict_t, s.sound ) },
	{ ASLIB_PROPERTY_DECL( int, team ), ASLIB_FOFFSET( edict_t, s.team ) },
	{ ASLIB_PROPERTY_DECL( int, light ), ASLIB_FOFFSET( edict_t, s.light ) },
	{ ASLIB_PROPERTY_DECL( const bool, inuse ), ASLIB_FOFFSET( edict_t, r.inuse ) },
	{ ASLIB_PROPERTY_DECL( uint, svflags ), ASLIB_FOFFSET( edict_t, r.svflags ) },
	{ ASLIB_PROPERTY_DECL( int, solid ), ASLIB_FOFFSET( edict_t, r.solid ) },
	{ ASLIB_PROPERTY_DECL( int, clipMask ), ASLIB_FOFFSET( edict_t, r.clipmask ) },
	{ ASLIB_PROPERTY_DECL( int, spawnFlags ), ASLIB_FOFFSET( edict_t, spawnflags ) },
	{ ASLIB_PROPERTY_DECL( int, style ), ASLIB_FOFFSET( edict_t, style ) },
	{ ASLIB_PROPERTY_DECL( int, moveType ), ASLIB_FOFFSET( edict_t, movetype ) },
	{ ASLIB_PROPERTY_DECL( int64, nextThink ), ASLIB_FOFFSET( edict_t, nextThink ) },
	{ ASLIB_PROPERTY_DECL( float, health ), ASLIB_FOFFSET( edict_t, health ) },
	{ ASLIB_PROPERTY_DECL( int, maxHealth ), ASLIB_FOFFSET( edict_t, max_health ) },
	{ ASLIB_PROPERTY_DECL( int, viewHeight ), ASLIB_FOFFSET( edict_t, viewheight ) },
	{ ASLIB_PROPERTY_DECL( int, takeDamage ), ASLIB_FOFFSET( edict_t, takedamage ) },
	{ ASLIB_PROPERTY_DECL( int, damage ), ASLIB_FOFFSET( edict_t, dmg ) },
	{ ASLIB_PROPERTY_DECL( int, projectileMaxDamage ), ASLIB_FOFFSET( edict_t, projectileInfo.maxDamage ) },
	{ ASLIB_PROPERTY_DECL( int, projectileMinDamage ), ASLIB_FOFFSET( edict_t, projectileInfo.minDamage ) },
	{ ASLIB_PROPERTY_DECL( int, projectileMaxKnockback ), ASLIB_FOFFSET( edict_t, projectileInfo.maxKnockback ) },
	{ ASLIB_PROPERTY_DECL( int, projectileMinKnockback ), ASLIB_FOFFSET( edict_t, projectileInfo.minKnockback ) },
	{ ASLIB_PROPERTY_DECL( float, projectileSplashRadius ), ASLIB_FOFFSET( edict_t, projectileInfo.radius ) },
	{ ASLIB_PROPERTY_DECL( int, count ), ASLIB_FOFFSET( edict_t, count ) },
	{ ASLIB_PROPERTY_DECL( float, wait ), ASLIB_FOFFSET( edict_t, wait ) },
	{ ASLIB_PROPERTY_DECL( float, delay ), ASLIB_FOFFSET( edict_t, delay ) },
	{ ASLIB_PROPERTY_DECL( float, random ), ASLIB_FOFFSET( edict_t, random ) },
	{ ASLIB_PROPERTY_DECL( int, waterLevel ), ASLIB_FOFFSET( edict_t, waterlevel ) },
	{ ASLIB_PROPERTY_DECL( float, attenuation ), ASLIB_FOFFSET( edict_t, attenuation ) },
	{ ASLIB_PROPERTY_DECL( int, mass ), ASLIB_FOFFSET( edict_t, mass ) },
	{ ASLIB_PROPERTY_DECL( int64, timeStamp ), ASLIB_FOFFSET( edict_t, timeStamp ) },

	{ ASLIB_PROPERTY_DECL( entThink @, think ), ASLIB_FOFFSET( edict_t, asThinkFunc ) },
	{ ASLIB_PROPERTY_DECL( entTouch @, touch ), ASLIB_FOFFSET( edict_t, asTouchFunc ) },
	{ ASLIB_PROPERTY_DECL( entUse @, use ), ASLIB_FOFFSET( edict_t, asUseFunc ) },
	{ ASLIB_PROPERTY_DECL( entPain @, pain ), ASLIB_FOFFSET( edict_t, asPainFunc ) },
	{ ASLIB_PROPERTY_DECL( entDie @, die ), ASLIB_FOFFSET( edict_t, asDieFunc ) },
	{ ASLIB_PROPERTY_DECL( entStop @, stop ), ASLIB_FOFFSET( edict_t, asStopFunc ) },

	// specific for ET_PARTICLES
	{ ASLIB_PROPERTY_DECL( int, particlesSpeed ), ASLIB_FOFFSET( edict_t, particlesInfo.speed ) },
	{ ASLIB_PROPERTY_DECL( int, particlesShaderIndex ), ASLIB_FOFFSET( edict_t, particlesInfo.shaderIndex ) },
	{ ASLIB_PROPERTY_DECL( int, particlesSpread ), ASLIB_FOFFSET( edict_t, particlesInfo.spread ) },
	{ ASLIB_PROPERTY_DECL( int, particlesSize ), ASLIB_FOFFSET( edict_t, particlesInfo.size ) },
	{ ASLIB_PROPERTY_DECL( int, particlesTime ), ASLIB_FOFFSET( edict_t, particlesInfo.time ) },
	{ ASLIB_PROPERTY_DECL( int, particlesFrequency ), ASLIB_FOFFSET( edict_t, particlesInfo.frequency ) },
	{ ASLIB_PROPERTY_DECL( bool, particlesSpherical ), ASLIB_FOFFSET( edict_t, particlesInfo.spherical ) },
	{ ASLIB_PROPERTY_DECL( bool, particlesBounce ), ASLIB_FOFFSET( edict_t, particlesInfo.bounce ) },
	{ ASLIB_PROPERTY_DECL( bool, particlesGravity ), ASLIB_FOFFSET( edict_t, particlesInfo.gravity ) },
	{ ASLIB_PROPERTY_DECL( bool, particlesExpandEffect ), ASLIB_FOFFSET( edict_t, particlesInfo.expandEffect ) },
	{ ASLIB_PROPERTY_DECL( bool, particlesShrinkEffect ), ASLIB_FOFFSET( edict_t, particlesInfo.shrinkEffect ) },

	ASLIB_PROPERTY_NULL
};

static const gs_asClassDescriptor_t asGameEntityClassDescriptor =
{
	"Entity",				   /* name */
	asOBJ_REF | asOBJ_NOCOUNT, /* object type flags */
	sizeof( edict_t ),		   /* size */
	gedict_Funcdefs,		   /* funcdefs */
	NULL,					   /* object behaviors */
	gedict_Methods,			   /* methods */
	gedict_Properties,		   /* properties */
	NULL, NULL,				   /* string factory hack */
};

//=======================================================================

static const gs_asClassDescriptor_t * const asGameClassesDescriptors[] =
{
	&asMatchClassDescriptor,
	&asGametypeClassDescriptor,
	&asTeamListClassDescriptor,
	&asScoreStatsClassDescriptor,
	&asGameClientDescriptor,
	&asGameEntityClassDescriptor,
	&asBotClassDescriptor,

	NULL
};

//=======================================================================

static edict_t *asFunc_G_Spawn( asstring_t *classname ) {
	edict_t *ent;

	if( !level.canSpawnEntities ) {
		G_Printf( "* WARNING: Spawning entities is disallowed during initialization. Returning null object\n" );
		return NULL;
	}

	ent = G_Spawn();

	if( classname && classname->len ) {
		ent->classname = G_RegisterLevelString( classname->buffer );
	}

	ent->scriptSpawned = true;
	ent->asScriptModule = static_cast<void *>(
		GAME_AS_ENGINE()->GetModule( game.asExport->asGetActiveContext()->GetFunction( 0 )->GetModuleName() )
		);

	G_asClearEntityBehaviors( ent );

	return ent;
}

static edict_t *asFunc_GetEntity( int entNum ) {
	if( entNum < 0 || entNum >= game.numentities ) {
		return NULL;
	}

	return &game.edicts[ entNum ];
}

static gclient_t *asFunc_GetClient( int clientNum ) {
	if( clientNum < 0 || clientNum >= gs.maxclients ) {
		return NULL;
	}

	return &game.clients[ clientNum ];
}

static g_teamlist_t *asFunc_GetTeamlist( int teamNum ) {
	if( teamNum < TEAM_SPECTATOR || teamNum >= GS_MAX_TEAMS ) {
		return NULL;
	}

	return &teamlist[teamNum];
}

static gsitem_t *asFunc_GS_FindItemByTag( int tag ) {
	return GS_FindItemByTag( tag );
}

static gsitem_t *asFunc_GS_FindItemByName( asstring_t *name ) {
	return ( !name || !name->len ) ? NULL : GS_FindItemByName( name->buffer );
}

static gsitem_t *asFunc_GS_FindItemByClassname( asstring_t *name ) {
	return ( !name || !name->len ) ? NULL : GS_FindItemByClassname( name->buffer );
}

static void asFunc_G_Match_RemoveProjectiles( edict_t *owner ) {
	G_Match_RemoveProjectiles( owner );
}

static void asFunc_G_Match_RemoveAllProjectiles( void ) {
	G_Match_RemoveProjectiles( NULL );
}

static void asFunc_G_ResetLevel( void ) {
	G_ResetLevel();
}

static void asFunc_G_Match_FreeBodyQueue( void ) {
	G_Match_FreeBodyQueue();
}

static void asFunc_G_Items_RespawnByType( unsigned int typeMask, int item_tag, float delay ) {
	G_Items_RespawnByType( typeMask, item_tag, delay );
}

static void asFunc_Print( const asstring_t *str ) {
	if( !str || !str->buffer ) {
		return;
	}

	G_Printf( "%s", str->buffer );
}

static void asFunc_PrintMsg( edict_t *ent, asstring_t *str ) {
	if( !str || !str->buffer ) {
		return;
	}

	G_PrintMsg( ent, "%s", str->buffer );
}

static void asFunc_CenterPrintMsg( edict_t *ent, asstring_t *str ) {
	if( !str || !str->buffer ) {
		return;
	}

	G_CenterPrintMsg( ent, "%s", str->buffer );
}

static void asFunc_CenterPrintFormatMsg1( edict_t *ent, asstring_t *format, asstring_t *arg1 ) {
	G_CenterPrintFormatMsg( ent, 1, format->buffer, arg1->buffer );
}

static void asFunc_CenterPrintFormatMsg2( edict_t *ent, asstring_t *format, asstring_t *arg1, asstring_t *arg2 ) {
	G_CenterPrintFormatMsg( ent, 2, format->buffer, arg1->buffer, arg2->buffer );
}

static void asFunc_CenterPrintFormatMsg3( edict_t *ent, asstring_t *format, asstring_t *arg1, asstring_t *arg2, asstring_t *arg3 ) {
	G_CenterPrintFormatMsg( ent, 3, format->buffer, arg1->buffer, arg2->buffer, arg3->buffer );
}

static void asFunc_CenterPrintFormatMsg4( edict_t *ent, asstring_t *format, asstring_t *arg1, asstring_t *arg2, asstring_t *arg3,
										  asstring_t *arg4 ) {
	G_CenterPrintFormatMsg( ent, 4, format->buffer, arg1->buffer, arg2->buffer, arg3->buffer, arg4->buffer );
}

static void asFunc_CenterPrintFormatMsg5( edict_t *ent, asstring_t *format, asstring_t *arg1, asstring_t *arg2, asstring_t *arg3,
										  asstring_t *arg4, asstring_t *arg5 ) {
	G_CenterPrintFormatMsg( ent, 5, format->buffer, arg1->buffer, arg2->buffer, arg3->buffer, arg4->buffer, arg5->buffer );
}

static void asFunc_CenterPrintFormatMsg6( edict_t *ent, asstring_t *format, asstring_t *arg1, asstring_t *arg2, asstring_t *arg3,
										  asstring_t *arg4, asstring_t *arg5, asstring_t *arg6 ) {
	G_CenterPrintFormatMsg( ent, 6, format->buffer, arg1->buffer, arg2->buffer, arg3->buffer, arg4->buffer, arg5->buffer, arg6->buffer );
}

static void asFunc_CenterPrintFormatMsg7( edict_t *ent, asstring_t *format, asstring_t *arg1, asstring_t *arg2, asstring_t *arg3,
										  asstring_t *arg4, asstring_t *arg5, asstring_t *arg6, asstring_t *arg7 ) {
	G_CenterPrintFormatMsg( ent, 7, format->buffer, arg1->buffer, arg2->buffer, arg3->buffer, arg4->buffer, arg5->buffer, arg6->buffer, arg7->buffer );
}

static void asFunc_Error( const asstring_t *str ) {
	G_Error( "%s", str && str->buffer ? str->buffer : "" );
}

static void asFunc_G_Sound( edict_t *owner, int channel, int soundindex, float attenuation ) {
	G_Sound( owner, channel, soundindex, attenuation );
}

static int asFunc_DirToByte( asvec3_t *vec ) {
	if( !vec ) {
		return 0;
	}

	return DirToByte( vec->v );
}

static int asFunc_PointContents( asvec3_t *vec ) {
	if( !vec ) {
		return 0;
	}

	return G_PointContents( vec->v );
}

static bool asFunc_InPVS( asvec3_t *origin1, asvec3_t *origin2 ) {
	return trap_inPVS( origin1->v, origin2->v );
}

static bool asFunc_WriteFile( asstring_t *path, asstring_t *data ) {
	int filehandle;

	if( !path || !path->len ) {
		return false;
	}
	if( !data || !data->buffer ) {
		return false;
	}

	if( trap_FS_FOpenFile( path->buffer, &filehandle, FS_WRITE ) == -1 ) {
		return false;
	}

	trap_FS_Write( data->buffer, data->len, filehandle );
	trap_FS_FCloseFile( filehandle );

	return true;
}

static bool asFunc_AppendToFile( asstring_t *path, asstring_t *data ) {
	int filehandle;

	if( !path || !path->len ) {
		return false;
	}
	if( !data || !data->buffer ) {
		return false;
	}

	if( trap_FS_FOpenFile( path->buffer, &filehandle, FS_APPEND ) == -1 ) {
		return false;
	}

	trap_FS_Write( data->buffer, data->len, filehandle );
	trap_FS_FCloseFile( filehandle );

	return true;
}

static asstring_t *asFunc_LoadFile( asstring_t *path ) {
	int filelen, filehandle;
	uint8_t *buf = NULL;
	asstring_t *data;

	if( !path || !path->len ) {
		return game.asExport->asStringFactoryBuffer( NULL, 0 );
	}

	filelen = trap_FS_FOpenFile( path->buffer, &filehandle, FS_READ );
	if( filehandle && filelen > 0 ) {
		buf = ( uint8_t * )G_Malloc( filelen + 1 );
		filelen = trap_FS_Read( buf, filelen, filehandle );
	}

	trap_FS_FCloseFile( filehandle );

	if( !buf ) {
		return game.asExport->asStringFactoryBuffer( NULL, 0 );
	}

	data = game.asExport->asStringFactoryBuffer( (char *)buf, filelen );
	G_Free( buf );

	return data;
}

static int asFunc_FileLength( asstring_t *path ) {
	if( !path || !path->len ) {
		return false;
	}

	return ( trap_FS_FOpenFile( path->buffer, NULL, FS_READ ) );
}

static void asFunc_Cmd_ExecuteText( asstring_t *str ) {
	if( !str || !str->buffer || !str->buffer[0] ) {
		return;
	}

	trap_Cmd_ExecuteText( EXEC_APPEND, str->buffer );
}

static bool asFunc_ML_FilenameExists( asstring_t *filename ) {
	return trap_ML_FilenameExists( filename->buffer );
}

static asstring_t *asFunc_ML_GetMapByNum( int num ) {
	char mapname[MAX_QPATH];
	asstring_t *data;

	if( !trap_ML_GetMapByNum( num, mapname, sizeof( mapname ) ) ) {
		return NULL;
	}

	data = game.asExport->asStringFactoryBuffer( (char *)mapname, strlen( mapname ) );
	return data;
}

static asstring_t *asFunc_LocationName( asvec3_t *origin ) {
	char buf[MAX_CONFIGSTRING_CHARS];
	asstring_t *data;

	G_MapLocationNameForTAG( G_MapLocationTAGForOrigin( origin->v ), buf, sizeof( buf ) );

	data = game.asExport->asStringFactoryBuffer( (char *)buf, strlen( buf ) );
	return data;
}

static int asFunc_LocationTag( asstring_t *str ) {
	return G_MapLocationTAGForName( str->buffer );
}

static asstring_t *asFunc_LocationForTag( int tag ) {
	char buf[MAX_CONFIGSTRING_CHARS];
	asstring_t *data;

	G_MapLocationNameForTAG( tag, buf, sizeof( buf ) );

	data = game.asExport->asStringFactoryBuffer( (char *)buf, strlen( buf ) );

	return data;
}

static int asFunc_ImageIndex( asstring_t *str ) {
	if( !str || !str->buffer ) {
		return 0;
	}

	return trap_ImageIndex( str->buffer );
}

static int asFunc_SkinIndex( asstring_t *str ) {
	if( !str || !str->buffer ) {
		return 0;
	}

	return trap_SkinIndex( str->buffer );
}

static int asFunc_ModelIndexExt( asstring_t *str, bool pure ) {
	int index;

	if( !str || !str->buffer ) {
		return 0;
	}

	index = trap_ModelIndex( str->buffer );
	if( index && pure ) {
		G_PureModel( str->buffer );
	}

	return index;
}

static int asFunc_ModelIndex( asstring_t *str ) {
	return asFunc_ModelIndexExt( str, false );
}

static int asFunc_SoundIndexExt( asstring_t *str, bool pure ) {
	int index;

	if( !str || !str->buffer ) {
		return 0;
	}

	index = trap_SoundIndex( str->buffer );
	if( index && pure ) {
		G_PureSound( str->buffer );
	}

	return index;
}

static int asFunc_SoundIndex( asstring_t *str ) {
	return asFunc_SoundIndexExt( str, false );
}

static void asFunc_RegisterCommand( asstring_t *str ) {
	if( !str || !str->buffer || !str->len ) {
		return;
	}

	G_AddCommand( str->buffer, NULL );
}

static void asFunc_RegisterCallvote( asstring_t *asname, asstring_t *asusage, asstring_t *astype, asstring_t *ashelp ) {
	if( !asname || !asname->buffer || !asname->buffer[0]  ) {
		return;
	}

	G_RegisterGametypeScriptCallvote( asname->buffer,
									  asusage ? asusage->buffer : NULL,
									  astype ? astype->buffer : NULL,
									  ashelp ? ashelp->buffer : NULL );
}

static asstring_t *asFunc_GetConfigString( int index ) {
	const char *cs = trap_GetConfigString( index );
	return game.asExport->asStringFactoryBuffer( (char *)cs, cs ? strlen( cs ) : 0 );
}

static void asFunc_SetConfigString( int index, asstring_t *str ) {
	if( !str || !str->buffer ) {
		return;
	}

	// write protect some configstrings
	if( index <= CS_POWERUPEFFECTS
		|| index == CS_AUTORECORDSTATE
		|| index == CS_MAXCLIENTS
		|| index == CS_WORLDMODEL
		|| index == CS_MAPCHECKSUM ) {
		G_Printf( "WARNING: ConfigString %i is write protected\n", index );
		return;
	}

	if( index >= CS_MMPLAYERINFOS && index < CS_MMPLAYERINFOS + MAX_MMPLAYERINFOS ) {
		G_Printf( "WARNING: ConfigString %i is write protected\n", index );
		return;
	}

	// prevent team name exploits
	if( index >= CS_TEAM_SPECTATOR_NAME && index < CS_TEAM_SPECTATOR_NAME + GS_MAX_TEAMS ) {
		bool forbidden = false;

		// never allow to change spectator and player teams names
		if( index < CS_TEAM_ALPHA_NAME ) {
			G_Printf( "WARNING: %s team name is write protected\n", GS_DefaultTeamName( index - CS_TEAM_SPECTATOR_NAME ) );
			return;
		}

		// don't allow empty team names
		if( !strlen( str->buffer ) ) {
			G_Printf( "WARNING: empty team names are not allowed\n" );
			return;
		}

		// never allow to change alpha and beta team names to a different team default name
		if( index == CS_TEAM_ALPHA_NAME ) {
			if( !Q_stricmp( str->buffer, GS_DefaultTeamName( TEAM_SPECTATOR ) ) ) {
				forbidden = true;
			}

			if( !Q_stricmp( str->buffer, GS_DefaultTeamName( TEAM_PLAYERS ) ) ) {
				forbidden = true;
			}

			if( !Q_stricmp( str->buffer, GS_DefaultTeamName( TEAM_BETA ) ) ) {
				forbidden = true;
			}
		}

		if( index == CS_TEAM_BETA_NAME ) {
			if( !Q_stricmp( str->buffer, GS_DefaultTeamName( TEAM_SPECTATOR ) ) ) {
				forbidden = true;
			}

			if( !Q_stricmp( str->buffer, GS_DefaultTeamName( TEAM_PLAYERS ) ) ) {
				forbidden = true;
			}

			if( !Q_stricmp( str->buffer, GS_DefaultTeamName( TEAM_ALPHA ) ) ) {
				forbidden = true;
			}
		}

		if( forbidden ) {
			G_Printf( "WARNING: %s team name can not be changed to %s\n", GS_DefaultTeamName( index - CS_TEAM_SPECTATOR_NAME ), str->buffer );
			return;
		}
	}

	trap_ConfigString( index, str->buffer );
}

static CScriptArrayInterface *asFunc_G_FindInRadius( asvec3_t *org, float radius ) {
	asITypeInfo *ot = asEntityArrayType();

	int touch[MAX_EDICTS];
	int numtouch = GClip_FindInRadius( org->v, radius, touch, MAX_EDICTS );
	CScriptArrayInterface *arr = game.asExport->asCreateArrayCpp( numtouch, ot );
	for( int i = 0; i < numtouch; i++ ) {
		*( (edict_t **)arr->At( i ) ) = game.edicts + touch[i];
	}

	return arr;
}

static CScriptArrayInterface *asFunc_G_FindByClassname( asstring_t *str ) {
	const char *classname = str->buffer;

	asITypeInfo *ot = asEntityArrayType();
	CScriptArrayInterface *arr = game.asExport->asCreateArrayCpp( 0, ot );

	int count = 0;
	edict_t *ent = NULL;
	while( ( ent = G_Find( ent, FOFS( classname ), classname ) ) != NULL ) {
		arr->Resize( count + 1 );
		*( (edict_t **)arr->At( count ) ) = ent;
		count++;
	}

	return arr;
}

static void asFunc_PositionedSound( asvec3_t *origin, int channel, int soundindex, float attenuation ) {
	if( !origin ) {
		return;
	}

	G_PositionedSound( origin->v, channel, soundindex, attenuation );
}

static void asFunc_G_GlobalSound( int channel, int soundindex ) {
	G_GlobalSound( channel, soundindex );
}

static void asFunc_G_LocalSound( gclient_t *target, int channel, int soundindex ) {
	edict_t *ent = NULL;

	if( !target ) {
		return;
	}

	if( target && !target->asFactored ) {
		int playerNum = target - game.clients;

		if( playerNum < 0 || playerNum >= gs.maxclients ) {
			return;
		}

		ent = game.edicts + playerNum + 1;
	}

	if( ent ) {
		G_LocalSound( ent, channel, soundindex );
	}
}

static void asFunc_G_AnnouncerSound( gclient_t *target, int soundindex, int team, bool queued, gclient_t *ignore ) {
	edict_t *ent = NULL, *passent = NULL;
	int playerNum;

	if( target && !target->asFactored ) {
		playerNum = target - game.clients;

		if( playerNum < 0 || playerNum >= gs.maxclients ) {
			return;
		}

		ent = game.edicts + playerNum + 1;
	}

	if( ignore && !ignore->asFactored ) {
		playerNum = ignore - game.clients;

		if( playerNum >= 0 && playerNum < gs.maxclients ) {
			passent = game.edicts + playerNum + 1;
		}
	}


	G_AnnouncerSound( ent, soundindex, team, queued, passent );
}

static asstring_t *asFunc_G_SpawnTempValue( asstring_t *key ) {
	const char *val;

	if( !key ) {
		return game.asExport->asStringFactoryBuffer( NULL, 0 );
	}

	if( level.spawning_entity == NULL ) {
		G_Printf( "WARNING: G_SpawnTempValue: Spawn temp values can only be grabbed during the entity spawning process\n" );
	}

	val = G_GetEntitySpawnKey( key->buffer, level.spawning_entity );

	return game.asExport->asStringFactoryBuffer( val, strlen( val ) );
}

static void asFunc_FireInstaShot( asvec3_t *origin, asvec3_t *dir, int range, int damage, int knockback, int stun, edict_t *owner ) {
	W_Fire_Instagun( owner, origin->v, dir->v, damage, knockback, stun, 0, range, MOD_INSTAGUN_S, 0 );
}

static edict_t *asFunc_FireWeakBolt( asvec3_t *origin, asvec3_t *dir, int speed, int damage, int knockback, int stun, edict_t *owner ) {
	return W_Fire_Electrobolt_Weak( owner, origin->v, dir->v, speed, damage, fmin( 1, knockback ), knockback, stun, 5000, MOD_ELECTROBOLT_W, 0 );
}

static void asFunc_FireStrongBolt( asvec3_t *origin, asvec3_t *dir, int range, int damage, int knockback, int stun, edict_t *owner ) {
	W_Fire_Electrobolt_FullInstant( owner, origin->v, dir->v, damage, damage, knockback, knockback, stun, range, range, MOD_ELECTROBOLT_S, 0 );
}

static edict_t *asFunc_FirePlasma( asvec3_t *origin, asvec3_t *dir, int speed, int radius, int damage, int knockback, int stun, edict_t *owner ) {
	return W_Fire_Plasma( owner, origin->v, dir->v, damage, 0, fmin( 1, knockback ), knockback, stun, fmin( 1, damage ), radius, speed, 5000, MOD_PLASMA_S, 0 );
}

static edict_t *asFunc_FireRocket( asvec3_t *origin, asvec3_t *dir, int speed, int radius, int damage, int knockback, int stun, edict_t *owner ) {
	return W_Fire_Rocket( owner, origin->v, dir->v, speed, damage, 0, fmin( 1, knockback ), knockback, stun, fmin( 1, damage ), radius, 5000, MOD_ROCKET_S, 0 );
}

static edict_t *asFunc_FireGrenade( asvec3_t *origin, asvec3_t *dir, int speed, int radius, int damage, int knockback, int stun, edict_t *owner ) {
	return W_Fire_Grenade( owner, origin->v, dir->v, speed, damage, 0, fmin( 1, knockback ), knockback, stun, fmin( 1, damage ), radius, 5000, MOD_GRENADE_S, 0 );
}

static void asFunc_FireRiotgun( asvec3_t *origin, asvec3_t *dir, int range, int spread, int count, int damage, int knockback, int stun, edict_t *owner ) {
	vec3_t fv, rv, uv;

	VectorNormalize2( dir->v, fv );
	MakeNormalVectors( fv, rv, uv );

	W_Fire_Riotgun( owner, origin->v, fv, rv, uv, rand() & 255, range, spread, spread, count, damage, knockback, stun, MOD_RIOTGUN_S, 0 );
}

static void asFunc_FireBullet( asvec3_t *origin, asvec3_t *dir, int range, int spread, int damage, int knockback, int stun, edict_t *owner ) {
	vec3_t fv, rv, uv;

	VectorNormalize2( dir->v, fv );
	MakeNormalVectors( fv, rv, uv );

	W_Fire_Bullet( owner, origin->v, fv, rv, uv, rand() & 255, range, spread, spread, damage, knockback, stun, MOD_MACHINEGUN_S, 0 );
}

static edict_t *asFunc_FireBlast( asvec3_t *origin, asvec3_t *dir, int speed, int radius, int damage, int knockback, int stun, edict_t *owner ) {
	return W_Fire_GunbladeBlast( owner, origin->v, dir->v, damage, 0, fmin( 1, knockback ), knockback, stun, fmin( 1, damage ), radius, speed, 5000, MOD_SPLASH, 0 );
}

static unsigned asFunc_G_RegisterHelpMessage( asstring_t *str ) {
	return G_RegisterHelpMessage( str->buffer );
}

static void asFunc_G_SetColorCorrection( int index ) {
	gs.gameState.stats[GAMESTAT_COLORCORRECTION] = index;
}

static int asFunc_G_GetDefaultColorCorrection( void ) {
	return level.colorCorrection;
}

static const gs_asglobfuncs_t asGameGlobFuncs[] =
{
	{ "Entity @G_SpawnEntity( const String &in )", asFUNCTION( asFunc_G_Spawn ), NULL },
	{ "const String @G_SpawnTempValue( const String &in )", asFUNCTION( asFunc_G_SpawnTempValue ), NULL },
	{ "Entity @G_GetEntity( int entNum )", asFUNCTION( asFunc_GetEntity ), NULL },
	{ "Client @G_GetClient( int clientNum )", asFUNCTION( asFunc_GetClient ), NULL },
	{ "Team @G_GetTeam( int team )", asFUNCTION( asFunc_GetTeamlist ), NULL },
	{ "Item @G_GetItem( int tag )", asFUNCTION( asFunc_GS_FindItemByTag ), NULL },
	{ "Item @G_GetItemByName( const String &in name )", asFUNCTION( asFunc_GS_FindItemByName ), NULL },
	{ "Item @G_GetItemByClassname( const String &in name )", asFUNCTION( asFunc_GS_FindItemByClassname ), NULL },
	{ "array<Entity @> @G_FindInRadius( const Vec3 &in, float radius )", asFUNCTION( asFunc_G_FindInRadius ), NULL },
	{ "array<Entity @> @G_FindByClassname( const String &in )", asFUNCTION( asFunc_G_FindByClassname ), NULL },

	// misc management utils
	{ "void G_RemoveProjectiles( Entity @ )", asFUNCTION( asFunc_G_Match_RemoveProjectiles ), NULL },
	{ "void G_RemoveAllProjectiles()", asFUNCTION( asFunc_G_Match_RemoveAllProjectiles ), NULL },
	{ "void G_ResetLevel()", asFUNCTION( asFunc_G_ResetLevel ), NULL },
	{ "void G_RemoveDeadBodies()", asFUNCTION( asFunc_G_Match_FreeBodyQueue ), NULL },
	{ "void G_Items_RespawnByType( uint typeMask, int item_tag, float delay )", asFUNCTION( asFunc_G_Items_RespawnByType ), NULL },

	// misc
	{ "void G_Print( const String &in )", asFUNCTION( asFunc_Print ), NULL },
	{ "void G_PrintMsg( Entity @, const String &in )", asFUNCTION( asFunc_PrintMsg ), NULL },
	{ "void G_CenterPrintMsg( Entity @, const String &in )", asFUNCTION( asFunc_CenterPrintMsg ), NULL },
	{ "void G_CenterPrintFormatMsg( Entity @, const String &in, const String &in )", asFUNCTION( asFunc_CenterPrintFormatMsg1 ), NULL },
	{ "void G_CenterPrintFormatMsg( Entity @, const String &in, const String &in, const String &in )", asFUNCTION( asFunc_CenterPrintFormatMsg2 ), NULL },
	{ "void G_CenterPrintFormatMsg( Entity @, const String &in, const String &in, const String &in"
	  ", const String &in )", asFUNCTION( asFunc_CenterPrintFormatMsg3 ), NULL },
	{ "void G_CenterPrintFormatMsg( Entity @, const String &in, const String &in, const String &in"
	  ", const String &in, const String &in )", asFUNCTION( asFunc_CenterPrintFormatMsg4 ), NULL },
	{ "void G_CenterPrintFormatMsg( Entity @, const String &in, const String &in, const String &in"
	  ", const String &in, const String &in, const String &in )", asFUNCTION( asFunc_CenterPrintFormatMsg5 ), NULL },
	{ "void G_CenterPrintFormatMsg( Entity @, const String &in, const String &in, const String &in"
	  ", const String &in, const String &in, const String &in, const String &in )", asFUNCTION( asFunc_CenterPrintFormatMsg6 ), NULL },
	{ "void G_CenterPrintFormatMsg( Entity @, const String &in, const String &in, const String &in"
	  ", const String &in, const String &in, const String &in, const String &in, const String &in )", asFUNCTION( asFunc_CenterPrintFormatMsg7 ), NULL },
	{ "void G_Error( const String &in )", asFUNCTION( asFunc_Error ), NULL },
	{ "void G_Sound( Entity @, int channel, int soundindex, float attenuation )", asFUNCTION( asFunc_G_Sound ), NULL },
	{ "void G_PositionedSound( const Vec3 &in, int channel, int soundindex, float attenuation )", asFUNCTION( asFunc_PositionedSound ), NULL },
	{ "void G_GlobalSound( int channel, int soundindex )", asFUNCTION( asFunc_G_GlobalSound ), NULL },
	{ "void G_LocalSound( Client @, int channel, int soundIndex )", asFUNCTION( asFunc_G_LocalSound ), NULL },
	{ "void G_AnnouncerSound( Client @, int soundIndex, int team, bool queued, Client @ )", asFUNCTION( asFunc_G_AnnouncerSound ), NULL },
	{ "int G_DirToByte( const Vec3 &in origin )", asFUNCTION( asFunc_DirToByte ), NULL },
	{ "int G_PointContents( const Vec3 &in origin )", asFUNCTION( asFunc_PointContents ), NULL },
	{ "bool G_InPVS( const Vec3 &in origin1, const Vec3 &in origin2 )", asFUNCTION( asFunc_InPVS ), NULL },
	{ "bool G_WriteFile( const String &, const String & )", asFUNCTION( asFunc_WriteFile ), NULL },
	{ "bool G_AppendToFile( const String &, const String & )", asFUNCTION( asFunc_AppendToFile ), NULL },
	{ "const String @G_LoadFile( const String & )", asFUNCTION( asFunc_LoadFile ), NULL },
	{ "int G_FileLength( const String & )", asFUNCTION( asFunc_FileLength ), NULL },
	{ "void G_CmdExecute( const String & )", asFUNCTION( asFunc_Cmd_ExecuteText ), NULL },
	{ "const String @G_LocationName( const Vec3 &in origin )", asFUNCTION( asFunc_LocationName ), NULL },
	{ "int G_LocationTag( const String & )", asFUNCTION( asFunc_LocationTag ), NULL },
	{ "const String @G_LocationName( int tag )", asFUNCTION( asFunc_LocationForTag ), NULL },

	{ "void __G_CallThink( Entity @ent )", asFUNCTION( G_CallThink ), &asEntityCallThinkFuncPtr },
	{ "void __G_CallTouch( Entity @ent, Entity @other, const Vec3 planeNormal, int surfFlags )", asFUNCTION( G_CallTouch ), &asEntityCallTouchFuncPtr },
	{ "void __G_CallUse( Entity @ent, Entity @other, Entity @activator )", asFUNCTION( G_CallUse ), &asEntityCallUseFuncPtr },
	{ "void __G_CallStop( Entity @ent )", asFUNCTION( G_CallStop ), &asEntityCallStopFuncPtr },
	{ "void __G_CallPain( Entity @ent, Entity @other, float kick, float damage )", asFUNCTION( G_CallPain ), &asEntityCallPainFuncPtr },
	{ "void __G_CallDie( Entity @ent, Entity @inflicter, Entity @attacker )", asFUNCTION( G_CallDie ), &asEntityCallDieFuncPtr },

	{ "int G_ImageIndex( const String &in )", asFUNCTION( asFunc_ImageIndex ), NULL },
	{ "int G_SkinIndex( const String &in )", asFUNCTION( asFunc_SkinIndex ), NULL },
	{ "int G_ModelIndex( const String &in )", asFUNCTION( asFunc_ModelIndex ), NULL },
	{ "int G_SoundIndex( const String &in )", asFUNCTION( asFunc_SoundIndex ), NULL },
	{ "int G_ModelIndex( const String &in, bool pure )", asFUNCTION( asFunc_ModelIndexExt ), NULL },
	{ "int G_SoundIndex( const String &in, bool pure )", asFUNCTION( asFunc_SoundIndexExt ), NULL },
	{ "void G_RegisterCommand( const String &in )", asFUNCTION( asFunc_RegisterCommand ), NULL },
	{ "void G_RegisterCallvote( const String &in, const String &in, const String &in, const String &in )", asFUNCTION( asFunc_RegisterCallvote ), NULL },
	{ "const String @G_ConfigString( int index )", asFUNCTION( asFunc_GetConfigString ), NULL },
	{ "void G_ConfigString( int index, const String &in )", asFUNCTION( asFunc_SetConfigString ), NULL },

	// projectile firing
	{ "void G_FireInstaShot( const Vec3 &in origin, const Vec3 &in dir, int range, int damage, int knockback, int stun, Entity @owner )", asFUNCTION( asFunc_FireInstaShot ), NULL },
	{ "Entity @G_FireWeakBolt( const Vec3 &in origin, const Vec3 &in dir, int speed, int damage, int knockback, int stun, Entity @owner )", asFUNCTION( asFunc_FireWeakBolt ), NULL },
	{ "void G_FireStrongBolt( const Vec3 &in origin, const Vec3 &in dir, int range, int damage, int knockback, int stun, Entity @owner )",asFUNCTION( asFunc_FireStrongBolt ), NULL },
	{ "Entity @G_FirePlasma( const Vec3 &in origin, const Vec3 &in dir, int speed, int radius, int damage, int knockback, int stun, Entity @owner )", asFUNCTION( asFunc_FirePlasma ), NULL },
	{ "Entity @G_FireRocket( const Vec3 &in origin, const Vec3 &in dir, int speed, int radius, int damage, int knockback, int stun, Entity @owner )", asFUNCTION( asFunc_FireRocket ), NULL },
	{ "Entity @G_FireGrenade( const Vec3 &in origin, const Vec3 &in dir, int speed, int radius, int damage, int knockback, int stun, Entity @owner )", asFUNCTION( asFunc_FireGrenade ), NULL },
	{ "void G_FireRiotgun( const Vec3 &in origin, const Vec3 &in dir, int range, int spread, int count, int damage, int knockback, int stun, Entity @owner )", asFUNCTION( asFunc_FireRiotgun ), NULL },
	{ "void G_FireBullet( const Vec3 &in origin, const Vec3 &in dir, int range, int spread, int damage, int knockback, int stun, Entity @owner )", asFUNCTION( asFunc_FireBullet ), NULL },
	{ "Entity @G_FireBlast( const Vec3 &in origin, const Vec3 &in dir, int speed, int radius, int damage, int knockback, int stun, Entity @owner )", asFUNCTION( asFunc_FireBlast ), NULL },

	{ "bool ML_FilenameExists( String & )", asFUNCTION( asFunc_ML_FilenameExists ), NULL },
	{ "const String @ML_GetMapByNum( int num )", asFUNCTION( asFunc_ML_GetMapByNum ), NULL },

	{ "uint G_RegisterHelpMessage( const String &in )", asFUNCTION( asFunc_G_RegisterHelpMessage ), NULL },

	// color correction
	{ "void G_SetColorCorrection( int index )", asFUNCTION( asFunc_G_SetColorCorrection ), NULL },
	{ "int G_GetDefaultColorCorrection()", asFUNCTION( asFunc_G_GetDefaultColorCorrection ), NULL },

	{ NULL }
};

// ============================================================================

static void asFunc_AI_AddGoal( edict_t *self, bool customReach )
{
	if( customReach )
		AI_AddGoalEntityCustom( self );
	else
		AI_AddGoalEntity( self );
}

static const gs_asglobfuncs_t asAIGlobFuncs[] =
{
	{ "int GetRootGoal()", asFUNCTION(AI_GetRootGoalEnt), NULL },
	{ "int GetNextGoal( int index )", asFUNCTION(AI_GetNextGoalEnt), NULL },
	{ "Entity @GetGoalEntity( int index )", asFUNCTION(AI_GetGoalEntity), NULL },

	{ "void AddGoal( Entity @ent )", asFUNCTION(AI_AddGoalEntity), NULL },
	{ "void AddGoal( Entity @ent, bool customReach )", asFUNCTION(asFunc_AI_AddGoal), NULL },
	{ "void RemoveGoal( Entity @ent )", asFUNCTION(AI_RemoveGoalEntity), NULL },
	{ "void ReachedGoal( Entity @ent )", asFUNCTION(AI_ReachedEntity), NULL },

	{ NULL }
};

// ============================================================================

static const gs_asglobproperties_t asGlobProps[] =
{
	{ "const int64 levelTime", &level.time },
	{ "const uint frameTime", &game.frametime },
	{ "const int64 realTime", &game.realtime },

	//{ "const uint serverTime", &game.serverTime }, // I think this one isn't script business
	{ "const int64 localTime", &game.localTime },
	{ "const int maxEntities", &game.maxentities },
	{ "const int numEntities", &game.numentities },
	{ "const int maxClients", &gs.maxclients },
	{ "GametypeDesc gametype", &level.gametype },
	{ "Match match", &level.gametype.match },

	{ NULL }
};

// ==========================================================================================

// map entity spawning
bool G_asCallMapEntitySpawnScript( const char *classname, edict_t *ent ) {
	char fdeclstr[MAX_STRING_CHARS];
	int error;
	asIScriptContext *asContext;
	asIScriptEngine *asEngine = GAME_AS_ENGINE();
	asIScriptModule *asSpawnModule;
	asIScriptFunction *asSpawnFunc;

	if( !asEngine ) {
		return false;
	}

	Q_snprintfz( fdeclstr, sizeof( fdeclstr ), "void %s( Entity @ent )", classname );

	// lookup the spawn function in gametype module first, fallback to map script
	asSpawnModule = asEngine->GetModule( GAMETYPE_SCRIPTS_MODULE_NAME );
	asSpawnFunc = asSpawnModule ? asSpawnModule->GetFunctionByDecl( fdeclstr ) : NULL;
	if( !asSpawnFunc ) {
		asSpawnModule = asEngine->GetModule( MAP_SCRIPTS_MODULE_NAME );
		asSpawnFunc = asSpawnModule ? asSpawnModule->GetFunctionByDecl( fdeclstr ) : NULL;
	}

	if( !asSpawnFunc ) {
		return false;
	}

	// this is in case we might want to call G_asReleaseEntityBehaviors
	// in the spawn function (an object may release itself, ugh)
	ent->asSpawnFunc = static_cast<void *>( asSpawnFunc );
	ent->asScriptModule = static_cast<void *>( asSpawnModule );
	ent->scriptSpawned = true;
	G_asClearEntityBehaviors( ent );

	// call the spawn function
	asContext = game.asExport->asAcquireContext( asEngine );
	error = asContext->Prepare( asSpawnFunc );
	if( error < 0 ) {
		return false;
	}

	// Now we need to pass the parameters to the script function.
	asContext->SetArgObject( 0, ent );

	error = asContext->Execute();
	if( G_ExecutionErrorReport( error ) ) {
		GT_asShutdownScript();
		ent->asScriptModule = NULL;
		ent->asSpawnFunc = NULL;
		ent->scriptSpawned = false;
		return false;
	}

	// check the inuse flag because the entity might have been removed at the spawn
	ent->scriptSpawned = ent->r.inuse;
	return true;
}

/*
* G_asResetEntityBehaviors
*/
void G_asResetEntityBehaviors( edict_t *ent ) {
	ent->asThinkFunc = asEntityCallThinkFuncPtr;
	ent->asTouchFunc = asEntityCallTouchFuncPtr;
	ent->asUseFunc = asEntityCallUseFuncPtr;
	ent->asStopFunc = asEntityCallStopFuncPtr;
	ent->asPainFunc = asEntityCallPainFuncPtr;
	ent->asDieFunc = asEntityCallDieFuncPtr;
}

/*
* G_asClearEntityBehaviors
*/
void G_asClearEntityBehaviors( edict_t *ent ) {
	ent->asThinkFunc = NULL;
	ent->asTouchFunc = NULL;
	ent->asUseFunc = NULL;
	ent->asStopFunc = NULL;
	ent->asPainFunc = NULL;
	ent->asDieFunc = NULL;
}

/*
* G_asReleaseEntityBehaviors
*
* Release callback function references held by the engine for script spawned entities
*/
void G_asReleaseEntityBehaviors( edict_t *ent ) {
	if( ent->scriptSpawned && game.asExport ) {
		if( ent->asThinkFunc ) {
			static_cast<asIScriptFunction*>( ent->asThinkFunc )->Release();
		}
		if( ent->asTouchFunc ) {
			static_cast<asIScriptFunction*>( ent->asTouchFunc )->Release();
		}
		if( ent->asUseFunc ) {
			static_cast<asIScriptFunction*>( ent->asUseFunc )->Release();
		}
		if( ent->asStopFunc ) {
			static_cast<asIScriptFunction*>( ent->asStopFunc )->Release();
		}
		if( ent->asPainFunc ) {
			static_cast<asIScriptFunction*>( ent->asPainFunc )->Release();
		}
		if( ent->asDieFunc ) {
			static_cast<asIScriptFunction*>( ent->asDieFunc )->Release();
		}
	}

	G_asClearEntityBehaviors( ent );
}

//"void %s_think( Entity @ent )"
void G_asCallMapEntityThink( edict_t *ent ) {
	int error;
	asIScriptContext *ctx;

	if( !ent->asThinkFunc ) {
		return;
	}

	ctx = game.asExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>( ent->asThinkFunc ) );
	if( error < 0 ) {
		return;
	}

	// Now we need to pass the parameters to the script function.
	ctx->SetArgObject( 0, ent );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) ) {
		GT_asShutdownScript();
	}
}

// "void %s_touch( Entity @ent, Entity @other, const Vec3 planeNormal, int surfFlags )"
void G_asCallMapEntityTouch( edict_t *ent, edict_t *other, cplane_t *plane, int surfFlags ) {
	int error;
	asIScriptContext *ctx;
	asvec3_t normal;

	if( !ent->asTouchFunc ) {
		return;
	}

	ctx = game.asExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>( ent->asTouchFunc ) );
	if( error < 0 ) {
		return;
	}

	if( plane ) {
		VectorCopy( plane->normal, normal.v );
	} else {
		VectorClear( normal.v );
	}

	// Now we need to pass the parameters to the script function.
	ctx->SetArgObject( 0, ent );
	ctx->SetArgObject( 1, other );
	ctx->SetArgObject( 2, &normal );
	ctx->SetArgDWord( 3, surfFlags );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) ) {
		GT_asShutdownScript();
	}
}

// "void %s_use( Entity @ent, Entity @other, Entity @activator )"
void G_asCallMapEntityUse( edict_t *ent, edict_t *other, edict_t *activator ) {
	int error;
	asIScriptContext *ctx;

	if( !ent->asUseFunc ) {
		return;
	}

	ctx = game.asExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>( ent->asUseFunc ) );
	if( error < 0 ) {
		return;
	}

	// Now we need to pass the parameters to the script function.
	ctx->SetArgObject( 0, ent );
	ctx->SetArgObject( 1, other );
	ctx->SetArgObject( 2, activator );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) ) {
		GT_asShutdownScript();
	}
}

// "void %s_pain( Entity @ent, Entity @other, float kick, float damage )"
void G_asCallMapEntityPain( edict_t *ent, edict_t *other, float kick, float damage ) {
	int error;
	asIScriptContext *ctx;

	if( !ent->asPainFunc ) {
		return;
	}

	ctx = game.asExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>( ent->asPainFunc ) );
	if( error < 0 ) {
		return;
	}

	// Now we need to pass the parameters to the script function.
	ctx->SetArgObject( 0, ent );
	ctx->SetArgObject( 1, other );
	ctx->SetArgFloat( 2, kick );
	ctx->SetArgFloat( 3, damage );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) ) {
		GT_asShutdownScript();
	}
}

// "void %s_die( Entity @ent, Entity @inflicter, Entity @attacker )"
void G_asCallMapEntityDie( edict_t *ent, edict_t *inflicter, edict_t *attacker, int damage, const vec3_t point ) {
	int error;
	asIScriptContext *ctx;

	if( !ent->asDieFunc ) {
		return;
	}

	ctx = game.asExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>( ent->asDieFunc ) );
	if( error < 0 ) {
		return;
	}

	// Now we need to pass the parameters to the script function.
	ctx->SetArgObject( 0, ent );
	ctx->SetArgObject( 1, inflicter );
	ctx->SetArgObject( 2, attacker );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) ) {
		GT_asShutdownScript();
	}
}

//"void %s_stop( Entity @ent )"
void G_asCallMapEntityStop( edict_t *ent ) {
	int error;
	asIScriptContext *ctx;

	if( !ent->asStopFunc ) {
		return;
	}

	ctx = game.asExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>( ent->asStopFunc ) );
	if( error < 0 ) {
		return;
	}

	// Now we need to pass the parameters to the script function.
	ctx->SetArgObject( 0, ent );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) ) {
		GT_asShutdownScript();
	}
}

// ======================================================================================

/*
* G_ExecutionErrorReport
*/
bool G_ExecutionErrorReport( int error ) {
	if( error == asEXECUTION_FINISHED ) {
		return false;
	}
	return true;
}

/*
* G_LoadGameScript
*/
asIScriptModule *G_LoadGameScript( const char *moduleName, const char *dir, const char *filename ) {
	return game.asExport->asLoadScriptProject( GAME_AS_ENGINE(), moduleName, GAME_SCRIPTS_DIRECTORY, dir, filename, NULL );
}

/*
* G_ResetGameModuleScriptData
*/
static void G_ResetGameModuleScriptData( void ) {
	game.asEngine = NULL;

	asEntityCallThinkFuncPtr = NULL;
	asEntityCallTouchFuncPtr = NULL;
	asEntityCallUseFuncPtr = NULL;
	asEntityCallStopFuncPtr = NULL;
	asEntityCallPainFuncPtr = NULL;
	asEntityCallDieFuncPtr = NULL;
}

/*
* G_InitializeGameModuleSyntax
*/
static void G_InitializeGameModuleSyntax( asIScriptEngine *asEngine ) {
	G_Printf( "* Initializing Game module syntax\n" );

	// register shared stuff
	GS_asInitializeEngine( asEngine );

	// register global enums
	GS_asRegisterEnums( asEngine, asGameEnums, NULL );

	// first register all class names so methods using custom classes work
	GS_asRegisterObjectClassNames( asEngine, asGameClassesDescriptors, NULL );

	// register classes
	GS_asRegisterObjectClasses( asEngine, asGameClassesDescriptors, NULL );

	// register global functions
	GS_asRegisterGlobalFunctions( asEngine, asGameGlobFuncs, NULL );
	GS_asRegisterGlobalFunctions( asEngine, asAIGlobFuncs, "AI" );

	// register global properties
	GS_asRegisterGlobalProperties( asEngine, asGlobProps, NULL );
}

/*
* G_asInitGameModuleEngine
*/
void G_asInitGameModuleEngine( void ) {
	bool asGeneric;
	asIScriptEngine *asEngine;

	G_ResetGameModuleScriptData();

	// initialize the engine
	game.asExport = trap_asGetAngelExport();
	if( !game.asExport ) {
		G_Printf( "* Couldn't initialize angelscript, missing symbol.\n" );
		return;
	}

	asEngine = game.asExport->asCreateEngine( &asGeneric );
	if( !asEngine ) {
		G_Printf( "* Couldn't initialize angelscript.\n" );
		return;
	}

	if( asGeneric ) {
		G_Printf( "* Generic calling convention detected, aborting.\n" );
		G_asShutdownGameModuleEngine();
		return;
	}

	game.asEngine = asEngine;

	G_InitializeGameModuleSyntax( asEngine );
}

/*
* G_asShutdownGameModuleEngine
*/
void G_asShutdownGameModuleEngine( void ) {
	if( game.asEngine == NULL ) {
		return;
	}

	game.asExport->asReleaseEngine( static_cast<asIScriptEngine *>( game.asEngine ) );
	G_ResetGameModuleScriptData();
}

/*
* G_asDumpAPI_f
*
* Dump all classes, global functions and variables into a file
*/
void G_asDumpAPI_f( void ) {
	game.asExport->asWriteEngineDocsToFile( GAME_AS_ENGINE(), 
		va( "AS_API/v%.g/", trap_Cvar_Value( "version" ) ), "game",
		false, false, ~(unsigned)0, 0 );
}
