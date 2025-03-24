#pragma once
#include"deFine.h"
#include"Packet.h"
#include"user.h"

typedef struct RoomInfo {
	UINT16 PacketId;
	vector<User> userinfo;//��������
	uint32_t roomId;

	char roomName[32];
	char hostName[32];
	UINT16 RoomMode;//����
	uint32_t userCount;//��������
	uint32_t maxUserCount;//���� �ִ�
	
	bool isAllReady() const {
		for (const auto& user : userinfo) {
			if (!user.ready) {
				return false;
			}
		}
		return true;
	}
}RoomInfo;



bool RoomInSide(RoomRequest& reqroom, unordered_map<uint32_t, RoomInfo>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo);
void RoomOutSide(RoomInfo& room, uint32_t userId, unordered_map<uint32_t, RoomInfo>& Rooms);
int RoomMake(RoomCreateRequest& room, unordered_map<uint32_t, RoomInfo>& Rooms, unordered_map<string, shared_ptr<User>>& UserInfo);
void RoomFixedUpdate();
#pragma once

//�̰� ��� �������� �Ѹ� ������. �濡 ��������.
struct RoomSet {
	//����� �ð��� �׳� 1�ʸ� �������� ����Ͽ� �����ϴ°� �� ������.
	uint32_t D_day;
	uint32_t DayTime;
	uint32_t vote;
	uint32_t nigthTime;
	uint32_t roomMode;
	/*Day�ϼ�
	�Ϸ�ð�
	�ִ��ο�
	��ǥ�İ�
	��ð�


	����
	��忡 ���� ��ǥġ ���� ����*/
};

struct readyStatus {
	UINT16 packetId;
	uint32_t userId;
	uint8_t ready;
};
