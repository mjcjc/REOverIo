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

    ROOM_LIST_REQUEST,     // 방 목록 요청
    ROOM_LIST_RESPONSE,    // 방 목록 응답

    ROOM_CREATE_REQUEST,   // 방 생성 요청
    ROOM_CREATE_RESPONSE,  // 방 생성 응답
    ROOM_CREATE_SUCCESS,
    ROOM_CREATE_FAIL,

    ROOM_ENTER_REQUEST,    // 방 입장 요청
    ROOM_ENTER_RESPONSE,   // 방 입장 응답
    ROOM_IN_FAIL,
    ROOM_IN_SUCCESS,

    ROOM_READY_RESPONSE,
    ROOM_ALL_READY,

    ROOM_LEAVE_REQUEST,    // 방 나가기 요청
    ROOM_LEAVE_RESPONSE,   // 방 나가기 응답

    ROOM_UPDATE_NOTIFY,    // 방 정보 변경 (인원, 설정 등 업데이트)

    ROOM_CLOSE_REQUEST,    // 방 닫기 요청
    ROOM_CLOSE_RESPONSE,   // 방 닫기 응답
    ROOM_CLOSED_NOTIFY,    // 방 종료 알림
};
enum ItemID : UINT16 // 메인미션 및 아이템을 구분 짓는 ID ==> 플레이어 인벤토리에 들어갈 수 있는 아이템 종류들
{
    //식물 테마 메인 미션 아이템
    MAIN_PLANT_SEED,    //씨앗
    MAIN_PLANT, //다 자란 식물 가공전 단계
    MAIN_PLANT_COAL, // 석탄
    MAIN_PLANT_EXTINGUISHER,// 소화기

    //가루형 테마 메인 미션 아이템
    MAIN_POWDER_RAW,    //원재료
    MAIN_POWDER_REFINED,    // 정제된 
    MAIN_POWDER_PROCESSED,  //가공된

    // 공용으로 사용하는 미션 아이템
    MAIN_BOX,   // 가공 완료된 목표 아이템
    MAIN_TOOLKIT, // 공구 박스
    MAIN_LANTEN, //랜턴

    //자판기에서 판매하는 아이템
    ITEM_STIMULANT, //각성제
    ITEM_GUN,   //총
    ITEM_ACCELERATOR,   //촉진제
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


typedef struct RoomInResponse {
    UINT16 PacketId;
    uint32_t roomId;
    char userName[32];
    char roomName[32];
    //roomresponse정보 뭐 넘겨줄지 생각
    ADD_SERIALIZE_FUNCS(RoomInResponse)
} RoomInResponse;

typedef struct RoomReadyResponse {
    UINT16 PacketId;
    uint32_t userId;
    uint8_t userReadyStatus;
};
#pragma pack(pop)


