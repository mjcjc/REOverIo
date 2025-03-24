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

// AcceptEx 함수 포인터 가져오기
bool LoadAcceptEx() {
    GUID GuidAcceptEx = WSAID_ACCEPTEX;
    DWORD dwBytes;
    return (WSAIoctl(ListenSock, SIO_GET_EXTENSION_FUNCTION_POINTER, &GuidAcceptEx, sizeof(GuidAcceptEx),
        &AcceptExFunc, sizeof(AcceptExFunc), &dwBytes, NULL, NULL) == 0);
}

// 새로운 AcceptEx 요청 등록
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
        cout << "[ERROR] WSASend 실패! 에러 코드: " << WSAGetLastError() << endl;
        delete[] sendOverEx->wsabuf.buf;
        delete sendOverEx;
    }
}
void RoomSendPacket(const RoomCreateRequest& room, SOCKET client_sock)
{
    int getRoomid = RoomMake(const_cast<RoomCreateRequest&>(room), Rooms, LoginUser);

    RoomCreateResponse response;
    //이건 친구가 수정 해주면 바꿔
    /*if (getRoomid == Rooms[getRoomid].roomId)
    {
        response.PacketId = ROOM_CREATE_SUCCESS;
    }
    else {
        response.PacketId = ROOM_CREATE_FAIL;
    }*/
    response.PacketId = ROOM_CREATE_RESPONSE;
    response.roomId = getRoomid;
    cout << "보낼때 룸 아이디" << response.roomId << endl;
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
    cout << "방 보냈음: " << room.userName << " " << room.RoomMode;
    if (retval == SOCKET_ERROR) {
        if (WSAGetLastError() != WSA_IO_PENDING) {
            std::cerr << "WSASend 에러: " << WSAGetLastError() << std::endl;
        }
    }
}
void RoomInSideSendPacket(const RoomRequest& room, SOCKET client_sock)
{
    RoomInResponse response;

    //친구가 수정 해주면 바꾸기
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
            std::cerr << "WSASend 에러: " << WSAGetLastError() << std::endl;
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
    cout << "패킷 ID : " << PacketId << endl;
    switch (PacketId) {
    case LOGIN_REQUEST: {
        if (length < sizeof(LoginRequest)) {
            cerr << "Invalid LoginRequest packet size" << endl;
            return;

        }
        LoginRequest packet = LoginRequest::deserialize(
            std::vector<char>(data, data + sizeof(LoginRequest))
        );

        cout << "받은 패킷 데이터:" << endl;
        cout << "PacketId: " << packet.PacketId << endl;
        cout << "Username: " << packet.username << endl;

        sendLogin(packet, client_sock);
        cout << "로그인 패킷 처리" << endl;
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

        cout << "받은 패킷 데이터:" << endl;
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
        cout << "받은 패킷 데이터:" << endl;
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
        cout << "받은 패킷 데이터:" << endl;
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

            DWORD flags = 0, recvBytes = 0;
            int retval = WSARecv(clientsoc, &recvOverEx->wsabuf, 1, &recvBytes, &flags, &recvOverEx->overlp, NULL);
            if (retval == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING) {
                cout << "WSARecv 실패: " << WSAGetLastError() << endl;
                closesocket(clientsoc);
                Userkey.erase(clientsoc);
                delete recvOverEx;
            }
        }
        else if (overEx->state == OverlState::RECV) {
            if (bytesTransferred == 0) {
                cout << "클라이언트 연결 종료\n";
                closesocket(clientsoc);
                Userkey.erase(clientsoc);
                delete overEx;
                continue;
            }

            // 패킷 처리
            ProcessPacket(overEx->buf, bytesTransferred, clientsoc);

            // 다시 WSARecv 등록 (비동기 처리 유지)
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
