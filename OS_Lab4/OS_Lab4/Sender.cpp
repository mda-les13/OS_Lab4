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

    std::string fileName = argv[1];
    int id = std::stoi(argv[2]);

    std::wstring eventName = L"SenderReadyEvent_" + std::to_wstring(id);
    HANDLE hSenderReadyEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, eventName.c_str());
    if (hSenderReadyEvent == NULL) {
        std::cerr << "Error opening sender ready event!" << std::endl;
        return 1;
    }

    HANDLE hMessageAvailableEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"MessageAvailableEvent");
    if (hMessageAvailableEvent == NULL) {
        std::cerr << "Error opening message available event!" << std::endl;
        CloseHandle(hSenderReadyEvent);
        return 1;
    }

    HANDLE hFileMutex = OpenMutex(SYNCHRONIZE, FALSE, L"FileMutex");
    if (hFileMutex == NULL) {
        std::cerr << "Error opening file mutex!" << std::endl;
        CloseHandle(hSenderReadyEvent);
        CloseHandle(hMessageAvailableEvent);
        return 1;
    }

    SetEvent(hSenderReadyEvent);

    while (true) {
        std::string command;
        std::cout << "Enter command (send/exit): ";
        std::cin >> command;

        if (command == "send") {
            std::string message;
            std::cout << "Enter message (max 20 characters): ";
            std::cin.ignore();
            std::getline(std::cin, message);

            if (message.length() > MAX_MESSAGE_LENGTH) {
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
            errno_t err = strncpy_s(msg.data, MAX_MESSAGE_LENGTH, message.c_str(), _TRUNCATE);
            if (err != 0) {
                std::cerr << "Error copying string!" << std::endl;
                outFile.close();
                ReleaseMutex(hFileMutex);
                continue;
            }

            outFile.write(msg.data, MAX_MESSAGE_LENGTH);
            outFile.close();

            ReleaseMutex(hFileMutex);

            SetEvent(hMessageAvailableEvent);
        }
        else if (command == "exit") {
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