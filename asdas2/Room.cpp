#include"Packet.h"
#include"User.h"

#include <algorithm>
#include"Room.h"


//TODO:�� request�ް� �װſ� ���� ������ �� ������ֱ�.

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
    cout << "�� ���鶧 id" << newRoom.roomId << endl;
    return newRoom.roomId;
}
bool RoomInSide(RoomRequest& reqroom, unordered_map<uint32_t, RoomInfo>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo)
{
    auto get_user = UserInfo.find(reqroom.userName);
    User& user = get_user->second;
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
    cout << " �� �����" << endl;

    user.userState = User::USER_STATE_ROOM;
    Inroom.userCount++;
    Inroom.userinfo.emplace_back(user);
    return true;

}
void RoomOutSide(RoomInfo& room, RoomRequest userinfo)
{
	auto get_user = UserMap.find(userinfo.userName);
	User& user = get_user->second;
	auto& userList = room.userinfo;
	auto findUser = find_if(userList.begin(), userList.end(),
		[&user](const User& u) {
			return strcmp(u.m_userId, user.m_userId) == 0; });
	userList.erase(findUser);
	user.userState = user.USER_STATE_LOBBY;
}

void RoomSomeReady()
{

}
void RoomFixedUpdate()
{

}