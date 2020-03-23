#include <iostream> 
#include <map> 
#include <WS2tcpip.h> 
#include <mutex>

using namespace std;

#pragma comment(lib, "Ws2_32.lib") 
#define MAX_BUFFER 1024
#define SERVER_PORT 3500

enum POSITION {
	X = 0,
	Y
};

struct OVER_EX { 
	WSAOVERLAPPED overlapped; 
	WSABUF dataBuffer[2]; 
}; 

typedef struct CLIENTSINFORM {
	int m_id;
	int m_pos[2];
	OVER_EX m_over;
	SOCKET m_socket;
	bool m_keyBuffer[MAX_BUFFER];
}ClientInf;

typedef struct OBJECT {
	int m_pos[2] = { -1000, -1000 };
}Obj;

#define MAX_USER 10
mutex g_lock;
static int num = 0;

map <SOCKET, ClientInf> clients;
Obj g_obj[MAX_USER];

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags); 
void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags);

void CALLBACK recv_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags)
{
	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	if (dataBytes == 0) {
		closesocket(clients[client_s].m_socket);
		clients.erase(client_s);
		return;
	}

	g_obj[clients[client_s].m_id].m_pos[X] = clients[client_s].m_pos[X];
	g_obj[clients[client_s].m_id].m_pos[Y] = clients[client_s].m_pos[Y];

	if (clients[client_s].m_keyBuffer[VK_LEFT]) {
		if (g_obj[clients[client_s].m_id].m_pos[X] > 0)
			g_obj[clients[client_s].m_id].m_pos[X] -= 100;
	}

	else if (clients[client_s].m_keyBuffer[VK_RIGHT]) {
		if (g_obj[clients[client_s].m_id].m_pos[X] < 800 - 100)
			g_obj[clients[client_s].m_id].m_pos[X] += 100;
	}

	else if (clients[client_s].m_keyBuffer[VK_DOWN]) {
		if (g_obj[clients[client_s].m_id].m_pos[Y] < 800 - 100)
			g_obj[clients[client_s].m_id].m_pos[Y] += 100;
	}

	else if (clients[client_s].m_keyBuffer[VK_UP]) {
		if (g_obj[clients[client_s].m_id].m_pos[Y] > 0)
			g_obj[clients[client_s].m_id].m_pos[Y] -= 100;
	}
	
	// Ŭ���̾�Ʈ�� closesocket�� ���� ���
	clients[client_s].m_over.dataBuffer[1].len = sizeof(POINT) * MAX_USER;
	memset(&(clients[client_s].m_over.overlapped), 0x00, sizeof(WSAOVERLAPPED)); 
	clients[client_s].m_over.overlapped.hEvent = (HANDLE)client_s; 

	clients[client_s].m_over.dataBuffer[1].buf = reinterpret_cast<char*>(g_obj);
	// &dataBytes -> �̰� ���Ϲ޴°ǵ� ���°� �� ���ٰ� �����ϼ���.
	WSASend(client_s, &(clients[client_s].m_over.dataBuffer[1]), 1, NULL, 0,
		&(clients[client_s].m_over.overlapped), send_callback); 
}

void CALLBACK send_callback(DWORD Error, DWORD dataBytes, LPWSAOVERLAPPED overlapped, DWORD lnFlags) 
{
	DWORD receiveBytes = 0;
	DWORD flags = 0;

	SOCKET client_s = reinterpret_cast<int>(overlapped->hEvent);

	if (dataBytes == 0) { 
		closesocket(clients[client_s].m_socket);
		clients.erase(client_s);
		return; 
	}  
	
	// Ŭ���̾�Ʈ�� closesocket�� ���� ���
	clients[client_s].m_over.dataBuffer[1].len = 2; 
	clients[client_s].m_over.dataBuffer[1].buf = reinterpret_cast<char*>(clients[client_s].m_pos);
	cout << "Sent : " << "xPos: " << g_obj[clients[client_s].m_id].m_pos[X] << ", yPos: " << g_obj[clients[client_s].m_id].m_pos[Y] << endl;
	memset(&(clients[client_s].m_over.overlapped), 0x00, sizeof(WSAOVERLAPPED));
	clients[client_s].m_over.overlapped.hEvent = (HANDLE)client_s; 
	WSARecv(client_s, &clients[client_s].m_over.dataBuffer[0], 1, 0, &flags, 
			& (clients[client_s].m_over.overlapped), recv_callback);
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
		if (num < 10) {
			clientSocket = accept(listenSocket, (struct sockaddr *)&clientAddr, &addrLen);
			if (clientSocket != INVALID_SOCKET) {
				g_lock.lock();
				clients[clientSocket].m_id = num;
				++num;	// �̰ɷ� Ŭ�󿡰� ID�� �ο�����!! -> send�����ؼ� ID�� Ŭ�󿡰� �˷��൵ �ǰ�...
						// �ƴϸ� �ٷ� Recv�ϰ� ������ �� �� �� �׸��°Ŵϱ� ���δ� �ѹ��� �Ѱ��ְ�...
				g_lock.unlock();
				cout << "���� Ȯ��" << endl;
			}
		}

		clients[clientSocket].m_pos[X] = 400.0f;
		clients[clientSocket].m_pos[Y] = 400.0f;
	
		clients[clientSocket] = ClientInf{};
		memset(&clients[clientSocket], 0, sizeof(ClientInf));
		clients[clientSocket].m_socket = clientSocket;
		clients[clientSocket].m_over.dataBuffer[0].len = MAX_BUFFER;
		clients[clientSocket].m_over.dataBuffer[0].buf = reinterpret_cast<char*>(clients[clientSocket].m_keyBuffer);
		flags = 0;
		clients[clientSocket].m_over.overlapped.hEvent = (HANDLE)clients[clientSocket].m_socket;

		// 1�� ������ ����! �츮�� �ϳ� �� ���̴�. ���δ�� MAX���� �� ��.
		// Recv ó���� 'recv_callback' ���� �Ѵ�.
		WSARecv(clients[clientSocket].m_socket, &clients[clientSocket].m_over.dataBuffer[0], 1, NULL,
			&flags, &(clients[clientSocket].m_over.overlapped), recv_callback);

	}
	closesocket(listenSocket);
	WSACleanup();
}