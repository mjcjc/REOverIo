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
        cout << "WSAStartup ����" << endl;
        return false;
    }
    listenSock = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSock == INVALID_SOCKET) {
        cout << "���� ���� ����" << endl;
        return false;
    }

    SOCKADDR_IN servaddr = {};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVERPORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listenSock, (SOCKADDR*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        cout << "bind ����" << endl;
        return false;
    }

    if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR) {
        cout << "listen ����" << endl;
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
    SOCKET client = socket(AF_INET, SOCK_STREAM, 0); // �� Ŭ���̾�Ʈ ���� ����
    if (client == INVALID_SOCKET) {
        cout << "Ŭ���̾�Ʈ ���� ���� ����" << endl;
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
            cout << "AcceptEx ����!" << endl;
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
        std::cerr << "WSASend ����: " << WSAGetLastError() << std::endl;
        delete[] overlp->wsabuf.buf;
    }
    else {
		cout << "��Ŷ ���� ����" << endl;
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
    cout << "������ �� ���̵�" << response.roomId << endl;

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

    // 1. B���� ���� �������� ������ RoomInResponse�� ����
    for (const auto& user : roomInfo->userinfo)
    {
        if (user.get() == joiningUser.get()) continue; // �ڱ� �ڽ� ����

        PlayerInfoGet info;
        info.packetId = static_cast<UINT16>(PacketStatus::PLAYER_READY_TOGGLE_SUCCESS);
        info.readyStatus = user->ready;
        strcpy_s(info.userName, user->m_userId);

        SendPacket(info.serialize(), client_sock);
        std::cout << "���� ���� ���� (PlayerInfoGet): " << info.userName << std::endl;
    }

    // 2. B �ڽſ��� RoomInResponse�� ���� ���� ����
    response.PacketId = static_cast<UINT16>(PacketStatus::ROOM_IN_SUCCESS);
    response.roomId = room.roomId;
    strcpy_s(response.roomName, roomInfo->roomName);
    strcpy_s(response.userName, room.userName);

    SendPacket(response.serialize(), client_sock); // B���� �ڱ� ����
    std::cout << "���� ���� ����: " << response.userName << std::endl;

    // 3. ���� ����(A)�鿡�� PlayerInfoGet���� B ���� ����
    PlayerInfoGet newUserNotify;
    newUserNotify.packetId = static_cast<UINT16>(PacketStatus::PLAYER_READY_TOGGLE_SUCCESS); // Ȥ�� PLAYER_JOIN_NOTIFY
    newUserNotify.readyStatus = joiningUser->ready;
    strcpy_s(newUserNotify.userName, room.userName);

    std::vector<char> notifyPacket = newUserNotify.serialize();
    for (const auto& user : roomInfo->userinfo)
    {
        if (user.get() == joiningUser.get()) continue; // B ����
        SendPacket(notifyPacket, user->sock);
        std::cout << "�ű� ���� ���� ���� ���: " << user->m_userId << std::endl;
    }

    // 4. �κ� �������� �� ��� ����
    RoomListSend();
}
void RoomUpdateSendPacket(RoomNOtify& room, SOCKET client_sock)
{
    cout << "���°�." << endl;
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
			cout << "���� ����" << endl;
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

    // �� ã�Ƽ� ��ε�ĳ��Ʈ
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
        std::cerr << "[����] �÷��̾ ���� ���� ã�� ����: " << RemoveItem.playerId << std::endl;
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
    InGamePlayer(Rooms, player); // ��ġ ����

    shared_ptr<RoomInfo> foundRoom = nullptr;

    // 1. �� ã��
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

    // 2. ���� ��� ��������
    for (auto& receiver : foundRoom->userinfo)
    {
        // 3. ��� �÷��̾� ������ ��ε�ĳ��Ʈ
        for (auto& playerData : players)
        {
            // �ڱ� �ڽ��̸� �ǳʶ�
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

            // ����ȭ �� ����
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
   // cout << "���� ��Ŷ ������:" << PacketId << endl;
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
        cout << "���� ��Ŷ ������:" << PacketId << endl;
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
        cout << "���� ��Ŷ ������:" << PacketId << endl;
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
        cout << "���� ��Ŷ ������:" << PacketId << endl;
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
        cout << "���� ��Ŷ ������:" << PacketId << endl;
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
        cerr << "[ERROR] �߸��� ��Ŷ ���� - ���� ���� (" << length << "����Ʈ)" << endl;
        return;
    }

    // ù 2����Ʈ�� ��ü ���̷� �����ϰ� ���� (�̹� ��Ŷ �������� ����)
    UINT16 PacketId = 0;
    std::memcpy(&PacketId, data + sizeof(UINT16), sizeof(UINT16)); // offset 2~3

   // cout << "[DEBUG] ���� PacketId: " << PacketId << endl;

    // payload: PacketId ������ ������ ������
    char* payload = data + sizeof(UINT16); // �� �����ͺ��� PacketId + ����ü ��ü
    size_t payloadLength = length - sizeof(UINT16); // ��ü���� ���� �ʵ� 2����Ʈ ��

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
        cerr << "[ERROR] �� �� ���� PacketId: " << PacketId << endl;
    }
}

int main() {
    if (!SockInit(ListenSock) || !LoadAcceptEx()) {
        return -1;
    }

    HANDLE hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (hIOCP == NULL) {
        cout << "IOCP ���� ����" << endl;
        return -1;
    }
    CreateIoCompletionPort((HANDLE)ListenSock, hIOCP, 0, 0);

    // ù ��° AcceptEx ��û ���
    PostAccept(hIOCP);

    DWORD bytesTransferred;
    ULONG_PTR completionKey;
    LPOVERLAPPED lpOverlapped;

    while (true) {
        BOOL result = GetQueuedCompletionStatus(hIOCP, &bytesTransferred, &completionKey, &lpOverlapped, INFINITE);
        if (!result) {
            cout << "[ERROR] GetQueuedCompletionStatus ����! ���� �ڵ�: " << GetLastError() << endl;
            continue;
        }

        stOverlappedEx* overEx = (stOverlappedEx*)lpOverlapped;
        SOCKET clientsoc = overEx->sock;

        if (overEx->state == OverlState::ACCEPT) {
            // AcceptEx �Ϸ� ó��
            shared_ptr<User> user = make_shared<User>();
            user->sock = clientsoc;
            Userkey[clientsoc] = user;

            CreateIoCompletionPort((HANDLE)clientsoc, hIOCP, (ULONG_PTR)clientsoc, 0);

            cout << "Ŭ���̾�Ʈ ���� �Ϸ�" << endl;

            // ���ο� AcceptEx ��û ��� (������ ���������� �޾ƾ� ��)
            PostAccept(hIOCP);

            // ù ��° WSARecv ��û
            stOverlappedEx* recvOverEx = new stOverlappedEx;
            memset(&recvOverEx->overlp, 0, sizeof(recvOverEx->overlp));
            recvOverEx->wsabuf.buf = recvOverEx->buf;
            recvOverEx->wsabuf.len = sizeof(recvOverEx->buf);
            recvOverEx->sock = clientsoc;
            recvOverEx->state = OverlState::RECV;

            cout << "ù WSARecv ��ϵ� (����: " << clientsoc << ")" << endl;

            DWORD flags = 0, recvBytes = 0;
            int retval = WSARecv(clientsoc, &recvOverEx->wsabuf, 1, &recvBytes, &flags, &recvOverEx->overlp, NULL);
            if (retval == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                cout << "WSARecv ����: " << WSAGetLastError() << endl;
                closesocket(clientsoc);
                Userkey.erase(clientsoc);
                delete recvOverEx;
            }
            else {
                cout << "[DEBUG] WSARecv ��� �Ϸ�" << endl;  // ���� �߰�
            }
        }
        else if (overEx->state == OverlState::RECV) {
            //cout << "[DEBUG] RECV ���� ����, ���ŵ� ����Ʈ ��: " << bytesTransferred << endl;

            if (bytesTransferred == 0) {
                cout << "Ŭ���̾�Ʈ ���� ����\n";
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

          /*  cout << "[DEBUG] ���ŵ� ù ����Ʈ: " << (unsigned int)(uint8_t)overEx->buf[0] << endl;
            cout << "[DEBUG] �� ��° ����Ʈ: " << (unsigned int)(uint8_t)overEx->buf[1] << endl;
            cout << "[DEBUG] ��ü ���� ����Ʈ HEX: ";
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
                cout << "�ٽ� Recv�ϴ°� ������!" << endl;
                delete overEx;
            }
        }
    }

    closesocket(ListenSock);
    WSACleanup();
    return 0;
}
