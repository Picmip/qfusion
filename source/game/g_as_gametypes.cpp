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

static void GT_ResetScriptData( void )
{
	level.gametype.initFunc = NULL;
	level.gametype.spawnFunc = NULL;
	level.gametype.matchStateStartedFunc = NULL;
	level.gametype.matchStateFinishedFunc = NULL;
	level.gametype.thinkRulesFunc = NULL;
	level.gametype.playerRespawnFunc = NULL;
	level.gametype.scoreEventFunc = NULL;
	level.gametype.scoreboardMessageFunc = NULL;
	level.gametype.selectSpawnPointFunc = NULL;
	level.gametype.clientCommandFunc = NULL;
	level.gametype.shutdownFunc = NULL;

	level.gametype.botWouldDropHealthFunc = NULL;
	level.gametype.botDropHealthFunc = NULL;
	level.gametype.botWouldDropArmorFunc = NULL;
	level.gametype.botDropArmorFunc = NULL;
	level.gametype.botWouldCloakFunc = NULL;
	level.gametype.setBotCloakEnabledFunc = NULL;
	level.gametype.isEntityCloakingFunc = NULL;
	level.gametype.playerOffenciveAbilitiesScoreFunc = NULL;
	level.gametype.playerDefenciveAbilitiesScoreFunc = NULL;
	level.gametype.onBotTouchedGoalFunc = NULL;
	level.gametype.onBotReachedGoalRadiusFunc = NULL;
}

void GT_asShutdownScript( void )
{
	int i;
	edict_t *e;

	if( game.asEngine == NULL ) {
		return;
	}

	// release the callback and any other objects obtained from the script engine before releasing the engine
	for( i = 0; i < game.numentities; i++ ) {
		e = &game.edicts[i];

		if( e->scriptSpawned && e->asScriptModule &&
			!strcmp( (static_cast<asIScriptModule*>(e->asScriptModule))->GetName(), GAMETYPE_SCRIPTS_MODULE_NAME ) ) {
			G_asReleaseEntityBehaviors( e );
			e->asScriptModule = NULL;
		}
	}

	GT_ResetScriptData();

	GAME_AS_ENGINE()->DiscardModule( GAMETYPE_SCRIPTS_MODULE_NAME );
}

//"void GT_SpawnGametype()"
void GT_asCallSpawn( void )
{
	int error;
	asIScriptContext *ctx;

	if( !level.gametype.spawnFunc )
		return;

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.spawnFunc) );
	if( error < 0 ) 
		return;

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();
}

//"void GT_MatchStateStarted()"
void GT_asCallMatchStateStarted( void )
{
	int error;
	asIScriptContext *ctx;

	if( !level.gametype.matchStateStartedFunc )
		return;

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.matchStateStartedFunc) );
	if( error < 0 ) 
		return;

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();
}

//"bool GT_MatchStateFinished( int incomingMatchState )"
bool GT_asCallMatchStateFinished( int incomingMatchState )
{
	int error;
	asIScriptContext *ctx;
	bool result;

	if( !level.gametype.matchStateFinishedFunc )
		return true;

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.matchStateFinishedFunc) );
	if( error < 0 ) 
		return true;

	// Now we need to pass the parameters to the script function.
	ctx->SetArgDWord( 0, incomingMatchState );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();

	// Retrieve the return from the context
	result = ctx->GetReturnByte() == 0 ? false : true;

	return result;
}

//"void GT_ThinkRules( void )"
void GT_asCallThinkRules( void )
{
	int error;
	asIScriptContext *ctx;

	if( !level.gametype.thinkRulesFunc )
		return;

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.thinkRulesFunc) );
	if( error < 0 ) 
		return;

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();
}

//"void GT_playerRespawn( Entity @ent, int old_team, int new_team )"
void GT_asCallPlayerRespawn( edict_t *ent, int old_team, int new_team )
{
	int error;
	asIScriptContext *ctx;

	if( !level.gametype.playerRespawnFunc )
		return;

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.playerRespawnFunc) );
	if( error < 0 ) 
		return;

	// Now we need to pass the parameters to the script function.
	ctx->SetArgObject( 0, ent );
	ctx->SetArgDWord( 1, old_team );
	ctx->SetArgDWord( 2, new_team );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();
}

//"void GT_scoreEvent( Client @client, String &score_event, String &args )"
void GT_asCallScoreEvent( gclient_t *client, const char *score_event, const char *args )
{
	int error;
	asIScriptContext *ctx;
	asstring_t *s1, *s2;

	if( !level.gametype.scoreEventFunc )
		return;

	if( !score_event || !score_event[0] )
		return;

	if( !args )
		args = "";

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.scoreEventFunc) );
	if( error < 0 ) 
		return;

	// Now we need to pass the parameters to the script function.
	s1 = angelExport->asStringFactoryBuffer( score_event, strlen( score_event ) );
	s2 = angelExport->asStringFactoryBuffer( args, strlen( args ) );

	ctx->SetArgObject( 0, client );
	ctx->SetArgObject( 1, s1 );
	ctx->SetArgObject( 2, s2 );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();

	angelExport->asStringRelease( s1 );
	angelExport->asStringRelease( s2 );
}

//"String @GT_ScoreboardMessage( uint maxlen )"
// The result is stored in scoreboardString.
void GT_asCallScoreboardMessage( unsigned int maxlen )
{
	asstring_t *string;
	int error;
	asIScriptContext *ctx;

	scoreboardString[0] = 0;

	if( !level.gametype.scoreboardMessageFunc )
		return;

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.scoreboardMessageFunc) );
	if( error < 0 ) 
		return;

	// Now we need to pass the parameters to the script function.
	ctx->SetArgDWord( 0, maxlen );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();

	string = ( asstring_t * )ctx->GetReturnObject( );
	if( !string || !string->len || !string->buffer )
		return;

	Q_strncpyz( scoreboardString, string->buffer, sizeof( scoreboardString ) );
}

//"Entity @GT_SelectSpawnPoint( Entity @ent )"
edict_t *GT_asCallSelectSpawnPoint( edict_t *ent )
{
	int error;
	asIScriptContext *ctx;
	edict_t *spot;

	if( !level.gametype.selectSpawnPointFunc )
		return SelectDeathmatchSpawnPoint( ent ); // should have a hardcoded backup

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.selectSpawnPointFunc) );
	if( error < 0 ) 
		return SelectDeathmatchSpawnPoint( ent );

	// Now we need to pass the parameters to the script function.
	ctx->SetArgObject( 0, ent );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();

	spot = ( edict_t * )ctx->GetReturnObject();
	if( !spot )
		spot = SelectDeathmatchSpawnPoint( ent );

	return spot;
}

//"bool GT_Command( Client @client, String &cmdString, String &argsString, int argc )"
bool GT_asCallGameCommand( gclient_t *client, const char *cmd, const char *args, int argc )
{
	int error;
	asIScriptContext *ctx;
	asstring_t *s1, *s2;

	if( !level.gametype.clientCommandFunc )
		return false; // should have a hardcoded backup

	// check for having any command to parse
	if( !cmd || !cmd[0] )
		return false;

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.clientCommandFunc) );
	if( error < 0 ) 
		return false;

	// Now we need to pass the parameters to the script function.
	s1 = angelExport->asStringFactoryBuffer( cmd, strlen( cmd ) );
	s2 = angelExport->asStringFactoryBuffer( args, strlen( args ) );

	ctx->SetArgObject( 0, client );
	ctx->SetArgObject( 1, s1 );
	ctx->SetArgObject( 2, s2 );
	ctx->SetArgDWord( 3, argc );

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();

	angelExport->asStringRelease( s1 );
	angelExport->asStringRelease( s2 );

	// Retrieve the return from the context
	return ctx->GetReturnByte() == 0 ? false : true;
}

//"void GT_Shutdown()"
void GT_asCallShutdown( void )
{
	int error;
	asIScriptContext *ctx;

	if( !level.gametype.shutdownFunc || !angelExport )
		return;

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.shutdownFunc) );
	if( error < 0 ) 
		return;

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		GT_asShutdownScript();
}

static bool CallBotWouldDropFunc(const gclient_t *client, void *func)
{
	if (!func || !angelExport)
		return false;

	auto ctx = angelExport->asAcquireContext(GAME_AS_ENGINE());
	int error = ctx->Prepare(static_cast<asIScriptFunction *>(func));
	if ( error < 0 )
		return false;

	ctx->SetArgObject( 0, const_cast<gclient_t *>(client) );

	error = ctx->Execute();
	if (G_ExecutionErrorReport(error))
		GT_asShutdownScript();

	if (error < 0)
		return false;

	return ctx->GetReturnByte() != 0;
}

static void CallBotDropFunc(gclient_t *client, void *func)
{
	if (!func || !angelExport)
		return;

	auto ctx = angelExport->asAcquireContext(GAME_AS_ENGINE());
	int error = ctx->Prepare(static_cast<asIScriptFunction *>(func));
	if ( error < 0 )
		return;

	ctx->SetArgObject( 0, const_cast<gclient_t *>(client) );

	error = ctx->Execute();
	if (G_ExecutionErrorReport(error))
		GT_asShutdownScript();
}

bool GT_asBotWouldDropHealth(const gclient_t *client)
{
	return CallBotWouldDropFunc(client, level.gametype.botWouldDropHealthFunc);
}

void GT_asBotDropHealth(gclient_t *client)
{
	return CallBotDropFunc(client, level.gametype.botDropHealthFunc);
}

bool GT_asBotWouldDropArmor(const gclient_t *client)
{
	return CallBotWouldDropFunc(client, level.gametype.botWouldDropArmorFunc);
}

void GT_asBotDropArmor(gclient_t *client)
{
	return CallBotDropFunc(client, level.gametype.botDropArmorFunc);
}

// May be reused for other abilities

static bool CallBotWouldUseAbility(const gclient_t *client, void *func)
{
	if (!func || !angelExport)
		return false;

	auto ctx = angelExport->asAcquireContext(GAME_AS_ENGINE());
	int error = ctx->Prepare(static_cast<asIScriptFunction *>(func));
	if (error < 0)
		return false;

	ctx->SetArgObject(0, const_cast<gclient_t*>(client));

	error = ctx->Execute();
	if (G_ExecutionErrorReport(error))
		GT_asShutdownScript();

	if (error < 0)
		return false;

	return ctx->GetReturnByte() != 0;
}

static void CallSetBotAbilityEnabled(gclient_t *client, bool enabled, void *func)
{
	if (!func || !angelExport)
		return;

	auto ctx = angelExport->asAcquireContext(GAME_AS_ENGINE());
	int error = ctx->Prepare(static_cast<asIScriptFunction *>(func));
	if (error < 0)
		return;

	ctx->SetArgObject(0, client);
	ctx->SetArgByte(1, enabled);

	error = ctx->Execute();
	if (G_ExecutionErrorReport(error))
		G_asShutdownMapScript();
}

bool GT_asBotWouldCloak(const gclient_t *client)
{
	return CallBotWouldUseAbility(client, level.gametype.botWouldCloakFunc);
}

bool GT_asIsEntityCloaking(const edict_t *ent)
{
	if (!level.gametype.isEntityCloakingFunc || !angelExport)
		return false;

	auto ctx = angelExport->asAcquireContext(GAME_AS_ENGINE());
	int error = ctx->Prepare(static_cast<asIScriptFunction *>(level.gametype.isEntityCloakingFunc));
	if (error < 0)
		return false;

	ctx->SetArgObject(0, const_cast<edict_t *>(ent));

	error = ctx->Execute();
	if (G_ExecutionErrorReport(error))
		GT_asShutdownScript();

	if (error < 0)
		return false;

	return ctx->GetReturnByte() != 0;
}

void GT_asSetBotCloakEnabled(gclient_t *client, bool enabled)
{
	CallSetBotAbilityEnabled(client, enabled, level.gametype.setBotCloakEnabledFunc);
}

static float CallBotPlayerAbilitiesScoreFunc(const gclient_t *client, void *func)
{
	if (!func || !angelExport)
		return 0.5f;

	auto ctx = angelExport->asAcquireContext(GAME_AS_ENGINE());
	int error = ctx->Prepare(static_cast<asIScriptFunction *>(func));
	if ( error < 0 )
		return 0.5f;

	ctx->SetArgObject( 0, const_cast<gclient_t *>(client) );

	error = ctx->Execute();
	if (G_ExecutionErrorReport(error))
		GT_asShutdownScript();

	if (error < 0)
		return 0.5f;

	return ctx->GetReturnFloat();
}

float GT_asPlayerOffenciveAbilitiesScore(const gclient_t *client)
{
	return CallBotPlayerAbilitiesScoreFunc(client, level.gametype.playerOffenciveAbilitiesScoreFunc);
}

float GT_asPlayerDefenciveAbilitiesScore(const gclient_t *client)
{
	return CallBotPlayerAbilitiesScoreFunc(client, level.gametype.playerDefenciveAbilitiesScoreFunc);
}

static void CallBotGoalCallbackFunc(const ai_handle_t *bot, const edict_t *goalEnt, void *func)
{
	if (!func || !angelExport)
		return;

	auto ctx = angelExport->asAcquireContext(GAME_AS_ENGINE());
	int error = ctx->Prepare(static_cast<asIScriptFunction *>(func));
	if( error < 0 )
		return;

	ctx->SetArgObject(0, const_cast<ai_handle_t *>(bot));
	ctx->SetArgObject(1, const_cast<edict_t *>(goalEnt));

	error = ctx->Execute();
	if (G_ExecutionErrorReport(error))
		GT_asShutdownScript();
}

void GT_asBotTouchedGoal(const ai_handle_t *bot, const edict_t *goalEnt)
{
	CallBotGoalCallbackFunc(bot, goalEnt, level.gametype.onBotTouchedGoalFunc);
}

void GT_asBotReachedGoalRadius(const ai_handle_t *bot, const edict_t *goalEnt)
{
	CallBotGoalCallbackFunc(bot, goalEnt, level.gametype.onBotReachedGoalRadiusFunc);
}

static bool G_asInitializeGametypeScript( asIScriptModule *asModule )
{
	int error;
	asIScriptContext *ctx;
	int funcCount;
	const char *fdeclstr;

	// grab script function calls
	funcCount = 0;

	fdeclstr = "void GT_InitGametype()";
	level.gametype.initFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.initFunc )
	{
		G_Printf( "* The function '%s' was not found. Can not continue.\n", fdeclstr );
		return false;
	}
	else
		funcCount++;

	fdeclstr = "void GT_SpawnGametype()";
	level.gametype.spawnFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.spawnFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_MatchStateStarted()";
	level.gametype.matchStateStartedFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.matchStateStartedFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "bool GT_MatchStateFinished( int incomingMatchState )";
	level.gametype.matchStateFinishedFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.matchStateFinishedFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_ThinkRules()";
	level.gametype.thinkRulesFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.thinkRulesFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_PlayerRespawn( Entity @ent, int old_team, int new_team )";
	level.gametype.playerRespawnFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.playerRespawnFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_ScoreEvent( Client @client, const String &score_event, const String &args )";
	level.gametype.scoreEventFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.scoreEventFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "String @GT_ScoreboardMessage( uint maxlen )";
	level.gametype.scoreboardMessageFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.scoreboardMessageFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "Entity @GT_SelectSpawnPoint( Entity @ent )";
	level.gametype.selectSpawnPointFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.selectSpawnPointFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "bool GT_Command( Client @client, const String &cmdString, const String &argsString, int argc )";
	level.gametype.clientCommandFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.clientCommandFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "bool GT_BotWouldDropHealth( Client @client )";
	level.gametype.botWouldDropHealthFunc = asModule->GetFunctionByDecl( fdeclstr );
	if ( !level.gametype.botWouldDropHealthFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_BotDropHealth( Client @client )";
	level.gametype.botDropHealthFunc = asModule->GetFunctionByDecl( fdeclstr );
	if ( !level.gametype.botDropHealthFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "bool GT_BotWouldDropArmor( Client @client )";
	level.gametype.botWouldDropArmorFunc = asModule->GetFunctionByDecl( fdeclstr );
	if ( !level.gametype.botWouldDropArmorFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_BotDropArmor( Client @client )";
	level.gametype.botDropArmorFunc = asModule->GetFunctionByDecl( fdeclstr );
	if ( !level.gametype.botDropArmorFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "bool GT_BotWouldCloak( Client @client )";
	level.gametype.botWouldCloakFunc = asModule->GetFunctionByDecl(fdeclstr);
	if ( !level.gametype.botWouldCloakFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_SetBotCloakEnabled( Client @client, bool enabled )";
	level.gametype.setBotCloakEnabledFunc = asModule->GetFunctionByDecl(fdeclstr);
	if ( !level.gametype.setBotCloakEnabledFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "bool GT_IsEntityCloaking( Entity @ent )";
	level.gametype.isEntityCloakingFunc = asModule->GetFunctionByDecl( fdeclstr );
	if ( !level.gametype.isEntityCloakingFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "float GT_PlayerOffenciveAbilitiesScore( Client @client )";
	level.gametype.playerOffenciveAbilitiesScoreFunc = asModule->GetFunctionByDecl( fdeclstr );
	if ( !level.gametype.playerOffenciveAbilitiesScoreFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "float GT_PlayerDefenciveAbilitiesScore( Client @client )";
	level.gametype.playerDefenciveAbilitiesScoreFunc = asModule->GetFunctionByDecl( fdeclstr );
	if ( !level.gametype.playerDefenciveAbilitiesScoreFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_BotTouchedGoal( Bot @bot, Entity @goalEnt )";
	level.gametype.onBotTouchedGoalFunc = asModule->GetFunctionByDecl( fdeclstr );
	if ( !level.gametype.onBotTouchedGoalFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_BotReachedGoalRadius( Bot @bot, Entity @goalEnt )";
	level.gametype.onBotReachedGoalRadiusFunc = asModule->GetFunctionByDecl( fdeclstr );
	if ( !level.gametype.onBotReachedGoalRadiusFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	fdeclstr = "void GT_Shutdown()";
	level.gametype.shutdownFunc = asModule->GetFunctionByDecl( fdeclstr );
	if( !level.gametype.shutdownFunc )
	{
		if( developer->integer || sv_cheats->integer )
			G_Printf( "* The function '%s' was not present in the script.\n", fdeclstr );
	}
	else
		funcCount++;

	//
	// execute the GT_InitGametype function
	//

	ctx = angelExport->asAcquireContext( GAME_AS_ENGINE() );

	error = ctx->Prepare( static_cast<asIScriptFunction *>(level.gametype.initFunc) );
	if( error < 0 ) 
		return false;

	error = ctx->Execute();
	if( G_ExecutionErrorReport( error ) )
		return false;

	return true;
}

bool GT_asLoadScript( const char *gametypeName )
{
	const char *moduleName = GAMETYPE_SCRIPTS_MODULE_NAME;
	asIScriptModule *asModule;

	GT_ResetScriptData();

	// Load the script
	asModule = G_LoadGameScript( moduleName, GAMETYPE_SCRIPTS_DIRECTORY, gametypeName, GAMETYPE_PROJECT_EXTENSION );
	if( asModule == NULL ) {
		return false;
	}

	// Initialize the script
	if( !G_asInitializeGametypeScript( asModule ) ) {
		GT_asShutdownScript();
		return false;
	}

	return true;
}
