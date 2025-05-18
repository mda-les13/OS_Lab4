#include <gtest/gtest.h>
#include <windows.h>
#include <fstream>
#include <string>
#include <cstring>

#define MAX_MESSAGE_LENGTH 20

struct Message {
    char data[MAX_MESSAGE_LENGTH];
};

// Тест для проверки ограничения длины сообщения
TEST(MessageTest, MessageLengthLimit) {
    std::string long_message(21, 'a');
    EXPECT_GT(long_message.length(), MAX_MESSAGE_LENGTH);
}

// Тест для проверки обрезания длинного сообщения
TEST(MessageTest, TruncateMessage) {
    char dest[MAX_MESSAGE_LENGTH];
    std::string long_message(25, 'a');
    
    errno_t err = strncpy_s(dest, MAX_MESSAGE_LENGTH, long_message.c_str(), _TRUNCATE);
    EXPECT_EQ(err, 0); // Проверка, что обрезание прошло без ошибок
    EXPECT_EQ(strlen(dest), MAX_MESSAGE_LENGTH - 1); // Проверка длины обрезанного сообщения
}

// Тест для проверки корректной записи сообщения в файл
TEST(FileWriteTest, WriteMessageToFile) {
    const std::string filename = "test_message.bin";
    
    // Очистка файла перед тестом
    std::ofstream ofs(filename, std::ios::binary | std::ios::trunc);
    ofs.close();
    
    // Запись сообщения
    std::ofstream outFile(filename, std::ios::binary | std::ios::app);
    ASSERT_TRUE(outFile.is_open());
    
    Message msg;
    std::string message = "Hello";
    errno_t err = strncpy_s(msg.data, MAX_MESSAGE_LENGTH, message.c_str(), _TRUNCATE);
    EXPECT_EQ(err, 0);
    
    outFile.write(msg.data, MAX_MESSAGE_LENGTH);
    outFile.close();
    
    // Чтение и проверка
    std::ifstream inFile(filename, std::ios::binary);
    ASSERT_TRUE(inFile.is_open());
    
    Message readMsg;
    inFile.read(readMsg.data, MAX_MESSAGE_LENGTH);
    EXPECT_EQ(std::string(readMsg.data), "Hello");
}

// Тест для проверки создания событий и мьютекса в Receiver
TEST(ReceiverSyncTest, CreateWindowsObjects) {
    HANDLE hSenderReadyEvent = CreateEventW(NULL, TRUE, FALSE, L"TestSenderReadyEvent");
    EXPECT_NE(hSenderReadyEvent, nullptr);
    if (hSenderReadyEvent) CloseHandle(hSenderReadyEvent);
    
    HANDLE hMessageAvailableEvent = CreateEventW(NULL, FALSE, FALSE, L"TestMessageAvailableEvent");
    EXPECT_NE(hMessageAvailableEvent, nullptr);
    if (hMessageAvailableEvent) CloseHandle(hMessageAvailableEvent);
    
    HANDLE hFileMutex = CreateMutexW(NULL, FALSE, L"TestFileMutex");
    EXPECT_NE(hFileMutex, nullptr);
    if (hFileMutex) CloseHandle(hFileMutex);
}

// Тест для проверки открытия событий и мьютекса в Sender
TEST(SenderSyncTest, OpenWindowsObjects) {
    // Создаем тестовые объекты
    HANDLE hSenderReadyEvent = CreateEventW(NULL, TRUE, FALSE, L"TestOpenEvent");
    HANDLE hMessageAvailableEvent = CreateEventW(NULL, FALSE, FALSE, L"TestMessageAvailable");
    HANDLE hFileMutex = CreateMutexW(NULL, FALSE, L"TestFileMutex");
    
    // Открываем их в режиме клиента
    HANDLE hOpenedSenderEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"TestOpenEvent");
    EXPECT_NE(hOpenedSenderEvent, nullptr);
    if (hOpenedSenderEvent) CloseHandle(hOpenedSenderEvent);
    
    HANDLE hOpenedMessageEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"TestMessageAvailable");
    EXPECT_NE(hOpenedMessageEvent, nullptr);
    if (hOpenedMessageEvent) CloseHandle(hOpenedMessageEvent);
    
    HANDLE hOpenedMutex = OpenMutexW(SYNCHRONIZE, FALSE, L"TestFileMutex");
    EXPECT_NE(hOpenedMutex, nullptr);
    if (hOpenedMutex) CloseHandle(hOpenedMutex);
    
    // Очистка
    CloseHandle(hSenderReadyEvent);
    CloseHandle(hMessageAvailableEvent);
    CloseHandle(hFileMutex);
}