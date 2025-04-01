#pragma once
#include"deFine.h"
#include"Packet.h"


//패킷 헤더 만들고 그걸로 길이 값 추측하는 거 필요. 그걸 통해서 enum값 정확한지 판단 필요
class User {
public:

	enum User_State {
		USER_STATE_NONE,
		USER_STATE_LOGIN,
		USER_STATE_LOBBY,
		USER_STATE_ROOM,
		USER_STATE_GAME,
		USER_STATE_END,
	};

	void Init(string* UserId);
	void Clear();
	void EnterRoom();
	char m_userId[32];
	User_State userState = USER_STATE_NONE;
	bool host = false;
	bool ready = false;
	string m_userKey;
	SOCKET sock;
	bool Login = false;
private:



};
