//
// Created by boujenah bruno on 07/01/2025.
//

#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Creature.h"
#include "Define.h"
#include "GossipDef.h"
#include "GameObject.h"
#include "Transport.h"
#include "CreatureAI.h"
#include "mod_guildhouse.h"


class GuildhouseMjManager : public CreatureScript {
public:
    GuildhouseMjManager() : CreatureScript("GuildMjHouseSpawner") {
    }

    struct GuildHouseSpawnerAI : ScriptedAI {
        explicit GuildHouseSpawnerAI(Creature *creature) : ScriptedAI(creature) {
        }

        void UpdateAI(uint32 /*diff*/) override {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    };

    CreatureAI *GetAI(Creature *creature) const override {
        return new GuildHouseSpawnerAI(creature);
    }

    bool OnGossipHello(Player *player, Creature *creature) override {
        // ----------------------------
        // Construction du menu Gossip
        // ----------------------------
        ClearGossipMenuFor(player);

        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Recruter un Aubergiste", GOSSIP_SENDER_MAIN, 10,
                         "Recruter un Aubergiste", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Acheter une boite aux lettres", GOSSIP_SENDER_MAIN, 184137,
                         "Acheter une boite aux lettres?", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Recruter un maître des écuries", GOSSIP_SENDER_MAIN, 37,
                         "Recruter un maître des écuries?", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Recruter un maître de classe", GOSSIP_SENDER_MAIN, 99);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Recruter un Vendeur", GOSSIP_SENDER_MAIN, 100);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Matérialiser un portail oû un object", GOSSIP_SENDER_MAIN, 101);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Recruter un  banquier", GOSSIP_SENDER_MAIN, 11,
                         "Recruter un  banquier?", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Recruter un commissaire priseur", GOSSIP_SENDER_MAIN, 30,
                         "Recruter un commissaire priseur?", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Recruter un commissaire priseur neutre", GOSSIP_SENDER_MAIN,
                         40, "Recruter un commissaire priseur neutre?", 0, false);
        AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Recruter un maître de profession", GOSSIP_SENDER_MAIN, 102);
        AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Recruter un maître de profession secondaire", GOSSIP_SENDER_MAIN,
                         103);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Invoquer un gardien des âmes", GOSSIP_SENDER_MAIN, 39,
                         "Invoquer un gardien des âmes?", 0, false);

        // On envoie le menu
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 /*sender*/, uint32 action) override {
        switch (action) {
            case 99: // Spawn Class Trainer
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Chevalier de la Mort", GOSSIP_SENDER_MAIN, 12,
                                 "Recruter un entraîneur Chevalier de la Mort ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Druide", GOSSIP_SENDER_MAIN, 2,
                                 "Recruter un entraîneur Druide ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Chasseur", GOSSIP_SENDER_MAIN, 3,
                                 "Recruter un entraîneur Chasseur ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Mage", GOSSIP_SENDER_MAIN, 4,
                                 "Recruter un entraîneur Mage ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Paladin", GOSSIP_SENDER_MAIN, 1,
                                 "Recruter un entraîneur Paladin ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Prêtre", GOSSIP_SENDER_MAIN, 5,
                                 "Recruter un entraîneur Prêtre ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Voleur", GOSSIP_SENDER_MAIN, 6,
                                 "Recruter un entraîneur Voleur ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Chaman", GOSSIP_SENDER_MAIN, 7,
                                 "Recruter un entraîneur Chaman ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Démoniste", GOSSIP_SENDER_MAIN, 8,
                                 "Recruter un entraîneur Démoniste ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Guerrier", GOSSIP_SENDER_MAIN, 9,
                                 "Recruter un entraîneur Guerrier ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retour", GOSSIP_SENDER_MAIN, 1000);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                break;
            }
            case 100: // Vendors
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Fournitures commerciales", GOSSIP_SENDER_MAIN, 32,
                                 "Recruter un fournisseur de fournitures commerciales ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de tabards", GOSSIP_SENDER_MAIN, 33,
                                 "Recruter un vendeur de tabards ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de nourriture et de boissons", GOSSIP_SENDER_MAIN,
                                 34, "Recruter un vendeur de nourriture et de boissons ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de réactifs", GOSSIP_SENDER_MAIN, 35,
                                 "Recruter un vendeur de réactifs ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de munitions et de réparations", GOSSIP_SENDER_MAIN,
                                 36, "Recruter un vendeur de munitions et de réparations ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de poisons", GOSSIP_SENDER_MAIN, 38,
                                 "Recruter un vendeur de poisons ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retourner !", GOSSIP_SENDER_MAIN, 1000);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                break;
            }
            case 1000:
                OnGossipHello(player, creature);
                break;
            case 101: // Objects & Portals
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Forge", GOSSIP_SENDER_MAIN, 1685, "Ajouter une forge ?", 0,
                                 false);
                AddGossipItemFor(player, GOSSIP_ICON_TALK, "Enclume", GOSSIP_SENDER_MAIN, 4087, "Ajouter une enclume ?",
                                 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Coffre de Guilde", GOSSIP_SENDER_MAIN, 187293,
                                 "Ajouter un coffre de guilde ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Chaise de barbier", GOSSIP_SENDER_MAIN, 191028,
                                 "Ajouter une chaise de barbier ?", 0, false);

                if (player->GetTeamId() == TEAM_ALLIANCE) {
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Forgefer", GOSSIP_SENDER_MAIN, 500003,
                                     "Matérialiser le portail de Forgefer ?", 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Darnassus", GOSSIP_SENDER_MAIN, 500001,
                                     "Matérialiser le portail de Darnassus ?", 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Exodar", GOSSIP_SENDER_MAIN, 500002,
                                     "Matérialiser le portail d'Exodar ?", 0, false);

                } else {
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Fosse de l'Ombre", GOSSIP_SENDER_MAIN, 500007,
                                     "Matérialiser le portail de la Fosse de l'Ombre ?", 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Tonnerre de Brume", GOSSIP_SENDER_MAIN,
                                     500006, "Matérialiser le portail de Tonnerre de Brume ?", 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Lune d'Argent", GOSSIP_SENDER_MAIN, 500005,
                                     "Matérialiser le portail de Lune d'Argent ?", 0, false);

                }
                // Portails communs
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Shattrath", GOSSIP_SENDER_MAIN, 500008,
                                 "Matérialiser le portail de Shattrath ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Dalaran", GOSSIP_SENDER_MAIN, 500009,
                                 "Matérialiser le portail de Dalaran ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retourner !", GOSSIP_SENDER_MAIN, 1000);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());

                break;
            }
            case 30: // Auctioneer (Alliance/Horde)
            {
                uint32 auctioneer = (player->GetTeamId() == TEAM_ALLIANCE) ? 200 : 201;
                SpawnNPC(auctioneer, player);
                break;
            }
            case 40: // Neutral Auctioneer
            {
                SpawnNPC(action, player);
                break;
            }
            case 102: // Spawn Profession Trainers
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Alchimie", GOSSIP_SENDER_MAIN, 24,
                                 "Recruter un entraîneur d'Alchimie ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Forge", GOSSIP_SENDER_MAIN, 13,
                                 "Recruter un entraîneur de Forge ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Ingénierie", GOSSIP_SENDER_MAIN, 15,
                                 "Recruter un entraîneur d'Ingénierie ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Couture", GOSSIP_SENDER_MAIN, 26,
                                 "Recruter un entraîneur de Couture ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Travail du cuir", GOSSIP_SENDER_MAIN, 22,
                                 "Recruter un entraîneur de Travail du cuir ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Dépeçage", GOSSIP_SENDER_MAIN, 23,
                                 "Recruter un entraîneur de Dépeçage ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Minage", GOSSIP_SENDER_MAIN, 14,
                                 "Recruter un entraîneur de Minage ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Herboristerie", GOSSIP_SENDER_MAIN, 25,
                                 "Recruter un entraîneur de Herboristerie ?", 0, false);

                if (player->GetTeamId() == TEAM_ALLIANCE) {
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Enchantement", GOSSIP_SENDER_MAIN, 18,
                                     "Recruter un entraîneur d'Enchantement ?", 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Joaillerie", GOSSIP_SENDER_MAIN, 16,
                                     "Recruter un entraîneur de Joaillerie ?", 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Inscription", GOSSIP_SENDER_MAIN, 20,
                                     "Recruter un entraîneur d'Inscription ?", 0, false);
                } else {
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Enchantement", GOSSIP_SENDER_MAIN, 19,
                                     "Recruter un entraîneur d'Enchantement ?", 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Joaillerie", GOSSIP_SENDER_MAIN, 17,
                                     "Recruter un entraîneur de Joaillerie ?", 0, false);
                    AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Inscription", GOSSIP_SENDER_MAIN, 21,
                                     "Recruter un entraîneur d'Inscription ?", 0, false);
                }

                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retourner !", GOSSIP_SENDER_MAIN, 1000);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                break;
            }
            case 103: // Secondary Profession Trainers
            {
                ClearGossipMenuFor(player);
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Entraîneur de Secourisme", GOSSIP_SENDER_MAIN, 27,
                                 "Recruter un entraîneur de Secourisme ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Entraîneur de Pêche", GOSSIP_SENDER_MAIN, 28,
                                 "Recruter un entraîneur de Pêche ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Entraîneur de Cuisine", GOSSIP_SENDER_MAIN, 29,
                                 "Recruter un entraîneur de Cuisine ?", 0, false);
                AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retourner !", GOSSIP_SENDER_MAIN, 1000);
                SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
                break;
            }
            case 999: // PVP toggle (inutilisé ?)
                break;

            // -------------------------------------------
            // Cases existants : spawns NPC / objects
            // -------------------------------------------
            case 11: // Banker

                SpawnNPC(action, player);
                break;
            case 10: // Innkeeper

                SpawnNPC(action, player);
                break;
            // Entraîneurs de classe
            case 1: // Paladin
            case 2: // Druid
            case 3: // Hunter
            case 4: // Mage
            case 5: // Priest
            case 6: // Rogue
            case 7: // Shaman
            case 8: // Warlock
            case 9: // Warrior
            case 12: // Death Knight

                SpawnNPC(action, player);
                break;

            // Entraîneurs de métiers (Forge, Enchant, etc.)
            case 13: // Blacksmithing
            case 14: // Mining
            case 15: // Engineering
            case 16: // Jewelcrafting (Alliance)
            case 17: // Jewelcrafting (Horde)
            case 18: // Enchanting (Alliance)
            case 19: // Enchanting (Horde)
            case 20: // Inscription (Alliance)
            case 21: // Inscription (Horde)
            case 22: // Leatherworking
            case 23: // Skinning
            case 24: // Alchemy
            case 25: // Herbalism
            case 26: // Tailoring
            case 29: // Cooking
            case 28: // Fishing
            case 27: // First Aid

                SpawnNPC(action, player);
                break;

            // Vendors divers
            case 32: // Trade Supplies
            case 33: // Tabard Vendor
            case 34: // Food & Drink Vendor
            case 35: // Reagent Vendor
            case 36: // Ammo & Repair Vendor
            case 37: // Stable Master
            case 38: // Poisons Vendor

                SpawnNPC(action, player);
                break;

            // Objects divers
            case 184137: // Mailbox

                SpawnObject(action, player);
                break;
            case 39: // Spirit Healer

                SpawnNPC(action, player);
                break;
            case 1685: // Forge
            case 4087: // Anvil
            case 187293: // Guild Vault
            case 191028: // Barber Chair

                SpawnObject(action, player);
                break;
            // Portails
            case 500001: // Darnassus Portal
            case 500002: // Exodar Portal
            case 500003: // Ironforge Portal
            case 500005: // Silvermoon Portal
            case 500006: // Thunder Bluff Portal
            case 500007: // Undercity Portal
            case 500008: // Shattrath Portal
            case 500009: // Dalaran Portal

                SpawnObject(action, player);
                break;
            default: ;
        }
        return true;
    }


    static void SpawnNPC(uint32 action, Player *player) {
        auto mapId = player->GetMapId();

        uint32 entry;

        float posX = player->GetPositionX();
        float posY = player->GetPositionY();
        float posZ = player->GetPositionZ();
        float ori = player->GetOrientation();

        const auto entryResult = WorldDatabase.Query(
            "select `entry` from `guild_house_spawns` where `actionId` = {} and `map` = {}", action, mapId);
        if (!entryResult)
            return;
        do {
            const Field *fields = entryResult-> Fetch();
            entry = fields[0].Get<uint32>();
        } while (entryResult ->NextRow());

        auto update = WorldDatabase.Query(
            "update `guild_house_spawns` SET `posX` = {}, `posY` = {} , `posZ` = {} , `orientation` = {} WHERE `map` = {} and `entry` = {}",
            posX, posY, posZ, ori, mapId, entry);

        QueryResult result = WorldDatabase.Query(
            "SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={} AND `map`={}",
            entry, mapId);
        if (!result)
            return;

        do {
            Field *fields = result->Fetch();
            posX = fields[0].Get<float>();
            posY = fields[1].Get<float>();
            posZ = fields[2].Get<float>();
            ori = fields[3].Get<float>();
        } while (result->NextRow());

        auto *c = new Creature();
        if (!c->Create(player->GetMap()->GenerateLowGuid<HighGuid::Unit>(), player->GetMap(),
                       GuildHouse_Utils::GetGuildPhase(player), entry, 0, posX, posY, posZ, ori)) {
            delete c;
            return;
        }

        c->SaveToDB(player->GetMapId(), (1 << player->GetMap()->GetSpawnMode()),
                    GuildHouse_Utils::GetGuildPhase(player));
        uint32 db_guid = c->GetSpawnId();

        c->CleanupsBeforeDelete();
        delete c;

        c = new Creature();
        if (!c->LoadCreatureFromDB(db_guid, player->GetMap())) {
            delete c;
            return;
        }

        sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));

        CloseGossipMenuFor(player);
    }

    static void SpawnObject(uint32 entry, Player *player) {
        auto mapId = player->GetMapId();

        float posX = player->GetPositionX();
        float posY = player->GetPositionY();
        float posZ = player->GetPositionZ();
        float ori = player->GetOrientation();

        auto update = WorldDatabase.Query(
            "update `guild_house_spawns` SET `posX` = {}, `posY` = {} , `posZ` = {} , `orientation` = {} WHERE `map` = {} and `entry` = {}",
            posX, posY, posZ, ori, mapId, entry);
        QueryResult result = WorldDatabase.Query(
            "SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={} and `map`= {}",
            entry, mapId);
        if (!result)
            return;

        do {
            Field *fields = result->Fetch();
            posX = fields[0].Get<float>();
            posY = fields[1].Get<float>();
            posZ = fields[2].Get<float>();
            ori = fields[3].Get<float>();
        } while (result->NextRow());

        uint32 objectId = entry;
        if (!objectId)
            return;

        const GameObjectTemplate *objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);
        if (!objectInfo)
            return;

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
            return;

        GameObject *obj = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry)
                              ? new StaticTransport()
                              : new GameObject();
        ObjectGuid::LowType guidLow = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();

        if (!obj->Create(guidLow, objectInfo->entry, player->GetMap(), GuildHouse_Utils::GetGuildPhase(player), posX,
                         posY, posZ, ori, G3D::Quat(), 0, GO_STATE_READY)) {
            delete obj;
            return;
        }

        obj->SaveToDB(player->GetMapId(), (1 << player->GetMap()->GetSpawnMode()),
                      GuildHouse_Utils::GetGuildPhase(player));
        guidLow = obj->GetSpawnId();
        delete obj;

        obj = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
        if (!obj->LoadGameObjectFromDB(guidLow, player->GetMap(), true)) {
            delete obj;
            return;
        }

        sObjectMgr->AddGameobjectToGrid(guidLow, sObjectMgr->GetGameObjectData(guidLow));
        CloseGossipMenuFor(player);
    }
};

void AddGuildHouseMjmanagerScripts() {
    new GuildhouseMjManager();
}
