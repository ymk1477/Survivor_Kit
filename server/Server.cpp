#include "Server.h"

using namespace std;
#pragma comment(lib, "Ws2_32.lib") 

int id = 0;

SockInf g_clients[MAX_USER];
R_Obj recvInform;;
S_Obj sendInform;
Player_Obj PLAYER[MAX_USER];

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	int clientId = reinterpret_cast<int>(overlapped->hEvent);

	if (dataBytes == 0) {
		closesocket(g_clients[clientId].socket);
		cout << clientId + 1 << "번 플레이어 나감" << endl;
		g_clients[clientId].isUsed = false;
		recvInform.isUsed[clientId] = false;
		return;
	}

	for (int i = 0; i < MAX_USER; ++i) {
		sendInform.isUsed[i] = g_clients[i].isUsed;
		sendInform.clientLoc[i] = g_clients[i].clientLoc;
	}

	cout << "X : " << g_clients[clientId].clientLoc.x << ", Y : " << g_clients[clientId].clientLoc.y << ", Z : " << g_clients[clientId].clientLoc.z << endl;

	// 클라이언트가 closesocket을 했을 경우
	g_clients[clientId].over.dataBuffer.len = sizeof(sendInform);
	memset(&(g_clients[clientId].over.overlapped), 0x00, sizeof(WSAOVERLAPPED));
	g_clients[clientId].over.overlapped.hEvent = (HANDLE)clientId;
	

	g_clients[clientId].over.dataBuffer.buf = reinterpret_cast<char*>(&sendInform);
	// &dataBytes -> 이거 리턴받는건데 비우는게 더 좋다고 말씀하셨음.
	WSASend(g_clients[clientId].socket, &(g_clients[clientId].over.dataBuffer), 1, NULL, 0,
		&(g_clients[clientId].over.overlapped), send_callback);
}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	DWORD flags = 0;

	int clientId = reinterpret_cast<int>(overlapped->hEvent);

	// 클라이언트가 closesocket을 했을 경우
	if (dataBytes == 0) {
		closesocket(g_clients[clientId].socket);
		cout << clientId + 1 << "번 플레이어 나감" << endl;
		g_clients[clientId].isUsed = false;
		sendInform.isUsed[clientId] = false;
		return;
	}

	g_clients[clientId].over.dataBuffer.len = sizeof(recvInform);
	g_clients[clientId].over.dataBuffer.buf = reinterpret_cast<char*>(&recvInform);

	/*cout << clientId + 1 << "번 플레이어" << endl;
	cout << "xPos: " << g_clients[clientId].clientLoc.x << ", yPos : " << g_clients[clientId].clientLoc.y << endl;*/

	memset(&(g_clients[clientId].over.overlapped), 0x00, sizeof(WSAOVERLAPPED));
	g_clients[clientId].over.overlapped.hEvent = (HANDLE)clientId;

	WSARecv(g_clients[clientId].socket, &g_clients[clientId].over.dataBuffer, 1, 0, &flags,
		&(g_clients[clientId].over.overlapped), recv_callback);
}

int GetId()
{
	for (int i = 0; i < MAX_USER; ++i) {
		if (g_clients[i].isUsed == false) {
			g_clients[i].isUsed = true;
			return i;
		}
	}
	return -1;
}

int main()
{
	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN serverAddr;
	memset(&serverAddr, 0, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	::bind(listenSocket, (struct sockaddr*)&serverAddr, sizeof(SOCKADDR_IN));

	listen(listenSocket, 5);
	SOCKADDR_IN clientAddr;
	int addrLen = sizeof(SOCKADDR_IN);
	memset(&clientAddr, 0, addrLen);
	SOCKET clientSocket;
	DWORD flags;

	while (true) {
		clientSocket = accept(listenSocket, (struct sockaddr*)&clientAddr, &addrLen);
		if (clientSocket != INVALID_SOCKET) {
			id = GetId();
			cout << id + 1 << "번 클라이언트 접속" << endl;
		}


		if (id < 0) {
			closesocket(clientSocket);
			continue;
		}

		memset(&g_clients[id], 0x00, sizeof(SockInf));
		g_clients[id].socket = clientSocket;
		g_clients[id].over.dataBuffer.len = sizeof(sendInform);
		g_clients[id].over.dataBuffer.buf = reinterpret_cast<char*>(&sendInform);
		ZeroMemory(&(g_clients[id].over.overlapped), sizeof(WSAOVERLAPPED));
		g_clients[id].clientId = id;
		g_clients[id].isUsed = true;
		flags = 0;

		sendInform.clientId = id;
		for (int i = 0; i < MAX_USER; ++i) {
			if (true == g_clients[i].isUsed) {
				//sendInform.clientPos[i] = g_clients[i].clientPos;
				sendInform.isUsed[i] = g_clients[i].isUsed;
			}
		}

		g_clients[id].over.overlapped.hEvent = (HANDLE)id;

		// 1은 버퍼의 개수! 우리는 하나 쓸 것이다. 무턱대고 MAX쓰면 안 돼.
		// Recv 처리는 'recv_callback' 에서 한다.
		WSASend(g_clients[id].socket, &g_clients[id].over.dataBuffer, 1, NULL,
			0, &(g_clients[id].over.overlapped), send_callback);
	}
	closesocket(listenSocket);
	WSACleanup();
}