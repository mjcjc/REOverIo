#include"Packet.h"
#include"User.h"
#include <algorithm>
#include"Room.h"

int GenerateRoomId()
{
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> dist(0, 9999);
    return dist(gen);
}

int RoomMake(RoomCreateRequest& room, unordered_map<uint32_t, RoomInfo>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo)
{
    shared_ptr<User> user = UserInfo[room.userName];
    RoomInfo newRoom;
    user->host = true;
    user->ready = true;
    user->userState = User::USER_STATE_ROOM;
    strcpy_s(newRoom.hostName, room.userName);
    strcpy_s(newRoom.roomName, room.roomName);
    newRoom.roomId = GenerateRoomId();
    newRoom.userCount = 0; // ���� �� 1����� ����
    newRoom.RoomMode = room.RoomMode;
    newRoom.maxUserCount = room.MaxCount;
    newRoom.userinfo.emplace_back(user); // shared_ptr �״�� ����
    Rooms[newRoom.roomId] = newRoom;
    cout << "�� ���鶧 id" << newRoom.roomId << endl;
    return newRoom.roomId;
}

bool RoomInSide(RoomRequest& reqroom, unordered_map<uint32_t, RoomInfo>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo)
{
    auto get_user = UserInfo.find(reqroom.userName);
    if (get_user == UserInfo.end()) return false;

    shared_ptr<User> user = get_user->second;
    auto roomId = Rooms.find(reqroom.roomId);
    if (roomId == Rooms.end()) {
        cout << "Not Found Room" << endl;
        return false;
    }

    RoomInfo& Inroom = roomId->second;
    if (Inroom.userCount >= Inroom.maxUserCount) {
        cout << "Room Max!" << endl;
        return false;
    }

    cout << " �� �����" << endl;
    user->userState = User::USER_STATE_ROOM;
    Inroom.userCount++;
    Inroom.userinfo.emplace_back(user);
    return true;
}

void RoomOutSide(RoomRequest& userinfo, unordered_map<uint32_t, RoomInfo>& Rooms)
{
    auto roomId = Rooms.find(userinfo.roomId);
    if (roomId == Rooms.end()) {
        cout << "Not Found Room" << endl;
        return;
    }

    RoomInfo& Outroom = roomId->second;
    auto userptr = find_if(Outroom.userinfo.begin(), Outroom.userinfo.end(), [&](const shared_ptr<User>& user) {
        return strcmp(user->m_userId, userinfo.userName) == 0;
        });

    if (userptr != Outroom.userinfo.end()) {
        (*userptr)->userState = User::USER_STATE_LOBBY;
    }

    Outroom.userCount--;
    Outroom.userinfo.erase(remove_if(Outroom.userinfo.begin(), Outroom.userinfo.end(), [&](const shared_ptr<User>& user) {
        return strcmp(user->m_userId, userinfo.userName) == 0;
        }), Outroom.userinfo.end());

    if (Outroom.userCount < 1) {
        Rooms.erase(roomId);
        cout << "�� ����" << endl;
    }
    else {
        cout << "�� �������" << endl;
    }
}

void RoomSomeReady(PlayerReadySend& Readyplayer, unordered_map<uint32_t, RoomInfo>& Rooms)
{
    auto roomId = Rooms.find(Readyplayer.roomID);
    if (roomId == Rooms.end()) {
        cout << "Not Found Room" << endl;
        return;
    }

    RoomInfo& room = roomId->second;
    auto userIt = find_if(room.userinfo.begin(), room.userinfo.end(), [&](const shared_ptr<User>& user) {
        return strcmp(user->m_userId, Readyplayer.userName) == 0;
        });

    if (userIt != room.userinfo.end()) {
        (*userIt)->ready = (Readyplayer.readyStatus != 0);
        Readyplayer.readyStatus = (*userIt)->ready ? 1 : 0;
    }
}

void RoomFixedUpdate(RoomNOtify& FixRoom, unordered_map<uint32_t, RoomInfo>& Rooms)
{
    auto roomId = Rooms.find(FixRoom.roomId);
    if (roomId == Rooms.end()) {
        cout << "Not Found Room" << endl;
        return;
    }

    RoomInfo& roomUpdate = roomId->second;
    roomUpdate.RoomMode = FixRoom.roomMode;
    roomUpdate.roomSet.DayTime = FixRoom.DayTime;
    roomUpdate.roomSet.D_day = FixRoom.D_day;
    roomUpdate.roomSet.nigthTime = FixRoom.nigthTime;
    roomUpdate.roomSet.vote = FixRoom.vote;

    cout << "�� ���� ����" << endl;
}

void LobbyUser(SOCKET client_sock, unordered_map<SOCKET, shared_ptr<User>>& userkey)
{
    if (userkey.contains(client_sock)) {
        userkey[client_sock]->userState = User::USER_STATE_LOBBY;
    }
}
