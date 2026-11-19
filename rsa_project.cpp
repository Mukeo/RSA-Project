#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdlib>
#include <ctime>

using namespace std;

typedef long long ll;

// ---------- Colors (set to false if your terminal shows odd symbols) ----------
// UI colors removed for plain output
const bool USE_COLOR = false;
string col(const string& code) { return ""; }
const string RESET  = col("0");
const string BOLD   = col("1");
const string DIM    = col("2");
const string PLUM   = col("38;5;141");   // headings
const string GOLD   = col("38;5;215");   // numbers / keys
const string MINT   = col("38;5;79");    // success
const string ROSE   = col("38;5;210");   // warnings / secret

const ll MAX_N = 2000000000LL;           // keeps (n-1)^2 inside long long

// ============================================================
//  PART 1: RSA core (math functions)
// ============================================================

// GCD using Euclid's method
ll gcd(ll a, ll b) {
    while (b != 0) {
        ll r = a % b;
        a = b;
        b = r;
    }
    return a;
}

// Prime check by trial division
bool isPrime(ll x) {
    if (x < 2) return false;
    for (ll i = 2; i * i <= x; i++)
        if (x % i == 0) return false;
    return true;
}

// Modular inverse using the extended Euclidean algorithm.
// Returns x such that (a * x) % m == 1 (a and m must be co-prime).
ll modInverse(ll a, ll m) {
    ll m0 = m, x0 = 0, x1 = 1;
    if (m == 1) return 0;
    while (a > 1) {
        ll q = a / m;
        ll t = m;
        m = a % m;
        a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }
    if (x1 < 0) x1 += m0;   // C++ % and division can give negatives
    return x1;
}

// Fast modular exponentiation (square and multiply): (base ^ exp) % mod
// The mod is applied at every step so numbers never become huge.
ll modPow(ll base, ll exp, ll mod) {
    ll result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

// ============================================================
//  PART 2: Key generation
// ============================================================
struct KeyPair {
    ll p = 0, q = 0, n = 0, phi = 0, e = 0, d = 0;
    bool ready = false;
};

// Builds a key pair from p, q, e. Returns false and sets err if invalid.
bool makeKeys(KeyPair& k, ll p, ll q, ll e, string& err) {
    if (!isPrime(p) || !isPrime(q)) { err = "p and q must both be prime."; return false; }
    if (p == q)                     { err = "p and q must be different primes."; return false; }
    ll n = p * q;
    if (n <= 255)                   { err = "n = p*q must be greater than 255 (ASCII characters). Use bigger primes."; return false; }
    if (n > MAX_N)                  { err = "n is too large for this program (limit 2,000,000,000). Use smaller primes."; return false; }
    ll phi = (p - 1) * (q - 1);
    if (e <= 1 || e >= phi)         { err = "e must satisfy 1 < e < phi(n) = " + to_string(phi) + "."; return false; }
    if (gcd(e, phi) != 1)           { err = "e must be co-prime with phi(n) = " + to_string(phi) + " (gcd must be 1)."; return false; }

    k.p = p; k.q = q; k.n = n; k.phi = phi; k.e = e;
    k.d = modInverse(e, phi);
    k.ready = true;
    return true;
}

// Random prime between lo and hi
ll randomPrime(ll lo, ll hi) {
    while (true) {
        ll x = lo + rand() % (hi - lo + 1);
        if (isPrime(x)) return x;
    }
}

// Pick a suitable e for the given phi
ll pickE(ll phi) {
    ll candidates[] = {65537, 257, 17, 5, 3};
    for (ll c : candidates)
        if (c < phi && gcd(c, phi) == 1) return c;
    for (ll c = 7; c < phi; c += 2)
        if (gcd(c, phi) == 1) return c;
    return 3;
}

void autoGenerate(KeyPair& k) {
    string err;
    while (true) {
        ll p = randomPrime(100, 1000);
        ll q = randomPrime(100, 1000);
        if (p == q) continue;
        ll e = pickE((p - 1) * (q - 1));
        if (makeKeys(k, p, q, e, err)) return;
    }
}

// ============================================================
//  PART 3: Encrypt / decrypt text
// ============================================================

// Each character -> ASCII number M -> C = M^e mod n
vector<ll> encryptText(const string& text, const KeyPair& k) {
    vector<ll> out;
    for (unsigned char ch : text)
        out.push_back(modPow(ch, k.e, k.n));
    return out;
}

// Each ciphertext number C -> M = C^d mod n -> character
string decryptNumbers(const vector<ll>& cipher, const KeyPair& k) {
    string out;
    for (ll c : cipher)
        out += (char)modPow(c, k.d, k.n);
    return out;
}

string numbersToString(const vector<ll>& v) {
    string s;
    for (size_t i = 0; i < v.size(); i++) {
        if (i) s += " ";
        s += to_string(v[i]);
    }
    return s;
}

// ============================================================
//  PART 4: Input helpers
// ============================================================
bool readLine(string& s) {
    if (!getline(cin, s)) return false;
    return true;
}

bool readNumber(const string& prompt, ll& value) {
    while (true) {
        cout << prompt;
        string s;
        if (!readLine(s)) return false;
        istringstream iss(s);
        ll v; char extra;
        if (iss >> v && !(iss >> extra)) { value = v; return true; }
        cout << ROSE << "Please enter a whole number." << RESET << "\n";
    }
}

void heading(const string& t) {
    cout << "\n" << BOLD << PLUM << "== " << t << " ==" << RESET << "\n";
}

// ============================================================
//  PART 5: Menu actions
// ============================================================
void showKeys(const KeyPair& k, bool teaching) {
    heading("Keys");
    cout << "p = " << GOLD << k.p << RESET << ",  q = " << GOLD << k.q << RESET << "\n";
    cout << "n = p x q = " << GOLD << k.n << RESET << "\n";
    cout << "phi(n) = (p-1)(q-1) = " << GOLD << k.phi << RESET << "\n";
    cout << "e = " << GOLD << k.e << RESET << ",  d = " << GOLD << k.d << RESET << "\n";
    cout << MINT << "Public key  (e, n) = (" << k.e << ", " << k.n << ")" << RESET << "\n";
    if (teaching)
        cout << ROSE << "Private key (d, n) = (" << k.d << ", " << k.n << ")   [teaching view only]" << RESET << "\n";
    else
        cout << ROSE << "Private key (d, n) = (hidden)" << RESET << "\n";
}

void generateKeysMenu(KeyPair& k) {
    heading("Key generation");
    cout << "1. Enter p, q, e yourself\n2. Generate automatically\n";
    ll choice;
    if (!readNumber("Choose (1/2): ", choice)) return;

    if (choice == 2) {
        autoGenerate(k);
        cout << MINT << "Keys generated." << RESET << "\n";
        showKeys(k, true);
        return;
    }

    ll p, q, e;
    string err;
    if (!readNumber("Enter prime p: ", p)) return;
    if (!readNumber("Enter prime q: ", q)) return;
    if (isPrime(p) && isPrime(q) && p != q) {
        ll phi = (p - 1) * (q - 1);
        cout << DIM << "phi(n) = " << phi << ". Choose e with 1 < e < " << phi
             << " and gcd(e, " << phi << ") = 1." << RESET << "\n";
    }
    if (!readNumber("Enter public exponent e: ", e)) return;

    KeyPair temp;
    if (makeKeys(temp, p, q, e, err)) {
        k = temp;
        cout << MINT << "Keys generated." << RESET << "\n";
        showKeys(k, true);
    } else {
        cout << ROSE << "Error: " << err << RESET << "\n";
    }
}

void encryptMenu(const KeyPair& k) {
    heading("Encrypt a message");
    cout << "Enter message: ";
    string msg;
    if (!readLine(msg)) return;
    if (msg.empty()) { cout << ROSE << "Message is empty." << RESET << "\n"; return; }

    vector<ll> cipher = encryptText(msg, k);
    cout << "\n" << DIM << "Char  ASCII   C = M^e mod n" << RESET << "\n";
    for (size_t i = 0; i < msg.size(); i++) {
        unsigned char ch = msg[i];
        cout << "  " << (ch == ' ' ? '_' : (char)ch) << "   " << (int)ch
             << "      " << GOLD << cipher[i] << RESET << "\n";
    }
    cout << "\nCiphertext: " << GOLD << numbersToString(cipher) << RESET << "\n";
}

void decryptMenu(const KeyPair& k) {
    heading("Decrypt a ciphertext");
    cout << "Enter ciphertext numbers separated by spaces: ";
    string line;
    if (!readLine(line)) return;

    istringstream iss(line);
    vector<ll> cipher;
    ll x;
    while (iss >> x) {
        if (x < 0 || x >= k.n) {
            cout << ROSE << "Each number must be between 0 and n-1 (" << k.n - 1 << ")." << RESET << "\n";
            return;
        }
        cipher.push_back(x);
    }
    if (cipher.empty()) { cout << ROSE << "No numbers entered." << RESET << "\n"; return; }

    cout << "\n" << DIM << "C       M = C^d mod n   Char" << RESET << "\n";
    for (ll c : cipher) {
        ll m = modPow(c, k.d, k.n);
        cout << c << "      " << m << "              " << (char)m << "\n";
    }
    cout << "\nPlain text: " << MINT << decryptNumbers(cipher, k) << RESET << "\n";
}

// ---------- Secure login demo ----------
// The server owns the key pair. The client only knows the public key.
map<string, string> serverUsers;   // username -> password (plain text, demo only)

void clientServerFlow(const KeyPair& k, bool isRegister) {
    heading(isRegister ? "Register" : "Login");

    cout << BOLD << "[SERVER]" << RESET << " Public key shared with client: "
         << GOLD << "(" << k.e << ", " << k.n << ")" << RESET << "\n";

    string user, pass;
    cout << "\n" << BOLD << "[CLIENT]" << RESET << " Username: ";
    if (!readLine(user)) return;
    cout << BOLD << "[CLIENT]" << RESET << " Password: ";
    if (!readLine(pass)) return;
    if (user.empty() || pass.empty()) {
        cout << ROSE << "Username and password cannot be empty." << RESET << "\n";
        return;
    }

    vector<ll> cipher = encryptText(pass, k);
    cout << BOLD << "[CLIENT]" << RESET << " Encrypting password with the public key...\n";
    cout << BOLD << "[CLIENT]" << RESET << " Sent over the network: " << GOLD << numbersToString(cipher) << RESET << "\n";
    cout << DIM << "[ATTACKER] Sniffed the packet. Has only the public key, cannot decrypt." << RESET << "\n";

    cout << "\n" << BOLD << "[SERVER]" << RESET << " Ciphertext received: " << GOLD << numbersToString(cipher) << RESET << "\n";
    string received = decryptNumbers(cipher, k);
    cout << BOLD << "[SERVER]" << RESET << " Decrypting with the private key...\n";

    if (isRegister) {
        if (serverUsers.count(user)) {
            cout << BOLD << "[SERVER]" << RESET << ROSE << " Username already exists." << RESET << "\n";
            return;
        }
        serverUsers[user] = received;
        cout << BOLD << "[SERVER]" << RESET << " " << MINT << "Account created for '" << user << "'." << RESET << "\n";
    } else {
        auto it = serverUsers.find(user);
        if (it == serverUsers.end()) {
            cout << BOLD << "[SERVER]" << RESET << ROSE << " No such user. Login failed." << RESET << "\n";
        } else if (it->second == received) {
            cout << BOLD << "[SERVER]" << RESET << " Password matches the stored password.\n";
            cout << BOLD << "[CLIENT]" << RESET << " " << MINT << "Login successful." << RESET << "\n";
        } else {
            cout << BOLD << "[SERVER]" << RESET << " Password does not match.\n";
            cout << BOLD << "[CLIENT]" << RESET << " " << ROSE << "Login failed." << RESET << "\n";
        }
    }
}

// ============================================================
//  PART 6: Main menu
// ============================================================
int main() {
    srand((unsigned)time(NULL));
    KeyPair keys;

    cout << BOLD << PLUM << "\n=========================================\n"
         << "          RSA ENCRYPTION PROJECT\n"
         << "=========================================" << RESET << "\n";

    while (true) {
        cout << "\n" << BOLD << "Menu" << RESET << "\n"
             << "  1. Generate keys\n"
             << "  2. Show keys\n"
             << "  3. Encrypt a message\n"
             << "  4. Decrypt a ciphertext\n"
             << "  5. Secure login demo: register\n"
             << "  6. Secure login demo: login\n"
             << "  7. Exit\n";
        ll choice;
        if (!readNumber("Choose an option: ", choice)) break;

        if (choice == 7) {
            cout << "Goodbye.\n";
            break;
        }
        if (choice < 1 || choice > 7) {
            cout << ROSE << "Invalid option." << RESET << "\n";
            continue;
        }
        if (choice == 1) {
            generateKeysMenu(keys);
            continue;
        }
        if (!keys.ready) {
            cout << ROSE << "Generate keys first (option 1)." << RESET << "\n";
            continue;
        }

        switch (choice) {
            case 2: showKeys(keys, true);        break;
            case 3: encryptMenu(keys);           break;
            case 4: decryptMenu(keys);           break;
            case 5: clientServerFlow(keys, true);  break;
            case 6: clientServerFlow(keys, false); break;
        }
    }
    return 0;
}
