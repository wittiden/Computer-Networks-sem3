#define WIN32_LEAN_AND_MEAN

#include "cursor_visibility.h"

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <chrono>

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
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	if (getaddrinfo("localhost", "1111", &hints, &addrResult) != 0) {
		std::cerr << "getaddrinfo failed with error: " << gai_strerrorA(WSAGetLastError()) << std::endl;
		WSACleanup();
		exit(1);
	}

	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ClientSocket == INVALID_SOCKET) {
		std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(addrResult);
		WSACleanup();
		exit(1);
	}

	if (connect(ClientSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen) != 0) {
		std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
		ClientSocket = INVALID_SOCKET;
		ClearAddrAndSockInfo(ClientSocket, addrResult);
	}

	hideCursor();
	std::cout << "Подключение к серверу устанавливается";
	for (size_t i = 0; i < 3; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		std::cout << ".";
	}
	showCursor();

	std::string str;
	std::cout << "\nВведите строку, нажмите 'Enter', чтобы отправить:\n";
	getline(std::cin, str);

	if (send(ClientSocket, str.c_str(), (int)str.length(), 0) == SOCKET_ERROR) {
		std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
	}

	if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR) {
		std::cerr << "Shutdown failed with error: " << WSAGetLastError() << std::endl;
		ClearAddrAndSockInfo(ClientSocket, addrResult);
	}

	char recvBuffer[512];
	int result;
	std::string receivedData;

	do {
		ZeroMemory(recvBuffer, sizeof(recvBuffer));

		hideCursor();
		result = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);

		if (result > 0) {
			receivedData.append(recvBuffer, result);
		}

	} while (result > 0);

	if (!receivedData.empty()) {
		std::cout << receivedData << std::endl;
	}

	ClearAddrAndSockInfo(ClientSocket, addrResult, 0);
}