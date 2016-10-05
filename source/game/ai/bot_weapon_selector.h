#ifndef QFUSION_BOT_WEAPON_SELECTOR_H
#define QFUSION_BOT_WEAPON_SELECTOR_H

#include "ai_base_enemy_pool.h"

class WorldState;

class GenericFireDef
{
    // Allow SelectedWeapons to use the default constructor
    friend class SelectedWeapons;

    float projectileSpeed;
    float splashRadius;
    ai_weapon_aim_type aimType;
    short weaponNum;
    bool isBuiltin;

    GenericFireDef() {}
public:
    GenericFireDef(int weaponNum_, const firedef_t *builtinFireDef)
    {
        this->projectileSpeed = builtinFireDef->speed;
        this->splashRadius = builtinFireDef->splash_radius;
        this->aimType = BuiltinWeaponAimType(weaponNum_);
        this->weaponNum = (short)weaponNum_;
        this->isBuiltin = true;
    }

    GenericFireDef(int weaponNum_, const AiScriptWeaponDef *scriptWeaponDef)
    {
        this->projectileSpeed = scriptWeaponDef->projectileSpeed;
        this->splashRadius = scriptWeaponDef->splashRadius;
        this->aimType = scriptWeaponDef->aimType;
        this->weaponNum = (short)weaponNum_;
        this->isBuiltin = false;
    }

    inline int WeaponNum() const { return weaponNum; }
    inline bool IsBuiltin() const { return isBuiltin; }

    inline ai_weapon_aim_type AimType() const { return aimType; }
    inline float ProjectileSpeed() const { return projectileSpeed; }
    inline float SplashRadius() const { return splashRadius; }
    inline bool IsContinuousFire() const { return isBuiltin; }
};

class SelectedWeapons
{
    friend class BotWeaponSelector;
    friend class Bot;

    GenericFireDef builtinFireDef;
    GenericFireDef scriptFireDef;

    unsigned instanceId;
    unsigned timeoutAt;

    bool preferBuiltinWeapon;
    bool hasSelectedBuiltinWeapon;
    bool hasSelectedScriptWeapon;

    SelectedWeapons()
        : instanceId(0),
          timeoutAt(0),
          preferBuiltinWeapon(true),
          hasSelectedBuiltinWeapon(false),
          hasSelectedScriptWeapon(false) {}
public:
    inline const GenericFireDef *BuiltinFireDef() const
    {
        return hasSelectedBuiltinWeapon ? &builtinFireDef : nullptr;
    }
    inline const GenericFireDef *ScriptFireDef() const
    {
        return hasSelectedScriptWeapon ? &scriptFireDef : nullptr;
    }
    inline int BuiltinWeaponNum() const
    {
        return hasSelectedBuiltinWeapon ? builtinFireDef.WeaponNum() : -1;
    }
    inline int ScriptWeaponNum() const
    {
        return hasSelectedScriptWeapon ? scriptFireDef.WeaponNum() : -1;
    }
    inline unsigned InstanceId() const { return instanceId; }
    inline bool AreValid() const { return timeoutAt > level.time; }
    inline void Invalidate() { timeoutAt = level.time; }
    inline unsigned TimeoutAt() const { return timeoutAt; }
    inline bool PreferBuiltinWeapon() const { return preferBuiltinWeapon; }
};

class SelectedEnemies
{
    friend class BotBrain;

    const Enemy *primaryEnemy;
    StaticVector<const Enemy *, AiBaseEnemyPool::MAX_ACTIVE_ENEMIES> activeEnemies;

    unsigned instanceId;
    unsigned timeoutAt;

    // TODO: Add secondary enemies

    inline void CheckValid(const char *function) const
    {
        for (const Enemy *enemy: activeEnemies)
            if (!enemy->IsValid())
                AI_FailWith(function, "An enemy is no longer valid");
    }

public:
    inline bool AreValid() const
    {
        return primaryEnemy && primaryEnemy->IsValid() && timeoutAt > level.time;
    }

    inline void Invalidate()
    {
        timeoutAt = level.time;
    }

    inline unsigned InstanceId() const { return instanceId; }

    bool IsPrimaryEnemy(const edict_t *ent) const
    {
        return primaryEnemy && primaryEnemy->ent == ent;
    }

    bool IsPrimaryEnemy(const Enemy *enemy) const
    {
        return primaryEnemy && primaryEnemy == enemy;
    }

    Vec3 LastSeenOrigin() const
    {
        CheckValid(__FUNCTION__);
        return primaryEnemy->LastSeenPosition();
    }

    Vec3 ActualOrigin() const
    {
        CheckValid(__FUNCTION__);
        return Vec3(primaryEnemy->ent->s.origin);
    }

    Vec3 LastSeenVelocity() const
    {
        CheckValid(__FUNCTION__);
        return primaryEnemy->LastSeenVelocity();
    }

    unsigned LastSeenAt() const
    {
        CheckValid(__FUNCTION__);
        return primaryEnemy->LastSeenAt();
    }

    Vec3 ActualVelocity() const
    {
        CheckValid(__FUNCTION__);
        return Vec3(primaryEnemy->ent->velocity);
    }

    Vec3 Mins() const
    {
        CheckValid(__FUNCTION__);
        return Vec3(primaryEnemy->ent->r.mins);
    }

    Vec3 Maxs() const
    {
        CheckValid(__FUNCTION__);
        return Vec3(primaryEnemy->ent->r.maxs);
    }

    Vec3 LookDir() const
    {
        CheckValid(__FUNCTION__);
        vec3_t lookDir;
        AngleVectors(primaryEnemy->ent->s.angles, lookDir, nullptr, nullptr);
        return Vec3(lookDir);
    }

    Vec3 EnemyAngles() const
    {
        CheckValid(__FUNCTION__);
        return Vec3(primaryEnemy->ent->s.angles);
    }

    int PendingWeapon() const
    {
        if (primaryEnemy && primaryEnemy->ent && primaryEnemy->ent->r.client)
            return primaryEnemy->ent->r.client->ps.stats[STAT_PENDING_WEAPON];
        return -1;
    }

    unsigned FireDelay() const
    {
        if (primaryEnemy && primaryEnemy->ent)
        {
            if (!primaryEnemy->ent->r.client)
                return 0;
            return (unsigned)primaryEnemy->ent->r.client->ps.stats[STAT_WEAPON_TIME];
        }
        return std::numeric_limits<unsigned>::max();
    }

    inline bool IsStaticSpot() const
    {
        return Ent()->r.client != nullptr;
    }

    inline const edict_t *Ent() const
    {
        CheckValid(__FUNCTION__);
        return primaryEnemy->ent;
    }

    inline const edict_t *TraceKey() const
    {
        CheckValid(__FUNCTION__);
        return primaryEnemy->ent;
    }

    inline const bool OnGround() const
    {
        CheckValid(__FUNCTION__);
        return primaryEnemy->ent->groundentity != nullptr;
    }

    inline bool HaveQuad() const
    {
        CheckValid(__FUNCTION__);
        for (const Enemy *enemy: activeEnemies)
            if (enemy->HasQuad())
                return true;
        return false;
    }

    inline bool HaveCarrier() const
    {
        CheckValid(__FUNCTION__);
        for (const Enemy *enemy: activeEnemies)
            if (enemy->IsCarrier())
                return true;
        return false;
    }

    inline bool Contain(const Enemy *enemy)
    {
        CheckValid(__FUNCTION__);
        for (const Enemy *activeEnemy: activeEnemies)
            if (activeEnemy == enemy)
                return true;
        return false;
    }

    inline const Enemy **begin() const { return (const Enemy **)activeEnemies.cbegin(); }
    inline const Enemy **end() const { return (const Enemy **)activeEnemies.cend(); }
};

class BotWeaponSelector
{
    edict_t *self;

    SelectedWeapons &selectedWeapons;
    const SelectedEnemies &selectedEnemies;

    float weaponChoiceRandom;
    unsigned weaponChoiceRandomTimeoutAt;

    unsigned nextFastWeaponSwitchActionCheckAt;
    const unsigned weaponChoicePeriod;
public:
    BotWeaponSelector(edict_t *self_);

    void Frame(const WorldState &recentWorldState);
    void Think(const WorldState &currentWorldState);
private:
    void Debug(const char *format, ...);

    inline bool BotHasQuad() const { return ::HasQuad(self); }
    inline bool BotHasShell() const { return ::HasShell(self); }
    inline bool BotHasPowerups() const { return ::HasPowerups(self); }
    inline bool BotIsCarrier() const { return ::IsCarrier(self); }

    inline float DamageToKill(const edict_t *client) const
    {
        return ::DamageToKill(client, g_armor_protection->value, g_armor_degradation->value);
    }

    inline const int *Inventory() const { return self->r.client->ps.inventory; }

    template <int Weapon> inline int AmmoReadyToFireCount() const
    {
        if (!Inventory()[Weapon])
            return 0;
        return Inventory()[WeaponAmmo<Weapon>::strongAmmoTag] + Inventory()[WeaponAmmo<Weapon>::weakAmmoTag];
    }

    inline int ShellsReadyToFireCount() const { return AmmoReadyToFireCount<WEAP_RIOTGUN>(); }
    inline int GrenadesReadyToFireCount() const { return AmmoReadyToFireCount<WEAP_GRENADELAUNCHER>(); }
    inline int RocketsReadyToFireCount() const { return AmmoReadyToFireCount<WEAP_ROCKETLAUNCHER>(); }
    inline int PlasmasReadyToFireCount() const { return AmmoReadyToFireCount<WEAP_PLASMAGUN>(); }
    inline int BulletsReadyToFireCount() const { return AmmoReadyToFireCount<WEAP_MACHINEGUN>(); }
    inline int LasersReadyToFireCount() const { return AmmoReadyToFireCount<WEAP_LASERGUN>(); }
    inline int BoltsReadyToFireCount() const { return AmmoReadyToFireCount<WEAP_ELECTROBOLT>(); }

    bool CheckFastWeaponSwitchAction(const WorldState &recentWorldState);

    void SuggestAimWeapon(const WorldState &currWorldState);
    void SuggestSniperRangeWeapon(const WorldState &currWorldState);
    void SuggestFarRangeWeapon(const WorldState &currWorldState);
    void SuggestMiddleRangeWeapon(const WorldState &worldState);
    void SuggestCloseRangeWeapon(const WorldState &worldState);

    int SuggestInstagibWeapon(const WorldState &currWorldState);
    int SuggestFinishWeapon(const WorldState &currWorldState);

    const AiScriptWeaponDef *SuggestScriptWeapon(const WorldState &currWorldState, int *effectiveTier);
    bool IsEnemyEscaping(const WorldState &currWorldState, bool *botMovesFast, bool *enemyMovesFast);

    int SuggestHitEscapingEnemyWeapon(const WorldState &currWorldState, bool botMovesFast, bool enemyMovesFast);

    bool CheckForShotOfDespair(const WorldState &currWorldState);
    int SuggestShotOfDespairWeapon(const WorldState &currWorldState);
    int SuggestQuadBearerWeapon(const WorldState &currWorldState);

    int ChooseWeaponByScores(struct WeaponAndScore *begin, struct WeaponAndScore *end);

    struct TargetEnvironment
    {
        // Sides are relative to direction from bot origin to target origin
        // Order: top, bottom, front, back, left, right
        trace_t sideTraces[6];

        enum Side { TOP, BOTTOM, FRONT, BACK, LEFT, RIGHT };

        float factor;
        static const float TRACE_DEPTH;
    };

    TargetEnvironment targetEnvironment;
    void TestTargetEnvironment(const Vec3 &botOrigin, const Vec3 &targetOrigin, const edict_t *traceKey);

    void SetSelectedWeapons(int builtinWeapon, int scriptWeapon, bool preferBuiltinWeapon, unsigned timeoutPeriod);
};

#endif