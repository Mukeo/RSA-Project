#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main() {
    const int port = 9090;
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }
    if (listen(server_fd, 5) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }
    std::cout << "HTTP server listening on http://localhost:" << port << std::endl;
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        // Simple request read (ignore contents)
        char buffer[1024];
        read(client_fd, buffer, sizeof(buffer));
        const char *response = "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html; charset=UTF-8\r\n"
                               "Content-Length: 86\r\n"
                               "Connection: close\r\n\r\n"
                               "<html><body><h1>RSA Project Server</h1><p>The RSA demo is ready.</p></body></html>";
        write(client_fd, response, strlen(response));
        close(client_fd);
    }
    return 0;
}
