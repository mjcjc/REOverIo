#pragma once  
#include"deFine.h"  
#include"Packet.h"  
#include"User.h"
struct Inventory {
    array<UINT16, 4> iteminfo;
};

struct GamePlayer {
    shared_ptr<User> user;
    SOCKET sock;
    float x,y,z;
    Inventory inven;
};


void MovePlayer();

