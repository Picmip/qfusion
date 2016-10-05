#include "bot_items_selector.h"
#include "bot.h"

void BotItemsSelector::UpdateInternalItemsWeights()
{
    // Compute it once, not on each loop step
    bool onlyGotGB = true;
    for (int weapon = WEAP_GUNBLADE + 1; weapon < WEAP_TOTAL; ++weapon)
    {
        if (Inventory()[weapon])
        {
            onlyGotGB = false;
            break;
        }
    }

    // Weights are set to zero by caller code.
    // Only non-zero weights should be set.
    FOREACH_NAVENT(goalEnt)
    {
        // Picking clients as goal entities is currently disabled
        if (goalEnt->IsClient())
            continue;

        if (goalEnt->Item())
            internalEntityWeights[goalEnt->Id()] = ComputeItemWeight(goalEnt->Item(), onlyGotGB);
    }
}

float BotItemsSelector::ComputeItemWeight(const gsitem_t *item, bool onlyGotGB) const
{
    switch (item->type)
    {
        case IT_WEAPON: return ComputeWeaponWeight(item, onlyGotGB);
        case IT_AMMO: return ComputeAmmoWeight(item);
        case IT_HEALTH: return ComputeHealthWeight(item);
        case IT_ARMOR: return ComputeArmorWeight(item);
        case IT_POWERUP: return ComputePowerupWeight(item);
    }
    return 0;
}

float BotItemsSelector::ComputeWeaponWeight(const gsitem_t *item, bool onlyGotGB) const
{
    if (Inventory()[item->tag])
    {
        // TODO: Precache
        const gsitem_t *ammo = GS_FindItemByTag(item->ammo_tag);
        if (Inventory()[ammo->tag] >= ammo->inventory_max)
            return 0;

        float ammoQuantityFactor = 1.0f - Inventory()[ammo->tag] / (float)ammo->inventory_max;

        switch (item->tag)
        {
            case WEAP_ELECTROBOLT: return ammoQuantityFactor;
            case WEAP_LASERGUN: return ammoQuantityFactor * 1.1f;
            case WEAP_PLASMAGUN: return ammoQuantityFactor * 1.1f;
            case WEAP_ROCKETLAUNCHER: return ammoQuantityFactor;
            default: return 0.5f * ammoQuantityFactor;
        }
    }

    // We may consider plasmagun in a bot's hand as a top tier weapon too
    const int topTierWeapons[4] = { WEAP_ELECTROBOLT, WEAP_LASERGUN, WEAP_ROCKETLAUNCHER, WEAP_PLASMAGUN };

    // TODO: Precompute
    float topTierWeaponGreed = 0.0f;
    for (int i = 0; i < 4; ++i)
    {
        if (!Inventory()[topTierWeapons[i]])
            topTierWeaponGreed += 1.0f;
    }

    for (int i = 0; i < 4; ++i)
    {
        if (topTierWeapons[i] == item->tag)
            return (onlyGotGB ? 1.5f : 0.9f) + (topTierWeaponGreed - 1.0f) / 3.0f;
    }

    return onlyGotGB ? 1.5f : 0.7f;
}

float BotItemsSelector::ComputeAmmoWeight(const gsitem_t *item) const
{
    if (Inventory()[item->tag] < item->inventory_max)
    {
        float quantityFactor = 1.0f - Inventory()[item->tag] / (float)item->inventory_max;

        for (int weapon = WEAP_GUNBLADE; weapon < WEAP_TOTAL; weapon++)
        {
            // TODO: Preache
            const gsitem_t *weaponItem = GS_FindItemByTag( weapon );
            if (weaponItem->ammo_tag == item->tag)
            {
                if (Inventory()[weaponItem->tag])
                {
                    switch (weaponItem->tag)
                    {
                        case WEAP_ELECTROBOLT: return quantityFactor;
                        case WEAP_LASERGUN: return quantityFactor * 1.1f;
                        case WEAP_PLASMAGUN: return quantityFactor * 1.1f;
                        case WEAP_ROCKETLAUNCHER: return quantityFactor;
                        default: return 0.5f * quantityFactor;
                    }
                }
                return quantityFactor * 0.33f;
            }
        }
    }
    return 0.0;
}

float BotItemsSelector::ComputeHealthWeight(const gsitem_t *item) const
{
    if (item->tag == HEALTH_MEGA || item->tag == HEALTH_ULTRA)
        return 2.5f;

    if (item->tag == HEALTH_SMALL)
        return 0.2f + 0.3f * (1.0f - self->health / (float)self->max_health);

    return std::max(0.0f, 1.0f - self->health / (float)self->max_health);
}

float BotItemsSelector::ComputeArmorWeight(const gsitem_t *item) const
{
    float currArmor = self->r.client->resp.armor;
    switch (item->tag)
    {
        case ARMOR_RA:
            return currArmor < 150.0f ? 2.0f : 0.0f;
        case ARMOR_YA:
            return currArmor < 125.0f ? 1.7f : 0.0f;
        case ARMOR_GA:
            return currArmor < 100.0f ? 1.4f : 0.0f;
        case ARMOR_SHARD:
        {
            if (currArmor < 25 || currArmor >= 150)
                return 0.4f;
            return 0.25f;
        }
    }
    return 0;
}

float BotItemsSelector::ComputePowerupWeight(const gsitem_t *item) const
{
    return 3.5f;
}

constexpr float MOVE_TIME_WEIGHT = 1.0f;
constexpr float WAIT_TIME_WEIGHT = 3.5f;

struct NavEntityAndWeight
{
    const NavEntity *goal;
    float weight;
    inline NavEntityAndWeight(const NavEntity *goal_, float weight_): goal(goal_), weight(weight_) {}
    // For sorting in descending by weight order operator < is negated
    inline bool operator<(const NavEntityAndWeight &that) const { return weight > that.weight; }
};

const NavEntity *BotItemsSelector::SuggestGoalNavEntity(const NavEntity *currGoalNavEntity)
{
    StaticVector<NavEntityAndWeight, MAX_NAVENTS> rawWeightCandidates;
    FOREACH_NAVENT(navEnt)
    {
        if (navEnt->IsDisabled())
            continue;

        // Since movable goals have been introduced (and clients qualify as movable goals), prevent picking itself as a goal.
        if (navEnt->Id() == ENTNUM(self))
            continue;

        if (navEnt->Item() && !G_Gametype_CanPickUpItem(navEnt->Item()))
            continue;

        // Reject an entity quickly if it looks like blocked by an enemy that is close to the entity.
        // Note than passing this test does not guarantee that entire path to the entity is not blocked by enemies.
        if (self->ai->botRef->routeCache->AreaDisabled(navEnt->AasAreaNum()))
            continue;

        // This is a coarse and cheap test, helps to reject recently picked armors and powerups
        unsigned spawnTime = navEnt->SpawnTime();
        // A feasible spawn time (non-zero) always >= level.time.
        if (!spawnTime || level.time - spawnTime > 15000)
            continue;

        float weight = GetEntityWeight(navEnt->Id());
        if (weight > 0)
            rawWeightCandidates.push_back(NavEntityAndWeight(navEnt, weight));
    }

    // Sort all pre-selected candidates by their raw weights
    std::sort(rawWeightCandidates.begin(), rawWeightCandidates.end());

    float currGoalEntWeight = 0.0f;
    const NavEntity *bestNavEnt = nullptr;
    float bestWeight = 0.000001f;
    // Test not more than 16 best pre-selected by raw weight candidates.
    // (We try to avoid too many expensive FindTravelTimeToGoalArea() calls,
    // thats why we start from the best item to avoid wasting these calls for low-priority items)
    for (unsigned i = 0, end = std::min(rawWeightCandidates.size(), 16U); i < end; ++i)
    {
        const NavEntity *navEnt = rawWeightCandidates[i].goal;
        float weight = rawWeightCandidates[i].weight;

        unsigned moveDuration = 1;
        unsigned waitDuration = 1;

        if (self->ai->botRef->currAasAreaNum != navEnt->AasAreaNum())
        {
            // We ignore cost of traveling in goal area, since:
            // 1) to estimate it we have to retrieve reachability to goal area from last area before the goal area
            // 2) it is relative low compared to overall travel cost, and movement in areas is cheap anyway
            moveDuration = self->ai->botRef->botBrain.FindTravelTimeToGoalArea(navEnt->AasAreaNum()) * 10U;
            // AAS functions return 0 as a "none" value, 1 as a lowest feasible value
            if (!moveDuration)
                continue;

            if (navEnt->IsDroppedEntity())
            {
                // Do not pick an entity that is likely to dispose before it may be reached
                if (navEnt->Timeout() <= level.time + moveDuration)
                    continue;
            }
        }

        unsigned spawnTime = navEnt->SpawnTime();
        // The entity is not spawned and respawn time is unknown
        if (!spawnTime)
            continue;

        // Entity origin may be reached at this time
        unsigned reachTime = level.time + moveDuration;
        if (reachTime < spawnTime)
            waitDuration = spawnTime - reachTime;

        if (waitDuration > navEnt->MaxWaitDuration())
            continue;

        float cost = 0.0001f + MOVE_TIME_WEIGHT * moveDuration + WAIT_TIME_WEIGHT * waitDuration;

        weight = (1000 * weight) / (cost * navEnt->CostInfluence()); // Check against cost of getting there

        // Store current weight of the current goal entity
        if (currGoalNavEntity == navEnt)
            currGoalEntWeight = weight;

        if (weight > bestWeight)
        {
            bestNavEnt = navEnt;
            bestWeight = weight;
        }
    }

    if (!bestNavEnt)
    {
        Debug("Can't find a feasible long-term goal nav. entity\n");
        return nullptr;
    }

    // If it is time to pick a new goal (not just re-evaluate current one), do not be too sticky to the current goal
    const float currToBestWeightThreshold = currGoalNavEntity != nullptr ? 0.6f : 0.8f;

    if (currGoalNavEntity && currGoalNavEntity == bestNavEnt)
    {
        constexpr const char *format = "current goal entity %s is kept as still having best weight %.3f\n";
        Debug(format, currGoalNavEntity->Name(), bestWeight);
        return currGoalNavEntity;
    }
    else if (currGoalEntWeight > 0 && currGoalEntWeight / bestWeight > currToBestWeightThreshold)
    {
        constexpr const char *format =
            "current goal entity %s is kept as having weight %.3f good enough to not consider picking another one\n";
        // If currGoalEntWeight > 0, currLongTermGoalEnt is guaranteed to be non-null
        Debug(format, currGoalNavEntity->Name(), currGoalEntWeight);
        return currGoalNavEntity;
    }
    else
    {
        if (currGoalNavEntity)
        {
            const char *format = "suggested %s weighted %.3f as a long-term goal instead of %s weighted now as %.3f\n";
            Debug(format, bestNavEnt->Name(), bestWeight, currGoalNavEntity->Name(), currGoalEntWeight);
        }
        else
        {
            Debug("suggested %s weighted %.3f as a new long-term goal\n", bestNavEnt->Name(), bestWeight);
        }
        return bestNavEnt;
    }
}