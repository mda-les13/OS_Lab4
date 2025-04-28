#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

#define MAX_MESSAGE_LENGTH 20

struct Message {
    char data[MAX_MESSAGE_LENGTH];
};

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: Sender.exe <filename> <id>" << std::endl;
        return 1;
    }

    char fileName[256];
    strcpy(fileName, argv[1]);
    int id = atoi(argv[2]);

    char eventName[256];
    sprintf(eventName, "SenderReadyEvent_%d", id);

    HANDLE hSenderReadyEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, eventName);
    if (hSenderReadyEvent == NULL) {
        std::cerr << "Error opening sender ready event!" << std::endl;
        return 1;
    }

    HANDLE hMessageAvailableEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, "MessageAvailableEvent");
    if (hMessageAvailableEvent == NULL) {
        std::cerr << "Error opening message available event!" << std::endl;
        CloseHandle(hSenderReadyEvent);
        return 1;
    }

    HANDLE hFileMutex = OpenMutex(SYNCHRONIZE, FALSE, "FileMutex");
    if (hFileMutex == NULL) {
        std::cerr << "Error opening file mutex!" << std::endl;
        CloseHandle(hSenderReadyEvent);
        CloseHandle(hMessageAvailableEvent);
        return 1;
    }

    SetEvent(hSenderReadyEvent);

    while (true) {
        char command[256];
        std::cout << "Enter command (send/exit): ";
        std::cin >> command;

        if (strcmp(command, "send") == 0) {
            char message[256];
            std::cout << "Enter message (max 20 characters): ";
            std::cin.ignore();
            std::cin.getline(message, sizeof(message));

            if (strlen(message) > MAX_MESSAGE_LENGTH) {
                std::cerr << "Message too long!" << std::endl;
                continue;
            }

            WaitForSingleObject(hFileMutex, INFINITE);

            std::ofstream outFile(fileName, std::ios::binary | std::ios::app);
            if (!outFile) {
                std::cerr << "Error opening file for writing!" << std::endl;
                ReleaseMutex(hFileMutex);
                continue;
            }

            Message msg;
            strncpy(msg.data, message, MAX_MESSAGE_LENGTH);

            outFile.write(msg.data, MAX_MESSAGE_LENGTH);
            outFile.close();

            ReleaseMutex(hFileMutex);

            SetEvent(hMessageAvailableEvent);
        }
        else if (strcmp(command, "exit") == 0) {
            break;
        }
        else {
            std::cerr << "Unknown command!" << std::endl;
        }
    }

    CloseHandle(hSenderReadyEvent);
    CloseHandle(hMessageAvailableEvent);
    CloseHandle(hFileMutex);

    return 0;
}