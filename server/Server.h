#pragma once
#include <iostream> 
#include <map> 
#include <WS2tcpip.h> 

#define MAX_BUFFER 1024
#define SERVER_PORT 9000
#define MAX_USER 4

typedef struct LOCATION {
	float x;
	float y;
	float z;
}Location;

struct OVER_EX {
	WSAOVERLAPPED overlapped;
	WSABUF dataBuffer;
};

typedef struct SOCKETINFORM {
	OVER_EX over;
	SOCKET socket;
	int clientId;
	bool isUsed = false;
	Location clientLoc = { 0, 0, 0 };
}SockInf;

typedef struct RECVOBJECT {
	int clientId;
	bool isUsed[MAX_USER] = { false };
	bool keyBuffer[MAX_BUFFER];
}R_Obj;

typedef struct SENDOBJECT {
	int clientId;
	bool isUsed[MAX_USER] = { false };
	Location clientLoc[MAX_USER];
}S_Obj;

typedef struct PLAYER {
	int clientid;
	Location clientLoc = { 0 ,0, 0 };
}Player_Obj;

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);
int GetId();