    #include"Packet.h"
    #include"deFine.h"
    #include"Item.h"
    #include"chrono"
    #include"GamePlayingPacket.h"


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


    int ItemSpawnManager(WorldObjectSpawnPacket& ClObj)
    {
        UINT16 itemID = ClObj.itemID;

        // 1. ���� ������ �����۸� ���� ���� �˻�
        switch (itemID)
        {
        case MAIN_PLANT_SEED:
        case MAIN_PLANT:
        case MAIN_PLANT_COAL:
        case MAIN_PLANT_EXTINGUISHER:
            if (ItemSpawns[itemID] >= 1) {
                std::cout << "�ش� ������ ���� �ʰ�!" << std::endl;
                return -1;
            }
            break;
        default:
            break;  // ���� ���� �������� ���
        }

        // 2. ���� ID ����
        UINT64 worldObjID = GenerateWorldObjectUUID();
        ClObj.worldObjectID = static_cast<UINT16>(worldObjID & 0xFFFF);

        // 3. ������Ʈ ���
        WorlditemObjectInfos objInfo;
        objInfo.itemID = itemID;
        objInfo.canPickup = true;
        objInfo.canUseDirectly = false;

        WorldObjects[ClObj.worldObjectID] = objInfo;
        ItemSpawns[itemID]++;

        std::cout << "������Ʈ ���� �Ϸ�: ID = " << ClObj.worldObjectID << std::endl;

        return ClObj.worldObjectID;
    }

