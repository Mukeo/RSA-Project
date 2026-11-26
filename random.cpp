#include <random>

bool isPrime(ull x) {
    if (x < 2) return false;
    for (ull i = 2; i * i <= x; i++)
        if (x % i == 0) return false;
    return true;
}

ull randomPrime(ull lo, ull hi) {
    static mt19937_64 rng(random_device{}());
    while (true) {
        ull x = lo + rng() % (hi - lo + 1);
        if (isPrime(x)) return x;
    }
}

// usage
ull p = randomPrime(100, 1000);
ull q = randomPrime(100, 1000);
while (q == p) q = randomPrime(100, 1000);
