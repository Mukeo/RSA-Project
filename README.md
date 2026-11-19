# RSA Project

A simple C++ console application that demonstrates RSA key generation, encryption, decryption, and a secure login demo. It also includes a minimal HTTP server listening on port **9090** that reports the service is ready.

## Build
```bash
# Compile the console app
g++ -std=c++17 -O2 -o rsa_project rsa_project.cpp

# Compile the HTTP server
g++ -std=c++17 -O2 -o rsa_server server.cpp
```

## Run
```bash
./rsa_project   # interactive console menu
./rsa_server    # start HTTP server on http://localhost:9090
```
