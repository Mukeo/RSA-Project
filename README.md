# RSA Project

A full-featured RSA Cryptosystem implementation in C++ featuring both a CLI console tool and a high-contrast Black & White interactive web simulator for client-server authentication.

## Features

- **RSA Core Engine**: Fast modular exponentiation, extended Euclidean algorithm for modular inverse, coprimality verification, and dynamic prime generation.
- **Client-Server Authentication Flow**:
  - Step 1: Key generation ($p, q, n, \phi, e, d$)
  - Step 2: Character-by-character modular exponentiation encryption ($C = M^e \pmod{n}$)
  - Step 3: Transmission of cipher numbers over the network
  - Step 4: Private key isolation on backend ($d, n$)
  - Step 5: Server-side modular exponentiation decryption ($M = C^d \pmod{n}$)
  - Step 6: Credential verification and access control
- **Interactive Web Interface**:
  - High-contrast minimalist Black & White UI
  - Real-time password encryption & decryption breakdown
  - Sign in, Registration, and Arbitrary Message Playground
  - Live Prime Generation, presets, and clipboard tools

## Build & Run

### 1. Web Simulator (HTTP Server)
```bash
# Compile
g++ -std=c++17 -O2 -o rsa_server server.cpp

# Run server
./rsa_server
```
Open **[http://localhost:9090](http://localhost:9090)** in your browser.

### 2. Interactive Console CLI
```bash
# Compile
g++ -std=c++17 -O2 -o rsa_project rsa_project.cpp

# Run CLI
./rsa_project
```
