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

};
#endif //MOD_GUILDHOUSE_H
