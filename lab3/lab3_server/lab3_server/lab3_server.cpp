#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <algorithm>
#include <thread>
#include <vector>

static void ClearAddrAndSockInfo(SOCKET socketName, ADDRINFO* pResult, bool ex = 1) {
    if (socketName != INVALID_SOCKET) {
        closesocket(socketName);
    }
    if (pResult != NULL) {
        freeaddrinfo(pResult);
    }
    WSACleanup();
    if (ex) exit(ex);
}

static std::string processString(const std::string& input) {
    std::string result = input;

    if (result.length() % 3 == 0) {
        for (int i = 0; i < result.length(); i++) {
            if (isdigit(result[i])) {
                int digit = result[i] - '0';
                if (digit != 0 && digit % 3 == 0) {
                    result.erase(i, 1);
                    i--;
                }
            }
        }
    }

    return result;
}

static void handleClient(SOCKET СlientSocket) {
    std::cout << "Обработка клиента в потоке " << std::this_thread::get_id() << std::endl;

    char recvBuffer[512];
    int result;
    std::string receivedData;

    do {
        ZeroMemory(recvBuffer, sizeof(recvBuffer));

        result = recv(СlientSocket, recvBuffer, sizeof(recvBuffer), 0);

        if (result > 0) {
            receivedData.append(recvBuffer, result);
        }

    } while (result > 0);

    if (!receivedData.empty()) {
        std::cout << "Получено от клиента: " << receivedData << std::endl;
        std::string processedString = processString(receivedData);
        std::cout << "Обработанная строка: " << processedString << std::endl;

        if (send(СlientSocket, processedString.c_str(), (int)processedString.length(), 0) == SOCKET_ERROR) {
            std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
        }
    }

    if (shutdown(СlientSocket, SD_SEND) == SOCKET_ERROR) {
        std::cerr << "Shutdown failed with error: " << WSAGetLastError() << std::endl;
    }
    closesocket(СlientSocket);

    std::cout << "Клиент отключен, поток завершен" << std::endl;
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

    if (getaddrinfo(NULL, "1111", &hints, &addrResult) != 0) {
        std::cerr << "getaddrinfo failed with error: " << gai_strerrorA(WSAGetLastError()) << std::endl;
        WSACleanup();
        exit(1);
    }

    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(addrResult);
        WSACleanup();    
        exit(1);
    }

    if (bind(ListenSocket, addrResult->ai_addr, (int)addrResult->ai_addrlen) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        ClearAddrAndSockInfo(ListenSocket, addrResult);
    }

    int availableConnections = SOMAXCONN;

    if (listen(ListenSocket, availableConnections) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        ClearAddrAndSockInfo(ListenSocket, addrResult);
    }

    std::cout << "Сервер запущен и ожидает подключения..." << std::endl;

    std::vector<std::thread> clientThreads;
    while (true) {
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::cout << "Новый клиент подключен!" << std::endl;

        clientThreads.emplace_back(handleClient, ClientSocket);

        clientThreads.back().detach();
    }

    closesocket(ListenSocket);
    ClearAddrAndSockInfo(INVALID_SOCKET, addrResult, 0);
    return 0;
}