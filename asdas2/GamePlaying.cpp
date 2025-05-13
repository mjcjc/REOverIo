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

        // 인벤토리 초기화
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
    
           // cout << "플레이어 위치 업데이트 완료" << endl;
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
                cout << "플레이어가 속한 방 ID: " << roomId << endl;
                cout << "아이템 ID: " << PickItem.itemID << ", 오브젝트 ID: " << PickItem.WorldObjectID << endl;

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
                cout << "플레이어가 속한 방 ID: " << roomId << endl;
                cout << "요청된 제거 슬롯 인덱스: " << DropItem.slotIndex << endl;

                size_t index = DropItem.slotIndex;

                if (index < player.inven.iteminfo.size())
                {
                    cout << "슬롯에 있는 아이템 ID: " << player.inven.iteminfo[index] << endl;
                    if (player.inven.iteminfo[index] == DropItem.itemID)
                    {
                        player.inven.iteminfo[index] = 0;
                        cout << "아이템 제거 완료" << endl;
                        return true;
                    }
                    else
                    {
                        cout << "[경고] 슬롯에 있는 아이템이 일치하지 않음!" << endl;
                    }
                }
                else
                {
                    cout << "[에러] 슬롯 인덱스 범위 초과" << endl;
                }

                return false; // 잘못된 인덱스거나 아이템이 다름
            }
        }
    }

    cout << "[경고] 해당 플레이어를 GameStartUsers에서 찾을 수 없음: " << playerId << endl;
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

    for (auto& [roomId, players] : GameStartUsers)
    {
        for (auto& player : players)
        {
            if (strcmp(player.user->m_userId, playerId) == 0)
            {
                cout << "플레이어가 속한 방 ID: " << roomId << endl;
                cout << "아이템 ID: " << eqItem.itemID << ", 슬롯 인덱스: " << eqItem.slotIndex << endl;

                player.EquipItemID = eqItem.itemID;
                player.playerEquiptHand = eqItem.isEquipped;
                player.inven.iteminfo[eqItem.slotIndex] = eqItem.itemID;
                return true;
            }
        }
    }

    cout << "[경고] 해당 플레이어를 GameStartUsers에서 찾을 수 없음: " << playerId << endl;
    return false;
}

std::vector<Job> AssignJobs(int playerCount)
{
    std::vector<Job> jobs;

    if (playerCount < 4) {
        // 최소 4명은 되어야 모든 직업 가능
        throw std::runtime_error("Not enough players for full role distribution.");
    }

    // 1. King 역할 2개 고정 배정
    jobs.push_back(Job::MafiaKing);
    jobs.push_back(Job::PoliceKing);

    // 2. 남은 인원 수
    int remaining = playerCount - 2;

    // 3. 반반 나눔 (짝수면 정확히 반, 홀수면 Police 쪽에 더 줄 수도 있음)
    int mafiaCount = remaining / 2;
    int policeCount = remaining - mafiaCount;

    jobs.insert(jobs.end(), mafiaCount, Job::Mafia);
    jobs.insert(jobs.end(), policeCount, Job::police);

    // 4. 셔플
    std::shuffle(jobs.begin(), jobs.end(), std::mt19937{ std::random_device{}() });

    return jobs;
}

//메인미션 판단하는 루프 필요
//
void MainMissiongauge(missionSeed & misPacket)
{
    misPacket.gauge;
    misPacket.itemId;
}