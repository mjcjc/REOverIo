#pragma once
#include "deFine.h"
#pragma pack(push, 1)
enum class  PacketStatus : UINT16
{
    NONE,

    LOGIN_REQUEST = 100,
    LOGIN_SUCCESS = 101,
    LOGIN_FAIL = 102,

    ROOM_CREATE_REQUEST = 110,       // �� ���� ��û
    ROOM_CREATE_SUCCESS = 111,
    ROOM_CREATE_FAIL = 112,

    ROOM_ENTER_REQUEST = 120,        // �� ���� ��û
    ROOM_IN_SUCCESS = 121,        // �� ���� ����
    ROOM_IN_FAIL = 122,              // �� ���� ����

    ROOM_LEAVE_REQUEST = 130,        // �� ������ ��û
    ROOM_LEAVE_SUCCESS = 131,        // �� ������ ����
    ROOM_LEAVE_FAIL = 132,           // �� ������ ����

    ROOM_UPDATE_NOTIFY = 140,        // �� ���� ���� (�漳�� ������Ʈ)  
    ROOM_UPDATE_SUCCESS = 141,       // �� ���� ����
    ROOM_UPDATE_FAIL = 142,           // �� ���� ����

    ROOM_CLOSE_REQUEST = 150,        // �� �ݱ� ��û
    ROOM_CLOSE_SUCCESS = 151,        // �� �ݱ� ����
    ROOM_CLOSE_FAIL = 152,           // �� �ݱ� ����

    ROOM_LOBY_UPDATE = 160,      //�� ���� ����Ʈ �޴°�

    USER_STATUS_LOBY = 200,    //�÷��̾� �κ� �ִ� ���
    USER_STATUS_LOGIN = 300,  //�÷��̾� �α����ϰ� �� ���?

    PLAYER_READY_TOGGLE_REQUEST = 170,  // �÷��̾� �غ� ���� ��� ��û
    PLAYER_READY_TOGGLE_SUCCESS = 171,  // �غ� ���� ���� ����
    PLAYER_READY_TOGGLE_FAIL = 172,     // �غ� ���� ���� ����

    ROOM_START_REQUEST = 180, //�濡�� ���� ����
    ROOM_START_SUCCESS = 181, //�濡�� ���� ���� ������
    ROOM_START_FAIL = 182, //�濡�� ���� ���� ���н�

};
enum ItemID : UINT16 // ���ι̼� �� �������� ���� ���� ID ==> �÷��̾� �κ��丮�� �� �� �ִ� ������ ������
{
    //�Ĺ� �׸� ���� �̼� ������
    MAIN_PLANT_SEED ,    //����
    MAIN_PLANT , //�� �ڶ� �Ĺ� ������ �ܰ�
    MAIN_PLANT_COAL , // ��ź
    MAIN_PLANT_EXTINGUISHER ,// ��ȭ��

    //������ �׸� ���� �̼� ������
    MAIN_POWDER_RAW ,    //�����
    MAIN_POWDER_REFINED ,    // ������ 
    MAIN_POWDER_PROCESSED ,  //������

    // �������� ����ϴ� �̼� ������
    MAIN_BOX,   // ���� �Ϸ�� ��ǥ ������
    MAIN_TOOLKIT , // ���� �ڽ�
    MAIN_LANTEN , //����

    //���Ǳ⿡�� �Ǹ��ϴ� ������
    ITEM_STIMULANT, //������
    ITEM_GUN ,   //��
    ITEM_ACCELERATOR ,   //������
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
enum class PlayerPacketStatus : UINT16
{
    NONE = 1000,

    //ITEM_START = 1100,

    // ===== PLAYER ���� =====
    PLAYER_STATUS_NOTIFY = 1001,

    // ===== ITEM: �Ա� =====
    ITEM_PICKUP_REQUEST = 1010,
    ITEM_PICKUP_SUCCESS = 1011,
    ITEM_PICKUP_FAILED = 1012,

    // ===== ITEM: ��� =====
    ITEM_USE_REQUEST = 1020,
    ITEM_USE_SUCCESS = 1021,
    ITEM_USE_FAILED = 1022,

    // ===== ITEM: ������ =====
    ITEM_DROP_REQUEST = 1030,
    ITEM_DROP_SUCCESS = 1031,
    ITEM_DROP_FAILED = 1032,

    // ===== ITEM: ���� =====
    ITEM_EQUIP_REQUEST = 1040 ,
    ITEM_EQUIP_SUCCESS = 1041,
    ITEM_EQUIP_FAILED = 1042,


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
typedef struct PlayerinfoStatus {
	UINT16 packetId;
	uint32_t roomID;
	ADD_SERIALIZE_FUNCS(PlayerinfoStatus)
};
//�� ������� ���� �����ִ°�
typedef struct RoomListGet
{
    UINT16 packetID;
	char roomName[32];
    char hostName[32];
    uint32_t userCount;
    uint32_t maxuserCount;
    UINT16 roomMode;
	ADD_SERIALIZE_FUNCS(RoomListGet)
};
struct RoomStart //���� �����ϰ� � ������ ��û�� ���´��� �׸��� ��� roomID���� ��û�� ���´��� ��
{
    UINT16 packetID;
    char hostName[32];
    UINT32 roomID;
    ADD_SERIALIZE_FUNCS(RoomStart)
};
struct PlayerStatus
{
    UINT16 packetID;

    char playerId[32]; // �÷��̾� ID

    float positionX; // �÷��̾� ��ġ
    float positionY;
    float positionZ;

    float rotationX; // �÷��̾� ȸ��
    float rotationY;
    float rotationZ;

    float viewDirectionX; // �÷��̾��� �ü� ���� (ī�޶��� ȸ����)
    float viewDirectionY;
    float viewDirectionZ;
    /*
     * 0 = Idle, 1 = walk
     * 0 = none, 1 = One, 2 = Hold , 3 = mouseOne, 4 = mouseHold
     * 0 = none, 1 = OneHand, 2 = TwoHand, 3 = aim
     */
    UINT16 movementState;    //  ������ ����
    UINT16 actionTriggerState;  // ������ Ʈ���� ���� ����
    UINT16 actionState;      //  ��ü ������ ���� ����
     float speed;             // �ȴ� ���۰� ���� �� ��
     float directionX;        // �����϶� �ִϸ��̼� ��� �ʿ��� ���� ����
     float directionY;
    ADD_SERIALIZE_FUNCS(PlayerStatus)
};

// ������ �Ա� �̺�Ʈ ����ü
struct ItemPickupEvent
{
    UINT16 packetID; // ��Ŷ ID

    char playerId[32]; // �÷��̾� ID

    UINT16 itemID;   // ���� ������ ID
    UINT16 WorldObjectID;
    ADD_SERIALIZE_FUNCS(ItemPickupEvent)
};

// ������ ��� �̺�Ʈ ����ü
struct ItemUseEvent
{
    UINT16 packetID; // ��Ŷ ID


    char playerId[32]; // �÷��̾� ID

    UINT16 itemID;    // ����� ������ ID
    UINT16 slotIndex; // �������� ��ġ�� ���� �ε���
    ADD_SERIALIZE_FUNCS(ItemUseEvent)
};
// ������ ������ �̺�Ʈ ����ü
struct ItemDropEvent
{
    UINT16 packetID; // ��Ŷ ID


    char playerId[32]; // �÷��̾� ID

    UINT16 itemID;    // ���� ������ ID
    UINT16 slotIndex; // ���� �������� �ִ� ���� �ε���
    ADD_SERIALIZE_FUNCS(ItemDropEvent)
};

// ������ ���� �̺�Ʈ ����ü
struct ItemEquipEvent
{
    UINT16 packetID; // ��Ŷ ID

    char playerId[32]; // �÷��̾� ID

    UINT16 itemID;     // ������ ������ ID
    UINT16 isEquipped; // 0: �������� ��� ���� ����, 1: �������� ��� ���� (���� ����)
    UINT16 slotIndex;  // ������ �������� ��ġ�� �κ��丮 ���� �ε��� (���� 0~3)
    ADD_SERIALIZE_FUNCS(ItemEquipEvent)
};

//�����ĺ��� �ִ� ��Ŷ
struct WorldObjectSpawnPacket 
{
	UINT16 packetID; // ��Ŷ ID
	
    UINT16 itemID;   // ������ ������Ʈ ID
	UINT16 WorldObjectID;
	
    float posX; // ������ ������Ʈ ��ġ
	float posY;
	float posZ;
	
    float rotX; // ������ ������Ʈ ��ġ
    float rotY;
    float rotZ;

    float velocityX; // ������ ������Ʈ ��ġ
    float velocityY;
    float velocityZ;

    ADD_SERIALIZE_FUNCS(WorldObjectSpawnPacket)
};
#pragma pack(pop)

