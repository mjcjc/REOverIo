#include"Packet.h"
#include"deFine.h"
#include"Item.h"
#include"chrono"
#include"GamePlayingPacket.h"

const std::unordered_map<UINT16, int> MaxItemCount = {
    { MAIN_PLANT_SEED, 10 },
    { MAIN_PLANT, 5 },
    { ITEM_GUN, 3 },
    { ITEM_BRAINWASH, 2 },
    // ��Ÿ �����۵� ���� �߰�
};
std::unordered_map<UINT16, WorlditemObjectInfos> WorldObjects;
std::unordered_map<UINT16, int> ItemSpawns;

UINT16 nextWorldObjectId = 1;

UINT64 GenerateWorldObjectUUID() {
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_int_distribution<UINT64> dist;
    UINT64 timestamp = static_cast<UINT64>(std::chrono::system_clock::now().time_since_epoch().count());
    UINT64 randomPart = dist(rng);
    return timestamp ^ randomPart; // �ð��� ������ XOR�ؼ� ������ Ȯ��
}

bool ItemSpawnManager(WorldObjectSpawnPacket& ClObj)
{
    UINT16 itemID = ClObj.itemID;

    // 1. ���� ���� üũ
    if (ItemSpawns[itemID] >= MaxItemCount.at(itemID)) {
        std::cout << "�ش� ������ ���� �ʰ�!" << std::endl;
        return false;
    }

    // 2. ���� UUID-like ID ����
    UINT64 worldObjID = GenerateWorldObjectUUID();
    ClObj.WorldObjectID = static_cast<UINT16>(worldObjID & 0xFFFF);  // ���� 16��Ʈ ���� ������ �߶� ���

    // 3. ���� ���� ������ ��ü ����
    WorlditemObjectInfos objInfo;
    objInfo.itemID = itemID;
    objInfo.canPickup = true;
    objInfo.canUseDirectly = false;

    // 4. ���
    WorldObjects[ClObj.WorldObjectID] = objInfo;  // Ű�� UINT16 ��� ��
    ItemSpawns[itemID]++;

    std::cout << "������Ʈ ���� �Ϸ�: ID = " << ClObj.WorldObjectID << std::endl;

    return true;
}
