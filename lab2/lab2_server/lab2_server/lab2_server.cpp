#define WIN32_LEAN_AND_MEAN

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

static std::string vowelsToUpper(std::string& str) {
	const std::string vowels = "aeiouy";

	for (char& c : str) {
		if (vowels.find(std::tolower(c)) != std::string::npos) {
			c = std::toupper(c);
		}
	}
	return str;
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
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo("localhost", "1111", &hints, &addrResult) != 0) {
		std::cerr << "getaddrinfo failed with error: " << gai_strerrorA(WSAGetLastError()) << std::endl;
		WSACleanup();
		exit(1);
	}

	SOCKET ServerSocket = INVALID_SOCKET;
	ServerSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
	if (ServerSocket == INVALID_SOCKET) {
		ServerSocket = INVALID_SOCKET;
		ClearAddrAndSockInfo(ServerSocket, addrResult);
	}

	if (bind(ServerSocket, addrResult->ai_addr, addrResult->ai_addrlen) == SOCKET_ERROR) {
		std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
		ClearAddrAndSockInfo(ServerSocket, addrResult);
	}


	char recvBuffer[512];
	SOCKADDR_IN clientAddr;
	int clientAddrLen = sizeof(clientAddr);

	while (true) {
		ZeroMemory(recvBuffer, sizeof(recvBuffer));
		ZeroMemory(&clientAddr, sizeof(clientAddr));

		int result = recvfrom(ServerSocket, recvBuffer, sizeof(recvBuffer), 0,(SOCKADDR*)&clientAddr, &clientAddrLen);

		if (result == SOCKET_ERROR) {
			std::cerr << "recvfrom failed with error: " << WSAGetLastError() << std::endl;
			continue;
		}

		if (result > 0) {
			std::string receivedStr(recvBuffer);
			std::cout << "Received from client: " << receivedStr << std::endl;
			vowelsToUpper(receivedStr);
			std::cout << "Sending to client: " << receivedStr << std::endl;

			if (sendto(ServerSocket, receivedStr.c_str(), receivedStr.length(), 0, (SOCKADDR*)&clientAddr, clientAddrLen) == SOCKET_ERROR) {
				std::cerr << "sendto failed with error: " << WSAGetLastError() << std::endl;
			}
		}
	}

	ClearAddrAndSockInfo(ServerSocket, addrResult, 0);
}