// parser.h
// Đây là tệp tiêu đề cho parser.cpp, định nghĩa các hàm để phân tích cú pháp các chuỗi đầu vào liên quan đến địa chỉ IP và cổng.

#ifndef PARSER_H
#define PARSER_H

#include <string>
#include <vector>

std::vector<std::string> parse_ips(const std::string& input);
std::vector<int> parse_ports(const std::string& input);

#endif