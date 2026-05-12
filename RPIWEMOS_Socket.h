#ifndef RPIWEMOS_SOCKET_H
#define RPIWEMOS_SOCKET_H

#include <string>
#include <chrono>
#include <mutex>
#include <atomic>

class SocketCommunicatie {
private:
    std::string ipAdresDoel;
    int poort;
    std::atomic<bool> isVerbonden; 
    int server_fd;
    std::string laatsteData;
    std::mutex data_mutex;
    std::chrono::time_point<std::chrono::steady_clock> laatsteOntvangstTijd;

public:
    SocketCommunicatie(std::string ipAdresDoel, int poort);
    ~SocketCommunicatie();

    bool verbind();
    void verzendData(const std::string& bericht);
    std::string ontvangData();
    bool checkConnectieStatus();
};

#endif
