#pragma once
#include "deFine.h"
#pragma pack(push, 1)
enum PacketStatus : UINT16
{
    SYS_USER_CONNECT,
    SYS_USER_DISCONNECT,
    SYS_END,

    // DB
    DB_END,

    // Client
    LOGIN_REQUEST,
    LOGIN_RESPONSE,

    ROOM_LIST_REQUEST,     // �� ��� ��û
    ROOM_LIST_RESPONSE,    // �� ��� ����

    ROOM_CREATE_REQUEST,   // �� ���� ��û
    ROOM_CREATE_RESPONSE,  // �� ���� ����
    ROOM_CREATE_SUCCESS,
    ROOM_CREATE_FAIL,

    ROOM_ENTER_REQUEST,    // �� ���� ��û
    ROOM_ENTER_RESPONSE,   // �� ���� ����
    ROOM_IN_FAIL,
    ROOM_IN_SUCCESS,

    ROOM_READY_RESPONSE,
    ROOM_ALL_READY,

    ROOM_LEAVE_REQUEST,    // �� ������ ��û
    ROOM_LEAVE_RESPONSE,   // �� ������ ����

    ROOM_UPDATE_NOTIFY,    // �� ���� ���� (�ο�, ���� �� ������Ʈ)

    ROOM_CLOSE_REQUEST,    // �� �ݱ� ��û
    ROOM_CLOSE_RESPONSE,   // �� �ݱ� ����
    ROOM_CLOSED_NOTIFY,    // �� ���� �˸�
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


typedef struct RoomInResponse {
    UINT16 PacketId;
    uint32_t roomId;
    char userName[32];
    char roomName[32];
    //roomresponse���� �� �Ѱ����� ����
    ADD_SERIALIZE_FUNCS(RoomInResponse)
} RoomInResponse;

typedef struct RoomReadyResponse {
    UINT16 PacketId;
    uint32_t userId;
    uint8_t userReadyStatus;
};
#pragma pack(pop)


