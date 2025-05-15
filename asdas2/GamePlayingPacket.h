// GamePlayingPacket.h
#pragma once
#include "deFine.h"
#include "Packet.h"
#include "User.h"
#include "Room.h"

struct InventorySlot {
    UINT16 itemID = 0;
    UINT16 worldObjectID = 0;
};

struct Inventory {
    std::array<InventorySlot, 4> slots;
};

enum Job : UINT16 {
    Mafia,
    police,
    MafiaKing,
    PoliceKing,
};

struct GamePlayer {
    shared_ptr<User> user;
    SOCKET sock;
    float x, y, z;
    float rotationX, rotationY, rotationZ;
    Inventory inven;
    UINT16 playerJob;
    UINT16 EquipItemID;
    UINT16 playerEquiptHand;
};

inline unordered_map<UINT16, vector<GamePlayer>> GameStartUsers;

void InitPlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, RoomStart& info);
void InGamePlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, PlayerStatus& SomePlayer);
bool InventoryItemAdd(ItemPickupEvent& PickItem);
bool InventoryItemRemove(ItemDropEvent& DropItem);
bool InventoryItemUse(ItemUseEvent& UseItem);
bool InventoryItemEquip(ItemEquipEvent& eqItem);
void PlaneGauge(planeMisson& planepacket);
