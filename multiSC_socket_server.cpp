#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

// Constants
#define PORT 8080
#define BUFFER_SIZE 1024

std::mutex cout_mutex; // Mutex for synchronizing console output

// Function to handle client connections
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::string welcome_message = "Welcome to the multi-threaded server!\n";
    
    // Send a welcome message to the client
    send(client_socket, welcome_message.c_str(), welcome_message.size(), 0);

    // Communication loop
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        
        if (bytes_received <= 0) {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Client disconnected." << std::endl;
            break;
        }
        
        // Display the received message
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Client: " << buffer << std::endl;
        }

        // Echo the message back to the client
        send(client_socket, buffer, bytes_received, 0);
    }

    // Close the socket
    close(client_socket);
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    socklen_t client_address_length = sizeof(client_address);
    
    // Create the server socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        std::cerr << "Failed to create socket." << std::endl;
        return -1;
    }

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the address
    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
        std::cerr << "Failed to bind socket." << std::endl;
        close(server_socket);
        return -1;
    }

    // Start listening for connections
    if (listen(server_socket, 10) == -1) {
        std::cerr << "Failed to listen on socket." << std::endl;
        close(server_socket);
        return -1;
    }

    std::cout << "Server is running on port " << PORT << "..." << std::endl;

    // Thread container
    std::vector<std::thread> threads;

    // Accept client connections
    while (true) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_length);
        if (client_socket == -1) {
            std::cerr << "Failed to accept connection." << std::endl;
            continue;
        }

        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "New client connected!" << std::endl;

        // Start a new thread to handle the client
        threads.emplace_back(std::thread(handle_client, client_socket));
    }

    // Clean up threads (should be handled more robustly in real systems)
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    // Close the server socket
    close(server_socket);
    return 0;
}