#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <vector>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring>

using namespace std;

void sendVote(const string &serverName, int portNum, const string &vote)
{
    if (vote.length() < 2)
    {
        return;
    }
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1)
    {
        cerr << "Failed to create client socket\n";
        return;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(portNum);

    struct hostent *host = gethostbyname(serverName.c_str());

    if (host == nullptr)
    {
        std::cerr << "Failed to get host by name\n";
        close(clientSocket);
        return;
    }

    memcpy(&serverAddress.sin_addr, host->h_addr, host->h_length);

    if (connect(clientSocket, reinterpret_cast<sockaddr *>(&serverAddress), sizeof(serverAddress)) == -1)
    {
        cerr << "Failed to connect to the server\n";
        close(clientSocket);
        return;
    }

    char responseBuffer[1024];
    memset(responseBuffer, 0, sizeof(responseBuffer));
    ssize_t bytesRead = recv(clientSocket, responseBuffer, sizeof(responseBuffer), 0);
    if (bytesRead == -1)
    {
        cerr << "Failed to receive response from server\n";
        close(clientSocket);
        return;
    }

    string response = responseBuffer;
    if (response == "SEND NAME PLEASE\n")
    {
        // Extract the voter's name from the vote
        size_t spacePos = vote.find(' ');
        // string firstName = vote.substr(0, spacePos);
        string rest_of_string = vote.substr(spacePos + 1);
        size_t secondSpacePos = rest_of_string.find(' ');

        // Send the voter's name to the server
        string fullName = vote.substr(0, spacePos + secondSpacePos + 1);
        if (send(clientSocket, fullName.c_str(), fullName.size(), 0) == -1)
        {
            cerr << "Failed to send name to server\n";
            close(clientSocket);
            return;
        }

        memset(responseBuffer, 0, sizeof(responseBuffer));
        bytesRead = recv(clientSocket, responseBuffer, sizeof(responseBuffer), 0);
        if (bytesRead == -1)
        {
            cerr << "Failed to receive response from server\n";
            close(clientSocket);
            return;
        }

        response = responseBuffer;
        if (response == "SEND VOTE PLEASE\n")
        {
            // Extract the party name from the vote
            string partyName = vote.substr(spacePos + secondSpacePos + 1 + 1); // Skip the space after last name

            // Send the party name to the server
            string partyVote = partyName + "\n";
            if (send(clientSocket, partyVote.c_str(), partyVote.size(), 0) == -1)
            {
                cerr << "Failed to send vote to server\n";
                close(clientSocket);
                return;
            }

            memset(responseBuffer, 0, sizeof(responseBuffer));
            bytesRead = recv(clientSocket, responseBuffer, sizeof(responseBuffer), 0);
            if (bytesRead == -1)
            {
                cerr << "Failed to receive response from server\n";
                close(clientSocket);
                return;
            }

            response = responseBuffer;
            cout << response; // Print the response from the server
        }
        else if (response == "ALREADY VOTED\n")
        {
            cout << response;
        }
        else
        {
            cerr << "Unexpected response from server\n";
        }
    }
    else
    {
        cerr << "Unexpected response from server\n";
    }

    close(clientSocket);
}

int main(int argc, char *argv[])
{
    // check for correct arguments
    if (argc < 4)
    {
        cout << "Usage: " << argv[0] << " [serverName] [portNum] [inputFile.txt]\n";
        return 1;
    }

    string serverName = argv[1];
    int portNum = stoi(argv[2]);
    string inputFile = argv[3];

    // tries to open input file
    ifstream file(inputFile);
    if (!file)
    {
        cerr << "Failed to open input file\n";
        return 1;
    }

    string vote;            // the vote string
    vector<thread> threads; // vector of threads

    // get every each line and create a thread that calls the sendVote function
    while (getline(file, vote))
    {

        threads.emplace_back(sendVote, serverName, portNum, vote);
    }
    // wait for threads to join
    for (auto &thread : threads)
    {
        thread.join();
    }

    return 0;
}
