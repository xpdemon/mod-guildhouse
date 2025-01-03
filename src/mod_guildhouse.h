//
// Created by boujenah bruno on 02/01/2025.
//

#ifndef MOD_GUILDHOUSE_H
#define MOD_GUILDHOUSE_H
#include "Define.h"
#include "Guild.h"


class GuildHouse_Utils final {

public:
    virtual ~GuildHouse_Utils() = default;
    static uint32 GetGuildPhase(const Guild* guild )
    {
        // Version "simple"
        //return player->GetGuildId() + 10;

        // Version "bitmask", si vous voulez
        if (guild->GetId() == 0)
            return 0;
        return 1 << (guild->GetId() - 1);
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

        if (!object->Create(guidLow, objectInfo->entry, map, GuildHouse_Utils::GetGuildPhase(player), posX, posY, posZ,
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

};
#endif //MOD_GUILDHOUSE_H
