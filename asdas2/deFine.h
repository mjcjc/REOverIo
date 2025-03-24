#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <random>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <stdio.h>
#include <MSWSock.h>
#include <memory>
using namespace std;

enum OverlState : UINT16
{
	 ACCEPT, 
	 RECV, 
	 SEND,
};