#pragma once  
#include"deFine.h"  
#include"Packet.h"  
#include"User.h"
#include"Room.h"


struct Inventory {
    array<UINT16, 4> iteminfo;
};

struct GamePlayer {
    shared_ptr<User> user;
    SOCKET sock;
    float x,y,z;
    Inventory inven;
};

inline unordered_map<UINT16, vector<GamePlayer>> GameStartUsers;
void MovePlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, RoomStart& info);
void InGamePlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, PlayerStatus& SomePlayer);
