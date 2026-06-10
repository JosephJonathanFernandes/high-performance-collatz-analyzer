#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <windows.h>

namespace collatz {
namespace research {

class DataExporter {
public:
    static void init_directories() {
        // Native Win32 directory creation to avoid cmd.exe parsing issues
        CreateDirectoryA("data", NULL);
        CreateDirectoryA("data\\csv", NULL);
        CreateDirectoryA("data\\json", NULL);
    }

    static void export_csv(const std::string& filename, const std::string& content) {
        init_directories();
        std::string path = "data/csv/" + filename;
        std::ofstream file(path);
        if (file.is_open()) {
            file << content;
            file.close();
            std::cout << "Data successfully exported to: " << path << "\n";
        } else {
            std::cerr << "Failed to open " << path << " for writing.\n";
        }
    }

    static void export_json(const std::string& filename, const std::string& content) {
        init_directories();
        std::string path = "data/json/" + filename;
        std::ofstream file(path);
        if (file.is_open()) {
            file << content;
            file.close();
            std::cout << "Metadata successfully exported to: " << path << "\n";
        } else {
            std::cerr << "Failed to open " << path << " for writing.\n";
        }
    }
};

} // namespace research
} // namespace collatz
