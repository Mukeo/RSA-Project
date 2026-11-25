# RSA Project

An interactive C++ implementation of the **RSA Cryptosystem** with a modern, high-contrast **Black & White** client-server authentication web simulator and console CLI.

---

## 🚀 Overview

This project provides a full end-to-end visualization of public-key cryptography and secure authentication:

1. **Key Generation**:
   - Primes $p$ and $q$
   - Modulus $n = p \times q$
   - Euler's totient $\phi(n) = (p-1)(q-1)$
   - Public exponent $e$ such that $\gcd(e, \phi(n)) = 1$
   - Private exponent $d = e^{-1} \pmod{\phi(n)}$ via Extended Euclidean algorithm
2. **Client-Side Encryption**:
   - Character-by-character modular exponentiation: $C = M^e \pmod{n}$
3. **Network Transmission**:
   - Transmits ciphertext sequence over the network.
4. **Private Key Isolation**:
   - Key $(d, n)$ resides strictly on the server and is never sent across the network.
5. **Server-Side Decryption**:
   - Character restoration: $M = C^d \pmod{n}$
6. **Authentication Verification**:
   - Compares decrypted password against stored credentials.

---

## 🛠️ Build and Run

### 1. Web Simulator (Port 9090)
```bash
# Compile the C++ HTTP server
g++ -std=c++17 -O2 -o rsa_server server.cpp

# Run server
./rsa_server
```
Open **[http://localhost:9090](http://localhost:9090)** in your web browser.

### 2. Interactive Console Application
```bash
# Compile the console CLI
g++ -std=c++17 -O2 -o rsa_project rsa_project.cpp

# Run CLI
./rsa_project
```

---

## 🎨 Features
- **Monochrome Minimalist UI**: High-contrast black and white aesthetic using `Inter` and `JetBrains Mono`.
- **Dual-Pane Simulator**: Live client input on the left and full step-by-step mathematical breakdown on the right.
- **Interactive Controls**:
  - Sign In & Registration modes
  - Arbitrary Message Playground (encrypt/decrypt custom text)
  - Quick Credential Presets (Valid / Wrong / New user)
  - Live Prime Generation ($\lightning$ Generate New RSA Primes)
  - One-click copy for network ciphertext
