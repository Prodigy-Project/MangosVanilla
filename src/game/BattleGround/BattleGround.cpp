/**
 * MaNGOS is a full featured server for World of Warcraft, supporting
 * the following clients: 1.12.x, 2.4.3, 3.3.5a, 4.3.4a and 5.4.8
 *
 * Copyright (C) 2005-2025 MaNGOS <https://www.getmangos.eu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * World of Warcraft, and all World of Warcraft or Warcraft art, images,
 * and lore are copyrighted by Blizzard Entertainment, Inc.
 */

#include "Object.h"
#include "Player.h"
#include "BattleGround.h"
#include "BattleGroundMgr.h"
#include "Creature.h"
#include "MapManager.h"
#include "Language.h"
#include "SpellAuras.h"
#include "World.h"
#include "Group.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Mail.h"
#include "WorldPacket.h"
#include "Formulas.h"
#include "GridNotifiersImpl.h"
#include "Chat.h"
#ifdef ENABLE_ELUNA
#include "LuaEngine.h"
#endif /* ENABLE_ELUNA */

namespace MaNGOS
{
    class BattleGroundChatBuilder
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="BattleGroundChatBuilder"/> class.
        /// </summary>
        /// <param name="msgtype">The msgtype.</param>
        /// <param name="textId">The text id.</param>
        /// <param name="source">The source.</param>
        /// <param name="args">The args.</param>
        BattleGroundChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, va_list* args = nullptr)
                : i_msgtype(msgtype), i_textId(textId), i_source(source), i_args(args) {}
        void operator()(WorldPacket& data, int32 loc_idx)
        {
            char const* text = sObjectMgr.GetMangosString(i_textId, loc_idx);

            ObjectGuid sourceGuid = i_source ? i_source->GetObjectGuid() : ObjectGuid();
            std::string sourceName = i_source ? i_source->GetName() : "";

            if (i_args)
            {
                // we need copy va_list before use or original va_list will corrupted
                va_list ap;
                va_copy(ap, *i_args);

                char str[2048];
                vsnprintf(str, 2048, text, ap);
                va_end(ap);

                ChatHandler::BuildChatPacket(data, i_msgtype, &str[0], LANG_UNIVERSAL, CHAT_TAG_NONE, sourceGuid, sourceName.c_str());
            }
            else
            {
                ChatHandler::BuildChatPacket(data, i_msgtype, text, LANG_UNIVERSAL, CHAT_TAG_NONE, sourceGuid, sourceName.c_str(), sourceGuid, sourceName.c_str());
            }
        }
    private:
        ChatMsg i_msgtype;
        int32 i_textId;
        Player const* i_source;
        va_list* i_args;
    };

    class BattleGroundYellBuilder
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="BattleGroundYellBuilder"/> class.
        /// </summary>
        /// <param name="language">The language.</param>
        /// <param name="textId">The text id.</param>
        /// <param name="source">The source.</param>
        /// <param name="args">The args.</param>
        BattleGroundYellBuilder(Language language, int32 textId, Creature const* source, va_list* args = NULL)
                : i_language(language), i_textId(textId), i_source(source), i_args(args) {}
        void operator()(WorldPacket& data, int32 loc_idx)
        {
            char const* text = sObjectMgr.GetMangosString(i_textId, loc_idx);

            if (i_args)
            {
                // we need copy va_list before use or original va_list will corrupted
                va_list ap;
                va_copy(ap, *i_args);

                char str[2048];
                vsnprintf(str, 2048, text, ap);
                va_end(ap);

                ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_YELL, &str[0], i_language, CHAT_TAG_NONE, i_source->GetObjectGuid(), i_source->GetName());
            }
            else
            {
                ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_YELL, text, i_language, CHAT_TAG_NONE, i_source->GetObjectGuid(), i_source->GetName());
            }
        }
    private:
        Language i_language;
        int32 i_textId;
        Creature const* i_source;
        va_list* i_args;
    };


    class BattleGround2ChatBuilder
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="BattleGround2ChatBuilder"/> class.
        /// </summary>
        /// <param name="msgtype">The msgtype.</param>
        /// <param name="textId">The text id.</param>
        /// <param name="source">The source.</param>
        /// <param name="arg1">The arg1.</param>
        /// <param name="arg2">The arg2.</param>
        BattleGround2ChatBuilder(ChatMsg msgtype, int32 textId, Player const* source, int32 arg1, int32 arg2)
                : i_msgtype(msgtype), i_textId(textId), i_source(source), i_arg1(arg1), i_arg2(arg2) {}
        void operator()(WorldPacket& data, int32 loc_idx)
        {
            char const* text = sObjectMgr.GetMangosString(i_textId, loc_idx);
            char const* arg1str = i_arg1 ? sObjectMgr.GetMangosString(i_arg1, loc_idx) : "";
            char const* arg2str = i_arg2 ? sObjectMgr.GetMangosString(i_arg2, loc_idx) : "";

            char str[2048];
            snprintf(str, 2048, text, arg1str, arg2str);

            ObjectGuid guid;
            if (i_source)
            {
                guid = i_source->GetObjectGuid();
            }
            ChatHandler::BuildChatPacket(data, i_msgtype, str, LANG_UNIVERSAL, CHAT_TAG_NONE, guid);
        }
    private:
        ChatMsg i_msgtype;
        int32 i_textId;
        Player const* i_source;
        int32 i_arg1;
        int32 i_arg2;
    };

    class BattleGround2YellBuilder
    {
    public:
        /// <summary>
        /// Initializes a new instance of the <see cref="BattleGround2YellBuilder"/> class.
        /// </summary>
        /// <param name="language">The language.</param>
        /// <param name="textId">The text id.</param>
        /// <param name="source">The source.</param>
        /// <param name="arg1">The arg1.</param>
        /// <param name="arg2">The arg2.</param>
        BattleGround2YellBuilder(uint32 language, int32 textId, Creature const* source, int32 arg1, int32 arg2)
                : i_language(language), i_textId(textId), i_source(source), i_arg1(arg1), i_arg2(arg2) {}
        void operator()(WorldPacket& data, int32 loc_idx)
        {
            char const* text = sObjectMgr.GetMangosString(i_textId, loc_idx);
            char const* arg1str = i_arg1 ? sObjectMgr.GetMangosString(i_arg1, loc_idx) : "";
            char const* arg2str = i_arg2 ? sObjectMgr.GetMangosString(i_arg2, loc_idx) : "";

            char str[2048];
            snprintf(str, 2048, text, arg1str, arg2str);

            ChatHandler::BuildChatPacket(data, CHAT_MSG_MONSTER_YELL, str, LANG_UNIVERSAL, CHAT_TAG_NONE, i_source ? i_source->GetObjectGuid() : ObjectGuid(), i_source ? i_source->GetName() : "");
        }
    private:

        uint32 i_language;
        int32 i_textId;
        Creature const* i_source;
        int32 i_arg1;
        int32 i_arg2;
    };
} // namespace MaNGOS

/**
 * @brief Broadcasts a worker function to all players in the battleground.
 *
 * @param _do The worker function.
 */
template<class Do>
void BattleGround::BroadcastWorker(Do& _do)
{
    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* plr = sObjectAccessor.FindPlayer(itr->first))
        {
            _do(plr);
        }
}

/**
 * @brief Constructor for BattleGround.
 */
BattleGround::BattleGround()
{
    m_TypeID = BattleGroundTypeId(0);
    m_Status = STATUS_NONE;
    m_ClientInstanceID = 0;
    m_EndTime = 0;
    m_BracketId = BG_BRACKET_ID_TEMPLATE;
    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_Winner = TEAM_NONE;
    m_StartTime = 0;
    m_Events = 0;
    m_Name = "";
    m_LevelMin = 0;
    m_LevelMax = 0;
    m_InBGFreeSlotQueue = false;
    m_BuffChange = false;
    m_MaxPlayersPerTeam = 0;
    m_MaxPlayers = 0;
    m_MinPlayersPerTeam = 0;
    m_MinPlayers = 0;
    m_StartDelayTime = 0;
    m_MapId = 0;
    m_Map = NULL;
    m_startMaxDist = 0;
    m_validStartPositionTimer = 0;

    m_TeamStartLocX[TEAM_INDEX_ALLIANCE]   = 0;
    m_TeamStartLocX[TEAM_INDEX_HORDE] = 0;

    m_TeamStartLocY[TEAM_INDEX_ALLIANCE] = 0;
    m_TeamStartLocY[TEAM_INDEX_HORDE] = 0;

    m_TeamStartLocZ[TEAM_INDEX_ALLIANCE] = 0;
    m_TeamStartLocZ[TEAM_INDEX_HORDE] = 0;

    m_TeamStartLocO[TEAM_INDEX_ALLIANCE] = 0;
    m_TeamStartLocO[TEAM_INDEX_HORDE] = 0;

    m_BgRaids[TEAM_INDEX_ALLIANCE] = NULL;
    m_BgRaids[TEAM_INDEX_HORDE] = NULL;

    m_PlayersCount[TEAM_INDEX_ALLIANCE] = 0;
    m_PlayersCount[TEAM_INDEX_HORDE] = 0;

    m_TeamScores[TEAM_INDEX_ALLIANCE] = 0;
    m_TeamScores[TEAM_INDEX_HORDE] = 0;

    m_PrematureCountDown = false;
    m_PrematureCountDownTimer = 0;

    m_StartDelayTimes[BG_STARTING_EVENT_FIRST] = BG_START_DELAY_2M;
    m_StartDelayTimes[BG_STARTING_EVENT_SECOND] = BG_START_DELAY_1M;
    m_StartDelayTimes[BG_STARTING_EVENT_THIRD] = BG_START_DELAY_30S;
    m_StartDelayTimes[BG_STARTING_EVENT_FOURTH] = BG_START_DELAY_NONE;
    // we must set to some default existing values
    m_StartMessageIds[BG_STARTING_EVENT_FIRST] = 0;
    m_StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_WS_START_ONE_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_THIRD] = LANG_BG_WS_START_HALF_MINUTE;
    m_StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_WS_HAS_BEGUN;
}

/**
 * @brief Destructor for BattleGround.
 */
BattleGround::~BattleGround()
{
#ifdef ENABLE_ELUNA
    // sEluna->OnBGDestroy(this, GetTypeID(), GetInstanceID());
#endif /* ENABLE_ELUNA */

    // remove objects and creatures
    // (this is done automatically in mapmanager update, when the instance is reset after the reset time)
    sBattleGroundMgr.RemoveBattleGround(GetInstanceID(), GetTypeID());

    // skip template bgs as they were never added to visible bg list
    BattleGroundBracketId bracketId = GetBracketId();
    if (bracketId != BG_BRACKET_ID_TEMPLATE)
    {
        sBattleGroundMgr.DeleteClientVisibleInstanceId(GetTypeID(), bracketId, GetClientInstanceID());
    }

    // unload map
    // map can be null at bg destruction
    if (m_Map)
    {
        m_Map->SetUnload();
    }

    // remove from bg free slot queue
    this->RemoveFromBGFreeSlotQueue();

    for (BattleGroundScoreMap::const_iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); ++itr)
    {
        delete itr->second;
    }
}

/**
 * @brief Updates the battleground.
 *
 * @param diff Time difference since last update.
 */
void BattleGround::Update(uint32 diff)
{
    if (!GetPlayersSize())
    {
        // BG is empty
        // if there are no players invited, delete BG
        // this will delete arena or bg object, where any player entered
        // [[   but if you use battleground object again (more battles possible to be played on 1 instance)
        //      then this condition should be removed and code:
        //      if (!GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
        //          this->AddToFreeBGObjectsQueue(); // not yet implemented
        //      should be used instead of current
        // ]]
        // BattleGround Template instance can not be updated, because it would be deleted
        if (!GetInvitedCount(HORDE) && !GetInvitedCount(ALLIANCE))
        {
            delete this;
        }
        return;
    }

    // remove offline players from bg after 5 minutes
    if (!m_OfflineQueue.empty())
    {
        BattleGroundPlayerMap::iterator itr = m_Players.find(*(m_OfflineQueue.begin()));
        if (itr != m_Players.end())
        {
            if (itr->second.OfflineRemoveTime <= sWorld.GetGameTime())
            {
                RemovePlayerAtLeave(itr->first, true, true);// remove player from BG
                m_OfflineQueue.pop_front();                 // remove from offline queue
                // do not use itr for anything, because it is erased in RemovePlayerAtLeave()
            }
        }
    }

    /*********************************************************/
    /***           BATTLEGROUND BALANCE SYSTEM              ***/
    /*********************************************************/

    // if less than minimum players are in on one side, then start premature finish timer
    if (GetStatus() == STATUS_IN_PROGRESS && sBattleGroundMgr.GetPrematureFinishTime() && (GetPlayersCountByTeam(ALLIANCE) < GetMinPlayersPerTeam() || GetPlayersCountByTeam(HORDE) < GetMinPlayersPerTeam()))
    {
        if (!m_PrematureCountDown)
        {
            m_PrematureCountDown = true;
            m_PrematureCountDownTimer = sBattleGroundMgr.GetPrematureFinishTime();
        }
        else if (m_PrematureCountDownTimer < diff)
        {
            EndBattleGround(GetPrematureWinner());
            m_PrematureCountDown = false;
        }
        else if (!sBattleGroundMgr.isTesting())
        {
            uint32 newtime = m_PrematureCountDownTimer - diff;
            // announce every minute
            if (newtime > (MINUTE * IN_MILLISECONDS))
            {
                if (newtime / (MINUTE * IN_MILLISECONDS) != m_PrematureCountDownTimer / (MINUTE * IN_MILLISECONDS))
                {
                    PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING, CHAT_MSG_SYSTEM, NULL, (uint32)(m_PrematureCountDownTimer / (MINUTE * IN_MILLISECONDS)));
                }
            }
            else
            {
                // announce every 15 seconds
                if (newtime / (15 * IN_MILLISECONDS) != m_PrematureCountDownTimer / (15 * IN_MILLISECONDS))
                {
                    PSendMessageToAll(LANG_BATTLEGROUND_PREMATURE_FINISH_WARNING_SECS, CHAT_MSG_SYSTEM, NULL, (uint32)(m_PrematureCountDownTimer / IN_MILLISECONDS));
                }
            }
            m_PrematureCountDownTimer = newtime;
        }
    }
    else if (m_PrematureCountDown)
    {
        m_PrematureCountDown = false;
    }

    /*********************************************************/
    /***           BATTLEGROUND STARTING SYSTEM             ***/
    /*********************************************************/

    if (GetStatus() == STATUS_WAIT_JOIN && GetPlayersSize())
    {
        float maxDist = GetStartMaxDist();
        if (maxDist > 0.0f)
        {
            if (m_validStartPositionTimer < diff)
            {
                for (BattleGroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
                {
                    if (Player* player = sObjectMgr.GetPlayer(itr->first))
                    {
                        float x, y, z, o;
                        GetTeamStartLoc(player->GetTeam(), x, y, z, o);
                        if (!player->IsWithinDist3d(x, y, z, maxDist))
                        {
                            player->TeleportTo(GetMapId(), x, y, z, o);
                        }
                    }
                }
                m_validStartPositionTimer = CHECK_PLAYER_POSITION_INVERVAL;
            }
            else
            {
                m_validStartPositionTimer -= diff;
            }
        }

        ModifyStartDelayTime(diff);

        if (!(m_Events & BG_STARTING_EVENT_1))
        {
            m_Events |= BG_STARTING_EVENT_1;

            StartingEventCloseDoors();
            SetStartDelayTime(m_StartDelayTimes[BG_STARTING_EVENT_FIRST]);
            // first start warning - 2 or 1 minute, only if defined
            if (m_StartMessageIds[BG_STARTING_EVENT_FIRST])
            {
                SendMessageToAll(m_StartMessageIds[BG_STARTING_EVENT_FIRST], CHAT_MSG_BG_SYSTEM_NEUTRAL);
            }
        }
        // After 1 minute or 30 seconds, warning is signaled
        else if (GetStartDelayTime() <= m_StartDelayTimes[BG_STARTING_EVENT_SECOND] && !(m_Events & BG_STARTING_EVENT_2))
        {
            m_Events |= BG_STARTING_EVENT_2;
            SendMessageToAll(m_StartMessageIds[BG_STARTING_EVENT_SECOND], CHAT_MSG_BG_SYSTEM_NEUTRAL);
        }
        // After 30 or 15 seconds, warning is signaled
        else if (GetStartDelayTime() <= m_StartDelayTimes[BG_STARTING_EVENT_THIRD] && !(m_Events & BG_STARTING_EVENT_3))
        {
            m_Events |= BG_STARTING_EVENT_3;
            SendMessageToAll(m_StartMessageIds[BG_STARTING_EVENT_THIRD], CHAT_MSG_BG_SYSTEM_NEUTRAL);
        }
        // Delay expired (after 2 or 1 minute)
        else if (GetStartDelayTime() <= 0 && !(m_Events & BG_STARTING_EVENT_4))
        {
            m_Events |= BG_STARTING_EVENT_4;

#ifdef ENABLE_ELUNA
            if (Eluna* e = this->GetBgMap()->GetEluna())
            {
                e->OnBGCreate(this, GetTypeID(), GetInstanceID());
            }
#endif /* ENABLE_ELUNA */

            StartingEventOpenDoors();

            SendMessageToAll(m_StartMessageIds[BG_STARTING_EVENT_FOURTH], CHAT_MSG_BG_SYSTEM_NEUTRAL);
            SetStatus(STATUS_IN_PROGRESS);
            SetStartDelayTime(m_StartDelayTimes[BG_STARTING_EVENT_FOURTH]);

            {
                PlaySoundToAll(SOUND_BG_START);

                // Announce BG starting
                if (sWorld.getConfig(CONFIG_BOOL_BATTLEGROUND_QUEUE_ANNOUNCER_START))
                {
                    sWorld.SendWorldText(LANG_BG_STARTED_ANNOUNCE_WORLD, GetName(), GetMinLevel(), GetMaxLevel());
                }
            }
        }
    }

    /*********************************************************/
    /***           BATTLEGROUND ENDING SYSTEM              ***/
    /*********************************************************/

    if (GetStatus() == STATUS_WAIT_LEAVE)
    {
        // Remove all players from battleground after 2 minutes
        m_EndTime -= diff;
        if (m_EndTime <= 0)
        {
            m_EndTime = 0;
            BattleGroundPlayerMap::iterator itr, next;
            for (itr = m_Players.begin(); itr != m_Players.end(); itr = next)
            {
                next = itr;
                ++next;
                // itr is erased here!
                RemovePlayerAtLeave(itr->first, true, true); // Remove player from BG
                // Do not change any battleground's private variables
            }
        }
    }

    // Update start time
    m_StartTime += diff;
}

/**
 * @brief Sets the team start location.
 *
 * @param team The team.
 * @param X The X coordinate.
 * @param Y The Y coordinate.
 * @param Z The Z coordinate.
 * @param O The orientation.
 */
void BattleGround::SetTeamStartLoc(Team team, float X, float Y, float Z, float O)
{
    PvpTeamIndex teamIdx = GetTeamIndexByTeamId(team);
    m_TeamStartLocX[teamIdx] = X;
    m_TeamStartLocY[teamIdx] = Y;
    m_TeamStartLocZ[teamIdx] = Z;
    m_TeamStartLocO[teamIdx] = O;
}

/**
 * @brief Sends a packet to all players in the battleground.
 *
 * @param packet The packet to send.
 */
void BattleGround::SendPacketToAll(WorldPacket* packet)
{
    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
        {
            continue;
        }

        if (Player* plr = sObjectMgr.GetPlayer(itr->first))
        {
            plr->GetSession()->SendPacket(packet);
        }
        else
        {
            sLog.outError("BattleGround:SendPacketToAll: %s not found!", itr->first.GetString().c_str());
        }
    }
}

/**
 * @brief Sends a packet to a specific team in the battleground.
 *
 * @param teamId The team ID.
 * @param packet The packet to send.
 * @param sender The sender of the packet.
 * @param self Whether to send the packet to the sender.
 */
void BattleGround::SendPacketToTeam(Team teamId, WorldPacket* packet, Player* sender, bool self)
{
    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
        {
            continue;
        }

        Player* plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr)
        {
            sLog.outError("BattleGround:SendPacketToTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        if (!self && sender == plr)
        {
            continue;
        }

        Team team = itr->second.PlayerTeam;
        if (!team)
        {
            team = plr->GetTeam();
        }

        if (team == teamId)
        {
            plr->GetSession()->SendPacket(packet);
        }
    }
}

/**
 * @brief Plays a sound to all players in the battleground.
 *
 * @param SoundID The sound ID.
 */
void BattleGround::PlaySoundToAll(uint32 SoundID)
{
    WorldPacket data;
    sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
    SendPacketToAll(&data);
}

/**
 * @brief Plays a sound to a specific team in the battleground.
 *
 * @param SoundID The sound ID.
 * @param teamId The team ID.
 */
void BattleGround::PlaySoundToTeam(uint32 SoundID, Team teamId)
{
    WorldPacket data;

    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
        {
            continue;
        }

        Player* plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr)
        {
            sLog.outError("BattleGround:PlaySoundToTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        Team team = itr->second.PlayerTeam;
        if (!team)
        {
            team = plr->GetTeam();
        }

        if (team == teamId)
        {
            sBattleGroundMgr.BuildPlaySoundPacket(&data, SoundID);
            plr->GetSession()->SendPacket(&data);
        }
    }
}

/**
 * @brief Casts a spell on a specific team in the battleground.
 *
 * @param SpellID The spell ID.
 * @param teamId The team ID.
 */
void BattleGround::CastSpellOnTeam(uint32 SpellID, Team teamId)
{
    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
        {
            continue;
        }

        Player* plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
        {
            sLog.outError("BattleGround:CastSpellOnTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        Team team = itr->second.PlayerTeam;
        if (!team)
        {
            team = plr->GetTeam();
        }

        if (team == teamId)
        {
            plr->CastSpell(plr, SpellID, true);
        }
    }
}

/**
 * @brief Rewards the honor to a specific team in the battleground.
 *
 * @param Honor The amount of honor to reward.
 * @param teamId The team ID.
 */
void BattleGround::RewardHonorToTeam(uint32 Honor, Team teamId)
{
    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
        {
            continue;
        }

        Player* plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
        {
            sLog.outError("BattleGround:RewardHonorToTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        Team team = itr->second.PlayerTeam;
        if (!team)
        {
            team = plr->GetTeam();
        }

        if (team == teamId)
        {
            UpdatePlayerScore(plr, SCORE_BONUS_HONOR, Honor);
        }
    }
}

/**
 * @brief Rewards the reputation to a specific team in the battleground.
 *
 * @param faction_id The faction ID.
 * @param Reputation The amount of reputation to reward.
 * @param teamId The team ID.
 */
void BattleGround::RewardReputationToTeam(uint32 faction_id, uint32 Reputation, Team teamId)
{
    FactionEntry const* factionEntry = sFactionStore.LookupEntry(faction_id);

    if (!factionEntry)
    {
        return;
    }

    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.OfflineRemoveTime)
        {
            continue;
        }

        Player* plr = sObjectMgr.GetPlayer(itr->first);

        if (!plr)
        {
            sLog.outError("BattleGround:RewardReputationToTeam: %s not found!", itr->first.GetString().c_str());
            continue;
        }

        Team team = itr->second.PlayerTeam;
        if (!team)
        {
            team = plr->GetTeam();
        }

        if (team == teamId)
        {
            plr->GetReputationMgr().ModifyReputation(factionEntry, Reputation);
        }
    }
}

/**
 * @brief Updates the state of the world for all players in the battleground.
 *
 * @param Field The field to update.
 * @param Value The value to set.
 */
void BattleGround::UpdateWorldState(uint32 Field, uint32 Value)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    SendPacketToAll(&data);
}

/**
 * @brief Updates the state of the world for a specific player in the battleground.
 *
 * @param Field The field to update.
 * @param Value The value to set.
 * @param Source The player to update.
 */
void BattleGround::UpdateWorldStateForPlayer(uint32 Field, uint32 Value, Player* Source)
{
    WorldPacket data;
    sBattleGroundMgr.BuildUpdateWorldStatePacket(&data, Field, Value);
    Source->GetSession()->SendPacket(&data);
}

/**
 * @brief Ends the battleground and declares a winner.
 *
 * @param winner The winning team.
 */
void BattleGround::EndBattleGround(Team winner)
{
#ifdef ENABLE_ELUNA
    if (Eluna* e = GetBgMap()->GetEluna())
    {
        e->OnBGEnd(this, GetTypeID(), GetInstanceID(), winner);
    }
#endif /* ENABLE_ELUNA */
    this->RemoveFromBGFreeSlotQueue();

    uint32 winner_rating = 0;
    WorldPacket data;
    int32 winmsg_id = 0;

    uint32 bgScoresWinner = TEAM_INDEX_NEUTRAL;
    uint64 battleground_id = 1;

    if (winner == ALLIANCE)
    {
        winmsg_id = LANG_BG_A_WINS;
        PlaySoundToAll(SOUND_ALLIANCE_WINS);                // alliance wins sound

        // reversed index for the bg score storage system
        bgScoresWinner = TEAM_INDEX_HORDE;
    }
    else if (winner == HORDE)
    {
        winmsg_id = LANG_BG_H_WINS;
        PlaySoundToAll(SOUND_HORDE_WINS);                   // horde wins sound

        // reversed index for the bg score storage system
        bgScoresWinner = TEAM_INDEX_ALLIANCE;
    }

    // store battleground scores
    if (sWorld.getConfig(CONFIG_BOOL_BATTLEGROUND_SCORE_STATISTICS))
    {
        static SqlStatementID insPvPstatsBattleground;
        QueryResult* result;

        SqlStatement stmt = CharacterDatabase.CreateStatement(insPvPstatsBattleground, "INSERT INTO `pvpstats_battlegrounds` (`id`, `winner_team`, `bracket_id`, `type`, `date`) VALUES (?, ?, ?, ?, NOW())");

        uint8 battleground_bracket = GetMinLevel() / 10;
        uint8 battleground_type = (uint8)GetTypeID();

        // query next id
        result = CharacterDatabase.Query("SELECT MAX(`id`) FROM `pvpstats_battlegrounds`");
        if (result)
        {
            Field* fields = result->Fetch();
            battleground_id = fields[0].GetUInt64() + 1;
            delete result;
        }

        stmt.PExecute(battleground_id, bgScoresWinner, battleground_bracket, battleground_type);
    }

    SetWinner(winner);

    SetStatus(STATUS_WAIT_LEAVE);
    // we must set it this way, because end time is sent in packet!
    m_EndTime = TIME_TO_AUTOREMOVE;

    for (BattleGroundPlayerMap::iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        Team team = itr->second.PlayerTeam;

        if (itr->second.OfflineRemoveTime)
        {
            continue;
        }

        Player* plr = sObjectMgr.GetPlayer(itr->first);
        if (!plr)
        {
            sLog.outError("BattleGround:EndBattleGround %s not found!", itr->first.GetString().c_str());
            continue;
        }

        // should remove spirit of redemption
        if (plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        {
            plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);
        }

        if (!plr->IsAlive())
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }
        else
        {
            // needed cause else in av some creatures will kill the players at the end
            plr->CombatStop();
            plr->GetHostileRefManager().deleteReferences();
        }

        // this line is obsolete - team is set ALWAYS
        // if (!team) team = plr->GetTeam();

        // store battleground score statistics for each player
        if (sWorld.getConfig(CONFIG_BOOL_BATTLEGROUND_SCORE_STATISTICS))
        {
            static SqlStatementID insPvPstatsPlayer;
            BattleGroundScoreMap::iterator score = m_PlayerScores.find(itr->first);
            SqlStatement stmt = CharacterDatabase.CreateStatement(insPvPstatsPlayer, "INSERT INTO `pvpstats_players` (`battleground_id`, `character_guid`, `score_killing_blows`, `score_deaths`, `score_honorable_kills`, `score_bonus_honor`, `score_damage_done`, `score_healing_done`, `attr_1`, `attr_2`, `attr_3`, `attr_4`, `attr_5`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");

            stmt.addUInt32(battleground_id);
            stmt.addUInt32(plr->GetGUIDLow());
            stmt.addUInt32(score->second->GetKillingBlows());
            stmt.addUInt32(score->second->GetDeaths());
            stmt.addUInt32(score->second->GetHonorableKills());
            stmt.addUInt32(score->second->GetBonusHonor());
            stmt.addUInt32(score->second->GetDamageDone());
            stmt.addUInt32(score->second->GetHealingDone());
            stmt.addUInt32(score->second->GetAttr1());
            stmt.addUInt32(score->second->GetAttr2());
            stmt.addUInt32(score->second->GetAttr3());
            stmt.addUInt32(score->second->GetAttr4());
            stmt.addUInt32(score->second->GetAttr5());

            stmt.Execute();
        }

        if (team == winner)
        {
            RewardMark(plr, ITEM_WINNER_COUNT);
            RewardQuestComplete(plr);
        }
        else
        {
            RewardMark(plr, ITEM_LOSER_COUNT);
        }

        plr->CombatStopWithPets(true);

        BlockMovement(plr);

        sBattleGroundMgr.BuildPvpLogDataPacket(&data, this);
        plr->GetSession()->SendPacket(&data);

        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID());
        sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, TIME_TO_AUTOREMOVE, GetStartTime());
        plr->GetSession()->SendPacket(&data);
    }

    if (winmsg_id)
    {
        SendMessageToAll(winmsg_id, CHAT_MSG_BG_SYSTEM_NEUTRAL);
    }
}

/**
 * @brief Gets the bonus honor from a kill.
 *
 * @param kills The number of kills.
 * @returns The amount of bonus honor.
 */
uint32 BattleGround::GetBonusHonorFromKill(uint32 kills) const
{
    // variable kills means how many honorable kills you scored (so we need kills * honor_for_one_kill)
    return (uint32)MaNGOS::Honor::hk_honor_at_level(GetMaxLevel(), kills);
}

/**
 * @brief Gets the battlemaster entry for the battleground.
 *
 * @returns The battlemaster entry ID.
 */
uint32 BattleGround::GetBattlemasterEntry() const
{
    switch (GetTypeID())
    {
    case BATTLEGROUND_AV: return 15972;
    case BATTLEGROUND_WS: return 14623;
    case BATTLEGROUND_AB: return 14879;
    default:              return 0;
    }
}

/**
 * @brief Rewards the mark to the player based on the battleground type and result.
 *
 * @param plr The player to reward.
 * @param count The count of marks to reward.
 */
void BattleGround::RewardMark(Player* plr, uint32 count)
{
    switch (GetTypeID())
    {
    case BATTLEGROUND_AV:
        if (count == ITEM_WINNER_COUNT)
        {
            RewardSpellCast(plr, SPELL_AV_MARK_WINNER);
        }
        else
        {
            RewardSpellCast(plr, SPELL_AV_MARK_LOSER);
        }
        break;
    case BATTLEGROUND_WS:
        if (count == ITEM_WINNER_COUNT)
        {
            RewardSpellCast(plr, SPELL_WS_MARK_WINNER);
        }
        else
        {
            RewardSpellCast(plr, SPELL_WS_MARK_LOSER);
        }
        break;
    case BATTLEGROUND_AB:
        if (count == ITEM_WINNER_COUNT)
        {
            RewardSpellCast(plr, SPELL_AB_MARK_WINNER);
        }
        else
        {
            RewardSpellCast(plr, SPELL_AB_MARK_LOSER);
        }
        break;
    default:
        break;
    }
}

/**
 * @brief Casts a reward spell on the player.
 *
 * @param plr The player to cast the spell on.
 * @param spell_id The ID of the spell to cast.
 */
void BattleGround::RewardSpellCast(Player* plr, uint32 spell_id)
{
    SpellEntry const* spellInfo = sSpellStore.LookupEntry(spell_id);
    if (!spellInfo)
    {
        sLog.outError("Battleground reward casting spell %u not exist.", spell_id);
        return;
    }

    plr->CastSpell(plr, spellInfo, true);
}

/**
 * @brief Rewards an item to the player.
 *
 * @param plr The player to reward.
 * @param item_id The ID of the item to reward.
 * @param count The count of items to reward.
 */
void BattleGround::RewardItem(Player* plr, uint32 item_id, uint32 count)
{
    ItemPosCountVec dest;
    uint32 no_space_count = 0;
    uint8 msg = plr->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, item_id, count, &no_space_count);

    if (msg == EQUIP_ERR_ITEM_NOT_FOUND)
    {
        sLog.outErrorDb("Battleground reward item (Entry %u) not exist in `item_template`.", item_id);
        return;
    }

    if (msg != EQUIP_ERR_OK)                                // convert to possible store amount
    {
        count -= no_space_count;
    }

    if (count != 0 && !dest.empty())                        // can add some
    {
        if (Item* item = plr->StoreNewItem(dest, item_id, true, 0))
        {
            plr->SendNewItem(item, count, true, false);
        }
    }

    if (no_space_count > 0)
    {
        SendRewardMarkByMail(plr, item_id, no_space_count);
    }
}

/**
 * @brief Sends the reward mark by mail if the player has no space in inventory.
 *
 * @param plr The player to send the mail to.
 * @param mark The ID of the mark item.
 * @param count The count of marks to send.
 */
void BattleGround::SendRewardMarkByMail(Player* plr, uint32 mark, uint32 count) const
{
    uint32 bmEntry = GetBattlemasterEntry();
    if (!bmEntry)
    {
        return;
    }

    ItemPrototype const* markProto = ObjectMgr::GetItemPrototype(mark);
    if (!markProto)
    {
        return;
    }

    if (Item* markItem = Item::CreateItem(mark, count, plr))
    {
        // save new item before send
        markItem->SaveToDB();                               // save for prevent lost at next mail load, if send fail then item will deleted

        int loc_idx = plr->GetSession()->GetSessionDbLocaleIndex();

        // subject: item name
        std::string subject = markProto->Name1;
        sObjectMgr.GetItemLocaleStrings(markProto->ItemId, loc_idx, &subject);

        // text
        std::string textFormat = plr->GetSession()->GetMangosString(LANG_BG_MARK_BY_MAIL);
        char textBuf[300];
        snprintf(textBuf, 300, textFormat.c_str(), GetName(), GetName());

        MailDraft(subject, textBuf)
            .AddItem(markItem)
            .SendMailTo(plr, MailSender(MAIL_CREATURE, bmEntry));
    }
}

/**
 * @brief Rewards the player for completing a battleground quest.
 *
 * @param plr The player to reward.
 */
void BattleGround::RewardQuestComplete(Player* plr)
{
    uint32 quest;
    switch (GetTypeID())
    {
    case BATTLEGROUND_AV:
        quest = SPELL_AV_QUEST_REWARD;
        break;
    case BATTLEGROUND_WS:
        quest = SPELL_WS_QUEST_REWARD;
        break;
    case BATTLEGROUND_AB:
        quest = SPELL_AB_QUEST_REWARD;
        break;
    default:
        return;
    }

    RewardSpellCast(plr, quest);
}

/**
 * @brief Blocks the movement of the player.
 *
 * @param plr The player to block movement for.
 */
void BattleGround::BlockMovement(Player* plr)
{
    plr->SetClientControl(plr, 0);                          // movement disabled NOTE: the effect will be automatically removed by client when the player is teleported from the battleground, so no need to send with uint8(1) in RemovePlayerAtLeave()
}

/**
 * @brief Removes the player from the battleground when they leave.
 *
 * @param guid The GUID of the player.
 * @param Transport Whether to transport the player out of the battleground.
 * @param SendPacket Whether to send a packet to the player.
 */
void BattleGround::RemovePlayerAtLeave(ObjectGuid guid, bool Transport, bool SendPacket)
{
    Team team = GetPlayerTeam(guid);
    bool participant = false;
    // Remove from lists/maps
    BattleGroundPlayerMap::iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
    {
        UpdatePlayersCountByTeam(team, true);               // -1 player
        m_Players.erase(itr);
        // check if the player was a participant of the match, or only entered through gm command (goname)
        participant = true;
    }

    BattleGroundScoreMap::iterator itr2 = m_PlayerScores.find(guid);
    if (itr2 != m_PlayerScores.end())
    {
        delete itr2->second;                                // delete player's score
        m_PlayerScores.erase(itr2);
    }

    Player* plr = sObjectMgr.GetPlayer(guid);

    if (plr)
    {
        // should remove spirit of redemption
        if (plr->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
        {
            plr->RemoveSpellsCausingAura(SPELL_AURA_MOD_SHAPESHIFT);
        }

        plr->RemoveSpellsCausingAura(SPELL_AURA_MOUNTED);

        if (!plr->IsAlive())                                // resurrect on exit
        {
            plr->ResurrectPlayer(1.0f);
            plr->SpawnCorpseBones();
        }
    }

    RemovePlayer(plr, guid);                                // BG subclass specific code

    if (participant) // if the player was a match participant, remove auras, calc rating, update queue
    {
        BattleGroundTypeId bgTypeId = GetTypeID();
        BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID());
        if (plr)
        {
            if (!team)
            {
                team = plr->GetTeam();
            }

            if (SendPacket)
            {
                WorldPacket data;
                sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_NONE, 0, 0);
                plr->GetSession()->SendPacket(&data);
            }

            // this call is important, because player, when joins to battleground, this method is not called, so it must be called when leaving bg
            plr->RemoveBattleGroundQueueId(bgQueueTypeId);
        }

        // remove from raid group if player is member
        if (Group* group = GetBgRaid(team))
        {
            if (!group->RemoveMember(guid, 0))              // group was disbanded
            {
                SetBgRaid(team, NULL);
                delete group;
            }
        }
        DecreaseInvitedCount(team);
        // we should update battleground queue, but only if bg isn't ending
        if (GetStatus() < STATUS_WAIT_LEAVE)
        {
            // a player has left the battleground, so there are free slots -> add to queue
            AddToBGFreeSlotQueue();
            sBattleGroundMgr.ScheduleQueueUpdate(bgQueueTypeId, bgTypeId, GetBracketId());
        }

        // Let others know
        WorldPacket data;
        sBattleGroundMgr.BuildPlayerLeftBattleGroundPacket(&data, guid);
        SendPacketToTeam(team, &data, plr, false);
    }

    if (plr)
    {
        // Do next only if found in battleground
        plr->SetBattleGroundId(0, BATTLEGROUND_TYPE_NONE);  // We're not in BG.
        // reset destination bg team
        plr->SetBGTeam(TEAM_NONE);

        if (Transport)
        {
            plr->TeleportToBGEntryPoint();
        }

        DETAIL_LOG("BATTLEGROUND: Removed player %s from BattleGround.", plr->GetName());
    }

    // battleground object will be deleted next BattleGround::Update() call
}

/**
 * @brief Resets the battleground when no players remain.
 */
void BattleGround::Reset()
{
    SetWinner(TEAM_NONE);
    SetStatus(STATUS_WAIT_QUEUE);
    SetStartTime(0);
    SetEndTime(0);

    m_Events = 0;

    // door-event2 is always 0
    m_ActiveEvents[BG_EVENT_DOOR] = 0;

    if (m_InvitedAlliance > 0 || m_InvitedHorde > 0)
    {
        sLog.outError("BattleGround system: bad counter, m_InvitedAlliance: %d, m_InvitedHorde: %d", m_InvitedAlliance, m_InvitedHorde);
    }

    m_InvitedAlliance = 0;
    m_InvitedHorde = 0;
    m_InBGFreeSlotQueue = false;

    m_Players.clear();

    for (BattleGroundScoreMap::const_iterator itr = m_PlayerScores.begin(); itr != m_PlayerScores.end(); ++itr)
    {
        delete itr->second;
    }
    m_PlayerScores.clear();
}

/**
 * @brief Starts the battleground.
 */
void BattleGround::StartBattleGround()
{
    SetStartTime(0);

    // add BG to free slot queue
    AddToBGFreeSlotQueue();

    // add bg to update list
    // This must be done here, because we need to have already invited some players when first BG::Update() method is executed
    // and it doesn't matter if we call StartBattleGround() more times, because m_BattleGrounds is a map and instance id never changes
    sBattleGroundMgr.AddBattleGround(GetInstanceID(), GetTypeID(), this);

#ifdef ENABLE_ELUNA
    if (Eluna* e = GetBgMap()->GetEluna())
    {
        e->OnBGCreate(this, GetTypeID(), GetInstanceID());
    }
#endif /* ENABLE_ELUNA */
}

/**
 * @brief Adds a player to the battleground.
 *
 * @param plr The player to add.
 */
void BattleGround::AddPlayer(Player* plr)
{
    // remove afk from player
    if (plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_AFK))
    {
        plr->ToggleAFK();
    }

    // score struct must be created in inherited class

    ObjectGuid guid = plr->GetObjectGuid();
    Team team = plr->GetBGTeam();

    BattleGroundPlayer bp;
    bp.OfflineRemoveTime = 0;
    bp.PlayerTeam = team;

    // Add to list/maps
    m_Players[guid] = bp;

    bool const isInBattleground = IsPlayerInBattleGround(guid);
    if (!isInBattleground)
    {
        UpdatePlayersCountByTeam(team, false); // +1 player
    }

    WorldPacket data;
    sBattleGroundMgr.BuildPlayerJoinedBattleGroundPacket(&data, plr);
    SendPacketToTeam(team, &data, plr, false);

    // setup BG group membership
    PlayerAddedToBGCheckIfBGIsRunning(plr);
    AddOrSetPlayerToCorrectBgGroup(plr, guid, team);

    // Log
    DETAIL_LOG("BATTLEGROUND: Player %s joined the battle.", plr->GetName());
}

/**
 * @brief Adds a player to their team's battleground group or sets their correct group if already in a group.
 *
 * @param plr The player to add or set.
 * @param plr_guid The GUID of the player.
 * @param team The team of the player.
 */
void BattleGround::AddOrSetPlayerToCorrectBgGroup(Player* plr, ObjectGuid plr_guid, Team team)
{
    if (Group* group = GetBgRaid(team))                     // raid already exist
    {
        if (group->IsMember(plr_guid))
        {
            uint8 subgroup = group->GetMemberGroup(plr_guid);
            plr->SetBattleGroundRaid(group, subgroup);
        }
        else
        {
            group->AddMember(plr_guid, plr->GetName());
            if (Group* originalGroup = plr->GetOriginalGroup())
            {
                if (originalGroup->IsLeader(plr_guid))
                {
                    group->ChangeLeader(plr_guid);
                }
            }
        }
    }
    else                                                    // first player joined
    {
        group = new Group;
        SetBgRaid(team, group);
        group->Create(plr_guid, plr->GetName());
    }
}

/**
 * @brief Handles player login to a running battleground.
 *
 * @param player The player logging in.
 */
void BattleGround::EventPlayerLoggedIn(Player* player)
{
    ObjectGuid playerGuid = player->GetObjectGuid();

    // player is correct pointer
    for (OfflineQueue::iterator itr = m_OfflineQueue.begin(); itr != m_OfflineQueue.end(); ++itr)
    {
        if (*itr == playerGuid)
        {
            m_OfflineQueue.erase(itr);
            break;
        }
    }
    m_Players[playerGuid].OfflineRemoveTime = 0;
    PlayerAddedToBGCheckIfBGIsRunning(player);
    // if battleground is starting, then add preparation aura
    // we don't have to do that, because preparation aura isn't removed when player logs out
}

/**
 * @brief Handles player logout from a running battleground.
 *
 * @param player The player logging out.
 */
void BattleGround::EventPlayerLoggedOut(Player* player)
{
    // player is correct pointer, it is checked in WorldSession::LogoutPlayer()
    m_OfflineQueue.push_back(player->GetObjectGuid());
    m_Players[player->GetObjectGuid()].OfflineRemoveTime = sWorld.GetGameTime() + MAX_OFFLINE_TIME;
    if (GetStatus() == STATUS_IN_PROGRESS)
    {
        // drop flag and handle other cleanups
        RemovePlayer(player, player->GetObjectGuid());
    }
}

/**
 * @brief Adds the battleground to the free slot queue.
 * This method should be called only once.
 */
void BattleGround::AddToBGFreeSlotQueue()
{
    // make sure to add only once
    if (!m_InBGFreeSlotQueue)
    {
        sBattleGroundMgr.BGFreeSlotQueue[m_TypeID].push_front(this);
        m_InBGFreeSlotQueue = true;
    }
}

/**
 * @brief Removes the battleground from the free slot queue.
 * This method must be called when deleting the battleground.
 */
void BattleGround::RemoveFromBGFreeSlotQueue()
{
    // set to be able to re-add if needed
    m_InBGFreeSlotQueue = false;
    BGFreeSlotQueueType& bgFreeSlot = sBattleGroundMgr.BGFreeSlotQueue[m_TypeID];
    for (BGFreeSlotQueueType::iterator itr = bgFreeSlot.begin(); itr != bgFreeSlot.end(); ++itr)
    {
        if ((*itr)->GetInstanceID() == GetInstanceID())
        {
            bgFreeSlot.erase(itr);
            return;
        }
    }
}

/**
 * @brief Gets the number of free slots for a team.
 *
 * @param team The team.
 * @returns The number of free slots for the team.
 */
uint32 BattleGround::GetFreeSlotsForTeam(Team team) const
{
    // return free slot count to MaxPlayerPerTeam
    if (GetStatus() == STATUS_WAIT_JOIN || GetStatus() == STATUS_IN_PROGRESS)
    {
        return (GetInvitedCount(team) < GetMaxPlayersPerTeam()) ? GetMaxPlayersPerTeam() - GetInvitedCount(team) : 0;
    }

    return 0;
}

/**
 * @brief Determines whether the battleground has free slots.
 *
 * @returns True if the battleground has free slots, false otherwise.
 */
bool BattleGround::HasFreeSlots() const
{
    return GetPlayersSize() < GetMaxPlayers();
}

/**
 * @brief Updates the player's score.
 *
 * @param Source The player whose score is being updated.
 * @param type The type of score to update.
 * @param value The value to update the score by.
 */
void BattleGround::UpdatePlayerScore(Player* Source, uint32 type, uint32 value)
{
    // this procedure is called from virtual function implemented in bg subclass
    BattleGroundScoreMap::const_iterator itr = m_PlayerScores.find(Source->GetObjectGuid());

    if (itr == m_PlayerScores.end())                        // player not found...
    {
        return;
    }

    switch (type)
    {
    case SCORE_KILLING_BLOWS:                           // Killing blows
        itr->second->KillingBlows += value;
        break;
    case SCORE_DEATHS:                                  // Deaths
        itr->second->Deaths += value;
        break;
    case SCORE_HONORABLE_KILLS:                         // Honorable kills
        itr->second->HonorableKills += value;
        break;
    case SCORE_BONUS_HONOR:                             // Honor bonus
        // reward honor instantly
        if (Source->AddHonorCP(value, HONORABLE, 0, 0))
        {
            itr->second->BonusHonor += value;
        }
        break;
    default:
        sLog.outError("BattleGround: Unknown player score type %u", type);
        break;
    }
}

/**
 * @brief Closes a door in the battleground.
 *
 * @param guid The GUID of the door to close.
 */
void BattleGround::DoorClose(ObjectGuid guid)
{
    GameObject* obj = GetBgMap()->GetGameObject(guid);
    if (obj)
    {
        // if doors are open, close it
        if (obj->getLootState() == GO_ACTIVATED && obj->GetGoState() != GO_STATE_READY)
        {
            // change state to allow door to be closed
            obj->SetLootState(GO_READY);
            obj->UseDoorOrButton(RESPAWN_ONE_DAY);
        }
    }
    else
    {
        sLog.outError("BattleGround: Door %s not found (can not close doors)", guid.GetString().c_str());
    }
}

/**
 * @brief Opens a door in the battleground.
 *
 * @param guid The GUID of the door to open.
 */
void BattleGround::DoorOpen(ObjectGuid guid)
{
    GameObject* obj = GetBgMap()->GetGameObject(guid);
    if (obj)
    {
        // change state to be sure they will be opened
        obj->SetLootState(GO_READY);
        obj->UseDoorOrButton(RESPAWN_ONE_DAY);
    }
    else
    {
        sLog.outError("BattleGround: Door %s not found! - doors will be closed.", guid.GetString().c_str());
    }
}

/**
 * @brief Handles the loading of a creature from the database.
 *
 * @param creature The creature being loaded.
 */
void BattleGround::OnObjectDBLoad(Creature* creature)
{
    const BattleGroundEventIdx eventId = sBattleGroundMgr.GetCreatureEventIndex(creature->GetGUIDLow());
    if (eventId.event1 == BG_EVENT_NONE)
    {
        return;
    }
    m_EventObjects[MAKE_PAIR32(eventId.event1, eventId.event2)].creatures.push_back(creature->GetObjectGuid());
    if (!IsActiveEvent(eventId.event1, eventId.event2))
    {
        SpawnBGCreature(creature->GetObjectGuid(), RESPAWN_ONE_DAY);
    }
}

/**
 * @brief Gets the GUID of a single creature for a specific event.
 *
 * @param event1 The first event identifier.
 * @param event2 The second event identifier.
 * @returns The GUID of the creature.
 */
ObjectGuid BattleGround::GetSingleCreatureGuid(uint8 event1, uint8 event2)
{
    GuidVector::const_iterator itr = m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.begin();
    if (itr != m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.end())
    {
        return *itr;
    }
    return ObjectGuid();
}

/**
 * @brief Handles the loading of a game object from the database.
 *
 * @param obj The game object being loaded.
 */
void BattleGround::OnObjectDBLoad(GameObject* obj)
{
    const BattleGroundEventIdx eventId = sBattleGroundMgr.GetGameObjectEventIndex(obj->GetGUIDLow());
    if (eventId.event1 == BG_EVENT_NONE)
    {
        return;
    }
    m_EventObjects[MAKE_PAIR32(eventId.event1, eventId.event2)].gameobjects.push_back(obj->GetObjectGuid());
    if (!IsActiveEvent(eventId.event1, eventId.event2))
    {
        SpawnBGObject(obj->GetObjectGuid(), RESPAWN_ONE_DAY);
    }
    else
    {
        // it's possible, that doors aren't spawned anymore (wsg)
        if (GetStatus() >= STATUS_IN_PROGRESS && IsDoor(eventId.event1, eventId.event2))
        {
            DoorOpen(obj->GetObjectGuid());
        }
    }
}

/**
 * @brief Determines whether the specified event is a door event.
 *
 * @param event1 The first event identifier.
 * @param event2 The second event identifier.
 * @returns True if the event is a door event, false otherwise.
 */
bool BattleGround::IsDoor(uint8 event1, uint8 event2)
{
    if (event1 == BG_EVENT_DOOR)
    {
        if (event2 > 0)
        {
            sLog.outError("BattleGround too high event2 for event1:%i", event1);
            return false;
        }
        return true;
    }
    return false;
}

/**
 * @brief Opens a door event in the battleground.
 *
 * @param event1 The first event identifier.
 * @param event2 The second event identifier.
 */
void BattleGround::OpenDoorEvent(uint8 event1, uint8 event2 /*=0*/)
{
    if (!IsDoor(event1, event2))
    {
        sLog.outError("BattleGround:OpenDoorEvent this is no door event1:%u event2:%u", event1, event2);
        return;
    }
    if (!IsActiveEvent(event1, event2))                 // maybe already despawned (eye)
    {
        sLog.outError("BattleGround:OpenDoorEvent this event isn't active event1:%u event2:%u", event1, event2);
        return;
    }
    GuidVector::const_iterator itr = m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.begin();
    for (; itr != m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.end(); ++itr)
    {
        DoorOpen(*itr);
    }
}

/**
 * @brief Spawns or despawns an event in the battleground.
 *
 * @param event1 The first event identifier.
 * @param event2 The second event identifier.
 * @param spawn Whether to spawn or despawn the event.
 */
void BattleGround::SpawnEvent(uint8 event1, uint8 event2, bool spawn)
{
    // stop if we want to spawn something which was already spawned
    // or despawn something which was already despawned
    if (event2 == BG_EVENT_NONE || (spawn && m_ActiveEvents[event1] == event2)
        || (!spawn && m_ActiveEvents[event1] != event2))
    {
        return;
    }

    if (spawn)
    {
        // if event gets spawned, the current active event must get despawned
        SpawnEvent(event1, m_ActiveEvents[event1], false);
        m_ActiveEvents[event1] = event2;                    // set this event to active
    }
    else
    {
        m_ActiveEvents[event1] = BG_EVENT_NONE;             // no event active if event2 gets despawned
    }

    GuidVector::const_iterator itr = m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.begin();
    for (; itr != m_EventObjects[MAKE_PAIR32(event1, event2)].creatures.end(); ++itr)
    {
        SpawnBGCreature(*itr, (spawn) ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);
    }

    GuidVector::const_iterator itr2 = m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.begin();
    for (; itr2 != m_EventObjects[MAKE_PAIR32(event1, event2)].gameobjects.end(); ++itr2)
    {
        SpawnBGObject(*itr2, (spawn) ? RESPAWN_IMMEDIATELY : RESPAWN_ONE_DAY);
    }
}

/**
 * @brief Spawns a game object in the battleground.
 *
 * @param guid The GUID of the game object to spawn.
 * @param respawntime The respawn time of the game object.
 */
void BattleGround::SpawnBGObject(ObjectGuid guid, uint32 respawntime)
{
    Map* map = GetBgMap();

    GameObject* obj = map->GetGameObject(guid);
    if (!obj)
    {
        return;
    }

    if (respawntime == 0)
    {
        // we need to change state from GO_JUST_DEACTIVATED to GO_READY in case battleground is starting again
        if (obj->getLootState() == GO_JUST_DEACTIVATED)
        {
            obj->SetLootState(GO_READY);
        }
        obj->SetRespawnTime(0);
        map->Add(obj);
    }
    else
    {
        map->Add(obj);
        obj->SetRespawnTime(respawntime);
        obj->SetLootState(GO_JUST_DEACTIVATED);
    }
}

/**
 * @brief Spawns a creature in the battleground.
 *
 * @param guid The GUID of the creature to spawn.
 * @param respawntime The respawn time of the creature.
 */
void BattleGround::SpawnBGCreature(ObjectGuid guid, uint32 respawntime)
{
    Map* map = GetBgMap();

    Creature* obj = map->GetCreature(guid);
    if (!obj)
    {
        return;
    }

    if (respawntime == 0)
    {
        obj->Respawn();
        map->Add(obj);
    }
    else
    {
        map->Add(obj);
        obj->SetRespawnDelay(respawntime);
        obj->SetDeathState(JUST_DIED);
        obj->RemoveCorpse();
    }
}

/**
 * @brief Sends a message to all players in the battleground.
 *
 * @param entry The entry ID of the message.
 * @param type The type of chat message.
 * @param source The source player of the message.
 */
void BattleGround::SendMessageToAll(int32 entry, ChatMsg type, Player const* source)
{
    MaNGOS::BattleGroundChatBuilder bg_builder(type, entry, source);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

/**
 * @brief Sends a yell to all players in the battleground.
 *
 * @param entry The entry ID of the yell.
 * @param language The language of the yell.
 * @param guid The GUID of the creature yelling.
 */
void BattleGround::SendYellToAll(int32 entry, uint32 language, ObjectGuid guid)
{
    Creature* source = GetBgMap()->GetCreature(guid);
    if (!source)
    {
        return;
    }
    MaNGOS::BattleGroundYellBuilder bg_builder(Language(language), entry, source);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGroundYellBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

/**
 * @brief Sends a formatted message to all players in the battleground.
 *
 * @param entry The entry ID of the message.
 * @param type The type of chat message.
 * @param source The source player of the message.
 * @param ... The arguments for the formatted message.
 */
void BattleGround::PSendMessageToAll(int32 entry, ChatMsg type, Player const* source, ...)
{
    va_list ap;
    va_start(ap, source);

    MaNGOS::BattleGroundChatBuilder bg_builder(type, entry, source, &ap);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGroundChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);

    va_end(ap);
}

/**
 * @brief Sends a formatted message with two arguments to all players in the battleground.
 *
 * @param entry The entry ID of the message.
 * @param type The type of chat message.
 * @param source The source player of the message.
 * @param arg1 The first argument for the formatted message.
 * @param arg2 The second argument for the formatted message.
 */
void BattleGround::SendMessage2ToAll(int32 entry, ChatMsg type, Player const* source, int32 arg1, int32 arg2)
{
    MaNGOS::BattleGround2ChatBuilder bg_builder(type, entry, source, arg1, arg2);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGround2ChatBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

/**
 * @brief Sends a formatted yell with two arguments to all players in the battleground.
 *
 * @param entry The entry ID of the yell.
 * @param language The language of the yell.
 * @param guid The GUID of the creature yelling.
 * @param arg1 The first argument for the formatted yell.
 * @param arg2 The second argument for the formatted yell.
 */
void BattleGround::SendYell2ToAll(int32 entry, uint32 language, ObjectGuid guid, int32 arg1, int32 arg2)
{
    Creature* source = GetBgMap()->GetCreature(guid);
    if (!source)
    {
        return;
    }
    MaNGOS::BattleGround2YellBuilder bg_builder(language, entry, source, arg1, arg2);
    MaNGOS::LocalizedPacketDo<MaNGOS::BattleGround2YellBuilder> bg_do(bg_builder);
    BroadcastWorker(bg_do);
}

/**
 * @brief Ends the battleground immediately.
 */
void BattleGround::EndNow()
{
    RemoveFromBGFreeSlotQueue();
    SetStatus(STATUS_WAIT_LEAVE);
    SetEndTime(0);
}

/**
 * @brief Handles the triggering of a buff in the battleground.
 *
 * @param go_guid The GUID of the game object representing the buff.
 */
void BattleGround::HandleTriggerBuff(ObjectGuid go_guid)
{
    GameObject* obj = GetBgMap()->GetGameObject(go_guid);
    if (!obj || obj->GetGoType() != GAMEOBJECT_TYPE_TRAP || !obj->isSpawned())
    {
        return;
    }

    obj->SetLootState(GO_JUST_DEACTIVATED);             // can be despawned or destroyed
    return;
}

/**
 * @brief Handles the event of a player being killed in the battleground.
 *
 * @param player The player who was killed.
 * @param killer The player who killed the other player.
 */
void BattleGround::HandleKillPlayer(Player* player, Player* killer)
{
    // add +1 deaths
    UpdatePlayerScore(player, SCORE_DEATHS, 1);

    // add +1 kills to group and +1 killing_blows to killer
    if (killer)
    {
        UpdatePlayerScore(killer, SCORE_HONORABLE_KILLS, 1);
        UpdatePlayerScore(killer, SCORE_KILLING_BLOWS, 1);

        for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        {
            Player* plr = sObjectMgr.GetPlayer(itr->first);

            if (!plr || plr == killer)
            {
                continue;
            }

            if (plr->GetTeam() == killer->GetTeam() && plr->IsAtGroupRewardDistance(player))
            {
                UpdatePlayerScore(plr, SCORE_HONORABLE_KILLS, 1);
            }
        }
    }

    player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SKINNABLE);
}

/**
 * @brief Returns the player's team based on battleground player info.
 * Used mainly in same faction arena matches.
 *
 * @param guid The GUID of the player.
 * @returns The team of the player.
 */
Team BattleGround::GetPlayerTeam(ObjectGuid guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
    {
        return itr->second.PlayerTeam;
    }
    return TEAM_NONE;
}

/**
 * @brief Determines whether a player is in the battleground.
 *
 * @param guid The GUID of the player.
 * @returns True if the player is in the battleground, false otherwise.
 */
bool BattleGround::IsPlayerInBattleGround(ObjectGuid guid)
{
    BattleGroundPlayerMap::const_iterator itr = m_Players.find(guid);
    if (itr != m_Players.end())
    {
        return true;
    }
    return false;
}

/**
 * @brief Checks if the battleground is running and updates the player's status accordingly.
 *
 * @param plr The player to check and update.
 */
void BattleGround::PlayerAddedToBGCheckIfBGIsRunning(Player* plr)
{
    if (GetStatus() != STATUS_WAIT_LEAVE)
    {
        return;
    }

    WorldPacket data;
    BattleGroundQueueTypeId bgQueueTypeId = BattleGroundMgr::BGQueueTypeId(GetTypeID());

    BlockMovement(plr);

    sBattleGroundMgr.BuildPvpLogDataPacket(&data, this);
    plr->GetSession()->SendPacket(&data);

    sBattleGroundMgr.BuildBattleGroundStatusPacket(&data, this, plr->GetBattleGroundQueueIndex(bgQueueTypeId), STATUS_IN_PROGRESS, GetEndTime(), GetStartTime());
    plr->GetSession()->SendPacket(&data);
}

/**
 * @brief Gets the count of alive players by team.
 *
 * @param team The team to get the count for.
 * @returns The count of alive players in the specified team.
 */
uint32 BattleGround::GetAlivePlayersCountByTeam(Team team) const
{
    int count = 0;
    for (BattleGroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
    {
        if (itr->second.PlayerTeam == team)
        {
            Player* pl = sObjectMgr.GetPlayer(itr->first);
            if (pl && pl->IsAlive())
            {
                ++count;
            }
        }
    }
    return count;
}

/**
 * @brief Sets the battleground raid group for a team.
 *
 * @param team The team to set the raid group for.
 * @param bg_raid The raid group to set.
 */
void BattleGround::SetBgRaid(Team team, Group* bg_raid)
{
    Group*& old_raid = m_BgRaids[GetTeamIndexByTeamId(team)];

    if (old_raid)
    {
        old_raid->SetBattlegroundGroup(NULL);
    }

    if (bg_raid)
    {
        bg_raid->SetBattlegroundGroup(this);
    }

    old_raid = bg_raid;
}

/**
 * @brief Gets the closest graveyard for a player.
 *
 * @param player The player to get the closest graveyard for.
 * @returns The closest graveyard entry.
 */
WorldSafeLocsEntry const* BattleGround::GetClosestGraveYard(Player* player)
{
    return sObjectMgr.GetClosestGraveYard(player->GetPositionX(), player->GetPositionY(), player->GetPositionZ(), player->GetMapId(), player->GetTeam());
}

/**
 * @brief Gets the winner in case of a premature finish of the battleground.
 * Different battlegrounds may have different criteria for choosing the winner besides simple player accounting.
 *
 * @returns The winner team.
 */
Team BattleGround::GetPrematureWinner()
{
    uint32 hordePlayers = GetPlayersCountByTeam(HORDE);
    uint32 alliancePlayers = GetPlayersCountByTeam(ALLIANCE);

    if (alliancePlayers > hordePlayers)
    {
        return ALLIANCE;
    }

    if (hordePlayers > alliancePlayers)
    {
        return HORDE;
    }

    return TEAM_NONE;
}
