#pragma once
#include "deFine.h"
#pragma pack(push, 1)
enum PacketStatus : UINT16
{
    NONE,

    // Client
    LOGIN_REQUEST,
    LOGIN_SUCCESS,
    LOGIN_FAIL,

    ROOM_CREATE_REQUEST,       // �� ���� ��û
    ROOM_CREATE_SUCCESS,
    ROOM_CREATE_FAIL,

    ROOM_ENTER_REQUEST,        // �� ���� ��û
    ROOM_IN_SUCCESS,        // �� ���� ����
    ROOM_IN_FAIL,              // �� ���� ����

    ROOM_LEAVE_REQUEST,        // �� ������ ��û
    ROOM_LEAVE_SUCCESS,        // �� ������ ����
    ROOM_LEAVE_FAIL,           // �� ������ ����

    ROOM_UPDATE_NOTIFY,        // �� ���� ���� (�漳�� ������Ʈ)
    ROOM_UPDATE_SUCCESS,       // �� ���� ����
    ROOM_UPDATE_FAIL,           // �� ���� ����

    ROOM_CLOSE_REQUEST,        // �� �ݱ� ��û
    ROOM_CLOSE_SUCCESS,        // �� �ݱ� ����
    ROOM_CLOSE_FAIL,           // �� �ݱ� ����

    ROOM_LOBY_UPDATE,      //�� ���� ����Ʈ �޴°�

    USER_STATUS_LOBY,    //�÷��̾� �κ� �ִ� ���
    USER_STATUS_LOGIN,  //�÷��̾� �α����ϰ� �� ���?

    PLAYER_READY_TOGGLE_REQUEST,  // �÷��̾� �غ� ���� ��� ��û
    PLAYER_READY_TOGGLE_SUCCESS,  // �غ� ���� ���� ����
    PLAYER_READY_TOGGLE_FAIL,     // �غ� ���� ���� ����

};
enum ItemID : UINT16 // ���ι̼� �� �������� ���� ���� ID ==> �÷��̾� �κ��丮�� �� �� �ִ� ������ ������
{
    //�Ĺ� �׸� ���� �̼� ������
    MAIN_PLANT_SEED,    //����
    MAIN_PLANT, //�� �ڶ� �Ĺ� ������ �ܰ�
    MAIN_PLANT_COAL, // ��ź
    MAIN_PLANT_EXTINGUISHER,// ��ȭ��

    //������ �׸� ���� �̼� ������
    MAIN_POWDER_RAW,    //�����
    MAIN_POWDER_REFINED,    // ������ 
    MAIN_POWDER_PROCESSED,  //������

    // �������� ����ϴ� �̼� ������
    MAIN_BOX,   // ���� �Ϸ�� ��ǥ ������
    MAIN_TOOLKIT, // ���� �ڽ�
    MAIN_LANTEN, //����

    //���Ǳ⿡�� �Ǹ��ϴ� ������
    ITEM_STIMULANT, //������
    ITEM_GUN,   //��
    ITEM_ACCELERATOR,   //������
    ITEM_BRAINWASH, //������
};

enum MapModInfo : UINT16 //�� ��� ����
{
    PLANT,
    POWDER,
};

enum IsActive : UINT16 // BOOL ���� �� �ѱ�ϱ� �켱 enum���� �ٲ۰�.
{
    True,
    False,
};

#define ADD_SERIALIZE_FUNCS(T) \
    std::vector<char> serialize() const { \
        std::vector<char> buffer(sizeof(T)); \
        std::memcpy(buffer.data(), this, sizeof(T)); \
        return buffer; \
    } \
    static T deserialize(const std::vector<char>& buffer) { \
        if (buffer.size() < sizeof(T)) { \
            throw std::runtime_error("Invalid packet size"); \
        } \
        T packet; \
        std::memcpy(&packet, buffer.data(), sizeof(T)); \
        return packet; \
    }
typedef struct LoginRequest {
    UINT16 PacketId;
    char username[32];
    ADD_SERIALIZE_FUNCS(LoginRequest)
} LoginRequest;

typedef struct LoginResponse {

    UINT16 PacketId;
    char username[32];
    char message[64];
    ADD_SERIALIZE_FUNCS(LoginResponse)
} LoginResponse;
typedef struct RoomCreateRequest {
    UINT16 PacketId;
    UINT16 RoomMode;
    uint32_t MaxCount;
    char roomName[32];
    char userName[32];
    ADD_SERIALIZE_FUNCS(RoomCreateRequest)

} RoomCreateRequest;

typedef struct RoomCreateResponse {
    UINT16 PacketId;
    uint32_t roomId; // ������ �� ID

    ADD_SERIALIZE_FUNCS(RoomCreateResponse)
} RoomCreateResponse;

typedef struct RoomRequest {
    UINT16 PacketId;
    uint32_t roomId;
    char userName[32];
    ADD_SERIALIZE_FUNCS(RoomRequest)
} RoomChatRequest;

typedef struct RoomNOtify {
    UINT16 packetId;
    char userName[32];
	uint32_t roomId;
    uint32_t D_day;
    uint32_t DayTime;
    uint32_t policeCount;
    uint32_t MaxCount;
    uint32_t vote;
    uint32_t nigthTime;
    uint32_t roomMode;
	ADD_SERIALIZE_FUNCS(RoomNOtify)
}RoomNOtify;

typedef struct RoomInResponse {
    UINT16 PacketId;
    uint32_t roomId;
    char userName[32];
    char roomName[32];
    //roomresponse���� �� �Ѱ����� ����
    ADD_SERIALIZE_FUNCS(RoomInResponse)
} RoomInResponse;
//���� �޴°�
typedef struct PlayerReadySend {
    UINT16 packetId;
    char userName[32];
    uint32_t roomID;
    UINT16 readyStatus;
	ADD_SERIALIZE_FUNCS(PlayerReadySend)
}PlayerReady;
//���� �����ִ� ��
typedef struct PlayerInfoGet {
    UINT16 packetId;
    char userName[32];
    UINT16 readyStatus;
	ADD_SERIALIZE_FUNCS(PlayerInfoGet)
};
//�÷��̾� ����
typedef struct PlayerStatus {
	UINT16 packetId;
	uint32_t roomID;
	ADD_SERIALIZE_FUNCS(PlayerStatus)
};
//�� ������� ���� �����ִ°�
typedef struct RoomListGet
{
    UINT16 packetID;
	char roomName[32];
    char userName[32];
    uint32_t userCount;
    uint32_t maxuserCount;
    UINT16 roomMode;
	ADD_SERIALIZE_FUNCS(RoomListGet)
};
#pragma pack(pop)

