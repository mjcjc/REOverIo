#pragma once
#include"deFine.h"
#include"Packet.h"
#include"user.h"
struct RoomSet {
	//참고로 시간은 그냥 1초를 기준으로 계산하여 전달하는게 더 나을듯.
	uint32_t D_day;
	uint32_t DayTime;
	uint32_t vote;
	uint32_t nigthTime;
	
};

typedef struct RoomInfo {
	UINT16 PacketId;
	vector<shared_ptr<User>> userinfo;//유저정보
	uint32_t roomId;
	RoomSet roomSet;
	char roomName[32];
	char hostName[32];
	UINT16 RoomMode;//방모드
	uint32_t userCount;//유저현재
	uint32_t maxUserCount;//유저 최대
	
	
}RoomInfo;
//이건 모든 유저에게 뿌릴 데이터. 방에 들어왔을때.



bool RoomInSide(RoomRequest& reqroom, unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo);
void RoomOutSide(RoomRequest& userinfo, unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms);
int RoomMake(RoomCreateRequest& room, unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo);
void RoomFixedUpdate(RoomNOtify& FixRoom , unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms);
void RoomSomeReady(PlayerReadySend& room, unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms);
void LobbyUser(SOCKET client_sock, unordered_map<SOCKET, shared_ptr<User>>& userkey);
void LoginScene(SOCKET client_sock, unordered_map<SOCKET, shared_ptr<User>>& userkey);
void GameStart(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, PlayerinfoStatus& info);

struct readyStatus {
	UINT16 packetId;
	uint32_t userId;
	uint8_t ready;
};
	