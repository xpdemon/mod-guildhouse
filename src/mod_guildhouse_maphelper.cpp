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



class MapHelperInstanceMap : public InstanceScript {
public:
    uint32 mapId;
    // Constructeur
    explicit MapHelperInstanceMap(InstanceMap *map, uint32 mapId)
        : InstanceScript(map) {
        this->mapId = mapId;
    }


    void OnPlayerEnter(Player *player) override {
        ChatHandler(player->GetSession()).PSendSysMessage("Vous venez d'entrer dans l'instance Map 44 !");
        const auto *guildData = GuildHouse_Utils::GetGuildData(player);
        InitMap(player, guildData);
        player->SetPhaseMask(guildData->phase, true);
        player->SetRestState(0);
    }

    // Méthode appelée quand un joueur quitte l'instance
    static void OnPlayerLeave(Player *player) {
        ChatHandler(player->GetSession()).PSendSysMessage("Vous quittez la Map 44 !");
        player->RemoveRestState();
        player->SetPhaseMask(GuildHouse_Utils::GetNormalPhase(player), true);
    }

private:
    void InitMap(Player *player, const GuildHouse_Utils::GuildData *guild_data) const {
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
                player->GetGuild()->GetId(), player->GetGUID().GetEntry());

            GuildHouse_Utils::SpawnStarterPortal(player, mapId);
            GuildHouse_Utils::SpawnButlerNPC(player, mapId);
        }
    }
};

class MapHelper : public InstanceMapScript {
public:
    // Constructeur
    // Le 2e paramètre est le mapId que vous ciblez
    explicit MapHelper(uint32 mapId)
        : InstanceMapScript("MapHelper", mapId) {
        this->mapId = mapId;
    }

    uint32 mapId;

    // Le core appelle cette méthode pour créer l'InstanceScript
    InstanceScript *GetInstanceScript(InstanceMap *map) const override {
        return new MapHelperInstanceMap(map, mapId);
    }
};


void AddGuildHouseMapScripts() {
    new MapHelper(44);
}
