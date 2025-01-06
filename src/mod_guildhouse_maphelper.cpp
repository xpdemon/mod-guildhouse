//
// Created by bruno boujenah on 03/01/2025.
//


#include "MapMgr.h"
#include "mod_guildhouse.h"
#include "Player.h"
#include "ScriptObject.h"


class MapHelper : public MapScript<Map> {
public:
    explicit MapHelper(const uint32 mapId) : MapScript(mapId) {
    }

    static GuildHouse_Utils::GuildData *GetGuildData(Player *player) {
        auto *guildData = player->CustomData.GetDefault<GuildHouse_Utils::GuildData>("phase");
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
        return guildData;
    }

    static void InitMap(Player *player, const Map *map, const GuildHouse_Utils::GuildData *guild_data) {
        if (guild_data->firstVisit) {
            CharacterDatabase.Query(
                "Update `guild_house` SET `instanceId` = {}, `firstVisit` ={}  WHERE `guild`={}",
                player->GetInstanceId(), false, player->GetGuild()->GetId());
            CharacterDatabase.Query(
                "INSERT INTO `character_instance` (`guid`, `instance`,`extended`) "
                "SELECT gm.`guid`, gh.`instanceId`,0 "
                "FROM `guild_member` gm "
                "JOIN `guild_house` gh ON gm.`guildid` = gh.`guild`"
                "WHERE gm.`guildid` = {} AND NOT gm.`guid`= {}",
                player->GetGuild()->GetId(), player->GetGUID());

            GuildHouse_Utils::SpawnStarterPortal(player, map->GetId());
            GuildHouse_Utils::SpawnButlerNPC(player, map->GetId());
        }
    }

    void OnPlayerEnter(Map *map, Player *player) override {
        const auto *guildData = GetGuildData(player);
        InitMap(player, map, guildData);
        player->SetPhaseMask(guildData->phase, true);
        player->SetRestState(0);
    }


    void OnPlayerLeave(Map *, Player *player) override {
        player->RemoveRestState();
        player->SetPhaseMask(GuildHouse_Utils::GetNormalPhase(player), true);
    };


};


void AddGuildHouseMapScripts() {
    new MapHelper(44);
}
