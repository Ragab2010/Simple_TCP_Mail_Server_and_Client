#include "ServerSocket.h"
// #include "SQLiteDatabase.h"
// #include "JsonHandler.h"
#include "ClientRequestHandler.h"
#include <thread>
#include <cstring>

void clientHandler(int clientSocket, SQLiteDatabase& db) {
    // Receive username from client
    char usernameBuffer[1024] = {0};
    if (read(clientSocket, usernameBuffer, sizeof(usernameBuffer)) < 0) {
        perror("Error reading (name of client) from client");
        close(clientSocket);
        return;
    }
    std::string username(usernameBuffer);

    // Print the client Connected
    std::cout << username << " Connected...." << std::endl;
    
    //create ClientRequestHandler
    ClientRequestHandler client;

    // Loop to handle client requests
    while (true) {

        // receive requestType ,sender, receiver, and message from client by json
        // receive JSON data from client
        char jsonData_buff[1024] = {0};
        if ( read(clientSocket, jsonData_buff, sizeof(jsonData_buff))< 0){
            perror("Error read json from client");
            break;
        }
        std::string jsonData(jsonData_buff);

        // DeSerialize parameters from JSON [requestType ,sender ,receiver , message ]
        auto [requestType ,sender ,receiver , message ] = JsonHandler::deserializeFromJson(jsonData);



        // Process client's choice
        if (requestType == "send") {
            client.handleSendRequest(clientSocket, db ,sender ,receiver , message );
        } else if (requestType == "retrieve_sent") {
            client.handleRetrieveSentRequest(clientSocket, db , sender);
        } else if (requestType == "retrieve_received") {
            client.handleRetrieveReceivedRequest(clientSocket, db ,receiver );
        } else if (requestType == "quit") {
            client.handleQuitRequest(clientSocket , username);
            break; // Exit loop when client quits
        } else {
            // Invalid choice
            if (write(clientSocket, "Invalid choice.", strlen("Invalid choice.") + 1) < 0) {
                perror("Error writing (Invalid choice) to client");
            }
        }
    }

    // Close client socket
    close(clientSocket);
}


int main() {
    // Set up server socket
    ServerSocket serverSocket(8080); // Change port number if needed
    serverSocket.init();

    // Set up SQLite database
    SQLiteDatabase db;
    db.openDatabase("email.db");
    db.createTable();
    ClientRequestHandler ClientRequestHandler;

    while (true) {
        // Accept client connections and handle requests
        int clientSocket = serverSocket.acceptClient();

        std::thread(clientHandler, clientSocket , std::ref(db)).detach();

    }

    // Close server socket and database connection
    serverSocket.closeSocket();
    db.closeDatabase();

    return 0;
}
