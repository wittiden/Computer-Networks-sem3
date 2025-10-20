#define WIN32_LEAN_AND_MEAN

#include "cursor_visibility.h"
#include "input_check.h"

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

static void ClearAddrAndSockInfo(SOCKET socketName, ADDRINFO* pResult, bool ex = 1) {
	closesocket(socketName);
	freeaddrinfo(pResult);
	WSACleanup();
	exit(ex);
}

int main(int argc, char* argv[]) {
	setlocale(0, "ru");

	WSADATA wsadata;
	if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
		std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
		exit(1);
	}
	
	ADDRINFO hints;
	ADDRINFO* addrResult = NULL;
	ZeroMemory(&hints, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	if (getaddrinfo("localhost", "1111", &hints, &addrResult) != 0) {
		std::cerr << "getaddrinfo failed with error: " << gai_strerrorA(WSAGetLastError()) << std::endl;
		WSACleanup();
		exit(1);
	}

	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ClientSocket == INVALID_SOCKET) {
		ClientSocket = INVALID_SOCKET;
		ClearAddrAndSockInfo(ClientSocket, addrResult);
	}

	std::string str;
	std::cout << "Введите строку, нажмите 'enter', чтобы остановить ввод:\n";
	number_filteredInput(str, 0, 1);
	if (sendto(ClientSocket, str.c_str(), str.length(), 0, addrResult->ai_addr, addrResult->ai_addrlen) == SOCKET_ERROR) {
		std::cerr << "sendto failed with error: " << WSAGetLastError() << std::endl;	
	}

	char recvBuffer[512];
	int result;
	SOCKADDR_IN serverAddr;
	int serverAddrLen = sizeof(serverAddr);
	do {
		ZeroMemory(recvBuffer, sizeof(recvBuffer));
		ZeroMemory(&serverAddr, sizeof(serverAddr));

		hideCursor();
		result = recvfrom(ClientSocket, recvBuffer, sizeof(recvBuffer), 0, (SOCKADDR*)&serverAddr, &serverAddrLen);
		std::cout << recvBuffer;
		showCursor();
	} while (result > 0);

	ClearAddrAndSockInfo(ClientSocket, addrResult, 0);
}