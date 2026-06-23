#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <atomic>
#include <winsock2.h>
#include "parser.h"
#include "scanner.h"
#include "threadpool.h"

// Biến Mutex chống xung đột luồng khi in ra màn hình Terminal
std::mutex print_mutex;

// Các biến nguyên tử (Atomic) giúp đếm số lượng an toàn giữa các luồng
std::atomic<int> open_ports_count(0);
std::atomic<int> closed_ports_count(0);

// Định nghĩa mã màu ANSI cho Terminal trên Windows 10/11
#define RESET   "\033[0m"
#define GREEN   "\033[32m"
#define RED     "\033[31m"
#define YELLOW  "\033[33m"
#define CYAN    "\033[36m"
#define BOLD    "\033[1m"

int main(int argc, char* argv[]) {
    std::string ip_input = "";
    std::string port_input = "";
    int num_threads = 1;
    int timeout_ms = 3000;
    bool verbose = false;

    // Phân tích CLI arguments
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" && i + 1 < argc) ip_input = argv[++i];
        else if (arg == "-v") verbose = true;
        else if (arg == "-p" && i + 1 < argc) port_input = argv[++i];
        else if (arg == "-n" && i + 1 < argc) num_threads = std::stoi(argv[++i]);
        else if (arg == "-t" && i + 1 < argc) timeout_ms = std::stoi(argv[++i]);
    }

    // Nếu không nhập -p, parser.cpp của bạn sẽ tự nạp dải 1-65535, do đó chỉ bắt buộc nhập -h
    if (ip_input.empty()) {
        std::cout << YELLOW << "Usage: scanport.exe -h <ip/subnet>  [-v] [-p <ports>] [-n <threads>] [-t <timeout>]\n" << RESET;
        return 1;
    }

    // Kích hoạt chế độ hỗ trợ mã màu ANSI trên Windows Terminal (Tránh lỗi hiện ký tự lạ)
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    GetConsoleMode(hOut, &dwMode);
    SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    // Khởi tạo thư viện Winsock của Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << RED << "[-] WSAStartup failed.\n" << RESET;
        return 1;
    }

    // Phân tích dữ liệu chuỗi IP và Port bằng module parser
    std::vector<std::string> ips = parse_ips(ip_input);
    std::vector<int> ports = parse_ports(port_input);
    
    std::cout << CYAN << BOLD << "====================================================\n";
    std::cout << "               PORT SCANNING TOOL ENGINE            \n";
    std::cout << "====================================================\n" << RESET;

    std::cout << "[*] Scanning in progress...\n";

    // Khối mã lệnh giới hạn vòng đời của ThreadPool
    {
        ThreadPool pool(num_threads);

        for (const std::string& ip : ips) {
            for (int port : ports) {
                // Đẩy công việc bốc từ hàng đợi vào pool luồng xử lý
                pool.enqueue([ip, port, timeout_ms, verbose]() {
                    bool is_open = check_port(ip, port, timeout_ms);
                    
                    if (is_open) {
                        open_ports_count++; // Tăng biến đếm nguyên tử một cách an toàn
                        
                        // Khóa Console khi in để giao diện hiển thị mượt mà không đè chữ
                        std::lock_guard<std::mutex> lock(print_mutex);
                        std::cout << GREEN << "[+] Host: " << ip << " \t| Port: " << port << " \t--> OPEN" << RESET << "\n";
                    } else {
                        closed_ports_count++;
                        
                        if (verbose) { // Chỉ in port đóng nếu người dùng cấu hình tham số -v
                            std::lock_guard<std::mutex> lock(print_mutex);
                            std::cout << RED << "[-] Host: " << ip << " \t| Port: " << port << " \t--> CLOSED" << RESET << "\n";
                        }
                    }
                });
            }
        }
    } // Đợi tất cả các luồng trong ThreadPool hoàn thành nhiệm vụ và giải phóng tại đây

    // --- PHẦN THỐNG KÊ KẾT QUẢ CUỐI CÙNG ---
    std::cout << "\n" << CYAN << BOLD << "====================================================\n";
    std::cout << "                  SCAN REPORT SUMMARY               \n";
    std::cout << "====================================================\n" << RESET;
    std::cout << "[+] Total Ports OPEN   : " << GREEN << BOLD << open_ports_count << RESET << "\n";
    std::cout << "[-] Total Ports CLOSED : " << RED << BOLD << closed_ports_count << RESET << "\n";
    std::cout << "[*] Total Tasks Scanned: " << CYAN << (open_ports_count + closed_ports_count) << RESET << "\n";
    std::cout << CYAN << BOLD << "====================================================\n" << RESET;
    
    // Dọn dẹp Winsock hệ thống
    WSACleanup();
    return 0;
}