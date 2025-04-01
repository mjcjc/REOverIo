#pragma once
#include"deFine.h"
#include"Packet.h"


//��Ŷ ��� ����� �װɷ� ���� �� �����ϴ� �� �ʿ�. �װ� ���ؼ� enum�� ��Ȯ���� �Ǵ� �ʿ�
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
