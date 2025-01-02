#include "ScriptMgr.h"
#include "Player.h"
#include "Configuration/Config.h"
#include "Creature.h"
#include "Guild.h"
#include "SpellAuraEffects.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "GuildMgr.h"
#include "Define.h"
#include "GossipDef.h"
#include "DataMap.h"
#include "GameObject.h"
#include "Transport.h"
#include "Maps/MapMgr.h"
#include "mod_guildhouse.h"

class GuildData : public DataMap::Base
{
public:
    GuildData() {}
    GuildData(uint32 phase, float posX, float posY, float posZ, float ori) : phase(phase), posX(posX), posY(posY), posZ(posZ), ori(ori) {}
    uint32 phase;
    float posX;
    float posY;
    float posZ;
    float ori;
};

class GuildHelper : public GuildScript
{

public:
    GuildHelper() : GuildScript("GuildHelper") {}

    void OnCreate(Guild* /*guild*/, Player* leader, const std::string& /*name*/) override
    {
        ChatHandler(leader->GetSession()).PSendSysMessage("Félicitations ! Vous possédez maintenant une guilde. Vous pouvez acheter une Maison de Guilde !");
    }

    void OnDisband(Guild* guild) override
    {

        if (RemoveGuildHouse(guild))
        {
            LOG_INFO("modules", "GUILDHOUSE: Deleting Guild House data due to disbanding of guild...");
        }
        else
        {
            LOG_INFO("modules", "GUILDHOUSE: Error deleting Guild House data during disbanding of guild!!");
        }
    }

    static bool RemoveGuildHouse(const Guild* guild)
    {
        uint32 guildPhase = GuildHouse_Utils::GetGuildPhase(guild);

        // Lets find all of the gameobjects to be removed
        QueryResult GameobjResult = WorldDatabase.Query("SELECT `guid` FROM `gameobject` WHERE `map`=1 AND `phaseMask`={}",
                                                        guildPhase);
        // Lets find all of the creatures to be removed
        QueryResult CreatureResult = WorldDatabase.Query("SELECT `guid` FROM `creature` WHERE `map`=1 AND `phaseMask`={}",
                                                         guildPhase);

        Map* map = sMapMgr->FindMap(1, 0);
        // Remove creatures from the deleted guild house map
        if (CreatureResult)
        {
            do
            {
                Field* fields = CreatureResult->Fetch();
                uint32 lowguid = fields[0].Get<int32>();
                if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
                {
                    if (Creature* creature = map->GetCreature(ObjectGuid::Create<HighGuid::Unit>(cr_data->id1, lowguid)))
                    {
                        creature->CombatStop();
                        creature->DeleteFromDB();
                        creature->AddObjectToRemoveList();
                    }
                }
            } while (CreatureResult->NextRow());
        }

        // Remove gameobjects from the deleted guild house map
        if (GameobjResult)
        {
            do
            {
                Field *fields = GameobjResult->Fetch();
                uint32 lowguid = fields[0].Get<int32>();
                if (GameObjectData const* go_data = sObjectMgr->GetGameObjectData(lowguid))
                {
                    if (GameObject* gobject = map->GetGameObject(ObjectGuid::Create<HighGuid::GameObject>(go_data->id, lowguid)))
                    {
                        gobject->SetRespawnTime(0);
                        gobject->Delete();
                        gobject->DeleteFromDB();
                        gobject->CleanupsBeforeDelete();
                        // delete gobject;
                    }
                }

            } while (GameobjResult->NextRow());
        }

        // Delete actual guild_house data from characters database
        CharacterDatabase.Query("DELETE FROM `guild_house` WHERE `guild`={}", guild->GetId());

        return true;
    }
};

class GuildHouseSeller : public CreatureScript
{

public:
    GuildHouseSeller() : CreatureScript("GuildHouseSeller") {}

    struct GuildHouseSellerAI : public ScriptedAI
    {
        explicit GuildHouseSellerAI(Creature* creature) : ScriptedAI(creature) {}

        void UpdateAI(uint32 /*diff*/) override
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    };

    CreatureAI * GetAI(Creature* creature) const override
    {
        return new GuildHouseSellerAI(creature);
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (!player->GetGuild())
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous n'êtes pas membre d'une guilde.");
            CloseGossipMenuFor(player);
            return false;
        }

        QueryResult has_gh = CharacterDatabase.Query("SELECT id, `guild` FROM `guild_house` WHERE guild = {}", player->GetGuildId());

        // Only show Teleport option if guild owns a guild house
        if (has_gh)
        {
            AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Téléportation à la maison de guilde", GOSSIP_SENDER_MAIN, 1);

            // Only show "Sell" option if they have a guild house & have permission to sell it
            Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId());
            Guild::Member const* memberMe = guild->GetMember(player->GetGUID());
            if (memberMe->IsRankNotLower(sConfigMgr->GetOption<int32>("GuildHouseSellRank", 0)))
            {
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Vendre la maison de guilde!", GOSSIP_SENDER_MAIN, 3, "Êtes vous certain de vouloir vendre la maison de guilde?", 0, false);
            }
        }
        else
        {
            // Only leader of the guild can buy guild house & only if they don't already have a guild house
            if (player->GetGuild()->GetLeaderGUID() == player->GetGUID())
            {
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Acheter une maison de guilde!", GOSSIP_SENDER_MAIN, 2);
            }
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Close", GOSSIP_SENDER_MAIN, 5);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        uint32 map;
        float posX;
        float posY;
        float posZ;
        float ori;

        switch (action)
        {
        case 100: // GM Island
            map = 1;
            posX = 16222.972f;
            posY = 16267.802f;
            posZ = 13.136777f;
            ori = 1.461173f;
            break;
        case 101: // Monastery
            map = 44;
            posX = 266.095f;
            posY = -99.3857f;
            posZ = 18.6794f;
            ori = 1.46117f;
        case 5: // close
            CloseGossipMenuFor(player);
            break;
        case 4: // --- MORE TO COME ---
            BuyGuildHouse(player->GetGuild(), player, creature);
            break;
        case 3: // sell back guild house
        {
            QueryResult has_gh = CharacterDatabase.Query("SELECT id, `guild` FROM `guild_house` WHERE guild={}", player->GetGuildId());
            if (!has_gh)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Votre guilde à maintenant une maison!");
                CloseGossipMenuFor(player);
                return false;
            }

            // calculate total gold returned: 1) cost of guild house and cost of each purchase made
            if (RemoveGuildHouse(player))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("You have successfully sold your Guild House.");
                player->GetGuild()->BroadcastToGuild(player->GetSession(), false, "Nous venons de vendre notre Maison de Guilde.", LANG_UNIVERSAL);
                player->ModifyMoney(+(sConfigMgr->GetOption<int32>("CostGuildHouse", 10000000) / 2));
                LOG_INFO("modules", "GUILDHOUSE: Successfully returned money and sold Guild House");
                CloseGossipMenuFor(player);
            }
            else
            {
                ChatHandler(player->GetSession()).PSendSysMessage("There was an error selling your Guild House.");
                CloseGossipMenuFor(player);
            }
            break;
        }
        case 2: // buy guild house
            BuyGuildHouse(player->GetGuild(), player, creature);
            break;
        case 1: // teleport to guild house
            TeleportGuildHouse(player->GetGuild(), player, creature);
            break;
        default: // close
            CloseGossipMenuFor(player);
            break;
        }

        if (action >= 100)
        {
            CharacterDatabase.Query("INSERT INTO `guild_house` (guild, phase, map, positionX, positionY, positionZ, orientation) VALUES ({}, {}, {}, {}, {}, {}, {})", player->GetGuildId(), GuildHouse_Utils::GetGuildPhase(player), map, posX, posY, posZ, ori);
            player->ModifyMoney(-(sConfigMgr->GetOption<int32>("CostGuildHouse", 10000000)));
            // Msg to purchaser and Msg Guild as purchaser
            ChatHandler(player->GetSession()).PSendSysMessage("Vous avez acheté une Maison de Guilde avec succès.");
            player->GetGuild()->BroadcastToGuild(player->GetSession(), false, "We now have a Guild House!", LANG_UNIVERSAL);
            player->GetGuild()->BroadcastToGuild(player->GetSession(), false, "Dans le chat, tapez .guildhouse teleport ou .gh tele pour me rejoindre là-bas !", LANG_UNIVERSAL);
            LOG_INFO("modules", "GUILDHOUSE: GuildId: '{}' has purchased a guildhouse", player->GetGuildId());

            // Spawn a portal and the guild house butler automatically as part of purchase.
            SpawnStarterPortal(player, map);
            SpawnButlerNPC(player, map);
            CloseGossipMenuFor(player);
        }

        return true;
    }


    static bool RemoveGuildHouse(const Player* player)
    {

        uint32 guildPhase = GuildHouse_Utils::GetGuildPhase(player);
        Map *map = sMapMgr->FindMap(1, 0);
        // Lets find all of the gameobjects to be removed
        QueryResult GameobjResult = WorldDatabase.Query(
            "SELECT `guid` FROM `gameobject` WHERE `map` = 1 AND `phaseMask` = '{}'", guildPhase);
        // Lets find all of the creatures to be removed
        QueryResult CreatureResult = WorldDatabase.Query(
            "SELECT `guid` FROM `creature` WHERE `map` = 1 AND `phaseMask` = '{}'", guildPhase);

        // Remove creatures from the deleted guild house map
        if (CreatureResult)
        {
            do
            {
                Field* fields = CreatureResult->Fetch();
                uint32 lowguid = fields[0].Get<uint32>();
                if (CreatureData const* cr_data = sObjectMgr->GetCreatureData(lowguid))
                {
                    if (Creature* creature = map->GetCreature(ObjectGuid::Create<HighGuid::Unit>(cr_data->id1, lowguid)))
                    {
                        creature->CombatStop();
                        creature->DeleteFromDB();
                        creature->AddObjectToRemoveList();
                    }
                }
            } while (CreatureResult->NextRow());
        }

        // Remove gameobjects from the deleted guild house map
        if (GameobjResult)
        {
            do
            {
                Field* fields = GameobjResult->Fetch();
                uint32 lowguid = fields[0].Get<uint32>();
                if (GameObjectData const* go_data = sObjectMgr->GetGameObjectData(lowguid))
                {
                    if (GameObject* gobject = map->GetGameObject(ObjectGuid::Create<HighGuid::GameObject>(go_data->id, lowguid)))
                    {
                        gobject->SetRespawnTime(0);
                        gobject->Delete();
                        gobject->DeleteFromDB();
                        gobject->CleanupsBeforeDelete();
                        // delete gobject;
                    }
                }

            } while (GameobjResult->NextRow());
        }

        // Delete actual guild_house data from characters database
        CharacterDatabase.Query("DELETE FROM `guild_house` WHERE `guild`={}", player->GetGuildId());

        return true;
    }

    static void SpawnStarterPortal(Player* player,uint32 mapId)
    {

        uint32 entry = 0;
        float posX;
        float posY;
        float posZ;
        float ori;

        Map* map = sMapMgr->FindMap(mapId, 0);

        if (player->GetTeamId() == TEAM_ALLIANCE)
        {
            // Portal to Stormwind
            entry = 500000;
        }
        else
        {
            // Portal to Orgrimmar
            entry = 500004;
        }

        if (entry == 0)
        {
            LOG_INFO("modules", "Error with SpawnStarterPortal in GuildHouse Module!");
            return;
        }

        QueryResult result = WorldDatabase.Query("SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={}", entry);

        if (!result)
        {
            LOG_INFO("modules", "GUILDHOUSE: Unable to find data on portal for entry: {}", entry);
            return;
        }

        do
        {
            Field* fields = result->Fetch();
            posX = fields[0].Get<float>();
            posY = fields[1].Get<float>();
            posZ = fields[2].Get<float>();
            ori = fields[3].Get<float>();

        } while (result->NextRow());

        uint32 objectId = entry;
        if (!objectId)
        {
            LOG_INFO("modules", "GUILDHOUSE: objectId IS NULL, should be '{}'", entry);
            return;
        }

        const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);

        if (!objectInfo)
        {
            LOG_INFO("modules", "GUILDHOUSE: objectInfo is NULL!");
            return;
        }

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
        {
            LOG_INFO("modules", "GUILDHOUSE: Unable to find displayId??");
            return;
        }

        GameObject* object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
        ObjectGuid::LowType guidLow = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();

        if (!object->Create(guidLow, objectInfo->entry, map, GuildHouse_Utils::GetGuildPhase(player), posX, posY, posZ, ori, G3D::Quat(), 0, GO_STATE_READY))
        {
            delete object;
            LOG_INFO("modules", "GUILDHOUSE: Unable to create object!!");
            return;
        }

        // fill the gameobject data and save to the db
        object->SaveToDB(sMapMgr->FindMap(1, 0)->GetId(), (1 << sMapMgr->FindMap(1, 0)->GetSpawnMode()), GuildHouse_Utils::GetGuildPhase(player));
        guidLow = object->GetSpawnId();
        // delete the old object and do a clean load from DB with a fresh new GameObject instance.
        // this is required to avoid weird behavior and memory leaks
        delete object;

        object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
        // this will generate a new guid if the object is in an instance
        if (!object->LoadGameObjectFromDB(guidLow, sMapMgr->FindMap(1, 0), true))
        {
            delete object;
            return;
        }

        // TODO: is it really necessary to add both the real and DB table guid here ?
        sObjectMgr->AddGameobjectToGrid(guidLow, sObjectMgr->GetGameObjectData(guidLow));
        CloseGossipMenuFor(player);
    }

    static void SpawnButlerNPC(const Player* player, uint32 mapId)
    {
        uint32 entry = 500031;
        float posX = 16202.185547f;
        float posY = 16255.916992f;
        float posZ = 21.160221f;
        float ori = 6.195375f;

        Map* map = sMapMgr->FindMap(mapId, 0);
        auto *creature = new Creature();

        if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, player->GetPhaseMaskForSpawn(), entry, 0, posX, posY, posZ, ori))
        {
            delete creature;
            return;
        }
        creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), GuildHouse_Utils::GetGuildPhase(player));
        uint32 lowguid = creature->GetSpawnId();

        creature->CleanupsBeforeDelete();
        delete creature;
        creature = new Creature();
        if (!creature->LoadCreatureFromDB(lowguid, map))
        {
            delete creature;
            return;
        }

        sObjectMgr->AddCreatureToGrid(lowguid, sObjectMgr->GetCreatureData(lowguid));

    }

    static bool BuyGuildHouse(const Guild* guild, Player* player, const Creature* creature)
    {
        QueryResult result = CharacterDatabase.Query("SELECT `id`, `guild` FROM `guild_house` WHERE `guild`={}", guild->GetId());

        if (result)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Votre guilde possède déjà une Maison de Guilde, vous ne pouvez pas en acheter une autre.");
            CloseGossipMenuFor(player);
            return false;
        }

        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "GM Island", GOSSIP_SENDER_MAIN, 100, "Buy Guild House on GM Island?", sConfigMgr->GetOption<int32>("CostGuildHouse", 10000000), false);
        // Removing this tease for now, as right now the phasing code is specific go GM Island, so it's not a simple thing to add new areas yet.
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Un Charmant monastère", GOSSIP_SENDER_MAIN, 101, "Acheter une Maison de Guilde dans un charmant monastère?", sConfigMgr->GetOption<int32>("CostGuildHouse", 10000000), false);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    static void TeleportGuildHouse(const Guild* guild, Player* player, const Creature* creature)
    {
        auto *guildData = player->CustomData.GetDefault<GuildData>("phase");
        const QueryResult result = CharacterDatabase.Query("SELECT `phase`, `map`,`positionX`, `positionY`, `positionZ`, `orientation` FROM `guild_house` WHERE `guild`={}", guild->GetId());

        if (!result)
        {
            ClearGossipMenuFor(player);
            if (player->GetGuild()->GetLeaderGUID() == player->GetGUID())
            {
                // Only leader of the guild can buy / sell guild house
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Acheter une maison de guilde!", GOSSIP_SENDER_MAIN, 2);
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Vendre la maison de guilde!", GOSSIP_SENDER_MAIN, 3, "Êtes vous certain de vouloir vendre la maison de guilde?", 0, false);
            }

            AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Se rendre à la maison de guilde", GOSSIP_SENDER_MAIN, 1);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Fermer", GOSSIP_SENDER_MAIN, 5);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            ChatHandler(player->GetSession()).PSendSysMessage("Votre guilde ne possède pas de Maison de Guilde. Vous pouvez en acheter une dès maintenant !");
            return;
        }

        do
        {

            Field* fields = result->Fetch();
            guildData->phase = fields[0].Get<uint32>();
            uint32 map = fields[1].Get<uint32>();
            guildData->posX = fields[2].Get<float>();
            guildData->posY = fields[3].Get<float>();
            guildData->posZ = fields[4].Get<float>();
            guildData->ori = fields[5].Get<float>();

            player->TeleportTo(map, guildData->posX, guildData->posY, guildData->posZ, guildData->ori);

        } while (result->NextRow());
    }
};

class GuildHousePlayerScript : public PlayerScript
{
public:
    GuildHousePlayerScript() : PlayerScript("GuildHousePlayerScript") {}

    void OnLogin(Player* player) override
    {
        CheckPlayer(player);
    }


    void OnMapChanged(Player * player) override
    {
         if (player -> GetMapId() == 44)
             CheckPlayer(player);
         else
             player->SetPhaseMask(GetNormalPhase(player), true);
    }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        if (newZone == 876)
            CheckPlayer(player);
        else
            player->SetPhaseMask(GetNormalPhase(player), true);
    }

    bool OnBeforeTeleport(Player* player, uint32 mapid, float x, float y, float z, float orientation, uint32 options, Unit* target) override
    {
        (void)mapid;
        (void)x;
        (void)y;
        (void)z;
        (void)orientation;
        (void)options;
        (void)target;

        if (player->GetZoneId() == 876 && player->GetAreaId() == 876) // GM Island
        {
            // Remove the rested state when teleporting from the guild house
            player->RemoveRestState();
        }

        return true;
    }

    static uint32 GetNormalPhase(const Player* player)
    {
        if (player->IsGameMaster())
            return PHASEMASK_ANYWHERE;

        uint32 phase = player->GetPhaseByAuras();
        if (!phase)
            return PHASEMASK_NORMAL;
        else
            return phase;
    }

    static void CheckPlayer(Player* player)
    {
        auto* guildData = player->CustomData.GetDefault<GuildData>("phase");
        QueryResult result = CharacterDatabase.Query("SELECT `id`, `guild`, `phase`, `map`,`positionX`, `positionY`, `positionZ`, `orientation` FROM guild_house WHERE `guild` = {}", player->GetGuildId());

        if (result)
        {
            do
            {
                // commented out due to travis, but keeping for future expansion into other areas
                Field *fields = result->Fetch();
                // uint32 id = fields[0].Get<uint32>();        // fix for travis
                // uint32 guild = fields[1].Get<uint32>();     // fix for travis
                guildData->phase = fields[2].Get<uint32>();
                // uint32 map = fields[3].Get<uint32>();       // fix for travis
                // guildData->posX = fields[4].Get<float>();   // fix for travis
                // guildData->posY = fields[5].Get<float>();   // fix for travis
                // guildData->posZ = fields[6].Get<float>();   // fix for travis
                // guildData->ori = fields[7].Get<float>();   // fix for travis

            } while (result->NextRow());
        }

        if (player->GetZoneId() == 876 && player->GetAreaId() == 876) // GM Island
        {
            // Set the guild house as a rested area
            player->SetRestState(0);

            // If player is not in a guild he doesnt have a guild house teleport away
            // TODO: What if they are in a guild, but somehow are in the wrong phaseMask and seeing someone else's area?

            if (!result || !player->GetGuild())
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Votre guilde ne possède pas de Maison de Guilde. Vous pouvez en acheter une dès maintenant !");
                teleportToDefault(player);
                return;
            }

            player->SetPhaseMask(guildData->phase, true);
        }
        else if (player->GetZoneId() == 44)
        {
            player->SetRestState(0);
            player->SetPhaseMask(guildData->phase, true);

            if (!result || !player->GetGuild())
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Votre guilde ne possède pas de Maison de Guilde. Vous pouvez en acheter une dès maintenant !");
                teleportToDefault(player);

            }
        }

        else
            player->SetPhaseMask(GetNormalPhase(player), true);
    }

    static void teleportToDefault(Player* player)
    {
        if (player->GetTeamId() == TEAM_ALLIANCE)
            player->TeleportTo(0, -8833.379883f, 628.627991f, 94.006599f, 1.0f);
        else
            player->TeleportTo(1, 1486.048340f, -4415.140625f, 24.187496f, 0.13f);
    }
};

using namespace Acore::ChatCommands;

class GuildHouseCommand : public CommandScript
{
public:
    GuildHouseCommand() : CommandScript("GuildHouseCommand") {}

    [[nodiscard]] ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable GuildHouseCommandTable =
        {
            {"teleport", HandleGuildHouseTeleCommand, SEC_PLAYER, Console::Yes},
            {"butler", HandleSpawnButlerCommand, SEC_PLAYER, Console::Yes},
        };

        static ChatCommandTable GuildHouseCommandBaseTable =
        {
            {"guildhouse", GuildHouseCommandTable},
            {"gh", GuildHouseCommandTable}
        };

        return GuildHouseCommandBaseTable;
    }

    static uint32 GetGuildPhase(const Player* player)
    {
        // Version "simple"
        //return player->GetGuildId() + 10;

        // Version "bitmask", si vous voulez
        if (player->GetGuildId() == 0)
            return 0;
        return 1 << (player->GetGuildId() - 1);
    }

    static bool HandleSpawnButlerCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();
        Map* map = player->GetMap();

        if (!player->GetGuild() || (player->GetGuild()->GetLeaderGUID() != player->GetGUID()))
        {
            handler->SendSysMessage("Vous devez être le Maître de Guilde d’une guilde pour utiliser cette commande !");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->GetAreaId() != 876)
        {
            handler->SendSysMessage("You must be in your Guild House to use this command!");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->FindNearestCreature(500031, VISIBLE_RANGE, true))
        {
            handler->SendSysMessage("Vous avez déjà le Majordome de la Maison de Guilde !");
            handler->SetSentErrorMessage(true);
            return false;
        }

        float posX = 16202.185547f;
        float posY = 16255.916992f;
        float posZ = 21.160221f;
        float ori = 6.195375f;

        auto *creature = new Creature();
        if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, GetGuildPhase(player), 500031, 0, posX, posY, posZ, ori))
        {
            handler->SendSysMessage("Vous avez déjà le Majordome de la Maison de Guilde !");
            handler->SetSentErrorMessage(true);
            delete creature;
            return false;
        }
        creature->SaveToDB(player->GetMapId(), (1 << player->GetMap()->GetSpawnMode()), GetGuildPhase(player));
        uint32 lowguid = creature->GetSpawnId();

        creature->CleanupsBeforeDelete();
        delete creature;
        creature = new Creature();
        if (!creature->LoadCreatureFromDB(lowguid, player->GetMap()))
        {
            handler->SendSysMessage("Something went wrong when adding the Butler.");
            handler->SetSentErrorMessage(true);
            delete creature;
            return false;
        }

        sObjectMgr->AddCreatureToGrid(lowguid, sObjectMgr->GetCreatureData(lowguid));
        return true;
    }

    static bool HandleGuildHouseTeleCommand(ChatHandler* handler)
    {
        Player* player = handler->GetSession()->GetPlayer();

        if (!player)
            return false;

        if (player->IsInCombat())
        {
            handler->SendSysMessage("You can't use this command while in combat!");
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto *guildData = player->CustomData.GetDefault<GuildData>("phase");
        QueryResult result = CharacterDatabase.Query("SELECT `id`, `guild`, `phase`, `map`,`positionX`, `positionY`, `positionZ`, `orientation` FROM `guild_house` WHERE `guild`={}", player->GetGuildId());

        if (!result)
        {
            handler->SendSysMessage("Your guild does not own a Guild House!");
            handler->SetSentErrorMessage(true);
            return false;
        }

        do
        {
            Field* fields = result->Fetch();
            // uint32 id = fields[0].Get<uint32>();        // fix for travis
            // uint32 guild = fields[1].Get<uint32>();     // fix for travis
            guildData->phase = fields[2].Get<uint32>();
            uint32 map = fields[3].Get<uint32>();
            guildData->posX = fields[4].Get<float>();
            guildData->posY = fields[5].Get<float>();
            guildData->posZ = fields[6].Get<float>();
            guildData->ori = fields[7].Get<float>();


            player->TeleportTo(map, guildData->posX, guildData->posY, guildData->posZ, guildData->ori);

        } while (result->NextRow());

        return true;
    }
};

class GuildHouseGlobal : public GlobalScript
{
public:
    GuildHouseGlobal() : GlobalScript("GuildHouseGlobal") {}

    void OnBeforeWorldObjectSetPhaseMask(WorldObject const* worldObject, uint32 & /*oldPhaseMask*/, uint32 & /*newPhaseMask*/, bool &useCombinedPhases, bool & /*update*/) override
    {
        if (worldObject->GetZoneId() == 876)
            useCombinedPhases = false;
        else
            useCombinedPhases = true;
    }
};

void AddGuildHouseScripts()
{
    new GuildHelper();
    new GuildHouseSeller();
    new GuildHousePlayerScript();
    new GuildHouseCommand();
    new GuildHouseGlobal();
}