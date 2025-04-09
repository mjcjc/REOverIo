#include "login.h"
#include "user.h"
bool Logrequest(LoginRequest& request, unordered_map<string, shared_ptr<User>>& user)
{
    string username(request.username);
    // ���ٸ� �α��� ����
    auto it = user.find(username);
    if (it != user.end() && username == it->second->m_userId) {
        if (it->second->Login) {
            cout << "������ �̹� ������ ����" << endl;
            return false;
        }
        else {
            cout << "Login Success" << endl;
            it->second->Login = true;
            it->second->userState = User::USER_STATE_LOBBY;
            return true;
        }

    }
    //���߿� ���� ������ ���� ����� ����ĭ ����� ���� ���� �ʿ�.
    else {
        shared_ptr<User> newUser = make_shared<User>();;
        newUser->Init(&username);
        newUser->userState = User::USER_STATE_LOBBY;
        newUser->Login = true;
        user[username] = newUser;
    }
    return true;
}
