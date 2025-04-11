#include "login.h"
#include "user.h"
bool Logrequest(LoginRequest& request, unordered_map<string, shared_ptr<User>>& user, SOCKET client_sock)
{
    string username(request.username);
    // 같다면 로그인 성공
    auto it = user.find(username);
    if (it != user.end() && username == it->second->m_userId) {
        if (it->second->Login) {
            cout << "누군가 이미 접속한 계정" << endl;
            return false;
        }
        else {
            cout << "Login Success" << endl;
            it->second->Login = true;
            it->second->userState = User::USER_STATE_LOGIN;
            return true;
        }

    }
    //나중에 계정 없을때 따로 만드는 생성칸 만들고 로직 변경 필요.
    else {
        shared_ptr<User> newUser = make_shared<User>();;
        newUser->Init(&username);
        newUser->userState = User::USER_STATE_LOGIN;
        newUser->Login = true;
        newUser->sock = client_sock;
        user[username] = newUser;
    }
    return true;
}
