#include "ScriptMgr.h"
#include "Player.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Configuration/Config.h"
#include "Creature.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Define.h"
#include "GossipDef.h"
#include "DataMap.h"
#include "GameObject.h"
#include "Transport.h"
#include "CreatureAI.h"

int cost, GuildHouseInnKeeper, GuildHouseBank, GuildHouseMailBox, GuildHouseAuctioneer, GuildHouseTrainer, GuildHouseVendor, GuildHouseObject, GuildHousePortal, GuildHouseSpirit, GuildHouseProf, GuildHouseBuyRank;

class GuildHouseSpawner : public CreatureScript
{

public:
    GuildHouseSpawner() : CreatureScript("GuildHouseSpawner") {}

    struct GuildHouseSpawnerAI : public ScriptedAI
    {
        GuildHouseSpawnerAI(Creature* creature) : ScriptedAI(creature) {}

        void UpdateAI(uint32 /*diff*/) override
        {
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
        }
    };

    CreatureAI* GetAI(Creature *creature) const override
    {
        return new GuildHouseSpawnerAI(creature);
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetGuild())
        {
            Guild* guild = sGuildMgr->GetGuildById(player->GetGuildId());
            Guild::Member const* memberMe = guild->GetMember(player->GetGUID());

            if (!memberMe->IsRankNotLower(GuildHouseBuyRank))
            {
                ChatHandler(player->GetSession()).PSendSysMessage("Vous n’êtes pas autorisé à effectuer des achats dans la Maison de Guilde.");
                return false;
            }
        }
        else
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous n'êtes pas dans une guilde!");
            return false;
        }

        ClearGossipMenuFor(player);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Recruter un Aubergiste", GOSSIP_SENDER_MAIN, 500032, "Recruter un Aubergiste", GuildHouseInnKeeper, false);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Acheter une boite aux lettres", GOSSIP_SENDER_MAIN, 184137, "Acheter une boite aux lettres?", GuildHouseMailBox, false);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Recruter un maître des écuries", GOSSIP_SENDER_MAIN, 28690, "Recruter un maître des écuries?", GuildHouseVendor, false);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Recruter un maître de classe", GOSSIP_SENDER_MAIN, 2);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Recruter un Vendeur", GOSSIP_SENDER_MAIN, 3);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Matérialiser un portail oû un object", GOSSIP_SENDER_MAIN, 4);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Recruter un  banquier", GOSSIP_SENDER_MAIN, 30605, "Recruter un  banquier?", GuildHouseBank, false);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Recruter un commissaire priseur", GOSSIP_SENDER_MAIN, 6, "Recruter un commissaire priseur?", GuildHouseAuctioneer, false);
        AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Recruter un commissaire priseur neutre", GOSSIP_SENDER_MAIN, 9858, "SRecruter un commissaire priseur neutre?", GuildHouseAuctioneer, false);
        AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Recruter un maître de profession", GOSSIP_SENDER_MAIN, 7);
        AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Recruter un maître de profession secondaire", GOSSIP_SENDER_MAIN, 8);
        AddGossipItemFor(player, GOSSIP_ICON_TALK, "Invoquer un gardien des âmes", GOSSIP_SENDER_MAIN, 6491, "Invoquer un gardien des âmes?", GuildHouseSpirit, false);
        SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {

        switch (action)
        {
 case 2: // Spawn Class Trainer
     ClearGossipMenuFor(player);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Chevalier de la Mort", GOSSIP_SENDER_MAIN, 29195, "Recruter un entraîneur Chevalier de la Mort ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Druide", GOSSIP_SENDER_MAIN, 26324, "Recruter un entraîneur Druide ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Chasseur", GOSSIP_SENDER_MAIN, 26325, "Recruter un entraîneur Chasseur ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Mage", GOSSIP_SENDER_MAIN, 26326, "Recruter un entraîneur Mage ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Paladin", GOSSIP_SENDER_MAIN, 26327, "Recruter un entraîneur Paladin ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Prêtre", GOSSIP_SENDER_MAIN, 26328, "Recruter un entraîneur Prêtre ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Voleur", GOSSIP_SENDER_MAIN, 26329, "Recruter un entraîneur Voleur ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Chaman", GOSSIP_SENDER_MAIN, 26330, "Recruter un entraîneur Chaman ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Démoniste", GOSSIP_SENDER_MAIN, 26331, "Recruter un entraîneur Démoniste ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Guerrier", GOSSIP_SENDER_MAIN, 26332, "Recruter un entraîneur Guerrier ?", GuildHouseTrainer, false);
     AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retourner !", GOSSIP_SENDER_MAIN, 9);
     SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
     break;

 case 3: // Vendors
     ClearGossipMenuFor(player);
     AddGossipItemFor(player, GOSSIP_ICON_TALK, "Fournitures commerciales", GOSSIP_SENDER_MAIN, 28692, "Recruter un fournisseur de fournitures commerciales ?", GuildHouseVendor, false);
     AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de tabards", GOSSIP_SENDER_MAIN, 28776, "Recruter un vendeur de tabards ?", GuildHouseVendor, false);
     AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de nourriture et de boissons", GOSSIP_SENDER_MAIN, 4255, "Recruter un vendeur de nourriture et de boissons ?", GuildHouseVendor, false);
     AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de réactifs", GOSSIP_SENDER_MAIN, 29636, "Recruter un vendeur de réactifs ?", GuildHouseVendor, false);
     AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de munitions et de réparations", GOSSIP_SENDER_MAIN, 29493, "Recruter un vendeur de munitions et de réparations ?", GuildHouseVendor, false);
     AddGossipItemFor(player, GOSSIP_ICON_TALK, "Vendeur de poisons", GOSSIP_SENDER_MAIN, 2622, "Recruter un vendeur de poisons ?", GuildHouseVendor, false);
     AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retourner !", GOSSIP_SENDER_MAIN, 9);
     SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
     break;

 case 4: // Objects & Portals
     ClearGossipMenuFor(player);
     AddGossipItemFor(player, GOSSIP_ICON_TALK, "Forge", GOSSIP_SENDER_MAIN, 1685, "Ajouter une forge ?", GuildHouseObject, false);
     AddGossipItemFor(player, GOSSIP_ICON_TALK, "Enclume", GOSSIP_SENDER_MAIN, 4087, "Ajouter une enclume ?", GuildHouseObject, false);
     AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Coffre de Guilde", GOSSIP_SENDER_MAIN, 187293, "Ajouter un coffre de guilde ?", GuildHouseObject, false);
     AddGossipItemFor(player, GOSSIP_ICON_INTERACT_1, "Chaise de barbier", GOSSIP_SENDER_MAIN, 191028, "Ajouter une chaise de barbier ?", GuildHouseObject, false);

     if (player->GetTeamId() == TEAM_ALLIANCE)
     {
         // Les joueurs ALLIANCE ont ces options
         AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Forgefer", GOSSIP_SENDER_MAIN, 500003, "Matérialiser le portail de Forgefer ?", GuildHousePortal, false);
         AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Darnassus", GOSSIP_SENDER_MAIN, 500001, "Matérialiser le portail de Darnassus ?", GuildHousePortal, false);
         AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Exodar", GOSSIP_SENDER_MAIN, 500002, "Matérialiser le portail d'Exodar ?", GuildHousePortal, false);
     }
     else
     {
         // Les joueurs HORDE ont ces options
         AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Fosse de l'Ombre", GOSSIP_SENDER_MAIN, 500007, "Matérialiser le portail de la Fosse de l'Ombre ?", GuildHousePortal, false);
         AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Tonnerre de Brume", GOSSIP_SENDER_MAIN, 500006, "Matérialiser le portail de Tonnerre de Brume ?", GuildHousePortal, false);
         AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Lune d'Argent", GOSSIP_SENDER_MAIN, 500005, "Matérialiser le portail de Lune d'Argent ?", GuildHousePortal, false);
     }

     // Ces deux portails fonctionnent pour n'importe quelle équipe
     AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Shattrath", GOSSIP_SENDER_MAIN, 500008, "Matérialiser le portail de Shattrath ?", GuildHousePortal, false);
     AddGossipItemFor(player, GOSSIP_ICON_TAXI, "Portail : Dalaran", GOSSIP_SENDER_MAIN, 500009, "Matérialiser le portail de Dalaran ?", GuildHousePortal, false);

     AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retourner !", GOSSIP_SENDER_MAIN, 9);
     SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
     break;

 case 6: // Auctioneer
 {
     uint32 auctioneer = 0;
     auctioneer = player->GetTeamId() == TEAM_ALLIANCE ? 8719 : 9856;
     SpawnNPC(auctioneer, player);
     break;
 }
 case 9858: // Neutral Auctioneer
     cost = GuildHouseAuctioneer;
     SpawnNPC(action, player);
     break;

 case 7: // Spawn Profession Trainers
     ClearGossipMenuFor(player);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Alchimie", GOSSIP_SENDER_MAIN, 19052, "Recruter un entraîneur d'Alchimie ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Forge", GOSSIP_SENDER_MAIN, 2836, "Recruter un entraîneur de Forge ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Ingénierie", GOSSIP_SENDER_MAIN, 8736, "Recruter un entraîneur d'Ingénierie ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Couture", GOSSIP_SENDER_MAIN, 2627, "Recruter un entraîneur de Couture ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Travail du cuir", GOSSIP_SENDER_MAIN, 19187, "Recruter un entraîneur de Travail du cuir ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Dépeçage", GOSSIP_SENDER_MAIN, 19180, "Recruter un entraîneur de Dépeçage ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Minage", GOSSIP_SENDER_MAIN, 8128, "Recruter un entraîneur de Minage ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Herboristerie", GOSSIP_SENDER_MAIN, 908, "Recruter un entraîneur de Herboristerie ?", GuildHouseProf, false);

     if (player->GetTeamId() == TEAM_ALLIANCE)
     {
         // Les joueurs ALLIANCE ont ces options
         AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Enchantement", GOSSIP_SENDER_MAIN, 18773, "Recruter un entraîneur d'Enchantement ?", GuildHouseProf, false);
         AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Joaillerie", GOSSIP_SENDER_MAIN, 18774, "Recruter un entraîneur de Joaillerie ?", GuildHouseProf, false);
         AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Inscription", GOSSIP_SENDER_MAIN, 30721, "Recruter un entraîneur d'Inscription ?", GuildHouseProf, false);
     }
     else
     {
         // Les joueurs HORDE ont ces options
         AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Enchantement", GOSSIP_SENDER_MAIN, 18753, "Recruter un entraîneur d'Enchantement ?", GuildHouseProf, false);
         AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur de Joaillerie", GOSSIP_SENDER_MAIN, 18751, "Recruter un entraîneur de Joaillerie ?", GuildHouseProf, false);
         AddGossipItemFor(player, GOSSIP_ICON_TRAINER, "Entraîneur d'Inscription", GOSSIP_SENDER_MAIN, 30722, "Recruter un entraîneur d'Inscription ?", GuildHouseProf, false);
     }

     AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retourner !", GOSSIP_SENDER_MAIN, 9);
     SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
     break;

 case 8: // Secondary Profession Trainers
     ClearGossipMenuFor(player);
     AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Entraîneur de Secourisme", GOSSIP_SENDER_MAIN, 19184, "Recruter un entraîneur de Secourisme ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Entraîneur de Pêche", GOSSIP_SENDER_MAIN, 2834, "Recruter un entraîneur de Pêche ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_MONEY_BAG, "Entraîneur de Cuisine", GOSSIP_SENDER_MAIN, 19185, "Recruter un entraîneur de Cuisine ?", GuildHouseProf, false);
     AddGossipItemFor(player, GOSSIP_ICON_CHAT, "Retourner !", GOSSIP_SENDER_MAIN, 9);
     SendGossipMenuFor(player, DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
     break;
        case 9: // Go back!
            OnGossipHello(player, creature);
            break;
        case 10: // PVP toggle
            break;
        case 30605: // Banker
            cost = GuildHouseBank;
            SpawnNPC(action, player);
            break;
        case 500032: // Innkeeper
            cost = GuildHouseInnKeeper;
            SpawnNPC(action, player);
            break;
        case 26327: // Paladin
        case 26324: // Druid
        case 26325: // Hunter
        case 26326: // Mage
        case 26328: // Priest
        case 26329: // Rogue
        case 26330: // Shaman
        case 26331: // Warlock
        case 26332: // Warrior
        case 29195: // Death Knight
            cost = GuildHouseTrainer;
            SpawnNPC(action, player);
            break;
        case 2836:  // Blacksmithing
        case 8128:  // Mining
        case 8736:  // Engineering
        case 18774: // Jewelcrafting (Alliance)
        case 18751: // Jewelcrafting (Horde)
        case 18773: // Enchanting (Alliance)
        case 18753: // Enchanting (Horde)
        case 30721: // Inscription (Alliance)
        case 30722: // Inscription (Horde)
        case 19187: // Leatherworking
        case 19180: // Skinning
        case 19052: // Alchemy
        case 908:   // Herbalism
        case 2627:  // Tailoring
        case 19185: // Cooking
        case 2834:  // Fishing
        case 19184: // First Aid
            cost = GuildHouseProf;
            SpawnNPC(action, player);
            break;
        case 28692: // Trade Supplies
        case 28776: // Tabard Vendor
        case 4255:  // Food & Drink Vendor
        case 29636: // Reagent Vendor
        case 29493: // Ammo & Repair Vendor
        case 28690: // Stable Master
        case 2622:  // Poisons Vendor
            cost = GuildHouseVendor;
            SpawnNPC(action, player);
            break;
        //
        // Objects
        //
        case 184137: // Mailbox
            cost = GuildHouseMailBox;
            SpawnObject(action, player);
            break;
        case 6491: // Spirit Healer
            cost = GuildHouseSpirit;
            SpawnNPC(action, player);
            break;
        case 1685:   // Forge
        case 4087:   // Anvil
        case 187293: // Guild Vault
        case 191028: // Barber Chair
            cost = GuildHouseObject;
            SpawnObject(action, player);
            break;
        case 500001: // Darnassus Portal
        case 500002: // Exodar Portal
        case 500003: // Ironforge Portal
        case 500005: // Silvermoon Portal
        case 500006: // Thunder Bluff Portal
        case 500007: // Undercity Portal
        case 500008: // Shattrath Portal
        case 500009: // Dalaran Portal
            cost = GuildHousePortal;
            SpawnObject(action, player);
            break;
        }
        return true;
    }

    uint32 GetGuildPhase(Player* player)
    {
        return player->GetGuildId() + 10;
    }

    void SpawnNPC(uint32 entry, Player* player)
    {
        if (player->FindNearestCreature(entry, VISIBILITY_RANGE, true))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous avez déjà cette créature!");
            CloseGossipMenuFor(player);
            return;
        }

        float posX;
        float posY;
        float posZ;
        float ori;

        QueryResult result = WorldDatabase.Query("SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={}", entry);

        if (!result)
            return;

        do
        {
            Field* fields = result->Fetch();
            posX = fields[0].Get<float>();
            posY = fields[1].Get<float>();
            posZ = fields[2].Get<float>();
            ori = fields[3].Get<float>();

        } while (result->NextRow());

        Creature* creature = new Creature();

        if (!creature->Create(player->GetMap()->GenerateLowGuid<HighGuid::Unit>(), player->GetMap(), GetGuildPhase(player), entry, 0, posX, posY, posZ, ori))
        {
            delete creature;
            return;
        }
        creature->SaveToDB(player->GetMapId(), (1 << player->GetMap()->GetSpawnMode()), GetGuildPhase(player));
        uint32 db_guid = creature->GetSpawnId();

        creature->CleanupsBeforeDelete();
        delete creature;
        creature = new Creature();
        if (!creature->LoadCreatureFromDB(db_guid, player->GetMap()))
        {
            delete creature;
            return;
        }

        sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));
        player->ModifyMoney(-cost);
        CloseGossipMenuFor(player);
    }

    void SpawnObject(uint32 entry, Player* player)
    {
        if (player->FindNearestGameObject(entry, VISIBLE_RANGE))
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Vous avez déjà cet objet!");
            CloseGossipMenuFor(player);
            return;
        }

        float posX;
        float posY;
        float posZ;
        float ori;

        QueryResult result = WorldDatabase.Query("SELECT `posX`, `posY`, `posZ`, `orientation` FROM `guild_house_spawns` WHERE `entry`={}", entry);

        if (!result)
            return;

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
            return;

        const GameObjectTemplate* objectInfo = sObjectMgr->GetGameObjectTemplate(objectId);

        if (!objectInfo)
            return;

        if (objectInfo->displayId && !sGameObjectDisplayInfoStore.LookupEntry(objectInfo->displayId))
            return;

        GameObject* object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
        ObjectGuid::LowType guidLow = player->GetMap()->GenerateLowGuid<HighGuid::GameObject>();

        if (!object->Create(guidLow, objectInfo->entry, player->GetMap(), GetGuildPhase(player), posX, posY, posZ, ori, G3D::Quat(), 0, GO_STATE_READY))
        {
            delete object;
            return;
        }

        // fill the gameobject data and save to the db
        object->SaveToDB(player->GetMapId(), (1 << player->GetMap()->GetSpawnMode()), GetGuildPhase(player));
        guidLow = object->GetSpawnId();
        // delete the old object and do a clean load from DB with a fresh new GameObject instance.
        // this is required to avoid weird behavior and memory leaks
        delete object;

        object = sObjectMgr->IsGameObjectStaticTransport(objectInfo->entry) ? new StaticTransport() : new GameObject();
        // this will generate a new guid if the object is in an instance
        if (!object->LoadGameObjectFromDB(guidLow, player->GetMap(), true))
        {
            delete object;
            return;
        }

        // TODO: is it really necessary to add both the real and DB table guid here ?
        sObjectMgr->AddGameobjectToGrid(guidLow, sObjectMgr->GetGameObjectData(guidLow));
        player->ModifyMoney(-cost);
        CloseGossipMenuFor(player);
    }
};

class GuildHouseButlerConf : public WorldScript
{
public:
    GuildHouseButlerConf() : WorldScript("GuildHouseButlerConf") {}

    void OnBeforeConfigLoad(bool /*reload*/) override
    {
        GuildHouseInnKeeper = sConfigMgr->GetOption<int32>("GuildHouseInnKeeper", 1000000);
        GuildHouseBank = sConfigMgr->GetOption<int32>("GuildHouseBank", 1000000);
        GuildHouseMailBox = sConfigMgr->GetOption<int32>("GuildHouseMailbox", 500000);
        GuildHouseAuctioneer = sConfigMgr->GetOption<int32>("GuildHouseAuctioneer", 500000);
        GuildHouseTrainer = sConfigMgr->GetOption<int32>("GuildHouseTrainerCost", 1000000);
        GuildHouseVendor = sConfigMgr->GetOption<int32>("GuildHouseVendor", 500000);
        GuildHouseObject = sConfigMgr->GetOption<int32>("GuildHouseObject", 500000);
        GuildHousePortal = sConfigMgr->GetOption<int32>("GuildHousePortal", 500000);
        GuildHouseProf = sConfigMgr->GetOption<int32>("GuildHouseProf", 500000);
        GuildHouseSpirit = sConfigMgr->GetOption<int32>("GuildHouseSpirit", 100000);
        GuildHouseBuyRank = sConfigMgr->GetOption<int32>("GuildHouseBuyRank", 4);
    }
};

void AddGuildHouseButlerScripts()
{
    new GuildHouseSpawner();
    new GuildHouseButlerConf();
}
