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


class GuildHelper : public GuildScript {
public:
    GuildHelper() : GuildScript("GuildHelper") {
    }

    void OnCreate(Guild * /*guild*/, Player *leader, const std::string & /*name*/) override {
        ChatHandler(leader->GetSession()).PSendSysMessage(
            "Félicitations ! Vous possédez maintenant une guilde. Vous pouvez acheter une Maison de Guilde !");
    }

    void OnDisband(Guild *guild) override {
        if (RemoveGuildHouse(guild)) {
            LOG_INFO("modules", "GUILDHOUSE: Deleting Guild House data due to disbanding of guild...");
        } else {
            LOG_INFO("modules", "GUILDHOUSE: Error deleting Guild House data during disbanding of guild!!");
        }
    }

    static bool RemoveGuildHouse(const Guild *guild) {
        uint32 guildPhase = GuildHouse_Utils::GetGuildPhase(guild);

        // Lets find all of the gameobjects to be removed
        QueryResult GameobjResult = WorldDatabase.Query(
            "SELECT `guid` FROM `gameobject` WHERE `map`=1 AND `phaseMask`={}",
            guildPhase);
        // Lets find all of the creatures to be removed
        QueryResult CreatureResult = WorldDatabase.Query(
            "SELECT `guid` FROM `creature` WHERE `map`=1 AND `phaseMask`={}",
            guildPhase);

        Map *map = sMapMgr->FindMap(1, 0);
        // Remove creatures from the deleted guild house map
        if (CreatureResult) {
            do {
                Field *fields = CreatureResult->Fetch();
                uint32 lowguid = fields[0].Get<int32>();
                if (CreatureData const *cr_data = sObjectMgr->GetCreatureData(lowguid)) {
                    if (Creature *creature = map->
                            GetCreature(ObjectGuid::Create<HighGuid::Unit>(cr_data->id1, lowguid))) {
                        creature->CombatStop();
                        creature->DeleteFromDB();
                        creature->AddObjectToRemoveList();
                    }
                }
            } while (CreatureResult->NextRow());
        }

        // Remove gameobjects from the deleted guild house map
        if (GameobjResult) {
            do {
                Field *fields = GameobjResult->Fetch();
                uint32 lowguid = fields[0].Get<int32>();
                if (GameObjectData const *go_data = sObjectMgr->GetGameObjectData(lowguid)) {
                    if (GameObject *gobject = map->GetGameObject(
                        ObjectGuid::Create<HighGuid::GameObject>(go_data->id, lowguid))) {
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

class GuildHouseSeller : public CreatureScript {
public:
    GuildHouseSeller() : CreatureScript("GuildHouseSeller") {
    }

    struct GuildHouseSellerAI : public ScriptedAI {
        explicit GuildHouseSellerAI(Creature *creature) : ScriptedAI(creature) {
        }

        void UpdateAI(uint32 /*diff*/) override {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    };

    CreatureAI *GetAI(Creature *creature) const override {
        return new GuildHouseSellerAI(creature);
    }

    bool OnGossipHello(Player *player, Creature *creature) override {
        if (!player->GetGuild()) {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous n'êtes pas membre d'une guilde.");
            CloseGossipMenuFor(player);
            return false;
        }

        QueryResult has_gh = CharacterDatabase.Query("SELECT id, `guild` FROM `guild_house` WHERE guild = {}",
                                                     player->GetGuildId());

        // Only show Teleport option if guild owns a guild house
        if (has_gh) {
            AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Téléportation à la maison de guilde", GOSSIP_SENDER_MAIN, 1);

            // Only show "Sell" option if they have a guild house & have permission to sell it
            Guild *guild = sGuildMgr->GetGuildById(player->GetGuildId());
            Guild::Member const *memberMe = guild->GetMember(player->GetGUID());
            if (memberMe->IsRankNotLower(sConfigMgr->GetOption<int32>("GuildHouseSellRank", 0))) {
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Vendre la maison de guilde!", GOSSIP_SENDER_MAIN, 3,
                                 "Êtes vous certain de vouloir vendre la maison de guilde?", 0, false);
            }
        } else {
            // Only leader of the guild can buy guild house & only if they don't already have a guild house
            if (player->GetGuild()->GetLeaderGUID() == player->GetGUID()) {
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Acheter une maison de guilde!", GOSSIP_SENDER_MAIN, 2);
            }
        }

        AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Close", GOSSIP_SENDER_MAIN, 5);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player *player, Creature *creature, uint32 /*sender*/, uint32 action) override {
        uint32 map;
        float posX;
        float posY;
        float posZ;
        float ori;

        switch (action) {
            case 100: // GM Island
                map = 1;
                posX = 16222.972f;
                posY = 16267.802f;
                posZ = 13.136777f;
                ori = 1.461173f;
                break;
            case 101: // Monastery
                map = 44;
                posX = 76.02f;
                posY = -0.76f;
                posZ = 18.6794f;
                ori = 6.22f;
            case 5: // close
                CloseGossipMenuFor(player);
                break;
            case 4: // --- MORE TO COME ---
                BuyGuildHouse(player->GetGuild(), player, creature);
                break;
            case 3: // sell back guild house
            {
                QueryResult has_gh = CharacterDatabase.Query("SELECT id, `guild` FROM `guild_house` WHERE guild={}",
                                                             player->GetGuildId());
                if (!has_gh) {
                    ChatHandler(player->GetSession()).PSendSysMessage("Votre guilde à maintenant une maison!");
                    CloseGossipMenuFor(player);
                    return false;
                }

                // calculate total gold returned: 1) cost of guild house and cost of each purchase made
                if (RemoveGuildHouse(player)) {
                    ChatHandler(player->GetSession()).PSendSysMessage("You have successfully sold your Guild House.");
                    player->GetGuild()->BroadcastToGuild(player->GetSession(), false,
                                                         "Nous venons de vendre notre Maison de Guilde.",
                                                         LANG_UNIVERSAL);
                    player->ModifyMoney(+(sConfigMgr->GetOption<int32>("CostGuildHouse", 10000000) / 2));
                    LOG_INFO("modules", "GUILDHOUSE: Successfully returned money and sold Guild House");
                    CloseGossipMenuFor(player);
                } else {
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

        if (action >= 100) {
            CharacterDatabase.Query(
                "INSERT INTO `guild_house` (guild, phase, map, positionX, positionY, positionZ, orientation) VALUES ({}, {}, {}, {}, {}, {}, {})",
                player->GetGuildId(), GuildHouse_Utils::GetGuildPhase(player), map, posX, posY, posZ, ori);
            player->ModifyMoney(-(sConfigMgr->GetOption<int32>("CostGuildHouse", 10000000)));
            // Msg to purchaser and Msg Guild as purchaser
            ChatHandler(player->GetSession()).PSendSysMessage("Vous avez acheté une Maison de Guilde avec succès.");
            player->GetGuild()->BroadcastToGuild(player->GetSession(), false, "We now have a Guild House!",
                                                 LANG_UNIVERSAL);
            player->GetGuild()->BroadcastToGuild(player->GetSession(), false,
                                                 "Dans le chat, tapez .guildhouse teleport ou .gh tele pour me rejoindre là-bas !",
                                                 LANG_UNIVERSAL);
            LOG_INFO("modules", "GUILDHOUSE: GuildId: '{}' has purchased a guildhouse", player->GetGuildId());

            CloseGossipMenuFor(player);
        }

        return true;
    }


    static bool RemoveGuildHouse(Player *player) {
        auto *guildData = GuildHouse_Utils::GetGuildData(player);
        Map *map = sMapMgr->FindMap(guildData->map, guildData->instanceId);
        // Lets find all of the gameobjects to be removed
        QueryResult GameobjResult = WorldDatabase.Query(
            "SELECT `guid` FROM `gameobject` WHERE `map` = {} AND `phaseMask` = '{}'", guildData->map,
            guildData->phase);
        // Lets find all of the creatures to be removed
        QueryResult CreatureResult = WorldDatabase.Query(
            "SELECT `guid` FROM `creature` WHERE `map` = {} AND `phaseMask` = '{}'", guildData->map, guildData->phase);

        // Remove creatures from the deleted guild house map
        if (CreatureResult) {
            do {
                Field *fields = CreatureResult->Fetch();
                uint32 lowguid = fields[0].Get<uint32>();
                if (CreatureData const *cr_data = sObjectMgr->GetCreatureData(lowguid)) {
                    if (Creature *creature = map->
                            GetCreature(ObjectGuid::Create<HighGuid::Unit>(cr_data->id1, lowguid))) {
                        creature->CombatStop();
                        creature->DeleteFromDB();
                        creature->AddObjectToRemoveList();
                    }
                }
            } while (CreatureResult->NextRow());
        }

        // Remove gameobjects from the deleted guild house map
        if (GameobjResult) {
            do {
                Field *fields = GameobjResult->Fetch();
                uint32 lowguid = fields[0].Get<uint32>();
                if (GameObjectData const *go_data = sObjectMgr->GetGameObjectData(lowguid)) {
                    if (GameObject *gobject = map->GetGameObject(
                        ObjectGuid::Create<HighGuid::GameObject>(go_data->id, lowguid))) {
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
        CharacterDatabase.Query("DELETE FROM `character_instance` WHERE `instance`={}", guildData->instanceId);

        return true;
    }


    static bool BuyGuildHouse(const Guild *guild, Player *player, const Creature *creature) {
        QueryResult result = CharacterDatabase.Query("SELECT `id`, `guild` FROM `guild_house` WHERE `guild`={}",
                                                     guild->GetId());

        if (result) {
            ChatHandler(player->GetSession()).PSendSysMessage(
                "Votre guilde possède déjà une Maison de Guilde, vous ne pouvez pas en acheter une autre.");
            CloseGossipMenuFor(player);
            return false;
        }

        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "GM Island", GOSSIP_SENDER_MAIN, 100,
                         "Buy Guild House on GM Island?", sConfigMgr->GetOption<int32>("CostGuildHouse", 10000000),
                         false);
        // Removing this tease for now, as right now the phasing code is specific go GM Island, so it's not a simple thing to add new areas yet.
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Un Charmant monastère", GOSSIP_SENDER_MAIN, 101,
                         "Acheter une Maison de Guilde dans un charmant monastère?",
                         sConfigMgr->GetOption<int32>("CostGuildHouse", 10000000), false);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    static void TeleportGuildHouse(const Guild *guild, Player *player, const Creature *creature) {
        auto *guildData = GuildHouse_Utils::GetGuildData(player);

        if (guildData->id == 0) {
            ClearGossipMenuFor(player);
            if (player->GetGuild()->GetLeaderGUID() == player->GetGUID()) {
                // Only leader of the guild can buy / sell guild house
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Acheter une maison de guilde!", GOSSIP_SENDER_MAIN, 2);
                AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Vendre la maison de guilde!", GOSSIP_SENDER_MAIN, 3,
                                 "Êtes vous certain de vouloir vendre la maison de guilde?", 0, false);
            }

            AddGossipItemFor(player, GOSSIP_ICON_TABARD, "Se rendre à la maison de guilde", GOSSIP_SENDER_MAIN, 1);
            AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Fermer", GOSSIP_SENDER_MAIN, 5);
            SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
            ChatHandler(player->GetSession()).PSendSysMessage(
                "Votre guilde ne possède pas de Maison de Guilde. Vous pouvez en acheter une dès maintenant !");
            return;
        }
        player->TeleportTo(guildData->map, guildData->posX, guildData->posY, guildData->posZ, guildData->ori);
    }
};

class GuildHousePlayerScript : public PlayerScript {
public:
    GuildHousePlayerScript() : PlayerScript("GuildHousePlayerScript") {
    }

    void OnLogin(Player *player) override {
        GuildHouse_Utils::CheckPlayer(player);
    }


    void OnUpdateZone(Player *player, uint32 newZone, uint32 /*newArea*/) override {
        if (newZone == 876)
            GuildHouse_Utils::CheckPlayer(player);
        else
            player->SetPhaseMask(GuildHouse_Utils::GetNormalPhase(player), true);
    }

    bool OnBeforeTeleport(Player *player, uint32 mapid, float x, float y, float z, float orientation, uint32 options,
                          Unit *target) override {
        (void) mapid;
        (void) x;
        (void) y;
        (void) z;
        (void) orientation;
        (void) options;
        (void) target;

        if (player->GetZoneId() == 876 && player->GetAreaId() == 876) // GM Island
        {
            // Remove the rested state when teleporting from the guild house
            player->RemoveRestState();
        }

        return true;
    }
};

using namespace Acore::ChatCommands;

class GuildHouseCommand : public CommandScript {
public:
    GuildHouseCommand() : CommandScript("GuildHouseCommand") {
    }

    [[nodiscard]] ChatCommandTable GetCommands() const override {
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

    static uint32 GetGuildPhase(const Player *player) {
        // Version "simple"
        //return player->GetGuildId() + 10;

        // Version "bitmask", si vous voulez
        if (player->GetGuildId() == 0)
            return 0;
        return 1 << (player->GetGuildId() - 1);
    }

    static bool HandleSpawnButlerCommand(ChatHandler *handler) {
        Player *player = handler->GetSession()->GetPlayer();
        Map *map = player->GetMap();

        if (!player->GetGuild() || (player->GetGuild()->GetLeaderGUID() != player->GetGUID())) {
            handler->SendSysMessage("Vous devez être le Maître de Guilde d’une guilde pour utiliser cette commande !");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->GetAreaId() != 876 || player->GetMapId() != 44) {
            handler->SendSysMessage("You must be in your Guild House to use this command!");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (player->FindNearestCreature(500031, VISIBLE_RANGE, true)) {
            handler->SendSysMessage("Vous avez déjà le Majordome de la Maison de Guilde !");
            handler->SetSentErrorMessage(true);
            return false;
        }

        float posX = player->GetPosition().GetPositionX() + 1.0f;
        float posY = player->GetPosition().GetPositionY();
        float posZ = player->GetPosition().GetPositionZ();
        float ori = player->GetOrientation();

        auto *creature = new Creature();
        if (!creature->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, GetGuildPhase(player), 500031, 0, posX, posY,
                              posZ, ori)) {
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
        if (!creature->LoadCreatureFromDB(lowguid, player->GetMap())) {
            handler->SendSysMessage("Something went wrong when adding the Butler.");
            handler->SetSentErrorMessage(true);
            delete creature;
            return false;
        }

        sObjectMgr->AddCreatureToGrid(lowguid, sObjectMgr->GetCreatureData(lowguid));
        return true;
    }

    static bool HandleGuildHouseTeleCommand(ChatHandler *handler) {
        Player *player = handler->GetSession()->GetPlayer();

        if (!player)
            return false;

        if (player->IsInCombat()) {
            handler->SendSysMessage("You can't use this command while in combat!");
            handler->SetSentErrorMessage(true);
            return false;
        }

        auto *guildData = GuildHouse_Utils::GetGuildData(player);

        if (guildData->id == 0) {
            handler->SendSysMessage("Your guild does not own a Guild House!");
            handler->SetSentErrorMessage(true);
            return false;
        }

        player->TeleportTo(guildData->map, guildData->posX, guildData->posY, guildData->posZ, guildData->ori);

        return true;
    }
};

class GuildHouseGlobal : public GlobalScript {
public:
    GuildHouseGlobal() : GlobalScript("GuildHouseGlobal") {
    }

    void OnBeforeWorldObjectSetPhaseMask(WorldObject const *worldObject, uint32 & /*oldPhaseMask*/,
                                         uint32 & /*newPhaseMask*/, bool &useCombinedPhases,
                                         bool & /*update*/) override {
        if (worldObject->GetZoneId() == 876)
            useCombinedPhases = false;
        else
            useCombinedPhases = true;
    }
};

void AddGuildHouseScripts() {
    new GuildHelper();
    new GuildHouseSeller();
    new GuildHousePlayerScript();
    new GuildHouseCommand();
    new GuildHouseGlobal();
}
