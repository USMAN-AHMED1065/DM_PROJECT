#include <iostream>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;

// ============================================
// CONSTANTS & CONFIGURATION
// ============================================

const int MAX_PRIMES = 10000;
const int MAX_STEPS = 1000;
const int MAX_VOTERS = 100;
const int MAX_BLOCKS = 500;
const int MAX_STRING_LEN = 256;
const int MAX_FACTORS = 50;
const int MAX_CRT_EQUATIONS = 20;
const int MAX_PROBLEMS = 100;

// ============================================
// FILE HANDLING STRUCTURES
// ============================================

struct ProblemInput {
    char type[50];          // "PRIME_CHECK", "MODEXP", "RSA_ENCRYPT", etc.
    char data[500];         // Problem data
    bool isValid;
};

struct ProblemOutput {
    char problemType[50];
    char input[500];
    char answer[500];
    char steps[5000];       // Detailed step-by-step solution
    char timestamp[50];
};

struct FileProcessor {
    ProblemInput problems[MAX_PROBLEMS];
    ProblemOutput outputs[MAX_PROBLEMS];
    int problemCount;
    int outputCount;
};

// ============================================
// EXISTING DATA STRUCTURES
// ============================================

struct ComputationStep {
    char description[MAX_STRING_LEN];
    long long value;
    long long intermediate1;
    long long intermediate2;
    bool isActive;
};

struct PrimeList {
    long long primes[MAX_PRIMES];
    int count;
};

struct PrimeFactorization {
    long long factors[MAX_FACTORS];
    int exponents[MAX_FACTORS];
    int count;
};

struct ModExpResult {
    long long base;
    long long exponent;
    long long modulus;
    long long result;
    ComputationStep steps[MAX_STEPS];
    int stepCount;
    int bitOperations;
    int multiplyOperations;
    double timeComplexity;
};

struct ExtendedGCDResult {
    long long gcd;
    long long x;
    long long y;
    ComputationStep steps[MAX_STEPS];
    int stepCount;
};

struct CRTEquation {
    long long remainder;
    long long modulus;
};

struct CRTSystem {
    CRTEquation equations[MAX_CRT_EQUATIONS];
    int count;
    long long solution;
    bool hasSolution;
};

struct RSAKeys {
    long long p;
    long long q;
    long long n;
    long long phi;
    long long e;
    long long d;
    bool keysGenerated;
};

// Global state
RSAKeys globalRSA;
FileProcessor fileProc;

// ============================================
// UTILITY FUNCTIONS
// ============================================

void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void setColor(int color) {
    #ifdef _WIN32
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, color);
    #endif
}

void showTitle(const char* title) {
    setColor(11);
    cout << "\n╔════════════════════════════════════════════════════════╗\n";
    cout << "║  " << title;
    for (int i = strlen(title); i < 52; i++) cout << " ";
    cout << "║\n";
    cout << "╚════════════════════════════════════════════════════════╝\n\n";
    setColor(7);
}

void showStep(int num, const char* text, long long value = -1) {
    setColor(14);
    cout << "→ Step " << num << ": ";
    setColor(7);
    cout << text;
    if (value != -1) {
        setColor(10);
        cout << " = " << value;
        setColor(7);
    }
    cout << "\n";
}

void showSuccess(const char* message) {
    setColor(10);
    cout << "\n✓ SUCCESS: " << message << "\n\n";
    setColor(7);
}

void showError(const char* message) {
    setColor(12);
    cout << "\n✗ ERROR: " << message << "\n\n";
    setColor(7);
}

// ============================================
// CORE MATHEMATICS
// ============================================

bool isPrime(long long n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;
    
    for (long long i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

long long gcd(long long a, long long b) {
    while (b) {
        long long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

long long modPowSimple(long long base, long long exp, long long mod) {
    long long result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp % 2 == 1)
            result = (result * base) % mod;
        base = (base * base) % mod;
        exp /= 2;
    }
    return result;
}

void modularExpBinary(long long base, long long exp, long long mod, ModExpResult* result) {
    result->base = base;
    result->exponent = exp;
    result->modulus = mod;
    result->stepCount = 0;
    result->bitOperations = 0;
    result->multiplyOperations = 0;
    
    long long res = 1;
    base %= mod;
    
    // Convert exponent to binary
    char binary[65];
    int binLen = 0;
    long long tempExp = exp;
    
    if (tempExp == 0) {
        strcpy(binary, "0");
        binLen = 1;
    } else {
        while (tempExp > 0) {
            binary[binLen++] = '0' + (tempExp % 2);
            tempExp /= 2;
        }
        binary[binLen] = '\0';
        
        // Reverse
        for (int i = 0; i < binLen / 2; i++) {
            char temp = binary[i];
            binary[i] = binary[binLen - 1 - i];
            binary[binLen - 1 - i] = temp;
        }
    }
    
    sprintf(result->steps[result->stepCount].description, 
            "Exponent %lld in binary: %s", exp, binary);
    result->steps[result->stepCount].isActive = true;
    result->stepCount++;
    
    // Process each bit
    for (int i = 0; i < binLen; i++) {
        result->bitOperations++;
        
        res = (res * res) % mod;
        result->multiplyOperations++;
        
        sprintf(result->steps[result->stepCount].description,
                "Bit %d: Square result -> %lld", i, res);
        result->steps[result->stepCount].value = res;
        result->steps[result->stepCount].isActive = true;
        result->stepCount++;
        
        if (binary[i] == '1') {
            res = (res * base) % mod;
            result->multiplyOperations++;
            
            sprintf(result->steps[result->stepCount].description,
                    "Bit is 1: Multiply by base -> %lld", res);
            result->steps[result->stepCount].value = res;
            result->steps[result->stepCount].isActive = true;
            result->stepCount++;
        }
    }
    
    result->result = res;
    result->timeComplexity = log2(exp > 0 ? exp : 1) * 2;
}

void extendedGCD(long long a, long long b, ExtendedGCDResult* result) {
    result->stepCount = 0;
    
    long long old_r = a, r = b;
    long long old_s = 1, s = 0;
    long long old_t = 0, t = 1;
    
    while (r != 0) {
        long long quotient = old_r / r;
        
        sprintf(result->steps[result->stepCount].description, 
                "q=%lld, r=%lld, s=%lld, t=%lld", quotient, r, s, t);
        result->steps[result->stepCount].value = r;
        result->steps[result->stepCount].isActive = true;
        result->stepCount++;
        
        long long temp = r;
        r = old_r - quotient * r;
        old_r = temp;
        
        temp = s;
        s = old_s - quotient * s;
        old_s = temp;
        
        temp = t;
        t = old_t - quotient * t;
        old_t = temp;
    }
    
    result->gcd = old_r;
    result->x = old_s;
    result->y = old_t;
}

long long modInverseExtendedGCD(long long a, long long m) {
    ExtendedGCDResult egcdResult;
    extendedGCD(a, m, &egcdResult);
    
    if (egcdResult.gcd != 1) {
        return -1;
    }
    
    long long x = egcdResult.x;
    return (x % m + m) % m;
}

void primeFactorize(long long n, PrimeFactorization* result) {
    result->count = 0;
    long long temp = n;
    
    if (temp % 2 == 0) {
        result->factors[result->count] = 2;
        result->exponents[result->count] = 0;
        while (temp % 2 == 0) {
            result->exponents[result->count]++;
            temp /= 2;
        }
        result->count++;
    }
    
    for (long long i = 3; i * i <= temp; i += 2) {
        if (temp % i == 0) {
            result->factors[result->count] = i;
            result->exponents[result->count] = 0;
            while (temp % i == 0) {
                result->exponents[result->count]++;
                temp /= i;
            }
            result->count++;
        }
    }
    
    if (temp > 1) {
        result->factors[result->count] = temp;
        result->exponents[result->count] = 1;
        result->count++;
    }
}

long long eulerTotient(long long n) {
    PrimeFactorization factors;
    primeFactorize(n, &factors);
    
    long long result = n;
    for (int i = 0; i < factors.count; i++) {
        result -= result / factors.factors[i];
    }
    return result;
}

long long generateLargePrime(long long min, long long max) {
    for (int attempt = 0; attempt < 1000; attempt++) {
        long long candidate = min + rand() % (max - min);
        if (candidate % 2 == 0) candidate++;
        if (isPrime(candidate)) {
            return candidate;
        }
    }
    return -1;
}

void generateRSAKeys(RSAKeys* keys, int bitSize) {
    long long min = 1LL << (bitSize - 1);
    long long max = (1LL << bitSize) - 1;
    
    keys->p = generateLargePrime(min, max);
    do {
        keys->q = generateLargePrime(min, max);
    } while (keys->q == keys->p);
    
    keys->n = keys->p * keys->q;
    keys->phi = (keys->p - 1) * (keys->q - 1);
    keys->e = 65537;
    
    while (gcd(keys->e, keys->phi) != 1) {
        keys->e += 2;
    }
    
    keys->d = modInverseExtendedGCD(keys->e, keys->phi);
    keys->keysGenerated = true;
}

long long rsaEncrypt(long long message, RSAKeys* keys) {
    if (!keys->keysGenerated) return -1;
    return modPowSimple(message, keys->e, keys->n);
}

long long rsaDecrypt(long long ciphertext, RSAKeys* keys) {
    if (!keys->keysGenerated) return -1;
    return modPowSimple(ciphertext, keys->d, keys->n);
}

bool solveCRT(CRTSystem* system) {
    for (int i = 0; i < system->count; i++) {
        for (int j = i + 1; j < system->count; j++) {
            if (gcd(system->equations[i].modulus, system->equations[j].modulus) != 1) {
                system->hasSolution = false;
                return false;
            }
        }
    }
    
    long long M = 1;
    for (int i = 0; i < system->count; i++) {
        M *= system->equations[i].modulus;
    }
    
    long long x = 0;
    for (int i = 0; i < system->count; i++) {
        long long Mi = M / system->equations[i].modulus;
        long long yi = modInverseExtendedGCD(Mi, system->equations[i].modulus);
        
        x = (x + system->equations[i].remainder * Mi % M * yi % M) % M;
    }
    
    system->solution = (x % M + M) % M;
    system->hasSolution = true;
    return true;
}

// ============================================
// FILE HANDLING - INPUT PROCESSING
// ============================================

void initFileProcessor() {
    fileProc.problemCount = 0;
    fileProc.outputCount = 0;
}

bool readInputFile(const char* filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        showError("Could not open input file!");
        return false;
    }
    
    fileProc.problemCount = 0;
    string line;
    
    while (getline(file, line) && fileProc.problemCount < MAX_PROBLEMS) {
        if (line.empty() || line[0] == '#') continue; // Skip comments
        
        ProblemInput* prob = &fileProc.problems[fileProc.problemCount];
        
        // Parse format: TYPE: DATA
        size_t colonPos = line.find(':');
        if (colonPos == string::npos) continue;
        
        string type = line.substr(0, colonPos);
        string data = line.substr(colonPos + 1);
        
        // Trim whitespace
        while (!type.empty() && isspace(type[0])) type.erase(0, 1);
        while (!type.empty() && isspace(type.back())) type.pop_back();
        while (!data.empty() && isspace(data[0])) data.erase(0, 1);
        while (!data.empty() && isspace(data.back())) data.pop_back();
        
        strncpy(prob->type, type.c_str(), 49);
        prob->type[49] = '\0';
        strncpy(prob->data, data.c_str(), 499);
        prob->data[499] = '\0';
        prob->isValid = true;
        
        fileProc.problemCount++;
    }
    
    file.close();
    return fileProc.problemCount > 0;
}

void getCurrentTimestamp(char* buffer, int size) {
    time_t now = time(0);
    struct tm* timeinfo = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

// ============================================
// PROBLEM SOLVERS WITH DETAILED STEPS
// ============================================

void solvePrimeCheck(const char* data, ProblemOutput* output) {
    strcpy(output->problemType, "PRIME_CHECK");
    strcpy(output->input, data);
    
    long long n = atoll(data);
    
    char steps[5000] = "";
    char temp[500];
    
    sprintf(temp, "Problem: Check if %lld is prime\n\n", n);
    strcat(steps, temp);
    
    sprintf(temp, "Step 1: Check basic conditions\n");
    strcat(steps, temp);
    
    if (n <= 1) {
        sprintf(temp, "  - %lld <= 1, NOT PRIME\n", n);
        strcat(steps, temp);
        sprintf(output->answer, "NOT PRIME");
        strcpy(output->steps, steps);
        return;
    }
    
    if (n == 2) {
        sprintf(temp, "  - %lld is 2, the smallest prime\n", n);
        strcat(steps, temp);
        sprintf(output->answer, "PRIME");
        strcpy(output->steps, steps);
        return;
    }
    
    if (n % 2 == 0) {
        sprintf(temp, "  - %lld is even, NOT PRIME\n", n);
        strcat(steps, temp);
        sprintf(output->answer, "NOT PRIME");
        strcpy(output->steps, steps);
        return;
    }
    
    sprintf(temp, "\nStep 2: Check divisibility up to sqrt(%lld) = %lld\n", n, (long long)sqrt(n));
    strcat(steps, temp);
    
    bool isPrimeResult = true;
    for (long long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) {
            sprintf(temp, "  - %lld is divisible by %lld\n", n, i);
            strcat(steps, temp);
            sprintf(temp, "  - %lld = %lld × %lld\n", n, i, n/i);
            strcat(steps, temp);
            isPrimeResult = false;
            break;
        }
    }
    
    if (isPrimeResult) {
        sprintf(temp, "  - No divisors found\n\nConclusion: %lld is PRIME\n", n);
        strcat(steps, temp);
        sprintf(output->answer, "PRIME");
    } else {
        sprintf(temp, "\nConclusion: %lld is NOT PRIME (composite)\n", n);
        strcat(steps, temp);
        sprintf(output->answer, "NOT PRIME");
    }
    
    strcpy(output->steps, steps);
}

void solveModExp(const char* data, ProblemOutput* output) {
    strcpy(output->problemType, "MODULAR_EXPONENTIATION");
    strcpy(output->input, data);
    
    long long base, exp, mod;
    sscanf(data, "%lld %lld %lld", &base, &exp, &mod);
    
    ModExpResult result;
    modularExpBinary(base, exp, mod, &result);
    
    char steps[5000] = "";
    char temp[500];
    
    sprintf(temp, "Problem: Compute %lld^%lld mod %lld\n\n", base, exp, mod);
    strcat(steps, temp);
    
    sprintf(temp, "Algorithm: Binary Method (Left-to-Right)\n\n");
    strcat(steps, temp);
    
    for (int i = 0; i < result.stepCount && i < 50; i++) {
        if (result.steps[i].isActive) {
            sprintf(temp, "%s\n", result.steps[i].description);
            strcat(steps, temp);
        }
    }
    
    if (result.stepCount > 50) {
        sprintf(temp, "\n... (%d more steps)\n", result.stepCount - 50);
        strcat(steps, temp);
    }
    
    sprintf(temp, "\nComplexity Analysis:\n");
    strcat(steps, temp);
    sprintf(temp, "  - Bit operations: %d\n", result.bitOperations);
    strcat(steps, temp);
    sprintf(temp, "  - Multiply operations: %d\n", result.multiplyOperations);
    strcat(steps, temp);
    sprintf(temp, "  - Time complexity: O(log n)\n");
    strcat(steps, temp);
    
    sprintf(temp, "\nFinal Answer: %lld^%lld mod %lld = %lld\n", 
            base, exp, mod, result.result);
    strcat(steps, temp);
    
    sprintf(output->answer, "%lld", result.result);
    strcpy(output->steps, steps);
}

void solveGCD(const char* data, ProblemOutput* output) {
    strcpy(output->problemType, "GCD");
    strcpy(output->input, data);
    
    long long a, b;
    sscanf(data, "%lld %lld", &a, &b);
    
    char steps[5000] = "";
    char temp[500];
    
    sprintf(temp, "Problem: Find gcd(%lld, %lld)\n\n", a, b);
    strcat(steps, temp);
    
    sprintf(temp, "Algorithm: Euclidean Algorithm\n\n");
    strcat(steps, temp);
    
    long long orig_a = a, orig_b = b;
    int step = 1;
    
    while (b != 0) {
        long long quotient = a / b;
        long long remainder = a % b;
        
        sprintf(temp, "Step %d: %lld = %lld × %lld + %lld\n", 
                step, a, quotient, b, remainder);
        strcat(steps, temp);
        
        a = b;
        b = remainder;
        step++;
    }
    
    sprintf(temp, "\nFinal Answer: gcd(%lld, %lld) = %lld\n", 
            orig_a, orig_b, a);
    strcat(steps, temp);
    
    sprintf(output->answer, "%lld", a);
    strcpy(output->steps, steps);
}

void solveExtGCD(const char* data, ProblemOutput* output) {
    strcpy(output->problemType, "EXTENDED_GCD");
    strcpy(output->input, data);
    
    long long a, b;
    sscanf(data, "%lld %lld", &a, &b);
    
    ExtendedGCDResult result;
    extendedGCD(a, b, &result);
    
    char steps[5000] = "";
    char temp[500];
    
    sprintf(temp, "Problem: Find gcd(%lld, %lld) and Bezout coefficients\n\n", a, b);
    strcat(steps, temp);
    
    sprintf(temp, "Algorithm: Extended Euclidean Algorithm\n\n");
    strcat(steps, temp);
    
    for (int i = 0; i < result.stepCount && i < 30; i++) {
        if (result.steps[i].isActive) {
            sprintf(temp, "Step %d: %s\n", i+1, result.steps[i].description);
            strcat(steps, temp);
        }
    }
    
    sprintf(temp, "\nResult:\n");
    strcat(steps, temp);
    sprintf(temp, "  gcd(%lld, %lld) = %lld\n", a, b, result.gcd);
    strcat(steps, temp);
    sprintf(temp, "\nBezout's Identity:\n");
    strcat(steps, temp);
    sprintf(temp, "  %lld = %lld × (%lld) + %lld × (%lld)\n", 
            result.gcd, a, result.x, b, result.y);
    strcat(steps, temp);
    
    sprintf(temp, "\nVerification:\n");
    strcat(steps, temp);
    long long verify = a * result.x + b * result.y;
    sprintf(temp, "  %lld × %lld + %lld × %lld = %lld ✓\n", 
            a, result.x, b, result.y, verify);
    strcat(steps, temp);
    
    sprintf(output->answer, "gcd=%lld, x=%lld, y=%lld", 
            result.gcd, result.x, result.y);
    strcpy(output->steps, steps);
}

void solveFactorization(const char* data, ProblemOutput* output) {
    strcpy(output->problemType, "PRIME_FACTORIZATION");
    strcpy(output->input, data);
    
    long long n = atoll(data);
    
    PrimeFactorization factors;
    primeFactorize(n, &factors);
    
    char steps[5000] = "";
    char temp[500];
    
    sprintf(temp, "Problem: Find prime factorization of %lld\n\n", n);
    strcat(steps, temp);
    
    sprintf(temp, "Algorithm: Trial Division\n\n");
    strcat(steps, temp);
    
    sprintf(temp, "Step 1: Check for factor 2\n");
    strcat(steps, temp);
    
    long long temp_n = n;
    if (temp_n % 2 == 0) {
        int count = 0;
        while (temp_n % 2 == 0) {
            count++;
            temp_n /= 2;
        }
        sprintf(temp, "  - Found 2^%d\n", count);
        strcat(steps, temp);
    } else {
        sprintf(temp, "  - Not divisible by 2\n");
        strcat(steps, temp);
    }
    
    sprintf(temp, "\nStep 2: Check odd factors from 3 to sqrt(%lld)\n", n);
    strcat(steps, temp);
    
    sprintf(temp, "\nPrime Factorization:\n");
    strcat(steps, temp);
    sprintf(temp, "%lld = ", n);
    strcat(steps, temp);
    
    for (int i = 0; i < factors.count; i++) {
        if (i > 0) strcat(steps, " × ");
        sprintf(temp, "%lld", factors.factors[i]);
        strcat(steps, temp);
        if (factors.exponents[i] > 1) {
            sprintf(temp, "^%d", factors.exponents[i]);
            strcat(steps, temp);
        }
    }
    strcat(steps, "\n");
    
    sprintf(temp, "\nEuler's Totient φ(%lld) = %lld\n", n, eulerTotient(n));
    strcat(steps, temp);
    
    char answer[200] = "";
    for (int i = 0; i < factors.count; i++) {
        if (i > 0) strcat(answer, " × ");
        sprintf(temp, "%lld^%d", factors.factors[i], factors.exponents[i]);
        strcat(answer, temp);
    }
    
    strcpy(output->answer, answer);
    strcpy(output->steps, steps);
}

void solveRSAEncrypt(const char* data, ProblemOutput* output) {
    strcpy(output->problemType, "RSA_ENCRYPT");
    strcpy(output->input, data);
    
    long long message;
    int bitSize;
    sscanf(data, "%lld %d", &message, &bitSize);
    
    char steps[5000] = "";
    char temp[500];
    
    sprintf(temp, "Problem: Encrypt message %lld using RSA with %d-bit keys\n\n", 
            message, bitSize);
    strcat(steps, temp);
    
    // Generate keys
    RSAKeys keys;
    generateRSAKeys(&keys, bitSize);
    
    sprintf(temp, "Step 1: RSA Key Generation\n");
    strcat(steps, temp);
    sprintf(temp, "  - Prime p = %lld\n", keys.p);
    strcat(steps, temp);
    sprintf(temp, "  - Prime q = %lld\n", keys.q);
    strcat(steps, temp);
    sprintf(temp, "  - Modulus n = p × q = %lld\n", keys.n);
    strcat(steps, temp);
    sprintf(temp, "  - Euler φ(n) = (p-1)(q-1) = %lld\n", keys.phi);
    strcat(steps, temp);
    sprintf(temp, "  - Public exponent e = %lld\n", keys.e);
    strcat(steps, temp);
    sprintf(temp, "  - Private exponent d = %lld\n", keys.d);
    strcat(steps, temp);
    
    sprintf(temp, "\nStep 2: Encryption\n");
    strcat(steps, temp);
    sprintf(temp, "  Formula: ciphertext = message^e mod n\n");
    strcat(steps, temp);
    
    long long ciphertext = rsaEncrypt(message, &keys);
    
    sprintf(temp, "  ciphertext = %lld^%lld mod %lld\n", message, keys.e, keys.n);
    strcat(steps, temp);
    sprintf(temp, "  ciphertext = %lld\n", ciphertext);
    strcat(steps, temp);
    
    sprintf(temp, "\nPublic Key: (e=%lld, n=%lld)\n", keys.e, keys.n);
    strcat(steps, temp);
    sprintf(temp, "Encrypted Message: %lld\n", ciphertext);
    strcat(steps, temp);
    
    sprintf(output->answer, "%lld", ciphertext);
    strcpy(output->steps, steps);
}

void solveRSADecrypt(const char* data, ProblemOutput* output) {
    strcpy(output->problemType, "RSA_DECRYPT");
    strcpy(output->input, data);
    
    long long ciphertext, p, q, e;
    sscanf(data, "%lld %lld %lld %lld", &ciphertext, &p, &q, &e);
    
    char steps[5000] = "";
    char temp[500];
    
    sprintf(temp, "Problem: Decrypt ciphertext %lld using RSA\n\n", ciphertext);
    strcat(steps, temp);
    
    // Setup keys
    RSAKeys keys;
    keys.p = p;
    keys.q = q;
    keys.n = p * q;
    keys.phi = (p - 1) * (q - 1);
    keys.e = e;
    keys.d = modInverseExtendedGCD(e, keys.phi);
    keys.keysGenerated = true;
    
    sprintf(temp, "Given:\n");
    strcat(steps, temp);
    sprintf(temp, "  - Ciphertext = %lld\n", ciphertext);
    strcat(steps, temp);
    sprintf(temp, "  - Prime p = %lld\n", p);
    strcat(steps, temp);
    sprintf(temp, "  - Prime q = %lld\n", q);
    strcat(steps, temp);
    sprintf(temp, "  - Public exponent e = %lld\n", e);
    strcat(steps, temp);
    
    sprintf(temp, "\nStep 1: Compute private key\n");
    strcat(steps, temp);
    sprintf(temp, "  - n = p × q = %lld\n", keys.n);
    strcat(steps, temp);
    sprintf(temp, "  - φ(n) = (p-1)(q-1) = %lld\n", keys.phi);
    strcat(steps, temp);
    sprintf(temp, "  - d = e^(-1) mod φ(n) = %lld\n", keys.d);
    strcat(steps, temp);
    
    sprintf(temp, "\nStep 2: Decryption\n");
    strcat(steps, temp);
    sprintf(temp, "  Formula: plaintext = ciphertext^d mod n\n");
    strcat(steps, temp);
    
    long long plaintext = rsaDecrypt(ciphertext, &keys);
    
    sprintf(temp, "  plaintext = %lld^%lld mod %lld\n", ciphertext, keys.d, keys.n);
    strcat(steps, temp);
    sprintf(temp, "  plaintext = %lld\n", plaintext);
    strcat(steps, temp);
    
    sprintf(output->answer, "%lld", plaintext);
    strcpy(output->steps, steps);
}

void solveCRTProblem(const char* data, ProblemOutput* output) {
    strcpy(output->problemType, "CHINESE_REMAINDER_THEOREM");
    strcpy(output->input, data);
    
    char steps[5000] = "";
    char temp[500];
    
    sprintf(temp, "Problem: Solve system of congruences\n\n");
    strcat(steps, temp);
    
    CRTSystem system;
    
    // Parse input: count rem1 mod1 rem2 mod2 ...
    istringstream iss(data);
    iss >> system.count;
    
    for (int i = 0; i < system.count; i++) {
        iss >> system.equations[i].remainder >> system.equations[i].modulus;
    }
    
    sprintf(temp, "System:\n");
    strcat(steps, temp);
    for (int i = 0; i < system.count; i++) {
        sprintf(temp, "  x ≡ %lld (mod %lld)\n", 
                system.equations[i].remainder, 
                system.equations[i].modulus);
        strcat(steps, temp);
    }
    
    sprintf(temp, "\nStep 1: Check if moduli are pairwise coprime\n");
    strcat(steps, temp);
    
    bool allCoprime = true;
    for (int i = 0; i < system.count; i++) {
        for (int j = i + 1; j < system.count; j++) {
            long long g = gcd(system.equations[i].modulus, system.equations[j].modulus);
            sprintf(temp, "  gcd(%lld, %lld) = %lld\n", 
                    system.equations[i].modulus, 
                    system.equations[j].modulus, g);
            strcat(steps, temp);
            if (g != 1) allCoprime = false;
        }
    }
    
    if (!allCoprime) {
        sprintf(temp, "\nERROR: Moduli are not pairwise coprime. No unique solution.\n");
        strcat(steps, temp);
        strcpy(output->answer, "NO SOLUTION");
        strcpy(output->steps, steps);
        return;
    }
    
    sprintf(temp, "\nStep 2: Apply Chinese Remainder Theorem\n");
    strcat(steps, temp);
    
    if (solveCRT(&system)) {
        long long M = 1;
        for (int i = 0; i < system.count; i++) {
            M *= system.equations[i].modulus;
        }
        
        sprintf(temp, "  M = product of all moduli = %lld\n", M);
        strcat(steps, temp);
        
        sprintf(temp, "\nSolution: x ≡ %lld (mod %lld)\n", system.solution, M);
        strcat(steps, temp);
        
        sprintf(temp, "\nVerification:\n");
        strcat(steps, temp);
        for (int i = 0; i < system.count; i++) {
            long long check = system.solution % system.equations[i].modulus;
            sprintf(temp, "  %lld ≡ %lld (mod %lld) %s\n", 
                    system.solution, check, 
                    system.equations[i].modulus,
                    check == system.equations[i].remainder ? "✓" : "✗");
            strcat(steps, temp);
        }
        
        sprintf(output->answer, "%lld", system.solution);
    } else {
        strcpy(output->answer, "NO SOLUTION");
    }
    
    strcpy(output->steps, steps);
}

// ============================================
// FILE PROCESSING ENGINE
// ============================================

void processProblems() {
    fileProc.outputCount = 0;
    
    for (int i = 0; i < fileProc.problemCount; i++) {
        ProblemInput* prob = &fileProc.problems[i];
        ProblemOutput* out = &fileProc.outputs[fileProc.outputCount];
        
        getCurrentTimestamp(out->timestamp, 50);
        
        if (strcmp(prob->type, "PRIME_CHECK") == 0) {
            solvePrimeCheck(prob->data, out);
        }
        else if (strcmp(prob->type, "MODEXP") == 0) {
            solveModExp(prob->data, out);
        }
        else if (strcmp(prob->type, "GCD") == 0) {
            solveGCD(prob->data, out);
        }
        else if (strcmp(prob->type, "EXTGCD") == 0) {
            solveExtGCD(prob->data, out);
        }
        else if (strcmp(prob->type, "FACTORIZE") == 0) {
            solveFactorization(prob->data, out);
        }
        else if (strcmp(prob->type, "RSA_ENCRYPT") == 0) {
            solveRSAEncrypt(prob->data, out);
        }
        else if (strcmp(prob->type, "RSA_DECRYPT") == 0) {
            solveRSADecrypt(prob->data, out);
        }
        else if (strcmp(prob->type, "CRT") == 0) {
            solveCRTProblem(prob->data, out);
        }
        else {
            strcpy(out->problemType, "UNKNOWN");
            strcpy(out->answer, "ERROR: Unknown problem type");
            strcpy(out->steps, "This problem type is not supported.");
        }
        
        fileProc.outputCount++;
    }
}

bool writeOutputFile(const char* filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        showError("Could not create output file!");
        return false;
    }
    
    file << "╔═══════════════════════════════════════════════════════════════╗\n";
    file << "║     DISCRETE MATHEMATICS TOOLKIT - SOLUTION REPORT            ║\n";
    file << "╚═══════════════════════════════════════════════════════════════╝\n\n";
    file << "Generated: " << fileProc.outputs[0].timestamp << "\n";
    file << "Total Problems Solved: " << fileProc.outputCount << "\n\n";
    file << "═══════════════════════════════════════════════════════════════\n\n";
    
    for (int i = 0; i < fileProc.outputCount; i++) {
        ProblemOutput* out = &fileProc.outputs[i];
        
        file << "───────────────────────────────────────────────────────────────\n";
        file << "PROBLEM #" << (i + 1) << ": " << out->problemType << "\n";
        file << "───────────────────────────────────────────────────────────────\n\n";
        
        file << "INPUT: " << out->input << "\n\n";
        
        file << "SOLUTION STEPS:\n";
        file << "───────────────────────────────────────────────────────────────\n";
        file << out->steps << "\n";
        
        file << "───────────────────────────────────────────────────────────────\n";
        file << "FINAL ANSWER: " << out->answer << "\n";
        file << "───────────────────────────────────────────────────────────────\n\n\n";
    }
    
    file << "═══════════════════════════════════════════════════════════════\n";
    file << "                    END OF REPORT                              \n";
    file << "═══════════════════════════════════════════════════════════════\n";
    
    file.close();
    return true;
}

// ============================================
// MENU SYSTEM WITH FILE HANDLING
// ============================================

void fileHandlingMenu() {
    clearScreen();
    showTitle("FILE HANDLING - BATCH PROBLEM SOLVER");
    
    cout << "This feature reads problems from an input file and generates\n";
    cout << "a detailed solution report with step-by-step explanations.\n\n";
    
    cout << "Supported Problem Types:\n";
    cout << "  • PRIME_CHECK: Is n prime?\n";
    cout << "  • MODEXP: Modular exponentiation\n";
    cout << "  • GCD: Greatest common divisor\n";
    cout << "  • EXTGCD: Extended GCD with Bezout coefficients\n";
    cout << "  • FACTORIZE: Prime factorization\n";
    cout << "  • RSA_ENCRYPT: RSA encryption\n";
    cout << "  • RSA_DECRYPT: RSA decryption\n";
    cout << "  • CRT: Chinese Remainder Theorem\n\n";
    
    cout << "Options:\n";
    cout << "1. Process Input File (problems.txt)\n";
    cout << "2. Create Sample Input File\n";
    cout << "3. View Last Results\n";
    cout << "0. Back\n\n";
    cout << "Choice: ";
    
    int choice;
    cin >> choice;
    cin.ignore();
    
    switch (choice) {
        case 1: {
            cout << "\nEnter input filename [problems.txt]: ";
            char inputFile[256];
            cin.getline(inputFile, 256);
            if (strlen(inputFile) == 0) {
                strcpy(inputFile, "problems.txt");
            }
            
            cout << "Enter output filename [solutions.txt]: ";
            char outputFile[256];
            cin.getline(outputFile, 256);
            if (strlen(outputFile) == 0) {
                strcpy(outputFile, "solutions.txt");
            }
            
            showStep(1, "Reading input file...");
            if (!readInputFile(inputFile)) {
                showError("Failed to read input file!");
                break;
            }
            
            cout << "  Found " << fileProc.problemCount << " problems\n";
            
            showStep(2, "Processing problems...");
            processProblems();
            
            showStep(3, "Writing output file...");
            if (writeOutputFile(outputFile)) {
                showSuccess("Solution report generated!");
                cout << "Problems solved: " << fileProc.outputCount << "\n";
                cout << "Output file: " << outputFile << "\n";
            }
            break;
        }
        
        case 2: {
            ofstream sample("problems.txt");
            if (!sample.is_open()) {
                showError("Could not create sample file!");
                break;
            }
            
            sample << "# Discrete Mathematics Toolkit - Sample Input File\n";
            sample << "# Format: PROBLEM_TYPE: data\n";
            sample << "# Lines starting with # are comments\n\n";
            
            sample << "# Check if numbers are prime\n";
            sample << "PRIME_CHECK: 17\n";
            sample << "PRIME_CHECK: 100\n\n";
            
            sample << "# Modular exponentiation: base exponent modulus\n";
            sample << "MODEXP: 3 13 7\n";
            sample << "MODEXP: 2 10 13\n\n";
            
            sample << "# GCD: a b\n";
            sample << "GCD: 48 18\n";
            sample << "GCD: 1071 462\n\n";
            
            sample << "# Extended GCD: a b\n";
            sample << "EXTGCD: 240 46\n\n";
            
            sample << "# Prime factorization\n";
            sample << "FACTORIZE: 60\n";
            sample << "FACTORIZE: 1024\n\n";
            
            sample << "# RSA Encryption: message bitsize\n";
            sample << "RSA_ENCRYPT: 42 10\n\n";
            
            sample << "# Chinese Remainder Theorem: count rem1 mod1 rem2 mod2 ...\n";
            sample << "CRT: 2 2 3 3 5\n";
            
            sample.close();
            
            showSuccess("Sample input file created: problems.txt");
            cout << "You can edit this file and add your own problems.\n";
            break;
        }
        
        case 3: {
            if (fileProc.outputCount == 0) {
                showError("No results available. Process a file first.");
                break;
            }
            
            cout << "\n";
            showTitle("LAST PROCESSED RESULTS");
            cout << "Total problems: " << fileProc.outputCount << "\n\n";
            
            for (int i = 0; i < fileProc.outputCount; i++) {
                ProblemOutput* out = &fileProc.outputs[i];
                cout << "Problem " << (i+1) << ": " << out->problemType << "\n";
                cout << "  Input: " << out->input << "\n";
                cout << "  Answer: " << out->answer << "\n\n";
            }
            break;
        }
    }
}

void mainMenu() {
    initFileProcessor();
    globalRSA.keysGenerated = false;
    
    while (true) {
        clearScreen();
        
        setColor(11);
        cout << "\n╔══════════════════════════════════════════════════════════╗\n";
        cout << "║  DISCRETE MATHEMATICS TOOLKIT - FINAL COMPETITION EDITION║\n";
        cout << "║         Number Theory & Cryptography with FILE I/O       ║\n";
        cout << "╚══════════════════════════════════════════════════════════╝\n";
        setColor(7);
        
        cout << "\n┌────────────────────────────────────────────────────────┐\n";
        cout << "│ 🔥 FILE HANDLING - BATCH PROBLEM SOLVER                │\n";
        cout << "├────────────────────────────────────────────────────────┤\n";
        cout << "│  1. Process Problems from File (AUTO-SOLVER)          │\n";
        cout << "└────────────────────────────────────────────────────────┘\n\n";
        
        cout << "┌────────────────────────────────────────────────────────┐\n";
        cout << "│ ADDITIONAL FEATURES                                    │\n";
        cout << "├────────────────────────────────────────────────────────┤\n";
        cout << "│  2. Interactive Problem Solving                        │\n";
        cout << "│  3. View Documentation                                 │\n";
        cout << "│  0. Exit                                               │\n";
        cout << "└────────────────────────────────────────────────────────┘\n\n";
        
        setColor(14);
        cout << "Enter your choice: ";
        setColor(7);
        
        int choice;
        cin >> choice;
        
        switch (choice) {
            case 1:
                fileHandlingMenu();
                break;
            
            case 2:
                cout << "\nInteractive features available in full version.\n";
                cout << "Use File Handling (Option 1) for competition!\n";
                break;
            
            case 3:
                cout << "\nDocumentation:\n";
                cout << "- Input Format: PROBLEM_TYPE: data\n";
                cout << "- Output: Detailed step-by-step solutions\n";
                cout << "- Supports: Prime checks, ModExp, GCD, RSA, CRT, etc.\n";
                break;
            
            case 0:
                cout << "\n";
                showSuccess("Thank you for using Discrete Math Toolkit!");
                cout << "Project by: [Your Team Name]\n";
                cout << "Course: CSC101 - Discrete Mathematics\n\n";
                return;
            
            default:
                showError("Invalid choice!");
        }
        
        cout << "\n";
        setColor(8);
        cout << "Press ENTER to continue...";
        setColor(7);
        cin.ignore();
        cin.get();
    }
}

int main() {
    #ifdef _WIN32
        SetConsoleOutputCP(65001);
    #endif
    
    srand(time(0));
    mainMenu();
    
    return 0;
}
