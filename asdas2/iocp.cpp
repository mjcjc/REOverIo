#include "User.h"
#include "Login.h"
#include "deFine.h"
#include"Room.h"
#include"stSendContext.h"
#include"OverlappedExPool.h"

#pragma comment(lib, "ws2_32.lib")

#define SERVERPORT 43535
#define BUFSIZE 256

using namespace std;

unordered_map<string, shared_ptr<User>> LoginUser;
unordered_map<SOCKET, shared_ptr<User>> Userkey;
unordered_map<uint32_t, RoomInfo> Rooms;

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

// AcceptEx �Լ� ������ ��������
bool LoadAcceptEx() {
    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes;
    return (WSAIoctl(ListenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),
        &AcceptExFunc, sizeof(AcceptExFunc), &dwBytes, NULL, NULL) == 0);
}

// ���ο� AcceptEx ��û ���
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
    auto overlp = GetOverlappedPool().Allocate();

    memset(&overlp->overlp, 0, sizeof(overlp->overlp));
    overlp->sock = client_sock;
    overlp->state = OverlState::SEND;

    overlp->wsabuf.buf = new char[data.size()];
    memcpy(overlp->wsabuf.buf, data.data(), data.size());
    overlp->wsabuf.len = static_cast<ULONG>(data.size());

    DWORD sendBytes;
    int retval = WSASend(client_sock, &overlp->wsabuf, 1, &sendBytes, 0, &overlp->overlp, NULL);
    if (retval == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        std::cerr << "WSASend ����: " << WSAGetLastError() << std::endl;
        delete[] overlp->wsabuf.buf;
        // shared_ptr�̱� ������ �ڵ� �ݳ���
    }
    // overlp�� shared_ptr�̹Ƿ� ���� ���� ���� ���ʿ�
}

void sendLogin(const LoginRequest& request, SOCKET client_sock) {
    LoginResponse response;
    strcpy_s(response.username, sizeof(response.username), request.username);
    if (Logrequest(const_cast<LoginRequest&>(request), LoginUser)) {
        strcpy_s(response.message, sizeof(response.message), "Login successful");
        response.PacketId = LOGIN_SUCCESS;
    }
    else {
        strcpy_s(response.message, sizeof(response.message), "Someone is Login");
        response.PacketId = LOGIN_FAIL;
    }
    std::vector<char> serializedData = response.serialize();
    SendPacket(serializedData, client_sock);
}
void RoomSendPacket(const RoomCreateRequest& room, SOCKET client_sock)
{
    int getRoomid = RoomMake(const_cast<RoomCreateRequest&>(room), Rooms, LoginUser);

    RoomCreateResponse response;

    if (getRoomid == Rooms[getRoomid].roomId)
    {
        response.PacketId = ROOM_CREATE_SUCCESS;
    }
    else {
       response.PacketId = ROOM_CREATE_FAIL;
    }
    response.roomId = getRoomid;
    cout << "������ �� ���̵�" << response.roomId << endl;

    std::vector<char> serializedData = response.serialize();
    SendPacket(serializedData, client_sock);
}
void RoomInSideSendPacket(RoomRequest const& room, SOCKET client_sock)
{
    RoomInResponse response;

    if (RoomInSide(const_cast<RoomRequest&>(room), Rooms, LoginUser)) {
        response.PacketId = ROOM_IN_SUCCESS;

        response.roomId = room.roomId;
        strcpy_s(response.roomName, Rooms[room.roomId].roomName);
		strcpy_s(response.userName, room.userName);
    }
    else {
        response.PacketId = ROOM_IN_FAIL;
    }

    std::vector<char> serializedData = response.serialize();
    SendPacket(serializedData, client_sock);
}
void RoomUpdateSendPacket(RoomNOtify& room, SOCKET client_sock)
{
    cout << "���°�." << endl;
    if (strcmp(room.userName, Rooms[room.roomId].hostName) == 0)
    {
        RoomFixedUpdate(room, Rooms);
        room.packetId = ROOM_UPDATE_SUCCESS;
    }
    else {
        room.packetId = ROOM_UPDATE_FAIL;
    }
    std::vector<char> serializedData = room.serialize();
    SendPacket(serializedData, client_sock);

}
void RoomReadySend(PlayerReadySend const& room, SOCKET client_sock)
{
    RoomSomeReady(const_cast<PlayerReadySend&>(room), Rooms);

	for (auto& user : Rooms[room.roomID].userinfo)
	{
        if (user.m_userId == room.userName) 
		{
            continue;
        }
		PlayerInfoGet response;
        response.packetId = PLAYER_READY_TOGGLE_SUCCESS;
        response.readyStatus = 1;
		strcpy(response.userName, room.userName);
        std::vector<char> serializedData = response.serialize();
        SendPacket(serializedData, client_sock);
	}

}
void RoomOutSideSendPacket(RoomRequest & room, SOCKET client_sock)
{
    //listget����ü

	RoomOutSide(room, Rooms);
    
    for (const auto& [roomid, room] : Rooms) {
		RoomListGet response;
		strcpy(response.roomName, room.roomName);
		response.userCount = room.userCount;
        response.packetID = ROOM_LOBY_UPDATE;
		std::vector<char> serializedData = response.serialize();
		SendPacket(serializedData, client_sock);    
    }
}

void RoomListSend(SOCKET client_sock)
{
    for (const auto& [roomid, room] : Rooms) {
        RoomListGet response;
        strcpy(response.roomName, room.roomName);
        response.userCount = room.userCount;
        response.packetID = ROOM_LOBY_UPDATE;
        std::vector<char> serializedData = response.serialize();
        SendPacket(serializedData, client_sock);
    }
}
void ProcessPacket(char const* data, size_t length, SOCKET client_sock)
{
    if (length < sizeof(UINT16)) {
        cerr << "Inval  id packet size" << endl;
        return;
    }


    UINT16 PacketId;
    std::memcpy(&PacketId, data, sizeof(UINT16));
    cout << "��Ŷ ID : " << PacketId << endl;
    switch (PacketId) {
    case LOGIN_REQUEST: {
        if (length < sizeof(LoginRequest)) {
            cerr << "Invalid LoginRequest packet size" << endl;
            return;

        }
        LoginRequest packet = LoginRequest::deserialize(
            std::vector<char>(data, data + sizeof(LoginRequest))
        );

        cout << "���� ��Ŷ ������:" << endl;
        cout << "PacketId: " << packet.PacketId << endl;
        cout << "Username: " << packet.username << endl;

        sendLogin(packet, client_sock);
        cout << "�α��� ��Ŷ ó��" << endl;
        break;
    }
    case ROOM_CREATE_REQUEST:
    {
        if (length < sizeof(RoomCreateRequest))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        RoomCreateRequest packet = RoomCreateRequest::deserialize(
            std::vector<char>(data, data + sizeof(RoomCreateRequest))
        );

        cout << "���� ��Ŷ ������:" << endl;
        cout << "PacketId: " << packet.PacketId << endl;
        cout << "Username: " << packet.userName << endl;
        RoomSendPacket(packet, client_sock);
    }
    break;
    case ROOM_ENTER_REQUEST:
    {
        if (length < sizeof(RoomRequest))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        RoomRequest packet = RoomRequest::deserialize(
            std::vector<char>(data, data + sizeof(RoomRequest))
        );
        cout << "���� ��Ŷ ������:" << endl;
        cout << "PacketId: " << packet.PacketId << endl;
        cout << "Username: " << packet.userName << endl;
        RoomInSideSendPacket(packet, client_sock);
    }
    break;
    case ROOM_LEAVE_REQUEST:
    {
        if (length < sizeof(RoomRequest))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        RoomRequest packet = RoomRequest::deserialize(
            std::vector<char>(data, data + sizeof(RoomRequest))
        );
        cout << "���� ��Ŷ ������:" << endl;
        cout << "PacketId: " << packet.PacketId << endl;
        cout << "Username: " << packet.userName << endl;
        RoomOutSideSendPacket(packet, client_sock);
    }
    break;
    case ROOM_UPDATE_NOTIFY:
    {
        if (length < sizeof(RoomNOtify))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        RoomNOtify packet = RoomNOtify::deserialize(
            std::vector<char>(data, data + sizeof(RoomNOtify))
        );
        cout << "���� ��Ŷ ������:" << endl;
        //cout << "PacketId: " << packet.PacketId << endl;
        RoomUpdateSendPacket(packet, client_sock);
    }
    break;
    case PLAYER_READY_TOGGLE_REQUEST:
    {
        if (length < sizeof(PlayerReadySend))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
        PlayerReadySend packet = PlayerReadySend::deserialize(
            std::vector<char>(data, data + sizeof(PlayerReadySend))
        );
        cout << "���� ��Ŷ ������:" << endl;
        RoomReadySend(packet, client_sock);
    }
    break;
    case USER_STATUS_LOBY:
    {
        if (length < sizeof(PlayerStatus))
        {
            cerr << "Invalid RoomInfo packet size" << endl;
        }
		PlayerStatus packet = PlayerStatus::deserialize(
			std::vector<char>(data, data + sizeof(PlayerStatus))
		);
		RoomListSend(client_sock);
    }
    break;
    default:
        cerr << "Unknown packet ID: " << PacketId << endl;
        break;
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

            DWORD flags = 0, recvBytes = 0;
            int retval = WSARecv(clientsoc, &recvOverEx->wsabuf, 1, &recvBytes, &flags, &recvOverEx->overlp, NULL);
            if (retval == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                cout << "WSARecv ����: " << WSAGetLastError() << endl;
                closesocket(clientsoc);
                Userkey.erase(clientsoc);
                delete recvOverEx;
            }
        }
        else if (overEx->state == OverlState::RECV) {
            if (bytesTransferred == 0) {
                cout << "Ŭ���̾�Ʈ ���� ����\n";
                closesocket(clientsoc);
                Userkey.erase(clientsoc);
                delete overEx;
                continue;
            }
        else if (overEx->state == OverlState::SEND) {
            delete[] overEx->wsabuf.buf;
            // shared_ptr �ڵ� �Ҹ�� Ǯ�� �ݳ���
        }

            // ��Ŷ ó��
            ProcessPacket(overEx->buf, bytesTransferred, clientsoc);

            // �ٽ� WSARecv ��� (�񵿱� ó�� ����)
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
