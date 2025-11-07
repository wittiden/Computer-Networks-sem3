#define WIN32_LEAN_AND_MEAN

#include "cursor_visibility.h"
#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

class Students {
    std::string fullName;
    std::string groupNumber;
    float scholarship;
    std::vector<int> grades;
    int id;
    static int count;
public:
    Students() { fullName = "unknown", groupNumber = "unknown", scholarship = 0.0f, id = ++count; }
    Students(std::string fullName, std::string groupNumber, float scholarship, std::vector<int> grades) {
        this->fullName = fullName;
        this->groupNumber = groupNumber;
        this->scholarship = scholarship;
        this->grades = grades;
        id = ++count;
    }
    Students(const Students& other) = default;
    ~Students() {}

    std::string getFullName() const { return fullName; }
    std::string getGroupNumber() const { return groupNumber; }
    float getScholarship() const { return scholarship; }
    std::vector<int> getGrades() const { return grades; }
    int getId() const { return id; }

    void setFullName(const std::string& fullName) { this->fullName = fullName; }
    void setGroupNumber(const std::string& groupNumber) { this->groupNumber = groupNumber; }
    void setScholarship(float& scholarship) { this->scholarship = scholarship; }
    void setGrades(const std::vector<int>& grades) { this->grades = grades; }

    std::string toString() const {
        std::string result = "ID: " + std::to_string(id) + "\n";
        result += "Ф.И.О.: " + fullName + "\n";
        result += "Группа: " + groupNumber + "\n";
        result += "Стипендия: " + std::to_string(scholarship) + "\n";
        result += "Оценки: ";
        for (int grade : grades) {
            result += std::to_string(grade) + " ";
        }
         return result;
    }
};
int Students::count = 0;

static void handleClient(SOCKET ClientSocket) {
    std::cout << "Обработка клиента в потоке " << std::this_thread::get_id() << std::endl;

    std::vector<Students> students;
    students.emplace_back(Students("Ivanov Ivan Dmitreevich", "478105", 205, { 9, 9, 8, 7, 10 }));
    students.emplace_back(Students("Lapatov Denis Anatolievich", "478103", 250, { 10, 10, 9, 7, 9 }));
    students.emplace_back(Students("Krutova Elena Mihailovna", "478103", 150, { 7, 4, 8, 6, 9 }));
    students.emplace_back(Students("Mihalchuk Petr Petrovich", "478104", 250, { 10, 9, 8, 10, 10 }));
    students.emplace_back(Students("Lubich Yana Victorovna", "478105", 310, { 10, 10, 10, 10, 10 }));
    students.emplace_back(Students("Ivanova Anna Sergeevna", "478102", 280, { 8, 9, 7, 8, 9 }));
    students.emplace_back(Students("Kozlov Alexey Vladimirovich", "478101", 190, { 6, 7, 8, 7, 6 }));

    char recvBuffer[512];
    int result = recv(ClientSocket, recvBuffer, sizeof(recvBuffer), 0);

    if (result > 0) {
        std::string searchLetter(recvBuffer, result);
        std::cout << "Получен запрос на букву: '" << searchLetter << "'" << std::endl;
        
        std::string response;
        bool found = false;
        for (const auto& student : students) {
            if (student.getFullName().substr(0, student.getFullName().find(' ')).find(searchLetter) != std::string::npos) {
                response += student.toString() + "\n--------------------\n";
                found = true;
            }
        }
        if (!found) {
            response = "Студенты с фамилией на букву '" + searchLetter + "' не найдены.\n";
        }

        send(ClientSocket, response.c_str(), response.length(), 0);
    }

    closesocket(ClientSocket);
    std::cout << "Клиент отключен" << std::endl;
}

static void ClearAddrAndSockInfo(SOCKET socketName, ADDRINFO* pResult, bool ex = 1) {
    closesocket(socketName);
    freeaddrinfo(pResult);
    WSACleanup();
    exit(ex);
}

int main() {
    setlocale(0, "ru");
    hideCursor();

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
        std::cerr << "getaddrinfo failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        exit(1);
    }

    SOCKET ListenSocket = socket(addrResult->ai_family, addrResult->ai_socktype, addrResult->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        freeaddrinfo(addrResult);
        WSACleanup();
        exit(1);
    }

    if (bind(ListenSocket, addrResult->ai_addr, addrResult->ai_addrlen) != 0) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        ClearAddrAndSockInfo(ListenSocket, addrResult);
    }

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        ClearAddrAndSockInfo(ListenSocket, addrResult);
    }

    std::cout << "Сервер запущен и ожидает подключений на порту 1111..." << std::endl;

    int count = 0;
    while (true) {
        SOCKET ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            continue;
        }
        count++;
        std::cout << "Новый клиент подключен, его номер [" << count << "]" << std::endl;

        std::thread clientThread(handleClient, ClientSocket);
        clientThread.detach();
    }
}