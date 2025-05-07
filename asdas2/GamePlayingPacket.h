#pragma once  
#include"deFine.h"  
#include"Packet.h"  
#include"User.h"
#include"Room.h"


struct Inventory {
    array<UINT16, 4> iteminfo;
    Inventory() {
        iteminfo.fill(0);
    }
};

struct GamePlayer {
    shared_ptr<User> user;
    SOCKET sock;
    float x,y,z;
	float rotationX, rotationY, rotationZ;
    Inventory inven;
	UINT16 EquipItem;
};

inline unordered_map<UINT16, vector<GamePlayer>> GameStartUsers;

void InitPlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, RoomStart& info);
void InGamePlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, PlayerStatus& SomePlayer);
bool InventoryItemAdd(ItemPickupEvent& PickItem);
bool InventoryItemRemove(ItemDropEvent& DropItem);
bool InventoryItemUse(ItemUseEvent& UseItem);
bool InventoryItemEquip(ItemEquipEvent& eqItem);