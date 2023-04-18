#include <WinSock2.h>
#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <algorithm>
#include <sstream>

int main()
{
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in addrIn;
    addrIn.sin_addr.S_un.S_addr = ADDR_ANY;
    addrIn.sin_port = htons(42069);
    addrIn.sin_family = AF_INET;
    bind(serverSocket, (const sockaddr*)&addrIn, sizeof(addrIn));
    listen(serverSocket, SOMAXCONN);
    printf("Listening on 0.0.0.0:%d\n", 42069);
    u_long async = 1;
    ioctlsocket(serverSocket, FIONBIO, &async);
    std::vector<SOCKET> connectedClients;
    while (true)
    {
        SOCKET client = accept(serverSocket, 0, 0);
        if (client != INVALID_SOCKET) {
            ioctlsocket(client, FIONBIO, &async);
            connectedClients.push_back(client);
            printf("Client connected: %d\n", client);
            std::stringstream ss;
            ss << "Your ID: " << client << std::endl;
            std::string data = ss.str();
            send(client, data.c_str(), data.length(), 0);
            for (SOCKET fd : connectedClients) {
                char buf[255];
                sprintf_s(buf, "Client %d has connected to the server\n", client);
                send(fd, buf, strlen(buf), 0);
            }
        }
        for (SOCKET s : connectedClients) {
            char buffer[255];
            memset(buffer, 0, 255);
            int res = recv(s, buffer, 255, 0);
            int err = WSAGetLastError();
            if (res != INVALID_SOCKET) {
                std::string msg(buffer, res);
                std::cout << "Received from client: " << s << " message: " << msg;
                std::stringstream ss;
                ss << s << ": " << msg;
                std::string data = ss.str();
                for (SOCKET fd : connectedClients) {
                    send(fd, data.c_str(), data.length(), 0);
                }
            }
            if (err == WSAECONNRESET) {
                connectedClients.erase(std::remove(connectedClients.begin(), connectedClients.end(), s));
                char buf[255];
                sprintf_s(buf, "Client %d has disconnected\n", s);
                printf("%s", buf);
                for (SOCKET fd : connectedClients) {
                    send(fd, buf, strlen(buf), 0);
                }
            }
        }
        Sleep(1);
    }
}
;