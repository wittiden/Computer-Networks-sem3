#define WIN32_LEAN_AND_MEAN

#include "cursor_visibility.h"
#include "input_check.h"

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <conio.h>
#include <thread>
#include <chrono>


void ClearAddrAndSockInfo(SOCKET socketName, ADDRINFO* pResult, bool ex = 1) {
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

	if (connect(ClientSocket, addrResult->ai_addr, addrResult->ai_addrlen) != 0) {
		std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
		ClientSocket = INVALID_SOCKET;
		ClearAddrAndSockInfo(ClientSocket, addrResult);
	}

	char recvBuffer[512];
	int result;
	bool flag = 1;
	std::string input;

	hideCursor();
	std::cout << "Подключение к сереру";
	for (int i = 0; i < 3; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(700));
		std::cout << ".";
	}
	std::cout << "\n\n";
	showCursor();

	do {
		ZeroMemory(recvBuffer, sizeof(recvBuffer));


		std::cout << "===! КАЛЬКУЛЯТОР !===\n";
		std::cout << "Введите 1 число: ";
		number_filteredInput(input, 1);
		if (send(ClientSocket, input.c_str(), (int)input.length(), 0) == SOCKET_ERROR) {
			std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
			break;
		}

		std::cout << "Введите 2 число: ";
		number_filteredInput(input, 1);
		if (send(ClientSocket, input.c_str(), (int)input.length(), 0) == SOCKET_ERROR) {
			std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
			break;
		}

		std::cout << "Введите операцию (+, -, *, /): ";
		number_filteredInput(input);
		if (send(ClientSocket, input.c_str(), (int)input.length(), 0) == SOCKET_ERROR) {
			std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
			break;
		}

		std::cout << "Ответ: ";
		result = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);
		if (result <= 0) break;

		std::cout << recvBuffer;
		
		hideCursor();
		std::cout << "\nНажмите 'esc' для выхода и 'enter' для продолжения\n\n";
		int key = _getch();
		if (key == 27) flag = 0;
		showCursor();

	} while (flag);

	if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR) {
		std::cerr << "Shutdown failed with error: " << WSAGetLastError() << std::endl;
		ClearAddrAndSockInfo(ClientSocket, addrResult);
	}

	ClearAddrAndSockInfo(ClientSocket, addrResult, 0);
}