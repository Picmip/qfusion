/*
Copyright (C) 1997-2001 Id Software, Inc.

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

#include "ai_local.h"

nav_ents_t *Ai::GetGoalentForEnt( edict_t *target )
{
	int entnum;

	if( !target )
		return NULL;

	entnum = ENTNUM( target );
	return nav.entsGoals[entnum];
}

//==========================================
// AI_ResetNavigation
// Init bot navigation. Called at first spawn & each respawn
//==========================================
void Ai::ResetNavigation()
{
	self->enemy = self->ai->latched_enemy = NULL;
	self->ai->state_combat_timeout = 0;

	self->ai->is_bunnyhop = false;

	self->ai->nearest_node_tries = 0;
	self->ai->longRangeGoalTimeout = 0;

	self->ai->blocked_timeout = level.time + 15000;

	self->ai->shortRangeGoalTimeout = level.time + AI_SHORT_RANGE_GOAL_DELAY;
	self->movetarget = NULL;

	self->ai->aiRef->ClearGoal();
}

//==========================================
// AI_PickLongRangeGoal
//
// Evaluate the best long range goal and send the bot on
// its way. This is a good time waster, so use it sparingly.
// Do not call it for every think cycle.
//
// jal: I don't think there is any problem by calling it,
// now that we have stored the costs at the nav.costs table (I don't do it anyway)
//==========================================
void Ai::PickLongRangeGoal()
{
#define WEIGHT_MAXDISTANCE_FACTOR 20000.0f
#define COST_INFLUENCE	0.5f
	int i;
	float weight, bestWeight = 0.0;
	int current_node;
	float cost;
	float dist;
	nav_ents_t *goalEnt, *bestGoalEnt = NULL;

	self->ai->aiRef->ClearGoal();

	if( G_ISGHOSTING( self ) )
		return;

	if( self->ai->longRangeGoalTimeout > level.time )
		return;

	if( !self->r.client->ps.pmove.stats[PM_STAT_MAXSPEED] ) {
		return;
	}

	self->ai->longRangeGoalTimeout = level.time + AI_LONG_RANGE_GOAL_DELAY + brandom( 0, 1000 );

	// look for a target
	current_node = Ai::FindClosestReachableNode( self->s.origin, self, ( ( 1 + self->ai->nearest_node_tries ) * NODE_DENSITY ), NODE_ALL );
	self->ai->current_node = current_node;

	if( current_node == NODE_INVALID )
	{
		if( nav.debugMode && bot_showlrgoal->integer )
			G_PrintChasersf( self, "%s: LRGOAL: Closest node not found. Tries:%i\n", self->ai->pers.netname, self->ai->nearest_node_tries );

		self->ai->nearest_node_tries++; // extend search radius with each try
		return;
	}

	self->ai->nearest_node_tries = 0;

	// Run the list of potential goal entities
	FOREACH_GOALENT( goalEnt )
	{
		i = goalEnt->id;
		if( !goalEnt->ent )
			continue;

		if( !goalEnt->ent->r.inuse )
		{
			goalEnt->node = NODE_INVALID;
			continue;
		}

		if( goalEnt->ent->r.client )
		{
			if( G_ISGHOSTING( goalEnt->ent ) || ( goalEnt->ent->flags & FL_NOTARGET ) || ( ( goalEnt->ent->flags & FL_BUSY ) && ( level.gametype.forceTeamHumans == level.gametype.forceTeamBots ) ) )
				goalEnt->node = NODE_INVALID;
			else
				goalEnt->node = Ai::FindClosestReachableNode( goalEnt->ent->s.origin, goalEnt->ent, NODE_DENSITY, NODE_ALL );
		}

		if( goalEnt->ent->item )
		{
			if( !G_Gametype_CanPickUpItem( goalEnt->ent->item ) )
				continue;
		}

		if( goalEnt->node == NODE_INVALID )
			continue;

		weight = self->ai->status.entityWeights[i];

		if( weight <= 0.0f )
			continue;

		// don't try to find cost for too far away objects
		dist = DistanceFast( self->s.origin, goalEnt->ent->s.origin );
		if( dist > WEIGHT_MAXDISTANCE_FACTOR * weight/* || dist < AI_GOAL_SR_RADIUS*/ )
			continue;

		cost = Ai::FindCost( current_node, goalEnt->node, self->ai->status.moveTypesMask );
		if( cost == NODE_INVALID )
			continue;

		cost -= brandom( 0, 2000 ); // allow random variations
		clamp_low( cost, 1 );
		weight = ( 1000 * weight ) / ( cost * COST_INFLUENCE ); // Check against cost of getting there

		if( weight > bestWeight )
		{
			bestWeight = weight;
			bestGoalEnt = goalEnt;
		}
	}

	if( bestGoalEnt )
	{
		self->ai->goalEnt = bestGoalEnt;
		self->ai->aiRef->SetGoal( bestGoalEnt->node );

		if( self->ai->goalEnt != NULL && nav.debugMode && bot_showlrgoal->integer )
			G_PrintChasersf( self, "%s: selected a %s at node %d for LR goal. (weight %f)\n", self->ai->pers.netname, self->ai->goalEnt->ent->classname, self->ai->goalEnt->node, bestWeight );

		return;
	}

	if( nav.debugMode && bot_showlrgoal->integer )
		G_PrintChasersf( self, "%s: did not find a LR goal.\n", self->ai->pers.netname );

#undef WEIGHT_MAXDISTANCE_FACTOR
#undef COST_INFLUENCE
}

//==========================================
// AI_PickShortRangeGoal
// Pick best goal based on importance and range. This function
// overrides the long range goal selection for items that
// are very close to the bot and are reachable.
//==========================================
void Ai::PickShortRangeGoal()
{
	edict_t *bestGoal = NULL;
	float bestWeight = 0;
	nav_ents_t *goalEnt;
	const gsitem_t *item;
	bool canPickupItems;
	int i;

	if( !self->r.client || G_ISGHOSTING( self ) )
		return;

	if( self->ai->state_combat_timeout > level.time )
	{
		self->ai->shortRangeGoalTimeout = self->ai->state_combat_timeout;
		return;
	}

	if( self->ai->shortRangeGoalTimeout > level.time )
		return;

	canPickupItems = (self->r.client->ps.pmove.stats[PM_STAT_FEATURES] & PMFEAT_ITEMPICK) != 0 ? true : false;

	self->ai->shortRangeGoalTimeout = level.time + AI_SHORT_RANGE_GOAL_DELAY;

	self->movetarget = NULL;

	FOREACH_GOALENT( goalEnt )
	{
		float dist;

		i = goalEnt->id;
		if( !goalEnt->ent->r.inuse || goalEnt->ent->r.solid == SOLID_NOT )
			continue;

		if( goalEnt->ent->r.client )
			continue;

		if( self->ai->status.entityWeights[i] <= 0.0f )
			continue;

		item = goalEnt->ent->item;
		if( canPickupItems && item ) {
			if( !G_Gametype_CanPickUpItem( item ) || !( item->flags & ITFLAG_PICKABLE ) ) {
				continue;
			}
		}

		dist = DistanceFast( self->s.origin, goalEnt->ent->s.origin );
		if( goalEnt == self->ai->goalEnt ) {
			if( dist > AI_GOAL_SR_LR_RADIUS )
				continue;			
		}
		else {
			if( dist > AI_GOAL_SR_RADIUS )
				continue;
		}		

		clamp_low( dist, 0.01f );

		if( ShortRangeReachable( goalEnt->ent->s.origin ) )
		{
			float weight;
			bool in_front = G_InFront( self, goalEnt->ent );

			// Long range goal gets top priority
			if( in_front && goalEnt == self->ai->goalEnt ) 
			{
				bestGoal = goalEnt->ent;
				break;
			}

			// get the one with the best weight
			weight = self->ai->status.entityWeights[i] / dist * (in_front ? 1.0f : 0.5f);
			if( weight > bestWeight )
			{
				bestWeight = weight;
				bestGoal = goalEnt->ent;
			}
		}
	}

	if( bestGoal )
	{
		self->movetarget = bestGoal;
		if( nav.debugMode && bot_showsrgoal->integer )
			G_PrintChasersf( self, "%i %s: selected a %s for SR goal.\n", level.framenum, self->ai->pers.netname, self->movetarget->classname );
	}
	else
	{
		// got nothing else to do so keep scanning
		self->ai->shortRangeGoalTimeout = level.time + AI_SHORT_RANGE_GOAL_DELAY_IDLE;
	}
}

//===================
//  AI_CategorizePosition
//  Categorize waterlevel and groundentity/stepping
//===================
void Ai::CategorizePosition( edict_t *ent )
{
	bool stepping = Ai::IsStep(ent);

	ent->was_swim = ent->is_swim;
	ent->was_step = ent->is_step;

	ent->is_ladder = Ai::IsLadder( ent->s.origin, ent->s.angles, ent->r.mins, ent->r.maxs, ent );

	G_CategorizePosition( ent );
	if( ent->waterlevel > 2 || ( ent->waterlevel && !stepping ) )
	{
		ent->is_swim = true;
		ent->is_step = false;
		return;
	}

	ent->is_swim = false;
	ent->is_step = stepping;
}

void Ai::UpdateStatus()
{
	if( !G_ISGHOSTING( self ) )
	{
		AI_ResetWeights( self->ai );

		self->ai->status.moveTypesMask = self->ai->pers.moveTypesMask;

		if( !GT_asCallBotStatus( self ) )
			self->ai->pers.UpdateStatus( self );

		self->ai->statusUpdateTimeout = level.time + AI_STATUS_TIMEOUT;

		// no cheating with moveTypesMask
		self->ai->status.moveTypesMask &= self->ai->pers.moveTypesMask;
	}
}

void Ai::Think()
{
	if( level.spawnedTimeStamp + 5000 > game.realtime || !level.canSpawnEntities )
	{
		self->nextThink = level.time + game.snapFrameTime;
		return;
	}

	// check for being blocked
	if( !G_ISGHOSTING( self ) )
	{
		Ai::CategorizePosition( self );

		if( VectorLengthFast( self->velocity ) > 37 )
			self->ai->blocked_timeout = level.time + 10000;

		// if completely stuck somewhere
		if( self->ai->blocked_timeout < level.time )
		{
			self->ai->pers.blockedTimeout( self );
			return;
		}
	}

	//update status information to feed up ai
	if( self->ai->statusUpdateTimeout <= level.time )
		UpdateStatus();

	if (NodeHasTimedOut())
		ClearGoal();

	if( self->ai->goal_node == NODE_INVALID )
		PickLongRangeGoal();

	//if( self == level.think_client_entity )
	PickShortRangeGoal();

	self->ai->pers.RunFrame( self );

	// Show the path
	if( nav.debugMode && bot_showpath->integer && self->ai->goal_node != NODE_INVALID )
	{
		// only draw the path of those bots which are being chased
		edict_t *chaser;
		bool chaserFound = false;

		for( chaser = game.edicts + 1; ENTNUM( chaser ) < gs.maxclients; chaser++ )
		{
			if( chaser->r.client->resp.chase.active && chaser->r.client->resp.chase.target == ENTNUM( self ) )
			{
				AITools_DrawPath( self, self->ai->goal_node );
				chaserFound = true;
			}
		}

		if( !chaserFound && game.numBots == 1 )
			AITools_DrawPath( self, self->ai->goal_node );
	}
}

