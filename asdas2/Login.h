#pragma once
#include "Packet.h"
#include "User.h"



bool Logrequest(LoginRequest& request, unordered_map<string, shared_ptr<User>>& user);
