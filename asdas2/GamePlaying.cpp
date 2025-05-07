#include"GamePlayingPacket.h"
#include"item.h"


void InitPlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, RoomStart& info)
{
    //TODO 유저별로 직업 설정 안됨
    uint32_t roomId = info.roomID;
   
    for (auto& user : Rooms[roomId]->userinfo)
    {
        GamePlayer player;
        player.user = user;
        player.sock = user->sock;
        player.x = 0;
        player.y = 0;
        player.z = 0;
        player.rotationX = 0;
        player.rotationY = 0;
        player.rotationZ = 0;
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


    for (auto& player : players)
    {
        if (strcmp(player.user->m_userId, SomePlayer.playerId) == 0)
        {
            player.x = SomePlayer.positionX;
            player.y = SomePlayer.positionY;
            player.z = SomePlayer.positionZ;
			player.rotationX = SomePlayer.rotationX;
			player.rotationY = SomePlayer.rotationY;
			player.rotationZ = SomePlayer.rotationZ;
    
            cout << "플레이어 위치 업데이트 완료" << endl;
            return;
        }
    }

    cout << "GameStartUser 안에서도 해당 플레이어를 찾지 못했음" << endl;
}

bool InventoryItemAdd(ItemPickupEvent& PickItem)
{
    const char* playerId = PickItem.playerId;
    bool found = false;

    for (auto& [roomId, players] : GameStartUsers)
    {
        for (auto& player : players)
        {
            if (strcmp(player.user->m_userId, playerId) == 0)
            {
                // 여기에 원하는 처리 수행
                cout << "플레이어가 속한 방 ID: " << roomId << endl;
                cout << "아이템 ID: " << PickItem.itemID << ", 오브젝트 ID: " << PickItem.WorldObjectID << endl;

                // 예: 인벤토리에 아이템 추가
                for (auto& slot : player.inven.iteminfo)
                {
                    if (slot == 0)
                    {
                        slot = PickItem.itemID;
                        cout << "아이템 추가 완료" << endl;
                        return true;
                        break;
                    }
                }

                found = true;
                break;
            }
        }
        if (found) break;
    }

    if (!found)
    {
        cout << "[경고] 해당 플레이어를 GameStartUsers에서 찾을 수 없음: " << playerId << endl;
    }
}

bool InventoryItemRemove(ItemDropEvent& DropItem)
{
    const char* playerId = DropItem.playerId;
    bool found = false;

    for (auto& [roomId, players] : GameStartUsers)
    {
        for (auto& player : players)
        {
            if (strcmp(player.user->m_userId, playerId) == 0)
            {
                // 여기에 원하는 처리 수행
                cout << "플레이어가 속한 방 ID: " << roomId << endl;
                cout << "아이템 ID: " << DropItem.itemID << ", 오브젝트 ID: " << DropItem.itemID << endl;

                // 예: 인벤토리에 아이템 추가
                for (auto& slot : player.inven.iteminfo)
                {
                    if (slot == DropItem.itemID)
                    {
                        slot = 0;
                        cout << "아이템 제거 완료" << endl;
                        return true;
                        break;
                    }
                }

                found = true;
                break;
            }
        }
        if (found) break;
    }
}
bool InventoryItemUse(ItemUseEvent& UseItem)
{
	const char* playerId = UseItem.playerId;
	bool found = false;
	for (auto& [roomId, players] : GameStartUsers)
	{
		for (auto& player : players)
		{
			if (strcmp(player.user->m_userId, playerId) == 0)
			{
				// 여기에 원하는 처리 수행
				cout << "플레이어가 속한 방 ID: " << roomId << endl;
				cout << "아이템 ID: " << UseItem.itemID << ", 슬롯 인덱스: " << UseItem.slotIndex << endl;
				// 예: 인벤토리에 아이템 추가
				for (auto& slot : player.inven.iteminfo)
				{
					if (slot == UseItem.itemID)
					{
						slot = 0;
						cout << "아이템 사용 완료" << endl;
						return true;
						break;
					}
				}
				found = true;
				break;
			}
		}
		if (found) break;
	}
}
bool InventoryItemEquip(ItemEquipEvent& eqItem)
{
	const char* playerId = eqItem.playerId;
	bool found = false;
	for (auto& [roomId, players] : GameStartUsers)
	{
		for (auto& player : players)
		{
			if (strcmp(player.user->m_userId, playerId) == 0)
			{
				// 여기에 원하는 처리 수행
				cout << "플레이어가 속한 방 ID: " << roomId << endl;
				cout << "아이템 ID: " << eqItem.itemID << ", 슬롯 인덱스: " << eqItem.slotIndex << endl;
				// 예: 인벤토리에 아이템 추가
				player.EquipItem = eqItem.itemID;
                return true;
				found = true;
				break;
			}
		}
		if (found) break;
	}
}