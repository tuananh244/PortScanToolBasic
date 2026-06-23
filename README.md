# Công cụ quét cổng mạng (Port Scanner)

Một công cụ rà quét cổng mạng  giao diện dòng lệnh (CLI) hiệu năng cao viết bằng ngôn ngữ C++. Chương trình hỗ trợ quét nhiều IP và Port cùng lúc thông qua kiến trúc đa luồng (Thread Pool) và cơ chế Socket không chặn (Non-blocking Socket) trên hệ điều hành Windows.

## ✨ Tính năng nổi bật

- **Qu quét đa luồng (Thread Pool):** Tối ưu hóa hiệu năng bằng cách duy trì số luồng cố định, tránh quá tải tài nguyên CPU.
- **Socket Không chặn (Non-blocking):** Kết hợp hàm `select()` để kiểm soát chính xác thời gian chờ (Timeout), không bị treo luồng khi gặp Tường lửa (Firewall).
- **Bộ phân tích cú pháp thông minh (Smart Parser):** - Hỗ trợ IP đơn lẻ, dải IP (Range: `192.168.1.10-50`), và dải mạng Subnet (CIDR: `192.168.1.0/24`).
  - Hỗ trợ quét nhiều cổng ngăn cách bởi dấu phẩy hoặc tự động quét toàn bộ cổng (`1-65535`) nếu bỏ trống tham số cổng.
- **Giao diện trực quan:** Hiển thị màu sắc ANSI sinh động (Xanh cho cổng MỞ, Đỏ cho cổng ĐÓNG) kèm bảng thống kê tổng hợp khi kết thúc.

## 📂 Cấu trúc thư mục mã nguồn
`main.cpp`: Xử lý tham số dòng lệnh, điều phối luồng và in kết quả/thống kê.

`parser.h / parser.cpp`: Xử lý chuỗi đầu vào của IP và port

`scanner.h / scanner.cpp`: Hàm quét port

`threadpool.h:` Xử lý hàng đợi công việc đồng thời một cách an toàn (Thread-safe).

## 🛠 Hướng dẫn cài đặt

Công cụ sử dụng các API mạng gốc của Windows (`Winsock2`), do đó khi biên dịch bằng `g++` (MinGW) trên Windows, bạn cần thêm cờ liên kết thư viện `-lws2_32`.

Mở Terminal tại thư mục chứa mã nguồn và chạy lệnh sau:

```bash
g++ main.cpp parser.cpp scanner.cpp -o scanport.exe -lws2_32
```

## 🚀 Hướng dẫn sử dụng
Chương trình nhận các tham số từ dòng lệnh (CLI):
- h: Địa chỉ IP, dải IP hoặc Subnet cần quét (Bắt buộc).
- p: Danh sách cổng mạng cần quét, cách nhau bởi dấu phẩy (Tùy chọn, mặc định quét từ 1-65535).
- n: Số lượng luồng thực thi chạy song song (Mặc định: 1).
- t: Thời gian chờ phản hồi cho mỗi cổng tính bằng mili-giây (Mặc định: 3000ms).
- v: Chế độ hiển thị chi tiết (Verbose), in ra màn hình thời gian thực cả các cổng đóng.

## 🚀 Ví dụ
1. Quét một dải IP (Range) với các cổng chỉ định:
```Bash
.\scanport.exe -h 192.168.1.10-25 -p 80,443,135,445 -n 10 -t 500
```
2. Quét toàn bộ một dải Subnet (CIDR):
```Bash
.\scanport.exe -h 192.168.1.0/24 -p 80,443 -n 20 -t 1000
```
3. Bật chế độ hiển thị chi tiết (-v) để theo dõi các cổng đóng:
```Bash
.\scanport.exe -h 127.0.0.1 -p 135,139,443,445 -n 4 -t 500 -v
```
4. Quét toàn bộ 65535 cổng của một máy (Bỏ qua tham số -p):
```Bash
.\scanport.exe -h 192.168.1.15 -n 50 -t 200
```