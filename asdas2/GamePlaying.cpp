#include"GamePlayingPacket.h"
#include"item.h"


void InitPlayer(unordered_map<uint32_t, shared_ptr<RoomInfo>>& Rooms, RoomStart& info)
{
    uint32_t roomId = info.roomID;
    auto& userList = Rooms[roomId]->userinfo;

    std::vector<Job> jobs = AssignJobs(static_cast<int>(userList.size()));
    int jobIndex = 0;

    for (auto& user : userList)
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
        player.playerJob = jobs[jobIndex++];

        // �κ��丮 �ʱ�ȭ
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
    
           // cout << "�÷��̾� ��ġ ������Ʈ �Ϸ�" << endl;
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
                cout << "�÷��̾ ���� �� ID: " << roomId << endl;
                cout << "������ ID: " << PickItem.itemID << ", ������Ʈ ID: " << PickItem.WorldObjectID << endl;

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
                cout << "�÷��̾ ���� �� ID: " << roomId << endl;
                cout << "��û�� ���� ���� �ε���: " << DropItem.slotIndex << endl;

                size_t index = DropItem.slotIndex;

                if (index < player.inven.iteminfo.size())
                {
                    cout << "���Կ� �ִ� ������ ID: " << player.inven.iteminfo[index] << endl;
                    if (player.inven.iteminfo[index] == DropItem.itemID)
                    {
                        player.inven.iteminfo[index] = 0;
                        cout << "������ ���� �Ϸ�" << endl;
                        return true;
                    }
                    else
                    {
                        cout << "[���] ���Կ� �ִ� �������� ��ġ���� ����!" << endl;
                    }
                }
                else
                {
                    cout << "[����] ���� �ε��� ���� �ʰ�" << endl;
                }

                return false; // �߸��� �ε����ų� �������� �ٸ�
            }
        }
    }

    cout << "[���] �ش� �÷��̾ GameStartUsers���� ã�� �� ����: " << playerId << endl;
    return false;
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

    for (auto& [roomId, players] : GameStartUsers)
    {
        for (auto& player : players)
        {
            if (strcmp(player.user->m_userId, playerId) == 0)
            {
                cout << "�÷��̾ ���� �� ID: " << roomId << endl;
                cout << "������ ID: " << eqItem.itemID << ", ���� �ε���: " << eqItem.slotIndex << endl;

                player.EquipItemID = eqItem.itemID;
                player.playerEquiptHand = eqItem.isEquipped;
                player.inven.iteminfo[eqItem.slotIndex] = eqItem.itemID;
                return true;
            }
        }
    }

    cout << "[���] �ش� �÷��̾ GameStartUsers���� ã�� �� ����: " << playerId << endl;
    return false;
}

std::vector<Job> AssignJobs(int playerCount)
{
    std::vector<Job> jobs;

    if (playerCount < 4) {
        // �ּ� 4���� �Ǿ�� ��� ���� ����
        throw std::runtime_error("Not enough players for full role distribution.");
    }

    // 1. King ���� 2�� ���� ����
    jobs.push_back(Job::MafiaKing);
    jobs.push_back(Job::PoliceKing);

    // 2. ���� �ο� ��
    int remaining = playerCount - 2;

    // 3. �ݹ� ���� (¦���� ��Ȯ�� ��, Ȧ���� Police �ʿ� �� �� ���� ����)
    int mafiaCount = remaining / 2;
    int policeCount = remaining - mafiaCount;

    jobs.insert(jobs.end(), mafiaCount, Job::Mafia);
    jobs.insert(jobs.end(), policeCount, Job::police);

    // 4. ����
    std::shuffle(jobs.begin(), jobs.end(), std::mt19937{ std::random_device{}() });

    return jobs;
}

//���ι̼� �Ǵ��ϴ� ���� �ʿ�
//
void MainMissiongauge(missionSeed & misPacket)
{
    misPacket.gauge;
    misPacket.itemId;
}