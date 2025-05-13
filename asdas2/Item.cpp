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
        return timestamp ^ randomPart; // 시간과 랜덤을 XOR해서 고유성 확보
    }


    int ItemSpawnManager(WorldObjectSpawnPacket& ClObj)
    {
        UINT16 itemID = ClObj.itemID;

        // 1. 제한 적용할 아이템만 직접 조건 검사
        switch (itemID)
        {
        case MAIN_PLANT_SEED:
        case MAIN_PLANT:
        case MAIN_PLANT_COAL:
        case MAIN_PLANT_EXTINGUISHER:
            if (ItemSpawns[itemID] >= 1) {
                std::cout << "해당 아이템 스폰 초과!" << std::endl;
                return -1;
            }
            break;
        default:
            break;  // 제한 없는 아이템은 통과
        }

        // 2. 고유 ID 생성
        UINT64 worldObjID = GenerateWorldObjectUUID();
        ClObj.worldObjectID = static_cast<UINT16>(worldObjID & 0xFFFF);

        // 3. 오브젝트 등록
        WorlditemObjectInfos objInfo;
        objInfo.itemID = itemID;
        objInfo.canPickup = true;
        objInfo.canUseDirectly = false;

        WorldObjects[ClObj.worldObjectID] = objInfo;
        ItemSpawns[itemID]++;

        std::cout << "오브젝트 스폰 완료: ID = " << ClObj.worldObjectID << std::endl;

        return ClObj.worldObjectID;
    }

