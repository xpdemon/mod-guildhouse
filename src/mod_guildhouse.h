//
// Created by boujenah bruno on 02/01/2025.
//

#ifndef MOD_GUILDHOUSE_H
#define MOD_GUILDHOUSE_H
#include "Define.h"
#include "Guild.h"
#include "MapMgr.h"
#include "GameObject.h"
#include "Transport.h"
#include "ScriptedGossip.h"
#include "GossipDef.h"
#include "Chat.h"



class GuildHouse_Utils {
public:
    virtual ~GuildHouse_Utils() = default;
    class GuildData : public DataMap::Base {
    public:
        GuildData(): id(0),
        guild(0),
        phase(0),
        map(0),
        posX(0),
        posY(0),
        posZ(0),
        ori(0),
        instanceId(0),
        firstVisit(false) {
        }

        GuildData(uint32 id,uint32 guild,uint32 phase, float posX, float posY, float posZ, float ori) : id(0), guild(0),
            phase(phase),
            map(0), posX(posX), posY(posY),
            posZ(posZ), ori(ori), instanceId(0), firstVisit(true) {
        }

        uint32 id;
        uint32 guild;
        uint32 phase;
        uint32 map;
        float posX;
        float posY;
        float posZ;
        float ori;
        uint32 instanceId;
        bool firstVisit;
    };
    static GuildData *GetGuildData(Player *player) {
        auto *guildData = player->CustomData.GetDefault<GuildData>("phase");
        QueryResult result = CharacterDatabase.Query(
            "SELECT `id`, `guild`, `phase`, `map`,`positionX`, `positionY`, `positionZ`, `orientation`,`instanceId`,`firstVisit` FROM guild_house WHERE `guild` = {}",
            player->GetGuildId());

        if (result) {
            do {
                Field *fields = result->Fetch();
                guildData->id = fields[0].Get<uint32>();
                guildData ->guild = fields[1].Get<uint32>();
                guildData->phase = fields[2].Get<uint32>();
                guildData->map = fields[3].Get<uint32>();
                guildData->posX = fields[4].Get<float>();
                guildData->posY = fields[5].Get<float>();
                guildData->posZ = fields[6].Get<float>();
                guildData->ori = fields[7].Get<float>();
                guildData->instanceId = fields[8].Get<uint32>();
                guildData->firstVisit = fields[9].Get<bool>();
            } while (result->NextRow());
        }
        return guildData;
    }
    static uint32 GetGuildPhase(const Guild *guild) {
        // Version "simple"
        //return player->GetGuildId() + 10;

        // Version "bitmask", si vous voulez
        if (guild->GetId() == 0)
            return 0;
        return 1 << (guild->GetId() - 1);
    }
    static uint32 GetGuildPhase(const Player *player) {
        // Version "simple"
        //return player->GetGuildId() + 10;

        // Version "bitmask", si vous voulez
        if (player->GetGuildId() == 0)
            return 0;
        return 1 << (player->GetGuildId() - 1);
    }
    static void SpawnStarterPortal(Player *player, uint32 mapId) {
        uint32 entry = 0;
        float posX;
        float posY;
        float posZ;
        float ori;


        Map *map = sMapMgr->FindMap(mapId, player->GetInstanceId());

        if (player->GetTeamId() == TEAM_ALLIANCE) {
            // Portal to Stormwind
            entry = 500000;
        } else {
            // Portal to Orgrimmar
            entry = 500004;
        }

        if (entry == 0) {
            LOG_INFO("modules", "Error with SpawnStarterPortal in GuildHouse Module!");
            return;
        }

        QueryResult result = WorldDatabase.Query(
            "SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={} and `map`={}",
            entry, mapId);

        if (!result) {
            LOG_INFO("modules", "GUILDHOUSE: Unable to find data on portal for entry: {}", entry);
            return;
        }

        do {
            Field *fields = result->Fetch();
            posX = fields[0].Get<float>();
            posY = fields[1].Get<float>();
            posZ = fields[2].Get<float>();
            ori = fields[3].Get<float>();
        } while (result->NextRow());

        uint32 objectId = entry;
        if (!objectId) {
            LOG_INFO("modules", "GUILDHOUSE: objectId IS NULL, should be '{}'", entry);
            return;
        }

        const GameObjectTemplate *objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);

        if (!objectInfo) {
            LOG_INFO("modules", "GUILDHOUSE: objectInfo is NULL!");
            return;
        }

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId)) {
            LOG_INFO("modules", "GUILDHOUSE: Unable to find displayId??");
            return;
        }

        GameObject *object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry)
                                 ? new StaticTransport()
                                 : new GameObject();
        ObjectGuid::LowType guidLow = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();

        if (!object->Create(guidLow, objectInfo->entry, map, GetGuildPhase(player), posX, posY, posZ,
                            ori, G3D::Quat(), 0, GO_STATE_READY)) {
            delete object;
            LOG_INFO("modules", "GUILDHOUSE: Unable to create object!!");
            return;
        }

        // fill the gameobject data and save to the db
        object->SaveToDB(sMapMgr->FindMap(mapId, player->GetInstanceId())->GetId(),
                         (1 << sMapMgr->FindMap(mapId, player->GetInstanceId())->GetSpawnMode()),
                         GuildHouse_Utils::GetGuildPhase(player));
        guidLow = object->GetSpawnId();
        // delete the old object and do a clean load from DB with a fresh new GameObject instance.
        // this is required to avoid weird behavior and memory leaks
        delete object;

        object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
        // this will generate a new guid if the object is in an instance
        if (!object->LoadGameObjectFromDB(guidLow, sMapMgr->FindMap(mapId, player->GetInstanceId()), true)) {
            delete object;
            return;
        }

        // TODO: is it really necessary to add both the real and DB table guid here ?
        sObjectMgr->AddGameobjectToGrid(guidLow, sObjectMgr->GetGameObjectData(guidLow));
        CloseGossipMenuFor(player);
    }
    static void SpawnButlerNPC(const Player *player, uint32 mapId) {
        uint32 entry = 500031;
        float posX;
        float posY;
        float posZ;
        float ori;

        Map *map = sMapMgr->FindMap(mapId, player->GetInstanceId());
        auto *creature = new Creature();

        QueryResult result = WorldDatabase.Query(
            "SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={} AND `map`={}",
            entry, mapId);

        if (!result) {
            LOG_INFO("modules", "GUILDHOUSE: Unable to find data for entry: {}", entry);
            return;
        }

        do {
            Field *fields = result->Fetch();
            posX = fields[0].Get<float>();
            posY = fields[1].Get<float>();
            posZ = fields[2].Get<float>();
            ori = fields[3].Get<float>();
        } while (result->NextRow());


        if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, player->GetPhaseMaskForSpawn(), entry, 0,
                              posX, posY, posZ, ori)) {
            delete creature;
            return;
        }
        creature->SaveToDB(map->GetId(), (1 << map->GetSpawnMode()), GetGuildPhase(player));
        uint32 lowguid = creature->GetSpawnId();

        creature->CleanupsBeforeDelete();
        delete creature;
        creature = new Creature();
        if (!creature->LoadCreatureFromDB(lowguid, map)) {
            delete creature;
            return;
        }

        sObjectMgr->AddCreatureToGrid(lowguid, sObjectMgr->GetCreatureData(lowguid));
    }
    static void teleportToDefault(Player *player) {
        if (player->GetTeamId() == TEAM_ALLIANCE)
            player->TeleportTo(0, -8833.379883f, 628.627991f, 94.006599f, 1.0f);
        else
            player->TeleportTo(1, 1486.048340f, -4415.140625f, 24.187496f, 0.13f);
    }
    static uint32 GetNormalPhase(const Player *player) {
        if (player->IsGameMaster())
            return PHASEMASK_ANYWHERE;

        uint32 phase = player->GetPhaseByAuras();
        if (!phase)
            return PHASEMASK_NORMAL;
        else
            return phase;
    }
    static void CheckPlayer(Player *player) {
        auto *guildData = player->CustomData.GetDefault<GuildData>("phase");
        QueryResult result = CharacterDatabase.Query(
            "SELECT `id`, `guild`, `phase`, `map`,`positionX`, `positionY`, `positionZ`, `orientation`,`instanceId`,`firstVisit` FROM guild_house WHERE `guild` = {}",
            player->GetGuildId());

        if (result) {
            do {
                Field *fields = result->Fetch();
                guildData->phase = fields[2].Get<uint32>();
                guildData->instanceId = fields[8].Get<uint32>();
                guildData->firstVisit = fields[9].Get<bool>();
            } while (result->NextRow());
        }

        if (player->GetZoneId() == 876 && player->GetAreaId() == 876) // GM Island
        {
            // Set the guild house as a rested area
            player->SetRestState(0);

            // If player is not in a guild he doesnt have a guild house teleport away
            // TODO: What if they are in a guild, but somehow are in the wrong phaseMask and seeing someone else's area?

            if (!result || !player->GetGuild()) {
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "Votre guilde ne possède pas de Maison de Guilde. Vous pouvez en acheter une dès maintenant !");
                teleportToDefault(player);
                return;
            }

            player->SetPhaseMask(guildData->phase, true);
        } else if (player->GetMapId() == 44) {
            player->SetPhaseMask(guildData->phase, true);

            if (!result || !player->GetGuild()) {
                ChatHandler(player->GetSession()).PSendSysMessage(
                    "Votre guilde ne possède pas de Maison de Guilde. Vous pouvez en acheter une dès maintenant !");
                teleportToDefault(player);
            }
        } else
            player->SetPhaseMask(GetNormalPhase(player), true);
    }


};
class HouseObjectManager {
    public:
        static GameObject *GetStarterPortal(const TeamId team, uint32 mapId) {
            uint32 entry = team == TEAM_ALLIANCE ? 500000 : 500004;
            float posX;
            float posY;
            float posZ;
            float ori;

            QueryResult result = WorldDatabase.Query(
                "SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={} and `map`={}",
                entry, mapId);
            do {
                Field *fields = result->Fetch();
                posX = fields[0].Get<float>();
                posY = fields[1].Get<float>();
                posZ = fields[2].Get<float>();
                ori = fields[3].Get<float>();
            } while (result->NextRow());

            uint32 objectId = entry;

            const GameObjectTemplate *objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);
            GameObject *object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry)
                                     ? new StaticTransport()
                                     : new GameObject();
            object -> SetPosition(posX, posY, posZ, ori);
            return object;

        }
        static GameObject *GetButlerNPC(uint32 mapId) {
            uint32 entry = 500031;
            float posX;
            float posY;
            float posZ;
            float ori;

            QueryResult result = WorldDatabase.Query(
                "SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={} AND `map`={}",
                entry, mapId);

            do {
                Field *fields = result->Fetch();
                posX = fields[0].Get<float>();
                posY = fields[1].Get<float>();
                posZ = fields[2].Get<float>();
                ori = fields[3].Get<float>();
            } while (result->NextRow());

            auto *object = new GameObject();
            object->SetPosition(posX, posY, posZ, ori);
            return object;
        }

    };
#endif //MOD_GUILDHOUSE_H
