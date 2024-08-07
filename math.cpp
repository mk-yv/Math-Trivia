#include <iostream>
#include <fstream>
#include <regex>
#include <string>
#include <thread>
#include <chrono>
#include <windows.h>
#include <sstream>

const std::string LOG_FILE_PATH = "C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Source\\cstrike\\consolelog.txt"; // You might need to change this
const std::regex LOG_REGEX(R"(^(\d{2}\/\d{2}\/\d{4} - \d{2}:\d{2}:\d{2}): (\*DEAD\* )?([^|]+) :\s+(.+)$)");

void send_command_to_css(const std::string& content);
void log_command();
std::string parse_log_entry(const std::string& entry);
std::string evaluate_math_expression(const std::string& expression);
void monitor_log_file();
bool is_valid_equation(const std::string& expression);

std::string parse_log_entry(const std::string& entry) {
    std::smatch match;
    if (std::regex_match(entry, match, LOG_REGEX)) {
        std::string message = match[4].str();
        if (is_valid_equation(message)) {
            return message;
        }
    }
    return "";
}

bool is_valid_equation(const std::string& expression) {
    std::regex equation_regex(R"(^\s*[-+]?\d+(\.\d+)?\s*([+\-*/]\s*[-+]?\d+(\.\d+)?\s*)+$)");
    return std::regex_match(expression, equation_regex);
}

std::string evaluate_math_expression(const std::string& expression) {
    std::istringstream iss(expression);
    double result, num;
    char op;
    iss >> result;
    while (iss >> op >> num) {
        switch (op) {
        case '+': result += num; break;
        case '-': result -= num; break;
        case '*': result *= num; break;
        case '/': result /= num; break;
        default: return "Error: Invalid operator";
        }
    }
    return std::to_string(static_cast<int>(std::round(result)));
}

void log_command() {
    HWND hwnd = FindWindowA(NULL, "Counter-Strike Source");
    if (!hwnd) {
        std::cerr << "Error: Please make sure Counter-Strike Source is open" << std::endl;
        return;
    }

    std::string command = "con_logfile consolelog.txt; con_timestamp 1";
    COPYDATASTRUCT cds = { 0, (DWORD)(command.size() + 1), (void*)command.c_str() };
    LRESULT result = SendMessageA(hwnd, WM_COPYDATA, 0, (LPARAM)&cds);

    std::cout << "Sent command: " << command << ", result: " << result << std::endl;
}

void send_command_to_css(const std::string& content) {
    HWND hwnd = FindWindowA(NULL, "Counter-Strike Source");
    if (!hwnd) {
        std::cerr << "Error: Please make sure Counter-Strike Source is open" << std::endl;
        return;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300)); // adjust if needed

    std::string command = "say " + content;
    COPYDATASTRUCT cds = { 0, (DWORD)(command.size() + 1), (void*)command.c_str() };
    LRESULT result = SendMessageA(hwnd, WM_COPYDATA, 0, (LPARAM)&cds);

    std::cout << "Sent command: " << command << ", result: " << result << std::endl;
}

void monitor_log_file() {
    std::ifstream log_file(LOG_FILE_PATH);
    if (!log_file.is_open()) {
        std::cerr << "Failed to open log file: " << LOG_FILE_PATH << std::endl;
        return;
    }

    log_file.seekg(0, std::ios::end);
    std::string line;
    while (true) {
        while (std::getline(log_file, line)) {
            std::string prompt = parse_log_entry(line);
            if (!prompt.empty()) {
                std::cout << "Extracted prompt: " << prompt << std::endl;
                std::string result = evaluate_math_expression(prompt);
                send_command_to_css(result);
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
        log_file.clear();
        log_file.seekg(0, std::ios::cur);
    }

    log_file.close();
}

int main() {
    log_command();
    monitor_log_file();
    return 0;
}
