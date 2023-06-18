#include "Master.hpp"

using namespace std;

// we start by initializing the server instance to null
PollerServer *PollerServer::serverInstance_ = nullptr;

// Server Constructor
PollerServer::PollerServer(int portNum, int numWorkerThreads, int bufferSize, const std::string &pollLogFileName, const std::string &pollStatsFileName)
    : portNum_(portNum), numWorkerThreads_(numWorkerThreads), bufferSize_(bufferSize), pollLogFileName_(pollLogFileName), pollStatsFileName_(pollStatsFileName), running_(true)
{
}

PollerServer::~PollerServer() {}

void PollerServer::start()
{
    // Open the poll-log file
    pollLog_.open(pollLogFileName_, std::ofstream::app);

    // Assign server instance to this instance
    serverInstance_ = this;

    // Create worker threads
    for (int i = 0; i < numWorkerThreads_; ++i)
    {
        workerThreads.emplace_back(&PollerServer::workerThread, this);
    }

    // Start the master thread
    masterThread();

    running_ = false;

    // notify all worker threads to continue execution (end what they are doing and exit)
    cv_.notify_all();
    cv_logfile.notify_all();

    // Wait for all worker threads to finish
    cout << "waiting for threads to join\n";
    for (auto &thread : workerThreads)
    {
        thread.join();
    }

    // Close the poll-log file
    pollLog_.close();
}

void PollerServer::handleSignalProxy(int signal)
{
    // this will call the actual member function on the server instance
    // since we cannot pass a PolleServer member function as a signal handler
    if (serverInstance_)
    {
        serverInstance_->handleSignal(signal);
    }
}

bool PollerServer::sortByValueDesc(const std::pair<std::string, int> &a, const std::pair<std::string, int> &b)
{
    return a.second > b.second;
}

// Hadles the SIGINT signal
void PollerServer::handleSignal(int signal)
{
    cout << "in here\n";
    if (signal == SIGINT)
    {
        cout << "sigint received\n";
        running_ = false;
        cv_.notify_all();
        cv_logfile.notify_all();

        // Store the map pairs in a vector
        std::vector<std::pair<std::string, int>> pairs(partyVotes_.begin(), partyVotes_.end());
        int count = 0;
        // Sort the vector by value in descending order
        std::sort(pairs.begin(), pairs.end(), sortByValueDesc);
        // Open pollStats file
        pollStats_.open(pollStatsFileName_, std::ofstream::trunc);

        // write the sorted pairs to the pollStats file
        for (const auto &pair : pairs)
        {
            count += pair.second;
            string partyName = (pair.first).substr(0, (pair.first).size() - 1);
            pollStats_ << partyName << ": " << pair.second << "\n";
        }

        pollStats_ << "TOTAL : " << count << "\n";
        pollStats_.flush();
        pollStats_.close();
    }
}

void PollerServer::masterThread()
{
    // create the server socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1)
    {
        std::cerr << "Failed to create server socket\n";
        return;
    }
    // set up the signal handler on this thread
    signal(SIGINT, handleSignalProxy);

    // Set the server socket to non-blocking mode
    int flags = fcntl(serverSocket, F_GETFL, 0);
    if (flags == -1)
    {
        std::cerr << "Failed to get socket flags\n";
        close(serverSocket);
        return;
    }
    if (fcntl(serverSocket, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        std::cerr << "Failed to set socket to non-blocking mode\n";
        close(serverSocket);
        return;
    }

    // init the server address and port
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(portNum_);

    // bind the server address to the socket
    if (bind(serverSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        std::cerr << "Failed to bind server socket\n";
        close(serverSocket);
        return;
    }
    std::cout << "opened socket\n";

    // listen for incomimg connections
    if (listen(serverSocket, 256) == -1)
    {
        std::cerr << "Failed to listen on server socket\n";
        close(serverSocket);
        return;
    }

    while (running_)
    {
        // cout << "trying again or locked \n";
        sockaddr_in clientAddress{};
        socklen_t clientAddressLength = sizeof(clientAddress);

        // accept the client connection
        int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr *>(&clientAddress), &clientAddressLength);
        if (clientSocket == -1)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // No pending connections, continue with the loop
                continue;
            }
            else
            {
                std::cerr << "Failed to accept client connection\n";
                break;
            }
        }

        // try lock the mutex and deposit in the condition variable which then checks if the buffer size is not full
        // and there is space to add a new connection and the flag is false to awake
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]()
                 { return buffer_.size() * sizeof(clientSocket) + sizeof(clientSocket) < bufferSize_ || !running_; });

        // if the flag is false close the socket
        if (!running_)
        {
            close(clientSocket);
            break;
        }

        // if it's awake push the clientSocket fd into the buffer so workers can read
        buffer_.push(clientSocket);

        // unlock the mutex and notify one worker thread
        lock.unlock();
        cv_.notify_all();
    }

    // if exited close the serverSocket
    close(serverSocket);
}

void PollerServer::workerThread()
{

    while (running_)
    {
        // worker thread tries to lock the mutex if buffer is not empty or flag is false
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this]()
                 { return !buffer_.empty() || !running_; });

        // if flag is false exit the loop
        if (!running_)
            break;

        // get the first client connection remove it from buffer, unlock the mutex and notify the other threads
        int clientSocket = buffer_.front();
        buffer_.pop();
        lock.unlock();
        cv_.notify_all();

        // Send the "SEND NAME PLEASE" message to the client

        const std::string sendNameMsg = "SEND NAME PLEASE\n";
        if (send(clientSocket, sendNameMsg.c_str(), sendNameMsg.size(), 0) == -1)
        {
            std::cerr << "Failed to send 'SEND NAME PLEASE' message to client\n";
            close(clientSocket);
            continue;
        }

        // Wait for the client to send the name

        char nameBuffer[1024];
        // clear the name buffer
        memset(nameBuffer, 0, sizeof(nameBuffer));
        // and read the name from the client
        ssize_t bytesReceived = recv(clientSocket, nameBuffer, sizeof(nameBuffer), 0);
        if (bytesReceived == -1)
        {
            std::cerr << "Failed to receive name from client\n";
            close(clientSocket);
            continue;
        }

        // Check if the voter has already voted
        std::string name = nameBuffer;

        // check if the name is duplicate and if yes send that the voter has allready voted
        if (find(voters_.begin(), voters_.end(), name) != voters_.end())
        {
            const std::string alreadyVotedMsg = "ALREADY VOTED\n";
            if (send(clientSocket, alreadyVotedMsg.c_str(), alreadyVotedMsg.size(), 0) == -1)
            {
                std::cerr << "Failed to send 'ALREADY VOTED' message to client\n";
            }
            close(clientSocket);
            continue;
        }
        // if not add the voter's name to the vector
        voters_.push_back(name);

        // Send "SEND VOTE PLEASE" message to the client
        const std::string sendVoteMsg = "SEND VOTE PLEASE\n";
        if (send(clientSocket, sendVoteMsg.c_str(), sendVoteMsg.size(), 0) == -1)
        {
            std::cerr << "Failed to send 'SEND VOTE PLEASE' message to client\n";
            close(clientSocket);
            continue;
        }

        // Wait for the client to send the party name
        char partyBuffer[1024];

        memset(partyBuffer, 0, sizeof(partyBuffer));

        bytesReceived = recv(clientSocket, partyBuffer, sizeof(partyBuffer), 0);
        if (bytesReceived == -1)
        {
            std::cerr << "Failed to receive party name from client\n";
            close(clientSocket);
            continue;
        }

        std::string party = partyBuffer;

        // Send "VOTE for [Party Name] RECORDED" message to the client
        std::string voteRecordedMsg = "VOTE for " + party + " RECORDED\n";
        if (send(clientSocket, voteRecordedMsg.c_str(), voteRecordedMsg.size(), 0) == -1)
        {
            std::cerr << "Failed to send 'VOTE RECORDED' message to client\n";
        }
        // close the socket
        close(clientSocket);

        // Acquire a lock before accessing the file
        std::unique_lock<std::mutex> lock_file(fileMutex_);
        cv_logfile.wait(lock_file, [this]()
                        { return !worker_inside || !running_; });
        worker_inside++;

        // Write the name and party to the poll-log file
        pollLog_ << name << " " << party;
        pollLog_.flush();

        // Update the server statistics (number of votes for each party)
        partyVotes_[party]++;

        worker_inside--;
        // unlock the access to the file
        lock_file.unlock();
        cv_logfile.notify_all();

        // clientSocket = 0;
    }

    cout << "thread exiting\n";
}
