#pragma once
#include "deFine.h"
#pragma pack(push, 1)
enum class  PacketStatus : UINT16
{
    NONE,

    LOGIN_REQUEST = 100,
    LOGIN_SUCCESS = 101,
    LOGIN_FAIL = 102,

    ROOM_CREATE_REQUEST = 110,       // 방 생성 요청
    ROOM_CREATE_SUCCESS = 111,
    ROOM_CREATE_FAIL = 112,

    ROOM_ENTER_REQUEST = 120,        // 방 입장 요청
    ROOM_IN_SUCCESS = 121,        // 방 입장 성공
    ROOM_IN_FAIL = 122,              // 방 입장 실패

    ROOM_LEAVE_REQUEST = 130,        // 방 나가기 요청
    ROOM_LEAVE_SUCCESS = 131,        // 방 나가기 성공
    ROOM_LEAVE_FAIL = 132,           // 방 나가기 실패

    ROOM_UPDATE_NOTIFY = 140,        // 방 정보 변경 (방설정 업데이트)  
    ROOM_UPDATE_SUCCESS = 141,       // 방 정보 성공
    ROOM_UPDATE_FAIL = 142,           // 방 정보 실패

    ROOM_CLOSE_REQUEST = 150,        // 방 닫기 요청
    ROOM_CLOSE_SUCCESS = 151,        // 방 닫기 성공
    ROOM_CLOSE_FAIL = 152,           // 방 닫기 실패

    ROOM_LOBY_UPDATE = 160,      //방 정보 리스트 받는거

    USER_STATUS_LOBY = 200,    //플레이어 로비에 있는 경우
    USER_STATUS_LOGIN = 300,  //플레이어 로그인하고 난 경우?

    PLAYER_READY_TOGGLE_REQUEST = 170,  // 플레이어 준비 상태 토글 요청
    PLAYER_READY_TOGGLE_SUCCESS = 171,  // 준비 상태 변경 성공
    PLAYER_READY_TOGGLE_FAIL = 172,     // 준비 상태 변경 실패

    ROOM_START_REQUEST = 180, //방에서 게임 시작
    ROOM_START_SUCCESS = 181, //방에서 게임 시작 성공시
    ROOM_START_FAIL = 182, //방에서 게임 시작 실패시

};
enum ItemID : UINT16 // 메인미션 및 아이템을 구분 짓는 ID ==> 플레이어 인벤토리에 들어갈 수 있는 아이템 종류들
{
    //식물 테마 메인 미션 아이템
    MAIN_PLANT_SEED ,    //씨앗
    MAIN_PLANT , //다 자란 식물 가공전 단계
    MAIN_PLANT_COAL , // 석탄
    MAIN_PLANT_EXTINGUISHER ,// 소화기

    //가루형 테마 메인 미션 아이템
    MAIN_POWDER_RAW ,    //원재료
    MAIN_POWDER_REFINED ,    // 정제된 
    MAIN_POWDER_PROCESSED ,  //가공된

    // 공용으로 사용하는 미션 아이템
    MAIN_BOX,   // 가공 완료된 목표 아이템
    MAIN_TOOLKIT , // 공구 박스
    MAIN_LANTEN , //랜턴

    //자판기에서 판매하는 아이템
    ITEM_STIMULANT, //각성제
    ITEM_GUN ,   //총
    ITEM_ACCELERATOR ,   //촉진제
    ITEM_BRAINWASH, //세뇌약
};

enum MapModInfo : UINT16 //맵 모드 정보
{
    PLANT,
    POWDER,
};

enum IsActive : UINT16 // BOOL 값을 못 넘기니까 우선 enum으로 바꾼거.
{
    True,
    False,
};
enum class PlayerPacketStatus : UINT16
{
    NONE = 1000,

    //ITEM_START = 1100,

    // ===== PLAYER 상태 =====
    PLAYER_STATUS_NOTIFY = 1001,

    // ===== ITEM: 먹기 =====
    ITEM_PICKUP_REQUEST = 1010,
    ITEM_PICKUP_SUCCESS = 1011,
    ITEM_PICKUP_FAILED = 1012,

    // ===== ITEM: 사용 =====
    ITEM_USE_REQUEST = 1020,
    ITEM_USE_SUCCESS = 1021,
    ITEM_USE_FAILED = 1022,

    // ===== ITEM: 버리기 =====
    ITEM_DROP_REQUEST = 1030,
    ITEM_DROP_SUCCESS = 1031,
    ITEM_DROP_FAILED = 1032,

    // ===== ITEM: 장착 =====
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
    uint32_t roomId; // 생성된 방 ID

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
    //roomresponse정보 뭐 넘겨줄지 생각
    ADD_SERIALIZE_FUNCS(RoomInResponse)
} RoomInResponse;
//내가 받는것
typedef struct PlayerReadySend {
    UINT16 packetId;
    char userName[32];
    uint32_t roomID;
    UINT16 readyStatus;
	ADD_SERIALIZE_FUNCS(PlayerReadySend)
}PlayerReady;
//내가 던져주는 것
typedef struct PlayerInfoGet {
    UINT16 packetId;
    char userName[32];
    UINT16 readyStatus;
	ADD_SERIALIZE_FUNCS(PlayerInfoGet)
};
//플레이어 상태
typedef struct PlayerinfoStatus {
	UINT16 packetId;
	uint32_t roomID;
	ADD_SERIALIZE_FUNCS(PlayerinfoStatus)
};
//방 만들어진 정보 던져주는거
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
struct RoomStart //방을 시작하고 어떤 유저가 요청을 보냈는지 그리고 어느 roomID에서 요청을 보냈는지 등
{
    UINT16 packetID;
    char hostName[32];
    UINT32 roomID;
    ADD_SERIALIZE_FUNCS(RoomStart)
};
struct PlayerStatus
{
    UINT16 packetID;

    char playerId[32]; // 플레이어 ID

    float positionX; // 플레이어 위치
    float positionY;
    float positionZ;

    float rotationX; // 플레이어 회전
    float rotationY;
    float rotationZ;

    float viewDirectionX; // 플레이어의 시선 방향 (카메라의 회전값)
    float viewDirectionY;
    float viewDirectionZ;
    /*
     * 0 = Idle, 1 = walk
     * 0 = none, 1 = One, 2 = Hold , 3 = mouseOne, 4 = mouseHold
     * 0 = none, 1 = OneHand, 2 = TwoHand, 3 = aim
     */
    UINT16 movementState;    //  움직임 제어
    UINT16 actionTriggerState;  // 아이템 트리거 동작 제어
    UINT16 actionState;      //  상체 움직임 상태 제어
     float speed;             // 걷는 동작과 연계 할 값
     float directionX;        // 움직일때 애니메이션 제어에 필요한 방향 정보
     float directionY;
    ADD_SERIALIZE_FUNCS(PlayerStatus)
};

// 아이템 먹기 이벤트 구조체
struct ItemPickupEvent
{
    UINT16 packetID; // 패킷 ID

    char playerId[32]; // 플레이어 ID

    UINT16 itemID;   // 먹은 아이템 ID
    UINT16 WorldObjectID;
    ADD_SERIALIZE_FUNCS(ItemPickupEvent)
};

// 아이템 사용 이벤트 구조체
struct ItemUseEvent
{
    UINT16 packetID; // 패킷 ID


    char playerId[32]; // 플레이어 ID

    UINT16 itemID;    // 사용한 아이템 ID
    UINT16 slotIndex; // 아이템이 위치한 슬롯 인덱스
    ADD_SERIALIZE_FUNCS(ItemUseEvent)
};
// 아이템 버리기 이벤트 구조체
struct ItemDropEvent
{
    UINT16 packetID; // 패킷 ID


    char playerId[32]; // 플레이어 ID

    UINT16 itemID;    // 버린 아이템 ID
    UINT16 slotIndex; // 버린 아이템이 있는 슬롯 인덱스
    ADD_SERIALIZE_FUNCS(ItemDropEvent)
};

// 아이템 장착 이벤트 구조체
struct ItemEquipEvent
{
    UINT16 packetID; // 패킷 ID

    char playerId[32]; // 플레이어 ID

    UINT16 itemID;     // 장착한 아이템 ID
    UINT16 isEquipped; // 0: 아이템을 들고 있지 않음, 1: 아이템을 들고 있음 (장착 여부)
    UINT16 slotIndex;  // 장착된 아이템이 위치한 인벤토리 슬롯 인덱스 (슬롯 0~3)
    ADD_SERIALIZE_FUNCS(ItemEquipEvent)
};

//고유식별값 주는 패킷
struct WorldObjectSpawnPacket 
{
	UINT16 packetID; // 패킷 ID
	
    UINT16 itemID;   // 생성된 오브젝트 ID
	UINT16 WorldObjectID;
	
    float posX; // 생성된 오브젝트 위치
	float posY;
	float posZ;
	
    float rotX; // 생성된 오브젝트 위치
    float rotY;
    float rotZ;

    float velocityX; // 생성된 오브젝트 위치
    float velocityY;
    float velocityZ;

    ADD_SERIALIZE_FUNCS(WorldObjectSpawnPacket)
};
#pragma pack(pop)

