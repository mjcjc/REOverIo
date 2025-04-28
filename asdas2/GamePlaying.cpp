#include"GamePlayingPacket.h"



void MovePlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, RoomStart& info)
{
    // info.roomID�� ���� ã�Ƽ�
    uint32_t roomId = info.roomID;

    // �ش� �� �ȿ� �ִ� ��� ������ �ϳ��� GamePlayer�� ����
    for (auto& user : Rooms[roomId]->userinfo)
    {
        GamePlayer player;
        player.user = user;
        player.sock = user->sock;
        player.x = 0;
        player.y = 0;
        player.z = 0;

        for (int i = 0; i < 4; ++i)
        {
            player.inven.iteminfo[i] = 0;
        }
        GameStartUsers[roomId].push_back(player);
    }
}

void InGamePlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, PlayerStatus& SomePlayer)
{
    shared_ptr<RoomInfo> foundRoom = nullptr;

    for (auto& [roomId, roomInfo] : Rooms)
    {
        for (auto& user : roomInfo->userinfo)
        {
            if (strcmp(user->m_userId, SomePlayer.playerId) == 0)
            {
                foundRoom = roomInfo;
                break;
            }
        }
        if (foundRoom) break;
    }

    if (!foundRoom)
    {
        cout << "���� ã�� ������" << endl;
        return;
    }

    auto it = GameStartUsers.find(foundRoom->roomId);
    if (it == GameStartUsers.end())
    {
        cout << "GameStartUser �ȿ� �ش� ���� ����" << endl;
        return;
    }

    vector<GamePlayer>& players = it->second;

    // ���⼭ vector ���� �ٽ� �˻��ؾ� ��!
    for (auto& player : players)
    {
        if (strcmp(player.user->m_userId, SomePlayer.playerId) == 0)
        {
            player.x = SomePlayer.positionX;
            player.y = SomePlayer.positionY;
            player.z = SomePlayer.positionZ;

            // rotation�̳� ���� ���� �͵� �����Ϸ��� ���⿡ �߰�
            cout << "�÷��̾� ��ġ ������Ʈ �Ϸ�" << endl;
            return;
        }
    }

    cout << "GameStartUser �ȿ����� �ش� �÷��̾ ã�� ������" << endl;
}
