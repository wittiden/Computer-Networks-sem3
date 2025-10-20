#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>

void ClearAddrAndSockInfo(SOCKET socketName, ADDRINFO* pResult, bool ex = 1) {
    closesocket(socketName);
    freeaddrinfo(pResult);
    WSACleanup();
    exit(ex);
}
double calculate(double a, double b, char operation) {
    switch (operation) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/':
        if (b != 0) return a / b;
        else return 0.0;
    }
}

int main(int argc, char* argv[])
{
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
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, "1111", &hints, &addrResult) != 0) {
        std::cerr << "getaddrinfo failed with error: " << gai_strerrorA(WSAGetLastError()) << std::endl;
        WSACleanup();
        exit(1);
    }


    SOCKET ClientSocket = INVALID_SOCKET;
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
        ListenSocket = INVALID_SOCKET;
        ClearAddrAndSockInfo(ListenSocket, addrResult);
    }

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        ClearAddrAndSockInfo(ListenSocket, addrResult);
    }

    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
        ClearAddrAndSockInfo(ListenSocket, addrResult);
    }

    closesocket(ListenSocket);

    char recvBuffer[512];
    int result;

    do {
        ZeroMemory(recvBuffer, sizeof(recvBuffer));
        result = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (result <= 0) break;
        std::cout << recvBuffer << std::endl;
        std::string firstNumStr = recvBuffer;           

        ZeroMemory(recvBuffer, sizeof(recvBuffer));
        result = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (result <= 0) break;
        std::cout << recvBuffer << std::endl;
        std::string secondNumStr = recvBuffer;

        ZeroMemory(recvBuffer, sizeof(recvBuffer));
        result = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);
        if (result <= 0) break;
        std::cout << recvBuffer << std::endl;
        char operation = recvBuffer[0];

        double a = std::atof(firstNumStr.c_str());
        double b = std::atof(secondNumStr.c_str());

        double calculateResult = calculate(a, b, operation);
        std::string stringResult = std::to_string(calculateResult);
        if (send(ClientSocket, stringResult.c_str(), (int)stringResult.length(), 0) == SOCKET_ERROR) {
            std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
            break;
        }


    } while (result > 0);

    if (shutdown(ClientSocket, SD_SEND) == SOCKET_ERROR) {
        std::cerr << "Shutdown failed with error: " << WSAGetLastError() << std::endl;
        ClearAddrAndSockInfo(ClientSocket, addrResult);
    }

    ClearAddrAndSockInfo(ClientSocket, addrResult, 0);
}
