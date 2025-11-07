#define WIN32_LEAN_AND_MEAN

#include "cursor_visibility.h"
#include "input_check.h"
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <thread>
#include <chrono>

static void ClearAddrAndSockInfo(SOCKET socketName, ADDRINFO* pResult, bool ex = true) {
    closesocket(socketName);
    freeaddrinfo(pResult);
    WSACleanup();
    exit(ex);
}

int main() {
    setlocale(0, "ru");

    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    ADDRINFO hints;
    ADDRINFO* addrResult = NULL;
    ZeroMemory(&hints, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo("localhost", "1111", &hints, &addrResult) != 0) {
        std::cerr << "getaddrinfo failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    SOCKET ClientSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        return 1;
    }

    if (connect(ClientSocket, addrResult->ai_addr, addrResult->ai_addrlen) != 0) {
        std::cerr << "Connect failed with error: " << WSAGetLastError() << std::endl;
        ClearAddrAndSockInfo(ClientSocket, addrResult);
    }

    hideCursor();
    std::cout << "Подключение к серверу";
    for (size_t i = 0; i < 3; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        std::cout << ".";
    }
    showCursor();

    std::string searchLetter;
    std::cout << "\nВведите букву для поиска студентов: ";
    number_filteredInput(searchLetter);

    hideCursor();
    send(ClientSocket, searchLetter.c_str(), searchLetter.length(), 0);
    std::cout << "Запрос отправлен. Ожидание ответа";
    for (size_t i = 0; i < 3; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(700));
        std::cout << ".";
    }
    std::cout << std::endl;

    if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR) {
        std::cerr << "Shutdown failed with error: " << WSAGetLastError() << std::endl;
        ClearAddrAndSockInfo(ClientSocket, addrResult);
    }

    char recvBuffer[1024];
    std::string receivedData;
    int result;

    do {
        result = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (result > 0) {
            receivedData.append(recvBuffer, result);
        }
        else if (result == 0) {
            std::cout << "Соединение закрыто сервером" << std::endl;
        }
        else {
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
        }
    } while (result > 0);

    if (!receivedData.empty()) {
        std::cout << "\nРезультаты поиска:\n";
        std::cout << "=========================================" << std::endl;
        std::cout << receivedData;
        std::cout << "=========================================" << std::endl;
    }

    ClearAddrAndSockInfo(ClientSocket, addrResult, 0);
}