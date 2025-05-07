#include"GamePlayingPacket.h"
#include"item.h"


void InitPlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, RoomStart& info)
{
    //TODO �������� ���� ���� �ȵ�
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
    
            cout << "�÷��̾� ��ġ ������Ʈ �Ϸ�" << endl;
            return;
        }
    }

    cout << "GameStartUser �ȿ����� �ش� �÷��̾ ã�� ������" << endl;
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
                // ���⿡ ���ϴ� ó�� ����
                cout << "�÷��̾ ���� �� ID: " << roomId << endl;
                cout << "������ ID: " << PickItem.itemID << ", ������Ʈ ID: " << PickItem.WorldObjectID << endl;

                // ��: �κ��丮�� ������ �߰�
                for (auto& slot : player.inven.iteminfo)
                {
                    if (slot == 0)
                    {
                        slot = PickItem.itemID;
                        cout << "������ �߰� �Ϸ�" << endl;
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
        cout << "[���] �ش� �÷��̾ GameStartUsers���� ã�� �� ����: " << playerId << endl;
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
                // ���⿡ ���ϴ� ó�� ����
                cout << "�÷��̾ ���� �� ID: " << roomId << endl;
                cout << "������ ID: " << DropItem.itemID << ", ������Ʈ ID: " << DropItem.itemID << endl;

                // ��: �κ��丮�� ������ �߰�
                for (auto& slot : player.inven.iteminfo)
                {
                    if (slot == DropItem.itemID)
                    {
                        slot = 0;
                        cout << "������ ���� �Ϸ�" << endl;
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
				// ���⿡ ���ϴ� ó�� ����
				cout << "�÷��̾ ���� �� ID: " << roomId << endl;
				cout << "������ ID: " << UseItem.itemID << ", ���� �ε���: " << UseItem.slotIndex << endl;
				// ��: �κ��丮�� ������ �߰�
				for (auto& slot : player.inven.iteminfo)
				{
					if (slot == UseItem.itemID)
					{
						slot = 0;
						cout << "������ ��� �Ϸ�" << endl;
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
				// ���⿡ ���ϴ� ó�� ����
				cout << "�÷��̾ ���� �� ID: " << roomId << endl;
				cout << "������ ID: " << eqItem.itemID << ", ���� �ε���: " << eqItem.slotIndex << endl;
				// ��: �κ��丮�� ������ �߰�
				player.EquipItem = eqItem.itemID;
                return true;
				found = true;
				break;
			}
		}
		if (found) break;
	}
}