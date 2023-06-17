#ifndef POLLER_SERVER_HPP
#define POLLER_SERVER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <signal.h>
#include <vector>
#include <queue>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>

// Poller Server Class
class PollerServer
{
public:
    // Constructor
    PollerServer(int portNum, int numWorkerThreads, int bufferSize, const std::string &pollLogFileName, const std::string &pollStatsFileName);

    // Starts the server
    void start();

    // static function that handles the SIGNINT
    static void handleSignalProxy(int signal);

private:
    int portNum_;          // port Number of the Server
    int numWorkerThreads_; // Number of Workers
    int bufferSize_;       // Buffer size to store connections
    int worker_inside;
    std::string pollLogFileName_;            // poll-log fileName
    std::string pollStatsFileName_;          // poll-stats fileName
    std::ofstream pollLog_;                  // poll-log output stream
    std::ofstream pollStats_;                // poll-stats output stream
    std::queue<int> buffer_;                 // buffer to keep connections
    std::mutex mtx_, fileMutex_;             // mutex syncing the buffer
    std::condition_variable cv_, cv_logfile; // condition variable to avoid busy waiting
    std::vector<std::thread> workerThreads;  // vector that keeps the worker threads
    std::vector<std::string> voters_;        // vector that keeps the names of the voters so that we don't have duplicates
    std::map<std::string, int> partyVotes_;  // hash map to keep the number of votes
    bool running_;                           // a running flag
    static PollerServer *serverInstance_;    // a static pointer that will be given the instance pointer value to be able to handle the signal

    // a helper function to sort the poller Stats by num of votes
    static bool sortByValueDesc(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b);

    // The signal handler function
    void handleSignal(int signal);

    /* The master thread function
       The master thread opens the socket and listens for connections accepts them
       which then deposits the fd in the buffer and notifies the worker threads
    */
    void masterThread();
    /* The worker thread function
       The worker threads get the connection they got from the buffer
       and write the vote to the pol-log file
    */
    void workerThread();
};

#endif // POLLER_SERVER_HPP
