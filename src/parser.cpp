// parser.cpp
// Đây là tệp phân tích cú pháp cho các chuỗi đầu vào, bao gồm cả địa chỉ IP và cổng. Nó sẽ xử lý các định dạng khác nhau như CIDR, phạm vi IP và danh sách cổng.

#include "parser.h"
#include <sstream>
#include <cstdint>

// Chuyển đổi địa chỉ IP từ chuỗi sang số nguyên 32-bit
uint32_t ipToUInt(const std::string& ip) {
    uint32_t result = 0;
    int octet;
    char dot;
    std::stringstream ss(ip);
    for (int i = 0; i < 4; ++i) {
        ss >> octet;
        result = (result << 8) | (octet & 0xFF);
        if (i < 3) ss >> dot;
    }
    return result;
}

// Chuyển đổi địa chỉ IP từ số nguyên 32-bit sang chuỗi
std::string uintToIp(uint32_t ip) {
    return std::to_string((ip >> 24) & 0xFF) + "." +
           std::to_string((ip >> 16) & 0xFF) + "." +
           std::to_string((ip >> 8) & 0xFF) + "." +
           std::to_string(ip & 0xFF);
}

// Phân tích chuỗi CIDR và thêm các địa chỉ IP vào danh sách
void parseCIDR(const std::string& cidrStr, std::vector<std::string>& ipList) {
    size_t slashPos = cidrStr.find('/');
    std::string ipPart = cidrStr.substr(0, slashPos);
    int prefix = std::stoi(cidrStr.substr(slashPos + 1));
    uint32_t ip = ipToUInt(ipPart);
    uint32_t mask = (0xFFFFFFFF << (32 - prefix)) & 0xFFFFFFFF;
    uint32_t network = ip & mask;
    uint32_t broadcast = network | ~mask;
    for (uint32_t i = network + 1; i < broadcast; ++i) {
        ipList.push_back(uintToIp(i));
    }
}

// Phân tích chuỗi phạm vi IP và thêm các địa chỉ IP vào danh sách
void parseRange(const std::string& rangeStr, std::vector<std::string>& ipList) {
    size_t dashPos = rangeStr.find('-');
    std::string startIpStr = rangeStr.substr(0, dashPos);
    std::string endStr = rangeStr.substr(dashPos + 1);

    if (endStr.find('.') != std::string::npos) {
        uint32_t startIp = ipToUInt(startIpStr);
        uint32_t endIp = ipToUInt(endStr);
        
        if (startIp <= endIp) {
            for (uint32_t i = startIp; i <= endIp; ++i) {
                ipList.push_back(uintToIp(i));
            }
        }
    } else {
        int endOctet = std::stoi(endStr);
        size_t lastDot = startIpStr.find_last_of('.');
        std::string baseIp = startIpStr.substr(0, lastDot + 1);
        int startOctet = std::stoi(startIpStr.substr(lastDot + 1));
        
        if (startOctet <= endOctet) {
            for (int i = startOctet; i <= endOctet; ++i) {
                ipList.push_back(baseIp + std::to_string(i));
            }
        }
    }
}

void listPorts(const std::string& portStr, std::vector<int>& portList) {
    size_t dashPos = portStr.find('-');
    if (dashPos != std::string::npos) {
        int startPort = std::stoi(portStr.substr(0, dashPos));
        int endPort = std::stoi(portStr.substr(dashPos + 1));
        for (int i = startPort; i <= endPort; ++i) {
            portList.push_back(i);
        }
    } else {
        portList.push_back(std::stoi(portStr));
    }
}

// Phân tích chuỗi đầu vào và trả về danh sách các địa chỉ IP hoặc cổng
std::vector<std::string> parse_ips(const std::string& input) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (token.find('/') != std::string::npos) {
            parseCIDR(token, result);
        } else if (token.find('-') != std::string::npos) {
            parseRange(token, result);
        } else {
            result.push_back(token);
        }
    }
    return result;
}


// Phân tích chuỗi danh sách cổng và trả về danh sách các cổng
std::vector<int> parse_ports(const std::string& input) {
    std::vector<int> result;

    // TRƯỜNG HỢP KHÔNG CÓ PORT
    if (input.empty()) {
        listPorts("1-65535", result);
        return result;
    }

    // Trường hợp có port
    std::stringstream ss(input);
    std::string token;
    while (std::getline(ss, token, ',')) {
        listPorts(token, result);
    }
    return result;
}