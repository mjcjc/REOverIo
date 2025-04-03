#pragma once
#include "ObjectPool.h"
#include "stSendContext.h" // stOverlappedEx 정의 포함해야 함
#include "deFine.h"
using OverlappedExPool = ObjectPool<stOverlappedEx>;

inline OverlappedExPool& GetOverlappedPool() {
    static OverlappedExPool pool;
    return pool;
}


