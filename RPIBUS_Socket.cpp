#include "RPIBUS_Socket.h"
#include <iostream>
#include <thread>
#include <array>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

using namespace std;
using namespace std::chrono;

SocketCommunicatie::SocketCommunicatie(string ip, int p) 
    : ipAdresDoel(move(ip)), poort(p), isVerbonden(false), server_fd(-1) {
    laatsteOntvangstTijd = steady_clock::now();
}

SocketCommunicatie::~SocketCommunicatie() {
    isVerbonden = false;
    if (server_fd >= 0) close(server_fd);
}

bool SocketCommunicatie::verbind() {
    struct sockaddr_in address{}; 
    int opt = 1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) return false;

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(poort);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) return false;
    if (listen(server_fd, 3) < 0) return false;

    isVerbonden = true;
    laatsteOntvangstTijd = steady_clock::now();

    thread([this]() {
        struct sockaddr_in client_address{};
        socklen_t addrlen = sizeof(client_address);
        array<char, 1024> buffer{}; 

        while (this->isVerbonden) {
            int new_socket = accept(this->server_fd, (struct sockaddr*)&client_address, &addrlen);
            if (new_socket >= 0) {
                buffer.fill(0); 
                ssize_t bytesRead = read(new_socket, buffer.data(), buffer.size() - 1);
                
                if (bytesRead > 0) {
                    string ontvangen(buffer.data(), bytesRead);
                    this->laatsteOntvangstTijd = steady_clock::now();

                    if (ontvangen.find("Heartbeat") == string::npos) {
                        lock_guard<mutex> lock(this->data_mutex);
                        this->laatsteData = ontvangen;
                        cout << "\n[BUS - Nieuwe Data Ontvangen]: " << ontvangen << endl;
                    } else {
                        cout << "[BUS - Heartbeat Ontvangen]: " << ontvangen << endl;
                    }
                }
                close(new_socket);
            }
        }
    }).detach();

    thread([this]() {
        while (this->isVerbonden) {
            this_thread::sleep_for(seconds(2));
            this->verzendData("Heartbeat: BUS Pi is online.");
        }
    }).detach();

    return true;
}

void SocketCommunicatie::verzendData(const string& bericht) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(poort);

    if (inet_pton(AF_INET, ipAdresDoel.c_str(), &serv_addr.sin_addr) <= 0) {
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0) {
        send(sock, bericht.c_str(), bericht.length(), 0);
    }
    close(sock);
}

string SocketCommunicatie::ontvangData() {
    lock_guard<mutex> lock(data_mutex);
    string data = laatsteData;
    laatsteData.clear(); 
    return data;
}

bool SocketCommunicatie::checkConnectieStatus() {
    auto nu = steady_clock::now();
    auto verstreken = duration_cast<seconds>(nu - laatsteOntvangstTijd).count();

    if (verstreken > 5) {
        if (isVerbonden) {
            cout << "\n[FOUT] BUS Communicatie langer dan 5 sec weggevallen!" << endl;
        }
        isVerbonden = false;
    } else {
        isVerbonden = true;
    }

    return isVerbonden;
}
