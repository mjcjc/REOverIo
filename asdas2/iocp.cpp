#include "User.h"
#include "Login.h"
#include "deFine.h"
#include"Room.h"
#include"stSendContext.h"
#include"OverlappedExPool.h"
#include"GamePlayingPacket.h"
#include"Item.h"
#pragma comment(lib, "ws2_32.lib")

#define SERVERPORT 43535
#define BUFSIZE 256

using namespace std;

unordered_map<string, shared_ptr<User>> LoginUser;
unordered_map<SOCKET, shared_ptr<User>> Userkey;
unordered_map<uint32_t, shared_ptr<RoomInfo>> Rooms;

SOCKET ListenSock;
LPFN_ACCEPTEX AcceptExFunc;

bool SockInit(SOCKET& listenSock) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cout << "WSAStartup 실패" << endl;
        return false;
    }
    listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == INVALID_SOCKET) {
        cout << "소켓 생성 실패" << endl;
        return false;
    }

    SOCKADDR_IN servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVERPORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenSock, (SOCKADDR*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        cout << "bind 실패" << endl;
        return false;
    }

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        cout << "listen 실패" << endl;
        return false;
    }

    return true;
}
bool LoadAcceptEx() {
    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes;
    return (WSAIoctl(ListenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),
        &AcceptExFunc, sizeof(AcceptExFunc), &dwBytes, NULL, NULL) == 0);
}
void PostAccept(HANDLE hIOCP) {
    SOCKET client = socket(AF_INET, SOCK_STREAM, 0); // 새 클라이언트 소켓 생성
    if (client == INVALID_SOCKET) {
        cout << "클라이언트 소켓 생성 실패" << endl;
        return;
    }

    stOverlappedEx* acceptOverEx = new stOverlappedEx;
    memset(&acceptOverEx->overlp, 0, sizeof(acceptOverEx->overlp));
    acceptOverEx->sock = client;
    acceptOverEx->state = OverlState::ACCEPT;

    DWORD bytesReceived = 0;
    if (!AcceptExFunc(ListenSock, client, acceptOverEx->buf, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
        &bytesReceived, &acceptOverEx->overlp)) {
        if (WSAGetLastError() != ERROR_IO_PENDING) {
            cout << "AcceptEx 실패!" << endl;
            closesocket(client);
            delete acceptOverEx;
        }
    }
}
void SendPacket(const std::vector<char>& data, SOCKET client_sock) {
    uint16_t totalLength = static_cast<uint16_t>(data.size() + sizeof(uint16_t));

    std::vector<char> packetWithLength;
    packetWithLength.resize(sizeof(uint16_t) + data.size());

    memcpy(packetWithLength.data(), &totalLength, sizeof(uint16_t));
    memcpy(packetWithLength.data() + sizeof(uint16_t), data.data(), data.size());

    auto overlp = GetOverlappedPool().Allocate();
    memset(&overlp->overlp, 0, sizeof(overlp->overlp));
    overlp->sock = client_sock;
    overlp->state = OverlState::SEND;

    overlp->wsabuf.buf = new char[packetWithLength.size()];
    memcpy(overlp->wsabuf.buf, packetWithLength.data(), packetWithLength.size());
    overlp->wsabuf.len = static_cast<ULONG>(packetWithLength.size());

    DWORD sendBytes;
    int retval = WSASend(client_sock, &overlp->wsabuf, 1, &sendBytes, 0, &overlp->overlp, NULL);
    if (retval == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cerr << "WSASend 실패: " << WSAGetLastError() << std::endl;
        delete[] overlp->wsabuf.buf;
    }
    else {
		cout << "패킷 전송 성공" << endl;
    }
}
void sendLogin(const LoginRequest& request, SOCKET client_sock) {
    LoginResponse response;
    strcpy_s(response.username, sizeof(response.username), request.username);
    if (Logrequest(const_cast<LoginRequest&>(request), LoginUser, client_sock)) {
        strcpy_s(response.message, sizeof(response.message), "Login successful");
        response.PacketId = static_cast<UINT16> (PacketStatus::LOGIN_SUCCESS);
    }
    else {
        strcpy_s(response.message, sizeof(response.message), "Someone is Login");
        response.PacketId = static_cast<UINT16>(PacketStatus::LOGIN_FAIL);
    }
    std::vector<char> serializedData = response.serialize();
    SendPacket(serializedData, client_sock);
}
void RoomMakeSendPacket(const RoomCreateRequest& room, SOCKET client_sock)
{

    int getRoomid = RoomMake(const_cast<RoomCreateRequest&>(room), Rooms, LoginUser);
    RoomCreateResponse response;

    if (getRoomid == Rooms[getRoomid]->roomId)
    {
        response.PacketId = static_cast<UINT16>(PacketStatus::ROOM_CREATE_SUCCESS);
    }
    else {
        response.PacketId = static_cast<UINT16>(PacketStatus::ROOM_CREATE_FAIL);
    }
    response.roomId = getRoomid;
    cout << "보낼때 룸 아이디" << response.roomId << endl;

    std::vector<char> serializedData = response.serialize();
    SendPacket(serializedData, client_sock);
    RoomListSend();
}
void RoomInSideSendPacket(RoomRequest const& room, SOCKET client_sock)
{
    RoomInResponse response;
    bool joinSuccess = RoomInSide(const_cast<RoomRequest&>(room), Rooms, LoginUser);
    if (!joinSuccess) return;

    auto roomInfo = Rooms[room.roomId];
    auto joiningUser = LoginUser[room.userName];

    // 1. B에게 기존 유저들의 정보를 RoomInResponse로 전송
    for (const auto& user : roomInfo->userinfo)
    {
        if (user.get() == joiningUser.get()) continue; // 자기 자신 제외

        PlayerInfoGet info;
        info.packetId = static_cast<UINT16>(PacketStatus::PLAYER_READY_TOGGLE_SUCCESS);
        info.readyStatus = user->ready;
        strcpy_s(info.userName, user->m_userId);

        SendPacket(info.serialize(), client_sock);
        std::cout << "기존 유저 전송 (PlayerInfoGet): " << info.userName << std::endl;
    }

    // 2. B 자신에게 RoomInResponse로 본인 정보 전송
    response.PacketId = static_cast<UINT16>(PacketStatus::ROOM_IN_SUCCESS);
    response.roomId = room.roomId;
    strcpy_s(response.roomName, roomInfo->roomName);
    strcpy_s(response.userName, room.userName);

    SendPacket(response.serialize(), client_sock); // B에게 자기 정보
    std::cout << "본인 정보 전송: " << response.userName << std::endl;

    // 3. 기존 유저(A)들에게 PlayerInfoGet으로 B 정보 전송
    PlayerInfoGet newUserNotify;
    newUserNotify.packetId = static_cast<UINT16>(PacketStatus::PLAYER_READY_TOGGLE_SUCCESS); // 혹은 PLAYER_JOIN_NOTIFY
    newUserNotify.readyStatus = joiningUser->ready;
    strcpy_s(newUserNotify.userName, room.userName);

    std::vector<char> notifyPacket = newUserNotify.serialize();
    for (const auto& user : roomInfo->userinfo)
    {
        if (user.get() == joiningUser.get()) continue; // B 제외
        SendPacket(notifyPacket, user->sock);
        std::cout << "신규 유저 정보 전송 대상: " << user->m_userId << std::endl;
    }

    // 4. 로비 유저에게 방 목록 갱신
    RoomListSend();
}
void RoomUpdateSendPacket(RoomNOtify& room, SOCKET client_sock)
{
    cout << "들어는감." << endl;
    if (strcmp(room.userName, Rooms[room.roomId]->hostName) == 0)
    {
        RoomFixedUpdate(room, Rooms);
        room.packetId = static_cast<UINT16>(PacketStatus::ROOM_UPDATE_SUCCESS);
        
    }
    else {
        room.packetId = static_cast<UINT16>(PacketStatus::ROOM_UPDATE_FAIL);
    }
    std::vector<char> serializedData = room.serialize();
    
    SendPacket(serializedData, client_sock);
    RoomListSend();

}
void RoomReadySend(PlayerReadySend const& room, SOCKET client_sock)
{
    RoomSomeReady(const_cast<PlayerReadySend&>(room), Rooms);
    PlayerInfoGet response;
    response.readyStatus = room.readyStatus;
    response.packetId = static_cast<UINT16>(PacketStatus::PLAYER_READY_TOGGLE_SUCCESS);
    strcpy(response.userName, room.userName);
    std::vector<char> serializedData = response.serialize();
    SendPacket(serializedData, client_sock);
    for (auto& user : Rooms[room.roomID]->userinfo)
    {
        if ((user)->m_userId == room.userName)
        {
            continue;
        }

        SendPacket(serializedData, (user)->sock);
    }

}
void RoomListSend()
{
    for (auto& [sock, user] : Userkey) {
        if (user->userState == User::USER_STATE_LOBBY) {
            for (const auto& [roomId, room] : Rooms) {
                RoomListGet response;
                response.packetID = static_cast<UINT16>(PacketStatus::ROOM_LOBY_UPDATE);
                strcpy_s(response.roomName, room->roomName);
                strcpy_s(response.hostName, room->hostName);
                response.userCount = room->userCount;
                response.maxuserCount = room->roomSet.maxUserCount;
                response.roomMode = room->RoomMode;
                std::vector<char> serializedData = response.serialize();
                SendPacket(serializedData, user->sock);
            }
        }
    }
}
void RoomOutSideSendPacket(RoomRequest& room, SOCKET client_sock)
{

    RoomOutSide(room, Rooms);
    room.PacketId = static_cast<UINT16>(PacketStatus::ROOM_LEAVE_SUCCESS);
    std::vector<char> serializedData = room.serialize();
    RoomListSend();
    SendPacket(serializedData, client_sock);

}
void InGameStart(RoomStart& play, SOCKET client_sock)
{
   
    if (GameStart(Rooms, play))
    {
        play.packetID = static_cast<UINT16>(PacketStatus::ROOM_START_SUCCESS);
        std::vector<char> serializedData = play.serialize();
		for (auto& user : Rooms[play.roomID]->userinfo)
		{
			SendPacket(serializedData, user->sock);
			cout << "게임 시작" << endl;
		}
    }
    else {
        play.packetID = static_cast<UINT16>(PacketStatus::ROOM_START_FAIL);
		std::vector<char> serializedData = play.serialize();
		SendPacket(serializedData, client_sock);
    }
}
void InitGameObject(WorldObjectSpawnPacket& ClObj)
{
    ItemSpawnManager(ClObj);
}
void InventoryAddPacket(ItemPickupEvent& PickItemReq, SOCKET client_sock)
{
    WorldObjectSpawnPacket spawnObj;
    spawnObj.itemID = PickItemReq.itemID;
    strncpy(spawnObj.playerId, PickItemReq.playerId, sizeof(spawnObj.playerId));

    InitGameObject(spawnObj); 

    ItemPickupEvent pickupAdd;
    pickupAdd.packetID = static_cast<UINT16>(PlayerPacketStatus::ITEM_PICKUP_FAILED);
    strcpy(pickupAdd.playerId, PickItemReq.playerId);
    pickupAdd.itemID = spawnObj.itemID;
    pickupAdd.WorldObjectID = spawnObj.worldObjectID;

    if (InventoryItemAdd(pickupAdd)) {
        pickupAdd.packetID = static_cast<UINT16>(PlayerPacketStatus::ITEM_PICKUP_SUCCESS);
    }

    SendPacket(pickupAdd.serialize(), client_sock);
}
void InventoryRemovePacket(ItemDropEvent& RemoveItem, SOCKET client_sock)
{
    ItemDropEvent response;


    UINT16 backupItemID = 0;
    for (auto& [roomId, players] : GameStartUsers)
    {
        for (GamePlayer& player : players)
        {
            if (strcmp(player.user->m_userId, RemoveItem.playerId) == 0)
            {
                if (RemoveItem.slotIndex < player.inven.slots.size())
                {
                    backupItemID = player.inven.slots[RemoveItem.slotIndex].itemID;
                }
                break;
            }
        }
    }

    if (InventoryItemRemove(RemoveItem))
    {
        response.packetID = static_cast<UINT16>(PlayerPacketStatus::ITEM_DROP_SUCCESS);
    }
    else {
        response.packetID = static_cast<UINT16>(PlayerPacketStatus::ITEM_DROP_FAILED);
    }

    WorldObjectSpawnPacket spawnPacket;
    spawnPacket.packetID = static_cast<UINT16>(PlayerPacketStatus::ITEM_DROP_SUCCESS);
    spawnPacket.itemID = backupItemID;

    for (auto& [roomId, players] : GameStartUsers)
    {
        for (GamePlayer& player : players)
        {
            if (strcmp(player.user->m_userId, RemoveItem.playerId) == 0)
            {
                if (RemoveItem.slotIndex < player.inven.slots.size())
                {
                    strncpy(spawnPacket.playerId, player.user->m_userId, sizeof(spawnPacket.playerId) - 1);
                    spawnPacket.playerId[sizeof(spawnPacket.playerId) - 1] = '\0';

                    spawnPacket.worldObjectID = ItemSpawnManager(spawnPacket);
                    response.slotIndex = RemoveItem.slotIndex;

                    spawnPacket.posX = RemoveItem.posX;
                    spawnPacket.posY = RemoveItem.posY;
                    spawnPacket.posZ = RemoveItem.posZ;
                    spawnPacket.rotX = RemoveItem.rotX;
                    spawnPacket.rotY = RemoveItem.rotY;
                    spawnPacket.rotZ = RemoveItem.rotZ;
                    spawnPacket.velocityX = RemoveItem.velocityX;
                    spawnPacket.velocityY = RemoveItem.velocityY;
                    spawnPacket.velocityZ = RemoveItem.velocityZ;
                }
                break;
            }
        }
    }

    // 방 찾아서 브로드캐스트
    shared_ptr<RoomInfo> foundRoom = nullptr;
    for (auto& [roomId, roomInfo] : Rooms)
    {
        for (auto& user : roomInfo->userinfo)
        {
            if (strcmp(user->m_userId, RemoveItem.playerId) == 0)
            {
                foundRoom = roomInfo;
                break;
            }
        }
        if (foundRoom) break;
    }

    if (!foundRoom)
    {
        std::cerr << "[에러] 플레이어가 속한 방을 찾지 못함: " << RemoveItem.playerId << std::endl;
        return;
    }

    for (auto& receiver : foundRoom->userinfo)
    {
        std::vector<char> serializedData = spawnPacket.serialize();
        SendPacket(serializedData, receiver->sock);
    }
}
void InventoryUsePacket(ItemUseEvent& UseItem, SOCKET client_sock)
{
	ItemUseEvent response;
    if (InventoryItemUse(UseItem))
    {
		response.packetID = static_cast<UINT16>(PlayerPacketStatus::ITEM_USE_SUCCESS);
    }
    else {
		response.packetID = static_cast<UINT16>(PlayerPacketStatus::ITEM_USE_FAILED);
    }
	strcpy(response.playerId, UseItem.playerId);
	response.itemID = UseItem.itemID;
	response.slotIndex = UseItem.slotIndex;
	std::vector<char> serializedData = response.serialize();
	SendPacket(serializedData, client_sock);
}
void InventoryEquipPacket(ItemEquipEvent& eqItem, SOCKET client_sock)
{
    ItemEquipEvent response;
    if (InventoryItemEquip(eqItem))
    {
        response.packetID = static_cast<UINT16>(PlayerPacketStatus::ITEM_EQUIP_SUCCESS);
    }
    else {
        response.packetID = static_cast<UINT16>(PlayerPacketStatus::ITEM_EQUIP_FAILED);
    }

    for (auto& [roomId, players] : GameStartUsers)
    {
        for (GamePlayer& player : players)
        {
            if (strcmp(player.user->m_userId, eqItem.playerId) == 0)
            {
            
                if (eqItem.slotIndex < player.inven.slots.size())
                {
                    response.slotIndex = eqItem.slotIndex;
                    response.itemID = player.inven.slots[eqItem.slotIndex].itemID;
                    response.isEquipped = player.playerEquiptHand;
                }
                break;
            }
        }
    }
    for (auto& [roomId, players] : GameStartUsers)
    {
        for (GamePlayer& player : players)
        {
            strcpy(response.playerId, eqItem.playerId);
            std::vector<char> serializedData = response.serialize();
            SendPacket(serializedData, player.sock);
        }
    }
}
void missionState(plantMission& planePacket, SOCKET client_sock)
{

    PlantGauge(planePacket);

}
void MoveBroadCast(PlayerStatus& player)
{
    InGamePlayer(Rooms, player); // 위치 갱신

    shared_ptr<RoomInfo> foundRoom = nullptr;

    // 1. 방 찾기
    for (auto& [roomId, roomInfo] : Rooms)
    {
        for (auto& user : roomInfo->userinfo)
        {
            if (strcmp(user->m_userId, player.playerId) == 0)
            {
                foundRoom = roomInfo;
                break;
            }
        }
        if (foundRoom) break;
    }

    if (!foundRoom) return;
    uint32_t roomId = foundRoom->roomId;    

    auto it = GameStartUsers.find(roomId);
    if (it == GameStartUsers.end()) return;

    vector<GamePlayer>& players = it->second;

    // 2. 방의 모든 유저에게
    for (auto& receiver : foundRoom->userinfo)
    {
        // 3. 모든 플레이어 정보를 브로드캐스트
        for (auto& playerData : players)
        {
            // 자기 자신이면 건너뜀
            if (strcmp(playerData.user->m_userId, receiver->m_userId) == 0)
                continue;

            PlayerStatus sendPacket{};
            sendPacket.packetID = static_cast<UINT16>(PlayerPacketStatus::PLAYER_STATUS_NOTIFY);
            strncpy(sendPacket.playerId, playerData.user->m_userId, sizeof(sendPacket.playerId));
            sendPacket.playerId[sizeof(sendPacket.playerId) - 1] = '\0';

            sendPacket.positionX = playerData.x;
            sendPacket.positionY = playerData.y;
            sendPacket.positionZ = playerData.z;

            sendPacket.rotationX = playerData.rotationX;
            sendPacket.rotationY = playerData.rotationY;
            sendPacket.rotationZ = playerData.rotationZ;

            // 직렬화 후 전송
            std::vector<char> serializedData = sendPacket.serialize();
            SendPacket(serializedData, receiver->sock);
        }
    }
}
bool IsLobbyPacket(UINT16 packetId)
{
    return packetId >= static_cast<UINT16>(PacketStatus::LOGIN_REQUEST) &&
        packetId <= static_cast<UINT16>(PacketStatus::USER_STATUS_LOGIN);
}
bool IsGamePacket(UINT16 packetId)
{
    return packetId >= static_cast<UINT16>(PlayerPacketStatus::PLAYER_STATUS_NOTIFY) &&
        packetId <= static_cast<UINT16>(PlayerPacketStatus::ITEM_EQUIP_FAILED);
}
void ProcessLobbyPacket(UINT16 PacketId, size_t length, SOCKET client_sock, char const* data)
{
    auto lobbyplayerstatus = static_cast<PacketStatus>(PacketId);
   // cout << "받은 패킷 데이터:" << PacketId << endl;
    switch (lobbyplayerstatus) {
    case PacketStatus::LOGIN_REQUEST: {
        if (length < sizeof(LoginRequest)) {
            cerr << "Invalid LoginRequest packet size" << endl;
            return;

        }
        LoginRequest packet = LoginRequest::deserialize(
            std::vector<char>(data, data + sizeof(LoginRequest))
        );
        sendLogin(packet, client_sock);
        break;
    }
    case PacketStatus::ROOM_CREATE_REQUEST:
    {
        if (length < sizeof(RoomCreateRequest))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        RoomCreateRequest packet = RoomCreateRequest::deserialize(
            std::vector<char>(data, data + sizeof(RoomCreateRequest))
        );

        
        RoomMakeSendPacket(packet, client_sock);
    }
    break;
    case PacketStatus::ROOM_ENTER_REQUEST:
    {
        if (length < sizeof(RoomRequest))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        RoomRequest packet = RoomRequest::deserialize(
            std::vector<char>(data, data + sizeof(RoomRequest))
        );
        RoomInSideSendPacket(packet, client_sock);
    }
    break;
    case PacketStatus::ROOM_LEAVE_REQUEST:
    {
        if (length < sizeof(RoomRequest))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        RoomRequest packet = RoomRequest::deserialize(
            std::vector<char>(data, data + sizeof(RoomRequest))
        );
        RoomOutSideSendPacket(packet, client_sock);
    }
    break;
    case PacketStatus::ROOM_UPDATE_NOTIFY:
    {
        if (length < sizeof(RoomNOtify))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        RoomNOtify packet = RoomNOtify::deserialize(
            std::vector<char>(data, data + sizeof(RoomNOtify))
        );
        RoomUpdateSendPacket(packet, client_sock);
    }
    break;
    case PacketStatus::PLAYER_READY_TOGGLE_REQUEST:
    {
        if (length < sizeof(PlayerReadySend))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        PlayerReadySend packet = PlayerReadySend::deserialize(
            std::vector<char>(data, data + sizeof(PlayerReadySend))
        );
        RoomReadySend(packet, client_sock);
    }
    break;
    case PacketStatus::ROOM_START_REQUEST:
    {
		if (length < sizeof(RoomStart))
		{
			cerr << "Invalid RoomInfo packet size" << endl;
		}
        RoomStart packet = RoomStart::deserialize(
            std::vector<char>(data, data + sizeof(RoomStart))
        );
        
        InGameStart(packet, client_sock);
		
    }
        break;
    case PacketStatus::USER_STATUS_LOBY:
    {
        if (length < sizeof(PlayerinfoStatus))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        PlayerinfoStatus packet = PlayerinfoStatus::deserialize(
            std::vector<char>(data, data + sizeof(PlayerinfoStatus))
        );
        LobbyUser(client_sock, Userkey);
        RoomListSend();
    }
    break;

    case PacketStatus::USER_STATUS_LOGIN:
    {
        if (length < sizeof(PlayerinfoStatus))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        PlayerinfoStatus packet = PlayerinfoStatus::deserialize(
            std::vector<char>(data, data + sizeof(PlayerinfoStatus))
        );
        LoginScene(client_sock, Userkey);
        break;
    }

    default:
        cerr << "Unknown packet ID: " << PacketId << endl;
        break;
    }
}
void ProcessInGamePacket(UINT16 PacketId, size_t length, SOCKET client_sock, char const* data)
{
    auto ingamestatus = static_cast<PlayerPacketStatus>(PacketId);

    switch (ingamestatus)
    {
    case PlayerPacketStatus::PLAYER_STATUS_NOTIFY:
    {
        if (length < sizeof(PlayerStatus))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        PlayerStatus packet = PlayerStatus::deserialize(
            std::vector<char>(data, data + sizeof(PlayerStatus))
        );
        MoveBroadCast(packet);
        break;
    }
    case PlayerPacketStatus::ITEM_PICKUP_REQUEST:
    {
        cout << "받은 패킷 데이터:" << PacketId << endl;
		if (length < sizeof(ItemPickupEvent))
		{
			cerr << "Invalid RoomInfo packet size" << endl;
		}
        ItemPickupEvent packet = ItemPickupEvent::deserialize(
			std::vector<char>(data, data + sizeof(ItemPickupEvent))
		);
        InventoryAddPacket(packet, client_sock);
		break;

    }
    case PlayerPacketStatus::ITEM_USE_REQUEST:
    {
        cout << "받은 패킷 데이터:" << PacketId << endl;
		if (length < sizeof(ItemUseEvent))
		{
			cerr << "Invalid RoomInfo packet size" << endl;
		}
        ItemUseEvent packet = ItemUseEvent::deserialize(
			std::vector<char>(data, data + sizeof(ItemUseEvent))
		);
        InventoryUsePacket(packet, client_sock);
		break;

    }
    case PlayerPacketStatus::ITEM_DROP_REQUEST:
    {
        cout << "받은 패킷 데이터:" << PacketId << endl;
		if (length < sizeof(ItemDropEvent))
		{
			cerr << "Invalid RoomInfo packet size" << endl;
		}
        ItemDropEvent packet = ItemDropEvent::deserialize(
			std::vector<char>(data, data + sizeof(ItemDropEvent))
		);
        InventoryRemovePacket(packet, client_sock);
		break;

    }
    case PlayerPacketStatus::ITEM_EQUIP_REQUEST:
    {
        cout << "받은 패킷 데이터:" << PacketId << endl;
		if (length < sizeof(ItemEquipEvent))
		{
			cerr << "Invalid RoomInfo packet size" << endl;
		}
        ItemEquipEvent packet = ItemEquipEvent::deserialize(
			std::vector<char>(data, data + sizeof(ItemEquipEvent))
		);
        InventoryEquipPacket(packet, client_sock);
		break;
    }
    case PlayerPacketStatus::MISSION_NOTIFY:
        if (length < sizeof(plantMission))
        {
            cerr << "Invalid missionSeed packet size" << endl;
        }
        plantMission packet = plantMission::deserialize(
            vector<char>(data, data + sizeof(plantMission))
        );
        missionState(packet, client_sock);
    }
}
void ProcessPacket(char* data, size_t length, SOCKET client_sock)
{
    if (length < sizeof(UINT16) * 2) {
        cerr << "[ERROR] 잘못된 패킷 수신 - 길이 부족 (" << length << "바이트)" << endl;
        return;
    }

    // 첫 2바이트는 전체 길이로 가정하고 무시 (이미 패킷 조립에서 사용됨)
    UINT16 PacketId = 0;
    std::memcpy(&PacketId, data + sizeof(UINT16), sizeof(UINT16)); // offset 2~3

   // cout << "[DEBUG] 받은 PacketId: " << PacketId << endl;

    // payload: PacketId 이후의 나머지 데이터
    char* payload = data + sizeof(UINT16); // 이 포인터부터 PacketId + 구조체 전체
    size_t payloadLength = length - sizeof(UINT16); // 전체에서 길이 필드 2바이트 뺌

    if (IsLobbyPacket(PacketId))
    {
        ProcessLobbyPacket(PacketId, payloadLength, client_sock, payload);
    }
    else if (IsGamePacket(PacketId))
    {
        ProcessInGamePacket(PacketId, payloadLength, client_sock, payload);
    }
    else
    {
        cerr << "[ERROR] 알 수 없는 PacketId: " << PacketId << endl;
    }
}

int main() {
    if (!SockInit(ListenSock) || !LoadAcceptEx()) {
        return -1;
    }

    HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (hIOCP == NULL) {
        cout << "IOCP 생성 실패" << endl;
        return -1;
    }
    CreateIoCompletionPort((HANDLE)ListenSock, hIOCP, 0, 0);

    // 첫 번째 AcceptEx 요청 등록
    PostAccept(hIOCP);

    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    LPOVERLAPPED lpOverlapped;

    while (true) {
        BOOL result = GetQueuedCompletionStatus(hIOCP, &bytesTransferred, &completionKey, &lpOverlapped, INFINITE);
        if (!result) {
            cout << "[ERROR] GetQueuedCompletionStatus 실패! 에러 코드: " << GetLastError() << endl;
            continue;
        }

        stOverlappedEx* overEx = (stOverlappedEx*)lpOverlapped;
        SOCKET clientsoc = overEx->sock;

        if (overEx->state == OverlState::ACCEPT) {
            // AcceptEx 완료 처리
            shared_ptr<User> user = make_shared<User>();
            user->sock = clientsoc;
            Userkey[clientsoc] = user;

            CreateIoCompletionPort((HANDLE)clientsoc, hIOCP, (ULONG_PTR)clientsoc, 0);

            cout << "클라이언트 연결 완료" << endl;

            // 새로운 AcceptEx 요청 등록 (연결을 지속적으로 받아야 함)
            PostAccept(hIOCP);

            // 첫 번째 WSARecv 요청
            stOverlappedEx* recvOverEx = new stOverlappedEx;
            memset(&recvOverEx->overlp, 0, sizeof(recvOverEx->overlp));
            recvOverEx->wsabuf.buf = recvOverEx->buf;
            recvOverEx->wsabuf.len = sizeof(recvOverEx->buf);
            recvOverEx->sock = clientsoc;
            recvOverEx->state = OverlState::RECV;

            cout << "첫 WSARecv 등록됨 (소켓: " << clientsoc << ")" << endl;

            DWORD flags = 0, recvBytes = 0;
            int retval = WSARecv(clientsoc, &recvOverEx->wsabuf, 1, &recvBytes, &flags, &recvOverEx->overlp, NULL);
            if (retval == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                cout << "WSARecv 실패: " << WSAGetLastError() << endl;
                closesocket(clientsoc);
                Userkey.erase(clientsoc);
                delete recvOverEx;
            }
            else {
                cout << "[DEBUG] WSARecv 등록 완료" << endl;  // 여기 추가
            }
        }
        else if (overEx->state == OverlState::RECV) {
            //cout << "[DEBUG] RECV 상태 진입, 수신된 바이트 수: " << bytesTransferred << endl;

            if (bytesTransferred == 0) {
                cout << "클라이언트 연결 종료\n";
                closesocket(clientsoc);
                Userkey.erase(clientsoc);
                delete overEx;
                continue;
            }

            auto userIt = Userkey.find(clientsoc);
            if (userIt == Userkey.end()) {
                delete overEx;
                continue;
            }
            auto& user = userIt->second;

          /*  cout << "[DEBUG] 수신된 첫 바이트: " << (unsigned int)(uint8_t)overEx->buf[0] << endl;
            cout << "[DEBUG] 두 번째 바이트: " << (unsigned int)(uint8_t)overEx->buf[1] << endl;
            cout << "[DEBUG] 전체 수신 바이트 HEX: ";
            for (int i = 0; i < bytesTransferred; ++i) {
                printf("%02X ", (unsigned char)overEx->buf[i]);
            }
            printf("\n");*/

            user->recvBuffer.insert(
                user->recvBuffer.end(),
                overEx->buf,
                overEx->buf + bytesTransferred
            );

            while (true) {
                if (user->recvBuffer.size() < sizeof(uint16_t))
                    break;

                uint16_t packetSize;
                memcpy(&packetSize, user->recvBuffer.data(), sizeof(uint16_t));

                if (user->recvBuffer.size() < packetSize)
                    break;

                std::vector<char> fullPacket(user->recvBuffer.begin(), user->recvBuffer.begin() + packetSize);
                ProcessPacket(fullPacket.data(), packetSize, clientsoc);

                user->recvBuffer.erase(user->recvBuffer.begin(), user->recvBuffer.begin() + packetSize);
            }

            memset(&overEx->overlp, 0, sizeof(overEx->overlp));
            DWORD flags = 0, recvBytes = 0;
            int ret = WSARecv(clientsoc, &overEx->wsabuf, 1, &recvBytes, &flags, &overEx->overlp, NULL);
            if (ret == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                cout << "다시 Recv하는거 실패함!" << endl;
                delete overEx;
            }
        }
    }

    closesocket(ListenSock);
    WSACleanup();
    return 0;
}
