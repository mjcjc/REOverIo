#include "User.h"
#include "Login.h"
#include "deFine.h"
#include"Room.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

#define SERVERPORT 43535
#define BUFSIZE 256

struct stOverlappedEx {
    WSAOVERLAPPED overlp;
    WSABUF wsabuf;
    char buf[BUFSIZE];
    SOCKET sock;
    OverlState state;
};
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
void sendLogin(const LoginRequest& request, SOCKET client_sock)
{
    LoginResponse response;
    response.PacketId = LOGIN_RESPONSE;
    strcpy_s(response.username, sizeof(response.username), request.username);
    if (Logrequest(const_cast<LoginRequest&>(request), LoginUser)){
        strcpy_s(response.message, sizeof(response.message), "Login successful");
    }
    else {
        strcpy_s(response.message, sizeof(response.message), "Someone is Login");
    }


    std::vector<char> serializedData = response.serialize();

    stOverlappedEx* sendOverEx = new stOverlappedEx;
    memset(&sendOverEx->overlp, 0, sizeof(sendOverEx->overlp));


    sendOverEx->wsabuf.buf = new char[serializedData.size()];
    memcpy(sendOverEx->wsabuf.buf, serializedData.data(), serializedData.size());
    sendOverEx->wsabuf.len = static_cast<ULONG>(serializedData.size());

    sendOverEx->sock = client_sock;
    sendOverEx->state = OverlState::SEND;

    DWORD sendBytes = 0;
    int retval = WSASend(client_sock, &sendOverEx->wsabuf, 1, &sendBytes, 0, &sendOverEx->overlp, NULL);
    if (retval == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
        cout << "[ERROR] WSASend ����! ���� �ڵ�: " << WSAGetLastError() << endl;
        delete[] sendOverEx->wsabuf.buf;
        delete sendOverEx;
    }
}
void RoomSendPacket(const RoomCreateRequest& room, SOCKET client_sock)
{
    int getRoomid = RoomMake(const_cast<RoomCreateRequest&>(room), Rooms, LoginUser);

    RoomCreateResponse response;
    //�̰� ģ���� ���� ���ָ� �ٲ�
    /*if (getRoomid == Rooms[getRoomid].roomId)
    {
        response.PacketId = ROOM_CREATE_SUCCESS;
    }
    else {
        response.PacketId = ROOM_CREATE_FAIL;
    }*/
    response.PacketId = ROOM_CREATE_RESPONSE;
    response.roomId = getRoomid;
    cout << "������ �� ���̵�" << response.roomId << endl;
    //response.roomCount = Rooms.size();
    //strcpy_s(response.userName, sizeof(response.userName), room.userName);
    //strcpy_s(response.roomName, sizeof(response.roomName), room.roomName);

    std::vector<char> serializedData = response.serialize();

    stOverlappedEx* overlp = new stOverlappedEx;
    memset(&overlp->overlp, 0, sizeof(overlp->overlp));
    overlp->wsabuf.buf = new char[serializedData.size()];
    memcpy(overlp->wsabuf.buf, serializedData.data(), serializedData.size());
    overlp->wsabuf.len = static_cast<ULONG>(serializedData.size());
    overlp->sock = client_sock;
    overlp->state = OverlState::SEND;

    DWORD sendBytes;
    int retval = WSASend(client_sock, &overlp->wsabuf, 1, &sendBytes, 0, &overlp->overlp, NULL);
    cout << "�� ������: " << room.userName << " " << room.RoomMode;
    if (retval == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            std::cerr << "WSASend ����: " << WSAGetLastError() << std::endl;
        }
    }
}
void RoomInSideSendPacket(const RoomRequest& room, SOCKET client_sock)
{
    RoomInResponse response;

    //ģ���� ���� ���ָ� �ٲٱ�
    if (RoomInSide(const_cast<RoomRequest&>(room), Rooms, LoginUser)) {
        //response.PacketId = ROOM_IN_SUCCESS;
        response.PacketId = ROOM_ENTER_RESPONSE;
        response.roomId = room.roomId;
        strcpy(response.roomName, Rooms[room.roomId].roomName);
    }
    /*else {
        response.PacketId = ROOM_IN_FAIL;
    }*/

    std::vector<char> serializedData = response.serialize();

    stOverlappedEx* overlp = new stOverlappedEx;
    memset(&overlp->overlp, 0, sizeof(overlp->overlp));
    overlp->wsabuf.buf = new char[serializedData.size()];
    memcpy(overlp->wsabuf.buf, serializedData.data(), serializedData.size());
    overlp->wsabuf.len = static_cast<ULONG>(serializedData.size());
    overlp->sock = client_sock;
    overlp->state = OverlState::SEND;

    DWORD sendBytes;
    int retval = WSASend(client_sock, &overlp->wsabuf, 1, &sendBytes, 0, &overlp->overlp, NULL);
    if (retval == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            std::cerr << "WSASend ����: " << WSAGetLastError() << std::endl;
        }
    }
}
void RoomOutSideSendPacket(const RoomRequest& room, SOCKET client_sock)
{
	RoomOutSide(Rooms[room.roomId], room);
}
void ProcessPacket(const char* data, size_t length, SOCKET client_sock)
{
    if (length < sizeof(UINT16)) {
        cerr << "Invalid packet size" << endl;
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
        break;
    }
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
        break;
    }
    case ROOM_LEAVE_REQUEST:
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
        break;
    case ROOM_UPDATE_NOTIFY:
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
