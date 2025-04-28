#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

#define MAX_MESSAGE_LENGTH 20
#define MAX_MESSAGES 100

struct Message {
    char data[MAX_MESSAGE_LENGTH];
};

int main() {
    char fileName[256];
    std::cout << "Enter the name of the binary file: ";
    std::cin.getline(fileName, sizeof(fileName));

    int numSenders;
    std::cout << "Enter the number of Sender processes: ";
    std::cin >> numSenders;

    HANDLE* hSenderReadyEvents = new HANDLE[numSenders];
    if (hSenderReadyEvents == NULL) {
        std::cerr << "Error allocating memory for sender ready events!" << std::endl;
        return 1;
    }

    for (int i = 0; i < numSenders; ++i) {
        char eventName[256];
        sprintf(eventName, "SenderReadyEvent_%d", i);

        hSenderReadyEvents[i] = CreateEvent(NULL, TRUE, FALSE, eventName);
        if (hSenderReadyEvents[i] == NULL) {
            std::cerr << "Error creating sender ready event " << i << "!" << std::endl;
            for (int j = 0; j < i; ++j) {
                CloseHandle(hSenderReadyEvents[j]);
            }
            delete[] hSenderReadyEvents;
            return 1;
        }
    }

    HANDLE hMessageAvailableEvent = CreateEvent(NULL, FALSE, FALSE, "MessageAvailableEvent");
    if (hMessageAvailableEvent == NULL) {
        std::cerr << "Error creating message available event!" << std::endl;
        for (int i = 0; i < numSenders; ++i) {
            CloseHandle(hSenderReadyEvents[i]);
        }
        delete[] hSenderReadyEvents;
        return 1;
    }

    HANDLE hFileMutex = CreateMutex(NULL, FALSE, "FileMutex");
    if (hFileMutex == NULL) {
        std::cerr << "Error creating file mutex!" << std::endl;
        CloseHandle(hMessageAvailableEvent);
        for (int i = 0; i < numSenders; ++i) {
            CloseHandle(hSenderReadyEvents[i]);
        }
        delete[] hSenderReadyEvents;
        return 1;
    }

    for (int i = 0; i < numSenders; ++i) {
        STARTUPINFO si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        char commandLine[256];
        sprintf(commandLine, "Sender.exe %s %d", fileName, i);

        if (!CreateProcess(NULL, commandLine, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
            std::cerr << "Error creating Sender process " << i << "!" << std::endl;
            CloseHandle(hFileMutex);
            CloseHandle(hMessageAvailableEvent);
            for (int j = 0; j <= i; ++j) {
                CloseHandle(hSenderReadyEvents[j]);
            }
            delete[] hSenderReadyEvents;
            return 1;
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }

    WaitForMultipleObjects(numSenders, hSenderReadyEvents, TRUE, INFINITE);

    while (true) {
        char command[256];
        std::cout << "Enter command (read/exit): ";
        std::cin >> command;

        if (strcmp(command, "read") == 0) {
            WaitForSingleObject(hMessageAvailableEvent, INFINITE);

            WaitForSingleObject(hFileMutex, INFINITE);

            std::ifstream inFile(fileName, std::ios::binary);
            if (!inFile) {
                std::cerr << "Error opening file for reading!" << std::endl;
                ReleaseMutex(hFileMutex);
                continue;
            }

            Message messages[MAX_MESSAGES];
            int messageCount = 0;
            Message msg;
            while (inFile.read(msg.data, MAX_MESSAGE_LENGTH) && messageCount < MAX_MESSAGES) {
                messages[messageCount++] = msg;
            }
            inFile.close();

            for (int i = 0; i < messageCount; ++i) {
                std::cout << "Received message: " << messages[i].data << std::endl;
            }

            ReleaseMutex(hFileMutex);
        }
        else if (strcmp(command, "exit") == 0) {
            break;
        }
        else {
            std::cerr << "Unknown command!" << std::endl;
        }
    }

    CloseHandle(hFileMutex);
    CloseHandle(hMessageAvailableEvent);
    for (int i = 0; i < numSenders; ++i) {
        CloseHandle(hSenderReadyEvents[i]);
    }
    delete[] hSenderReadyEvents;

    return 0;
}