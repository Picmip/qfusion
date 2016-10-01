#include "ai_local.h"

AiGametypeBrain AiGametypeBrain::instance;

void AiGametypeBrain::ClearGoals(const NavEntity *canceledGoal, const Ai *goalGrabber)
{
    if (!canceledGoal)
        return;

    // find all bots which have this node as goal and tell them their goal is reached
    for (edict_t *ent = game.edicts + 1; PLAYERNUM(ent) < gs.maxclients; ent++)
    {
        if (!ent->ai || ent->ai->type == AI_INACTIVE)
            continue;
        if (ent->ai->aiRef == goalGrabber)
            continue;

        if (ent->ai->aiRef->aiBaseBrain->longTermGoal == canceledGoal)
            ent->ai->aiRef->aiBaseBrain->ClearLongAndShortTermGoal(canceledGoal);
        else if (ent->ai->aiRef->aiBaseBrain->shortTermGoal == canceledGoal)
            ent->ai->aiRef->aiBaseBrain->ClearLongAndShortTermGoal(canceledGoal);
    }
}

void AiGametypeBrain::Frame()
{
    if (!GS_TeamBasedGametype())
    {
        AiBaseTeamBrain::GetBrainForTeam(TEAM_PLAYERS)->Update();
        return;
    }

    for (int team = TEAM_ALPHA; team < GS_MAX_TEAMS; ++team)
    {
        AiBaseTeamBrain::GetBrainForTeam(team)->Update();
    }
}
