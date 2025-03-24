#include"Packet.h"
#include"User.h"

#include <algorithm>
#include"Room.h"


//TODO:방 request받고 그거에 대해 수락한 뒤 만들어주기.

int GenerateRoomId()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, 9999);
    return dist(gen);
}

int RoomMake(RoomCreateRequest& room, unordered_map<uint32_t, RoomInfo>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo)
{
    User user;
	user = *UserInfo[room.userName];
    RoomInfo newRoom;
    user.host = true;
    user.userState = User::USER_STATE_ROOM;
    strcpy(newRoom.hostName, room.userName);
    strcpy(newRoom.roomName, room.roomName);
    newRoom.roomId = GenerateRoomId();
    newRoom.userCount = 1;
    newRoom.RoomMode = room.RoomMode;
    newRoom.maxUserCount = room.MaxCount;
    newRoom.userinfo.emplace_back(user);
    Rooms[newRoom.roomId] = newRoom;
    cout << "방 만들때 id" << newRoom.roomId << endl;
    return newRoom.roomId;
}
bool RoomInSide(RoomRequest& reqroom, unordered_map<uint32_t, RoomInfo>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo)
{
    auto get_user = UserInfo.find(reqroom.userName);
	shared_ptr<User> user = get_user->second;
    cout << reqroom.roomId << endl;
    auto roomId = Rooms.find(reqroom.roomId);
    if (roomId == Rooms.end())
    {
        cout << "Not Found Room" << endl;
        return false;
    }
    RoomInfo& Inroom = roomId->second;
    if (Inroom.maxUserCount < Inroom.userCount) {
        cout << "Room Max!" << endl;
        return false;
    }
    cout << " 들어가 졌어용" << endl;

    user->userState = User::USER_STATE_ROOM;
    Inroom.userCount++;
    Inroom.userinfo.emplace_back(user);
    return true;

}
void RoomOutSide(RoomInfo& room, RoomRequest userinfo, unordered_map<uint32_t, RoomInfo>& Rooms)
{
	auto roomId = Rooms.find(userinfo.roomId);
	if (roomId == Rooms.end())
	{
		cout << "Not Found Room" << endl;
		return;
	}
	RoomInfo& Outroom = roomId->second;
	Outroom.userCount--;
	Outroom.userinfo.erase(remove_if(Outroom.userinfo.begin(), Outroom.userinfo.end(), [&](User& user) {
		return user.m_userId == userinfo.userName;
		}), Outroom.userinfo.end());
	cout << "나갔어용" << endl;
}

void RoomSomeReady()
{

}
void RoomFixedUpdate()
{

}