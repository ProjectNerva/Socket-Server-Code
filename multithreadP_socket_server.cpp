#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <ctime>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fstream>
#include <signal.h> // clean-up operations before termination
#include <mutex>

#define MAX_LEN 200
#define DIRECTORY_PATH "test_ChatHistory"
#define NUM_COLORS 5

// TODO: have another folder read all the online/live connections that can be connected. Displays ip and port #. Maybe security risk. 
// TODO: chat history txt in completely separate file *DOING*

using namespace std;

struct clientInfo {
    int id;
    string name;
    int socket;
    pthread_t thread;
};

vector<clientInfo> clients;
vector<string> history_txt; // history 
string norm_color = "\033[0m"; // normal color
string error_color = "\033[31m"; // error color
string success_color = "\033[32m";
string colors[] = {"\033[32m", "\033[33m", "\033[34m", "\033[35m","\033[36m"};
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER, cout_mutex = PTHREAD_MUTEX_INITIALIZER;
int seed = 0;

// function definition
void set_name(int id, char name[]);
string color(int code);
void shared_print(string str, bool endLine); // synchronization of cout statements (?)
void handleSignal(int signal);
int broadcastMessage(string message, int sender_id);
int broadcastMessage(int num, int sender_id);
void endConnection(int id);
void *handleClient(void *arg);
void saveChatHistory(string directoryPath, vector<string> txt); // TODO
string getCurrentTime();

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << error_color << "usage: port" << norm_color <<  endl;
        exit(0);
    }

    int PORT = atoi(argv[1]);

    signal(SIGINT, handleSignal); // Ctrl C bind; signal handler

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1)
    {
        perror("socket");
        exit(-1);
    }
    else
    {
        cout << success_color << "Socket creation success..." << norm_color << endl;
    }
    
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = inet_addr("192.168.4.190");
    memset(&serverAddress.sin_zero, 0, sizeof(serverAddress.sin_zero));

    // bind the socket
    if ((::bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1))
    {
        perror("bind error");
        exit(-1);
    }
    else
    {
        cout << success_color << "Bind success..." << norm_color << endl;
    }

    // listening for connections
    if ((listen(serverSocket, 5)) == -1)
    {
        perror("listen error");
        exit(-1);
    }
    else
    {
        cout << success_color << "Listen success..." << norm_color << endl;
    }

    cout << "Setup complete" << endl;

    sockaddr_in client;
    int clientSocket;
    unsigned int len = sizeof(sockaddr_in); // 16 (?/)

    cout << "Connected to chatroom!" << endl;
    cout << colors[NUM_COLORS-1] << " ======== Welcome to Chatroom ======== " << endl << norm_color;

    while(1)
    {
        // acceptibng the connection
        if ((clientSocket = accept(serverSocket, (struct sockaddr *)&client, &len)) == -1)
        {
            perror("accept error");
            exit(-1);
        }

        seed++;
        pthread_t clientThread;
        pthread_mutex_lock(&clients_mutex);
        // clientInfo clientData={seed, string("Anonymous"), clientSocket, clientThread};
        // clients.push_back( clientData );
        clients.push_back( (clientInfo){seed, string("Anonymous"), clientSocket, clientThread} );
        pthread_mutex_unlock(&clients_mutex);

        pthread_create(&clients.back().thread, NULL, handleClient, &clients.back());
    }

    saveChatHistory(DIRECTORY_PATH, history_txt);

    close(serverSocket);
    return 0;
}

void set_name(int id, char name[])
{
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id == id)
        {
            clients[i].name = string(name);
            break;
        }
    }
}

string color(int code)
{
    return colors[code%NUM_COLORS];
}

void shared_print(string str, bool endLine = true)
{
    pthread_mutex_lock(&cout_mutex);
    cout << str;
    if (endLine) cout << endl;
    pthread_mutex_unlock(&cout_mutex);
}

void handleSignal(int signal)
{
    shared_print("Server shutting down...");
    saveChatHistory(DIRECTORY_PATH, history_txt);
    shared_print(success_color + "Chat history saved..." + norm_color);
    exit(0);
}

int broadcastMessage(string message, int sender_id)
{
    char temp[MAX_LEN];
    strcpy(temp, message.c_str());
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id != sender_id)
        {
            send(clients[i].socket, temp, sizeof(temp), 0);
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    return 0;
}

int broadcastMessage(int num, int sender_id)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id != sender_id)
        {
            send(clients[i].socket, &num, sizeof(num), 0);
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    return 0;
}

void endConnection(int id)
{
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < clients.size(); i++)
    {
        if (clients[i].id == id)
        {
            close(clients[i].socket);
            pthread_cancel(clients[i].thread);
            clients.erase(clients.begin() + i);
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);
}

void *handleClient(void *arg)
{
    clientInfo *client = (clientInfo *)arg;
    int clientSocket = client->socket;
    int id = client->id;
    char name[MAX_LEN], buffer[MAX_LEN];

    // receive and set client name
    recv(clientSocket, name, sizeof(name), 0);
    set_name(id, name);

    // display welcome message
    string welcome_message = getCurrentTime() + " " + color(id-1) + string(name) + " has joined" + norm_color;
    broadcastMessage(welcome_message, id);
    shared_print(welcome_message);

    history_txt.push_back(welcome_message);

    while (true)
    {
        // Clear buffer and receive message
        memset(buffer, 0, sizeof(buffer)); // Clear buffer
        int bytes_received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0 || strcmp(buffer, "#exit") == 0)
        {
            string leave_message = getCurrentTime() + " " + color(id-1) + string(name) + " has left" + norm_color;
            broadcastMessage(leave_message, id);
            shared_print(leave_message);
            endConnection(id);

                history_txt.push_back(leave_message);

            break;
        }

        buffer[bytes_received] = '\0'; // Null-terminate the received string
        string message = getCurrentTime() + " " + color(id-1) + string(name) + norm_color + ": " + string(buffer);
        shared_print(message);
        broadcastMessage(message, id);
        
        history_txt.push_back(message);
    }

    pthread_exit(NULL);
    return NULL;
}

// collect from a vector of string of text and have it iterate to write on the thing 
void saveChatHistory(string directoryPath, vector<string> txt)
{
    string timestamp = getCurrentTime();
    replace(timestamp.begin(), timestamp.end(), ':', '-');
    replace(timestamp.begin(), timestamp.end(), ' ', '_');

    // have a new file_txtHistory.txt be created with the date of creation on it, as it always should be
    string fileName = "file_txtHistory" + timestamp + ".htxt";

    ofstream outfile(directoryPath + "/" + fileName);

    // catch error
    if (outfile.is_open())
    {
        // actually writing it into the file 
        for (int i = 0; i < txt.size(); i++)
        {
            outfile << txt[i] << endl;
        }

        cout << "File saved successfully" << endl;

        outfile.close();
    }
    else
    {
        cerr << "error: Cannot open file to save chat history" << endl;
    }
}

string getCurrentTime()
{
    time_t now = time(nullptr);
    char buffer[64];
    strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S]", localtime(&now));
    return string(buffer);
}