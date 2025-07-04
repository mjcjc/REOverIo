#include"Packet.h"
#include"User.h"
#include"GamePlayingPacket.h"
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
int RoomMake(RoomCreateRequest& room, unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo)
{
    shared_ptr<User> user = UserInfo[room.userName];
    auto newRoom = make_shared<RoomInfo>();
    user->host = true;
    user->ready = true;
    user->userState = User::USER_STATE_ROOM;
    strcpy_s(newRoom->hostName, room.userName);
    strcpy_s(newRoom->roomName, room.roomName);
    newRoom->roomId = GenerateRoomId();
    newRoom->userCount = 0;
    newRoom->RoomMode = room.RoomMode;
    newRoom->roomSet.maxUserCount = room.MaxCount;
    Rooms[newRoom->roomId] = newRoom;
    cout << "방 만들때 id" << newRoom->roomId << endl;
    return newRoom->roomId;
}
bool RoomInSide(RoomRequest& reqroom, unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo)
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
    shared_ptr<RoomInfo> Inroom = roomId->second;
    if (Inroom->userCount >= Inroom->roomSet.maxUserCount) {
        cout << "Room Max!" << endl;
        return false;
    }
    user->userState = User::USER_STATE_ROOM;
    Inroom->userCount++;
    Inroom->userinfo.emplace_back(user);
    cout << " 들어가 졌어용" << endl;
    return true;

}
void RoomOutSide(RoomRequest& userinfo, unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms)
{

	auto roomId = Rooms.find(userinfo.roomId);
	if (roomId == Rooms.end())
	{
		cout << "Not Found Room" << endl;
		return;
	}
    shared_ptr<RoomInfo> Outroom = roomId->second;
    auto userptr = find_if(Outroom->userinfo.begin(), Outroom->userinfo.end(), [&](const shared_ptr<User>& user) {
        return user->m_userId == userinfo.userName;
        });

    if (userptr != Outroom->userinfo.end()) {
        if (userptr != Outroom->userinfo.end()) {
            (*userptr)->userState = User::USER_STATE_LOBBY;
        }
    }

    Outroom->userCount--;
    Outroom->userinfo.erase(remove_if(Outroom->userinfo.begin(), Outroom->userinfo.end(), [&](const shared_ptr<User>& user) {
        return user->m_userId == userinfo.userName;
        }), Outroom->userinfo.end());

    if (Outroom->userCount < 1)
    {
        Rooms.erase(roomId);
        cout << "방 삭제" << endl;
    }
    else
    {
        cout << "방 나갔어용" << endl;
    }

}

void RoomSomeReady(PlayerReadySend& Readyplayer, unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms)
{  
   auto roomId = Rooms.find(Readyplayer.roomID);  
   if (roomId == Rooms.end())  
   {  
       cout << "Not Found Room" << endl;  
       return;  
   }  
   shared_ptr<RoomInfo> room = roomId->second;
   auto userIt = find_if(room->userinfo.begin(), room->userinfo.end(), [&](const shared_ptr<User>& user) {
       return strcmp(user->m_userId, Readyplayer.userName) == 0;  
   });  
   if (userIt != room->userinfo.end())
   {  
       if (Readyplayer.readyStatus == 0)
       {
           (*userIt)->ready = false;
           Readyplayer.readyStatus = 0;
       }else
	   {
		   (*userIt)->ready = true;
		   Readyplayer.readyStatus = 1;
       };
   }
   

}
void RoomFixedUpdate(RoomNOtify& FixRoom, unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms)
{
	auto roomId = Rooms.find(FixRoom.roomId);
	if (roomId == Rooms.end())
	{
		cout << "Not Found Room" << endl;
		return;
	}

    shared_ptr<RoomInfo> roomUpdate = roomId->second;

    roomUpdate->RoomMode = FixRoom.roomMode;
    roomUpdate->roomSet.DayTime = FixRoom.DayTime;
    roomUpdate->roomSet.D_day = FixRoom.D_day;
    roomUpdate->roomSet.nigthTime = FixRoom.nigthTime;
    roomUpdate->roomSet.vote = FixRoom.vote;
    roomUpdate->roomSet.maxUserCount = FixRoom.MaxCount;
	roomUpdate->roomSet.policeCount = FixRoom.policeCount;

	cout << "방 정보 수정" << endl;
}
bool GameStart(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, RoomStart& info)
{
    int count = 0;
    //방 아이디 찾아서 유저 객체를 가져오고 반복문을 돌려 멤버변수의 ready값이 true이면 play함수 호출
    for (auto readyuser : Rooms[info.roomID]->userinfo)
    {
		if (readyuser->ready == true)
		{
            count++;
		}
    }
	if (count == Rooms[info.roomID]->userCount)
	{
        InitPlayer(Rooms, info);
        return true;
    }
    else {
		cout << "게임 시작 못함" << endl;
        return false;
    }
}

void LobbyUser(SOCKET client_sock, unordered_map<SOCKET, shared_ptr<User>>& userkey)
{
    cout << "이쪽 한번 전송됨" << endl;
	if (userkey[client_sock]->userState != User::USER_STATE_LOBBY)
	{
		userkey[client_sock]->userState = User::USER_STATE_LOBBY;
	}
}

void LoginScene(SOCKET client_sock, unordered_map<SOCKET, shared_ptr<User>>& userkey)
{
    if (userkey[client_sock]->userState != User::USER_STATE_NONE)
    {
        userkey[client_sock]->userState = User::USER_STATE_NONE;
    }

}