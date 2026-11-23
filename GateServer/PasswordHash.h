#pragma once
#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>

namespace PasswordHash {

    constexpr int    kIterations = 100000;
    constexpr size_t kSaltBytes = 16;
    constexpr size_t kHashBytes = 32;
    constexpr const char* kPrefix = "pbkdf2_sha256$";

    inline std::string HexEncode(const unsigned char* data, size_t len) {
        static const char* digits = "0123456789abcdef";
        std::string out;
        out.reserve(len * 2);
        for (size_t i = 0; i < len; ++i) {
            out.push_back(digits[data[i] >> 4]);
            out.push_back(digits[data[i] & 0x0F]);
        }
        return out;
    }

    inline bool HexDecode(const std::string& hex, std::vector<unsigned char>& out) {
        if (hex.empty() || hex.size() % 2 != 0) return false;
        auto val = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
            return -1;
        };
        out.clear();
        out.reserve(hex.size() / 2);
        for (size_t i = 0; i < hex.size(); i += 2) {
            int hi = val(hex[i]), lo = val(hex[i + 1]);
            if (hi < 0 || lo < 0) return false;
            out.push_back(static_cast<unsigned char>((hi << 4) | lo));
        }
        return true;
    }

    inline bool Pbkdf2(const std::string& password,
        const std::vector<unsigned char>& salt,
        int iterations,
        std::vector<unsigned char>& out) {
        out.resize(kHashBytes);
        int rc = PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()),
            salt.data(), static_cast<int>(salt.size()),
            iterations, EVP_sha256(),
            static_cast<int>(kHashBytes), out.data());
        return rc == 1;
    }

    inline bool ConstantTimeEqual(const std::vector<unsigned char>& a,
        const std::vector<unsigned char>& b) {
        if (a.size() != b.size()) return false;
        unsigned char diff = 0;
        for (size_t i = 0; i < a.size(); ++i) diff |= static_cast<unsigned char>(a[i] ^ b[i]);
        return diff == 0;
    }

    inline bool ConstantTimeStringEqual(const std::string& a, const std::string& b) {
        if (a.size() != b.size()) return false;
        unsigned char diff = 0;
        for (size_t i = 0; i < a.size(); ++i)
            diff |= static_cast<unsigned char>(a[i] ^ b[i]);
        return diff == 0;
    }

    // 返回 "pbkdf2_sha256$<iter>$<salt_hex>$<hash_hex>"；失败返回空串
    inline std::string HashPassword(const std::string& password) {
        std::vector<unsigned char> salt(kSaltBytes);
        if (RAND_bytes(salt.data(), static_cast<int>(salt.size())) != 1) return "";

        std::vector<unsigned char> hash;
        if (!Pbkdf2(password, salt, kIterations, hash)) return "";

        return std::string(kPrefix) + std::to_string(kIterations) + "$" +
            HexEncode(salt.data(), salt.size()) + "$" +
            HexEncode(hash.data(), hash.size());
    }

    inline bool IsHashed(const std::string& stored) {
        static const std::string prefix = kPrefix;
        return stored.compare(0, prefix.size(), prefix) == 0;
    }

    inline bool VerifyPassword(const std::string& password, const std::string& stored) {
        static const std::string prefix = kPrefix;
        if (stored.compare(0, prefix.size(), prefix) != 0) return false;

        size_t p1 = stored.find('$', prefix.size());
        if (p1 == std::string::npos) return false;
        size_t p2 = stored.find('$', p1 + 1);
        if (p2 == std::string::npos) return false;

        int iterations = 0;
        try {
            iterations = std::stoi(stored.substr(prefix.size(), p1 - prefix.size()));
        }
        catch (...) {
            return false;
        }
        if (iterations <= 0 || iterations > 10000000) return false;

        std::vector<unsigned char> salt, expected;
        if (!HexDecode(stored.substr(p1 + 1, p2 - p1 - 1), salt)) return false;
        if (!HexDecode(stored.substr(p2 + 1), expected)) return false;

        std::vector<unsigned char> actual;
        if (!Pbkdf2(password, salt, iterations, actual)) return false;
        return ConstantTimeEqual(actual, expected);
    }

    // 旧格式：客户端原来那套逐字符异或，密钥 = 长度 % 255（仅用于惰性迁移校验）
    inline std::string LegacyXor(const std::string& input) {
        std::string result = input;
        size_t len = result.size() % 255;
        for (size_t i = 0; i < len; ++i) {
            result[i] = static_cast<char>(
                static_cast<unsigned char>(result[i]) ^ static_cast<unsigned char>(len));
        }
        return result;
    }

} // namespace PasswordHash