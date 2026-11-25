#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <map>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <unordered_map>

using namespace std;

typedef long long ll;

// ================= RSA core =================
ll gcd(ll a, ll b) { while (b) { ll r = a % b; a = b; b = r; } return a; }
bool isPrime(ll x) { if (x < 2) return false; for (ll i = 2; i * i <= x; ++i) if (x % i == 0) return false; return true; }
ll modInverse(ll a, ll m) { ll m0 = m, x0 = 0, x1 = 1; if (m == 1) return 0; while (a > 1) { ll q = a / m; ll t = m; m = a % m; a = t; t = x0; x0 = x1 - q * x0; x1 = t; } if (x1 < 0) x1 += m0; return x1; }
ll modPow(ll base, ll exp, ll mod) { ll res = 1; base %= mod; while (exp) { if (exp & 1) res = (res * base) % mod; base = (base * base) % mod; exp >>= 1; } return res; }

struct KeyPair {
    ll p = 61;
    ll q = 53;
    ll n = 3233;
    ll phi = 3120;
    ll e = 17;
    ll d = 2753;
    bool ready = true;
};

static KeyPair globalKey;
static unordered_map<string, string> userStore = {
    {"Praveen", "SRM123"}
};

bool makeKeys(KeyPair &k, ll p, ll q, ll e, string &err) {
    if (!isPrime(p) || !isPrime(q)) { err = "p and q must both be prime."; return false; }
    if (p == q) { err = "p and q must be different primes."; return false; }
    ll n = p * q; if (n <= 255) { err = "n must be >255"; return false; }
    ll phi = (p - 1) * (q - 1);
    if (e <= 1 || e >= phi) { err = "e out of range"; return false; }
    if (gcd(e, phi) != 1) { err = "e not coprime"; return false; }
    k.p = p; k.q = q; k.n = n; k.phi = phi; k.e = e; k.d = modInverse(e, phi); k.ready = true; return true;
}

ll randomPrime(ll lo, ll hi) { while (true) { ll x = lo + rand() % (hi - lo + 1); if (isPrime(x)) return x; } }
ll pickE(ll phi) { ll candidates[] = {65537, 257, 17, 5, 3}; for (ll c : candidates) if (c < phi && gcd(c, phi) == 1) return c; for (ll c = 7; c < phi; c += 2) if (gcd(c, phi) == 1) return c; return 3; }
void autoGenerate(KeyPair &k) { string err; while (true) { ll p = randomPrime(50, 200); ll q = randomPrime(50, 200); if (p == q) continue; ll e = pickE((p - 1) * (q - 1)); if (makeKeys(k, p, q, e, err)) return; } }

vector<ll> encryptText(const string &txt, const KeyPair &k) { vector<ll> out; for (unsigned char ch : txt) out.push_back(modPow(ch, k.e, k.n)); return out; }
string numbersToString(const vector<ll> &v) { ostringstream oss; for (size_t i = 0; i < v.size(); ++i) { if (i) oss << ' '; oss << v[i]; } return oss.str(); }
string decryptNumbers(const vector<ll> &cipher, const KeyPair &k) { string out; for (ll c : cipher) out += static_cast<char>(modPow(c, k.d, k.n)); return out; }

// ================= Simple HTTP server =================
const int PORT = 9090;

string readFile(const string &path) {
    ifstream in(path, ios::binary);
    if (!in) return "";
    stringstream ss; ss << in.rdbuf(); return ss.str();
}

void sendResponse(int client, const string &status, const string &contentType, const string &body) {
    ostringstream resp;
    resp << "HTTP/1.1 " << status << "\r\n"
         << "Content-Type: " << contentType << "; charset=utf-8\r\n"
         << "Access-Control-Allow-Origin: *\r\n"
         << "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
         << "Access-Control-Allow-Headers: *\r\n"
         << "Content-Length: " << body.size() << "\r\n"
         << "Connection: close\r\n\r\n"
         << body;
    string s = resp.str();
    write(client, s.c_str(), s.size());
}

string urlDecode(const string &src) {
    string ret;
    for (size_t i = 0; i < src.length(); ++i) {
        if (src[i] == '%' && i + 2 < src.length()) {
            int ii = 0;
            stringstream ss;
            ss << hex << src.substr(i + 1, 2);
            ss >> ii;
            ret += static_cast<char>(ii);
            i += 2;
        } else if (src[i] == '+') {
            ret += ' ';
        } else {
            ret += src[i];
        }
    }
    return ret;
}

map<string, string> parseQuery(const string &qs) {
    map<string, string> m; string key, val; bool readingKey = true;
    for (size_t i = 0; i < qs.size(); ++i) {
        char c = qs[i];
        if (c == '=') { readingKey = false; }
        else if (c == '&') { m[key] = urlDecode(val); key.clear(); val.clear(); readingKey = true; }
        else { if (readingKey) key += c; else val += c; }
    }
    if (!key.empty()) m[key] = urlDecode(val);
    return m;
}

string escapeJson(const string &s) {
    ostringstream o;
    for (char c : s) {
        if (c == '"') o << "\\\"";
        else if (c == '\\') o << "\\\\";
        else if (c == '\b') o << "\\b";
        else if (c == '\f') o << "\\f";
        else if (c == '\n') o << "\\n";
        else if (c == '\r') o << "\\r";
        else if (c == '\t') o << "\\t";
        else if (static_cast<unsigned char>(c) < 32) {
            o << "\\u00" << hex << (int)c;
        } else {
            o << c;
        }
    }
    return o.str();
}

int main() {
    srand((unsigned)time(NULL));
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) { cerr << "socket error" << endl; return 1; }
    int opt = 1; setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_addr.s_addr = INADDR_ANY; addr.sin_port = htons(PORT);
    if (::bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { cerr << "bind error" << endl; return 1; }
    if (listen(server_fd, 5) < 0) { cerr << "listen error" << endl; return 1; }
    cout << "Server listening on http://localhost:" << PORT << "/" << endl;

    while (true) {
        sockaddr_in clientAddr{}; socklen_t clLen = sizeof(clientAddr);
        int client = accept(server_fd, (sockaddr*)&clientAddr, &clLen);
        if (client < 0) { cerr << "accept error" << endl; continue; }
        char buffer[4096]; ssize_t bytes = read(client, buffer, sizeof(buffer) - 1);
        if (bytes <= 0) { close(client); continue; }
        buffer[bytes] = '\0';
        string req(buffer);
        size_t pos = req.find("\r\n");
        string requestLine = req.substr(0, pos);
        istringstream rl(requestLine);
        string method, url, version; rl >> method >> url >> version;
        string path = url; string query;
        size_t qpos = url.find('?');
        if (qpos != string::npos) { path = url.substr(0, qpos); query = url.substr(qpos + 1); }

        auto params = parseQuery(query);

        if (path == "/" || path == "/index.html") {
            string body = readFile("index.html");
            if (body.empty()) body = "<h1>RSA Demo Server</h1>";
            sendResponse(client, "200 OK", "text/html", body);
        } else if (path == "/style.css") {
            string body = readFile("style.css");
            sendResponse(client, "200 OK", "text/css", body);
        } else if (path == "/api/key") {
            ostringstream ss;
            ss << "{\"e\":" << globalKey.e << ",\"n\":" << globalKey.n << ",\"p\":" << globalKey.p << ",\"q\":" << globalKey.q << ",\"phi\":" << globalKey.phi << ",\"d\":" << globalKey.d << "}";
            sendResponse(client, "200 OK", "application/json", ss.str());
        } else if (path == "/api/generate" || path == "/generate") {
            autoGenerate(globalKey);
            ostringstream ss;
            ss << "{\"status\":\"ok\",\"p\":" << globalKey.p << ",\"q\":" << globalKey.q
               << ",\"n\":" << globalKey.n << ",\"phi\":" << globalKey.phi
               << ",\"e\":" << globalKey.e << ",\"d\":" << globalKey.d << "}";
            sendResponse(client, "200 OK", "application/json", ss.str());
        } else if (path == "/api/reset") {
            globalKey.p = 61;
            globalKey.q = 53;
            globalKey.n = 3233;
            globalKey.phi = 3120;
            globalKey.e = 17;
            globalKey.d = 2753;
            globalKey.ready = true;
            userStore["Praveen"] = "SRM123";
            sendResponse(client, "200 OK", "application/json", "{\"status\":\"reset_ok\"}");
        } else if (path == "/api/users") {
            ostringstream ss;
            ss << "[";
            size_t idx = 0;
            for (auto const &[u, p] : userStore) {
                if (idx > 0) ss << ",";
                ss << "\"" << escapeJson(u) << "\"";
                idx++;
            }
            ss << "]";
            sendResponse(client, "200 OK", "application/json", ss.str());
        } else if (path == "/api/auth" || path == "/login_demo") {
            string user = params.count("user") && !params["user"].empty() ? params["user"] : "Praveen";
            string pass = params.count("pass") && !params["pass"].empty() ? params["pass"] : "SRM123";
            string action = params.count("action") && !params["action"].empty() ? params["action"] : "login";

            if (action == "register") {
                userStore[user] = pass;
            }

            // Step 2: Encrypt each char
            vector<ll> cipher;
            ostringstream encStepsJson;
            encStepsJson << "[";
            for (size_t i = 0; i < pass.size(); ++i) {
                unsigned char ch = (unsigned char)pass[i];
                ll c = modPow(ch, globalKey.e, globalKey.n);
                cipher.push_back(c);
                if (i > 0) encStepsJson << ",";
                encStepsJson << "{\"char\":\"" << escapeJson(string(1, ch)) << "\",\"ascii\":" << (int)ch << ",\"cipher\":" << c << "}";
            }
            encStepsJson << "]";

            // Step 3: Ciphertext string
            string cipherStr = numbersToString(cipher);

            // Step 5: Decrypt
            ostringstream decStepsJson;
            decStepsJson << "[";
            string decryptedStr;
            for (size_t i = 0; i < cipher.size(); ++i) {
                ll c = cipher[i];
                ll m = modPow(c, globalKey.d, globalKey.n);
                decryptedStr += static_cast<char>(m);
                if (i > 0) decStepsJson << ",";
                decStepsJson << "{\"step\":" << (i + 1) << ",\"cipher\":" << c << ",\"decrypted\":" << m << ",\"char\":\"" << escapeJson(string(1, (char)m)) << "\"}";
            }
            decStepsJson << "]";

            // Step 6: Verification
            bool userExists = (userStore.find(user) != userStore.end());
            string stored = userExists ? userStore[user] : "";
            bool match = userExists && (stored == decryptedStr);
            string resultMsg;

            if (action == "register") {
                resultMsg = "Registration & Login successful";
                match = true;
            } else {
                if (!userExists) {
                    resultMsg = "User not found (Register first)";
                    match = false;
                } else if (match) {
                    resultMsg = "Login successful";
                } else {
                    resultMsg = "Invalid password";
                    match = false;
                }
            }

            ll ed = globalKey.e * globalKey.d;
            ll edModPhi = (globalKey.phi > 0) ? (ed % globalKey.phi) : 1;

            ostringstream resp;
            resp << "{"
                 << "\"status\":\"" << (match ? "success" : "fail") << "\","
                 << "\"message\":\"" << escapeJson(resultMsg) << "\","
                 << "\"action\":\"" << escapeJson(action) << "\","
                 << "\"user\":\"" << escapeJson(user) << "\","
                 << "\"pass\":\"" << escapeJson(pass) << "\","
                 << "\"key\":{"
                 << "\"p\":" << globalKey.p << ","
                 << "\"q\":" << globalKey.q << ","
                 << "\"n\":" << globalKey.n << ","
                 << "\"phi\":" << globalKey.phi << ","
                 << "\"e\":" << globalKey.e << ","
                 << "\"d\":" << globalKey.d << ","
                 << "\"ed\":" << ed << ","
                 << "\"edModPhi\":" << edModPhi
                 << "},"
                 << "\"encryptionSteps\":" << encStepsJson.str() << ","
                 << "\"ciphertextStr\":\"" << escapeJson(cipherStr) << "\","
                 << "\"decryptionSteps\":" << decStepsJson.str() << ","
                 << "\"decryptedPass\":\"" << escapeJson(decryptedStr) << "\","
                 << "\"storedPass\":\"" << escapeJson(stored) << "\","
                 << "\"match\":" << (match ? "true" : "false")
                 << "}";

            sendResponse(client, "200 OK", "application/json", resp.str());
        } else if (path == "/api/encrypt" || path == "/encrypt") {
            string msg = params.count("msg") ? params["msg"] : "";
            auto cipher = encryptText(msg, globalKey);
            string body = numbersToString(cipher);
            if (path == "/api/encrypt") {
                ostringstream ss;
                ss << "{\"plaintext\":\"" << escapeJson(msg) << "\",\"ciphertext\":\"" << escapeJson(body) << "\"}";
                sendResponse(client, "200 OK", "application/json", ss.str());
            } else {
                sendResponse(client, "200 OK", "text/plain", body);
            }
        } else if (path == "/api/decrypt" || path == "/decrypt") {
            string txt = params.count("txt") ? params["txt"] : "";
            vector<ll> nums; istringstream iss(txt); ll v;
            while (iss >> v) nums.push_back(v);
            string plain = decryptNumbers(nums, globalKey);
            if (path == "/api/decrypt") {
                ostringstream ss;
                ss << "{\"ciphertext\":\"" << escapeJson(txt) << "\",\"plaintext\":\"" << escapeJson(plain) << "\"}";
                sendResponse(client, "200 OK", "application/json", ss.str());
            } else {
                sendResponse(client, "200 OK", "text/plain", plain);
            }
        } else if (path == "/register") {
            string user = params["user"];
            string pass = params["pass"];
            if (user.empty() || pass.empty()) {
                sendResponse(client, "400 Bad Request", "text/plain", "Missing user or pass");
            } else if (userStore.count(user)) {
                sendResponse(client, "200 OK", "text/plain", "Username already exists");
            } else {
                userStore[user] = pass;
                sendResponse(client, "200 OK", "text/plain", "Registered successfully");
            }
        } else if (path == "/login") {
            string user = params["user"];
            string pass = params["pass"];
            if (user.empty() || pass.empty()) {
                sendResponse(client, "400 Bad Request", "text/plain", "Missing user or pass");
            } else {
                auto it = userStore.find(user);
                if (it == userStore.end()) {
                    sendResponse(client, "200 OK", "text/plain", "User not found");
                } else if (it->second == pass) {
                    sendResponse(client, "200 OK", "text/plain", "Login successful");
                } else {
                    sendResponse(client, "200 OK", "text/plain", "Invalid password");
                }
            }
        } else {
            sendResponse(client, "404 Not Found", "text/plain", "Not found");
        }
        close(client);
    }
    return 0;
}
