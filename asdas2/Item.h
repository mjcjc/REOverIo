#pragma once
#include <unordered_map>
#include "deFine.h"

struct WorlditemObjectInfos {
    UINT16 itemID;
    bool canPickup;
    bool canUseDirectly;
    bool destructible;
    float expireTime;
};


extern unordered_map<UINT16, WorlditemObjectInfos> WorldObjects;
extern unordered_map<UINT16, int> ItemSpawns;
extern const unordered_map<UINT16, int> MaxItemCount;
extern UINT16 nextWorldObjectId;

UINT64 GenerateWorldObjectUUID();
bool ItemSpawnManager(WorldObjectSpawnPacket& ClObj);