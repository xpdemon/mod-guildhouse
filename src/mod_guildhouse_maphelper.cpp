//
// Created by xpdemon on 03/01/2025.
//

#include "MapMgr.h"
#include "ScriptMgr.h"
#include "Player.h"
#include "Configuration/Config.h"
#include "Guild.h"
#include "Define.h"
#include "mod_guildhouse.h"




class MapHelper : public MapScript<Map> {
public:
    explicit MapHelper(const uint32 mapId) : MapScript(mapId) {
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
                player->GetGuild()->GetId(), player-> GetGUID().GetEntry());

            GuildHouse_Utils::SpawnStarterPortal(player, map->GetId());
            GuildHouse_Utils::SpawnButlerNPC(player, map->GetId());
        }
    }

    void OnPlayerEnter(Map *map, Player *player) override {
        const auto *guildData = GuildHouse_Utils::GetGuildData(player);
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
