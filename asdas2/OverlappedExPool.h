#pragma once
#include "ObjectPool.h"
#include "stSendContext.h" // stOverlappedEx ���� �����ؾ� ��
#include "deFine.h"
using OverlappedExPool = ObjectPool<stOverlappedEx>;

inline OverlappedExPool& GetOverlappedPool() {
    static OverlappedExPool pool;
    return pool;
}


