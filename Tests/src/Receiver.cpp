#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#define MAX_MESSAGE_LENGTH 20

struct Message {
    char data[MAX_MESSAGE_LENGTH];
};

int main() {
    std::string fileName;
    std::cout << "Enter the name of the binary file: ";
    std::cin >> fileName;

    int numSenders;
    std::cout << "Enter the number of Sender processes: ";
    std::cin >> numSenders;

    HANDLE* hSenderReadyEvents = new HANDLE[numSenders];
    if (hSenderReadyEvents == nullptr) {
        std::cerr << "Error allocating memory for sender ready events!" << std::endl;
        return 1;
    }

    for (int i = 0; i < numSenders; ++i) {
        std::wstring eventName = L"SenderReadyEvent_" + std::to_wstring(i);
        hSenderReadyEvents[i] = CreateEventW(NULL, TRUE, FALSE, eventName.c_str());
        if (hSenderReadyEvents[i] == NULL) {
            std::cerr << "Error creating sender ready event " << i << "!" << std::endl;
            for (int j = 0; j < i; ++j) {
                CloseHandle(hSenderReadyEvents[j]);
            }
            delete[] hSenderReadyEvents;
            return 1;
        }
    }

    HANDLE hMessageAvailableEvent = CreateEventW(NULL, FALSE, FALSE, L"MessageAvailableEvent");
    if (hMessageAvailableEvent == NULL) {
        std::cerr << "Error creating message available event!" << std::endl;
        for (int i = 0; i < numSenders; ++i) {
            CloseHandle(hSenderReadyEvents[i]);
        }
        delete[] hSenderReadyEvents;
        return 1;
    }

    HANDLE hFileMutex = CreateMutexW(NULL, FALSE, L"FileMutex");
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
        STARTUPINFOW si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        std::wstring commandLine = L"Sender.exe " + std::wstring(fileName.begin(), fileName.end()) + L" " + std::to_wstring(i);
        if (!CreateProcessW(NULL, &commandLine[0], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
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
        std::string command;
        std::cout << "Enter command (read/exit): ";
        std::cin >> command;

        if (command == "read") {
            WaitForSingleObject(hMessageAvailableEvent, INFINITE);

            WaitForSingleObject(hFileMutex, INFINITE);

            std::ifstream inFile(fileName, std::ios::binary);
            if (!inFile) {
                std::cerr << "Error opening file for reading!" << std::endl;
                ReleaseMutex(hFileMutex);
                continue;
            }

            std::vector<Message> messages;
            Message msg;
            while (inFile.read(msg.data, MAX_MESSAGE_LENGTH)) {
                messages.push_back(msg);
            }
            inFile.close();

            for (const auto& m : messages) {
                std::cout << "Received message: " << m.data << std::endl;
            }

            ReleaseMutex(hFileMutex);
        }
        else if (command == "exit") {
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