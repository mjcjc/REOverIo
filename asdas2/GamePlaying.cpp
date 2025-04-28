#include"GamePlayingPacket.h"



void MovePlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, RoomStart& info)
{
    // info.roomID로 방을 찾아서
    uint32_t roomId = info.roomID;

    // 해당 방 안에 있는 모든 유저를 하나씩 GamePlayer로 만들어서
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
        cout << "방을 찾지 못했음" << endl;
        return;
    }

    auto it = GameStartUsers.find(foundRoom->roomId);
    if (it == GameStartUsers.end())
    {
        cout << "GameStartUser 안에 해당 방이 없음" << endl;
        return;
    }

    vector<GamePlayer>& players = it->second;

    // 여기서 vector 안을 다시 검색해야 함!
    for (auto& player : players)
    {
        if (strcmp(player.user->m_userId, SomePlayer.playerId) == 0)
        {
            player.x = SomePlayer.positionX;
            player.y = SomePlayer.positionY;
            player.z = SomePlayer.positionZ;

            // rotation이나 방향 같은 것도 복사하려면 여기에 추가
            cout << "플레이어 위치 업데이트 완료" << endl;
            return;
        }
    }

    cout << "GameStartUser 안에서도 해당 플레이어를 찾지 못했음" << endl;
}
