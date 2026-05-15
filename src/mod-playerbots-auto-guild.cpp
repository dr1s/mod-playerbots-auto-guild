/*
 * mod-playerbots-auto-guild.cpp
 *
 * Automatically assigns playerbots to pre-existing faction guilds.
 *
 * On bot login and via periodic scan:
 *   - Detects if player is a bot
 *   - Resolves target guild by ID or name from config
 *   - If bot has no guild: adds to faction guild
 *   - If ForceAssign is enabled and bot is in a different guild: removes from current guild, adds to faction guild
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Config.h"
#include "Log.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"

static uint32 g_HordeGuildId    = 0;
static uint32 g_AllianceGuildId = 0;
static bool   g_ForceAssign     = false;
static uint32 g_CheckInterval   = 60;

static bool IsPlayerBot(Player* player)
{
    if (!player)
        return false;
    PlayerbotAI* ai = sPlayerbotsMgr.GetPlayerbotAI(player);
    return ai && ai->IsBotAI();
}

static void ResolveGuilds()
{
    g_HordeGuildId    = sConfigMgr->GetOption<uint32>("PlayerbotsAutoGuild.HordeGuildId", 0);
    g_AllianceGuildId = sConfigMgr->GetOption<uint32>("PlayerbotsAutoGuild.AllianceGuildId", 0);
    g_ForceAssign     = sConfigMgr->GetOption<bool>  ("PlayerbotsAutoGuild.ForceAssign", false);
    g_CheckInterval   = sConfigMgr->GetOption<uint32>("PlayerbotsAutoGuild.CheckIntervalSeconds", 60);

    if (g_HordeGuildId == 0)
    {
        std::string name = sConfigMgr->GetOption<std::string>("PlayerbotsAutoGuild.HordeGuildName", "");
        if (!name.empty())
        {
            if (Guild* g = sGuildMgr->GetGuildByName(name))
            {
                g_HordeGuildId = g->GetId();
                LOG_INFO("server.loading", "[PlayerbotsAutoGuild] Resolved Horde guild by name: '{}' (ID: {})", name, g_HordeGuildId);
            }
            else
            {
                LOG_WARN("server.loading", "[PlayerbotsAutoGuild] Horde guild '{}' not found", name);
            }
        }
    }

    if (g_AllianceGuildId == 0)
    {
        std::string name = sConfigMgr->GetOption<std::string>("PlayerbotsAutoGuild.AllianceGuildName", "");
        if (!name.empty())
        {
            if (Guild* g = sGuildMgr->GetGuildByName(name))
            {
                g_AllianceGuildId = g->GetId();
                LOG_INFO("server.loading", "[PlayerbotsAutoGuild] Resolved Alliance guild by name: '{}' (ID: {})", name, g_AllianceGuildId);
            }
            else
            {
                LOG_WARN("server.loading", "[PlayerbotsAutoGuild] Alliance guild '{}' not found", name);
            }
        }
    }

    if (g_HordeGuildId)
        LOG_INFO("server.loading", "[PlayerbotsAutoGuild] Horde guild ID: {}", g_HordeGuildId);
    else
        LOG_WARN("server.loading", "[PlayerbotsAutoGuild] No Horde guild configured");

    if (g_AllianceGuildId)
        LOG_INFO("server.loading", "[PlayerbotsAutoGuild] Alliance guild ID: {}", g_AllianceGuildId);
    else
        LOG_WARN("server.loading", "[PlayerbotsAutoGuild] No Alliance guild configured");
}

static uint32 GetTargetGuildId(TeamId teamId)
{
    return teamId == TEAM_HORDE ? g_HordeGuildId : g_AllianceGuildId;
}

static void EnsureBotInGuild(Player* bot)
{
    if (!bot || !bot->IsInWorld())
        return;

    uint32 targetGuildId = GetTargetGuildId(bot->GetTeamId());
    if (targetGuildId == 0)
        return;

    uint32 currentGuildId = bot->GetGuildId();

    if (currentGuildId == targetGuildId)
        return;

    Guild* targetGuild = sGuildMgr->GetGuildById(targetGuildId);
    if (!targetGuild)
        return;

    if (currentGuildId != 0)
    {
        if (!g_ForceAssign)
            return;

        if (Guild* currentGuild = sGuildMgr->GetGuildById(currentGuildId))
        {
            currentGuild->DeleteMember(bot->GetGUID(), false, false);
            LOG_INFO("module", "[PlayerbotsAutoGuild] Removed bot {} from guild {} (ID: {})", bot->GetName(), currentGuild->GetName(), currentGuildId);
        }
    }

    if (targetGuild->AddMember(bot->GetGUID()))
        LOG_INFO("module", "[PlayerbotsAutoGuild] Added bot {} to {} guild '{}' (ID: {})", bot->GetName(),
                 bot->GetTeamId() == TEAM_HORDE ? "Horde" : "Alliance", targetGuild->GetName(), targetGuildId);
}

class PlayerbotsAutoGuildPlayerScript : public PlayerScript
{
public:
    PlayerbotsAutoGuildPlayerScript() : PlayerScript("PlayerbotsAutoGuildPlayerScript") {}

    void OnPlayerLogin(Player* player) override
    {
        if (IsPlayerBot(player))
            EnsureBotInGuild(player);
    }
};

class PlayerbotsAutoGuildWorldScript : public WorldScript
{
public:
    PlayerbotsAutoGuildWorldScript() : WorldScript("PlayerbotsAutoGuildWorldScript"), m_timer(0) {}

    void OnStartup() override
    {
        ResolveGuilds();
    }

    void OnUpdate(uint32 diff) override
    {
        m_timer += diff;
        if (m_timer < g_CheckInterval * 1000)
            return;
        m_timer = 0;

        for (auto const& entry : ObjectAccessor::GetPlayers())
        {
            Player* player = entry.second;
            if (player && IsPlayerBot(player))
                EnsureBotInGuild(player);
        }
    }

private:
    uint32 m_timer;
};

void Addmod_playerbots_auto_guildScripts()
{
    new PlayerbotsAutoGuildPlayerScript();
    new PlayerbotsAutoGuildWorldScript();
}
