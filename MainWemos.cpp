#include "RPIWEMOS_Socket.h"
#include <iostream>
#include <thread>

using namespace std;
using namespace std::chrono;

int main() {
    cout << "Starten als WEMOS Pi (Zelf: 10.0.0.2)" << endl;
    
    // Wij zijn WEMOS, dus we sturen data naar de BUS op 10.0.0.1
    SocketCommunicatie comm("10.0.0.1", 50001);
    
    if (comm.verbind()) {
        cout << "WEMOS Socket server gestart, luisteren op poort 50001..." << endl;
    } else {
        cerr << "Fout bij het starten van de WEMOS server!" << endl;
        return 1;
    }

    // Houd het programma actief
    while (true) {
        comm.checkConnectieStatus();
        this_thread::sleep_for(seconds(1));
    }
    return 0;
}
