#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <iomanip>
#define _CRT_SECURE_NO_WARNINGS

using namespace std;
namespace fs = std::filesystem;

// Структура для хранения информации о файле
struct FileInfo {
    fs::file_time_type last_write_time;
};

// Функция для сравнения времени последней модификации файлов
bool is_file_updated(const fs::path& path, const unordered_map<string, FileInfo>& file_map) {
    auto it = file_map.find(path.string());
    if (it == file_map.end()) {
        // Если файла нет в реестре, значит он новый
        return true;
    }
    else {
        // Если файл есть в реестре, проверяем время последней модификации
        return fs::last_write_time(path) > it->second.last_write_time;
    }
}

// Функция для преобразования времени в строку
string file_time_to_string(fs::file_time_type ftime) {
    auto sctp = chrono::time_point_cast<chrono::system_clock::duration>(ftime - fs::file_time_type::clock::now()
        + chrono::system_clock::now());
    time_t tt = chrono::system_clock::to_time_t(sctp);
    tm local_tm;
    localtime_s(&local_tm, &tt);
    stringstream ss;
    ss << put_time(&local_tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

int main() {
    setlocale(LC_ALL, "ru");
    string directory = R"(C:\Users\PC\Desktop\Тест)"; // Укажите путь к вашему каталогу
    unordered_map<string, FileInfo> file_map;

    // Инициализация начального состояния файлов в каталоге
    for (const auto& entry : fs::directory_iterator(directory)) {
        if (fs::is_regular_file(entry)) {
            file_map[entry.path().string()] = { fs::last_write_time(entry) };
        }
    }

    // Отслеживание новых и обновленных файлов
    while (true) {
        cout << "Проверка новых и обновленных файлов..." << endl;
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (fs::is_regular_file(entry)) {
                auto current_time = fs::last_write_time(entry);
                auto it = file_map.find(entry.path().string());
                if (it == file_map.end()) {
                    // Новый файл
                    cout << "Обнаружен новый файл: " << entry.path().string()
                        << " | Дата и время создания: " << file_time_to_string(current_time) << endl;
                    file_map[entry.path().string()] = { current_time };
                }
                else if (current_time > it->second.last_write_time) {
                    // Обновленный файл
                    cout << "Обнаружен обновленный файл: " << entry.path().string()
                        << " | Дата и время обновления: " << file_time_to_string(current_time) << endl;
                    file_map[entry.path().string()] = { current_time };
                }
            }
        }
        // Ждем некоторое время перед следующей проверкой
        this_thread::sleep_for(chrono::seconds(10));
    }

    return 0;
}
