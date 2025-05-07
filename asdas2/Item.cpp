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
    // 기타 아이템도 여기 추가
};
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

bool ItemSpawnManager(WorldObjectSpawnPacket& ClObj)
{
    UINT16 itemID = ClObj.itemID;

    // 1. 스폰 제한 체크
    if (ItemSpawns[itemID] >= MaxItemCount.at(itemID)) {
        std::cout << "해당 아이템 스폰 초과!" << std::endl;
        return false;
    }

    // 2. 고유 UUID-like ID 생성
    UINT64 worldObjID = GenerateWorldObjectUUID();
    ClObj.WorldObjectID = static_cast<UINT16>(worldObjID & 0xFFFF);  // 만약 16비트 제한 있으면 잘라서 사용

    // 3. 서버 내부 관리용 객체 생성
    WorlditemObjectInfos objInfo;
    objInfo.itemID = itemID;
    objInfo.canPickup = true;
    objInfo.canUseDirectly = false;

    // 4. 등록
    WorldObjects[ClObj.WorldObjectID] = objInfo;  // 키는 UINT16 사용 중
    ItemSpawns[itemID]++;

    std::cout << "오브젝트 스폰 완료: ID = " << ClObj.WorldObjectID << std::endl;

    return true;
}
