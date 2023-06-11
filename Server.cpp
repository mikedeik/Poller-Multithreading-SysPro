#include "Master.hpp"

int main(int argc, char* argv[]) {
    if (argc < 6) {
        std::cout << "Usage: " << argv[0] << " [portnum] [numWorkerthreads] [bufferSize] [poll-log] [poll-stats]\n";
        return 1;
    }

    int portNum = std::stoi(argv[1]);
    int numWorkerThreads = std::stoi(argv[2]);
    int bufferSize = std::stoi(argv[3]);
    std::string pollLogFileName = argv[4];
    std::string pollStatsFileName = argv[5];

    PollerServer server(portNum, numWorkerThreads, bufferSize, pollLogFileName, pollStatsFileName);
    server.start();

    return 0;
}
