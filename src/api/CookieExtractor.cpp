#include "CookieExtractor.hpp"
#include "../util/Platform.hpp"

#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cstring>
#include <filesystem>
#include <unordered_set>

namespace fs = std::filesystem;

namespace ymcli {

static bool isPrintableAscii(const std::string& str) {
    if (str.empty()) return false;
    for (unsigned char c : str) {
        if (c < 0x20 || c >= 0x7F) {
            return false;
        }
    }
    return true;
}

static std::string decryptChromeValue(const std::string& encrypted, const std::string& password) {
    if (encrypted.length() < 3) return "";
    
    std::string payload = encrypted;
    if (payload.starts_with("v10") || payload.starts_with("v11")) {
        payload = payload.substr(3);
    }

    unsigned char derived_key[16];
    const unsigned char salt[] = "saltysalt";
    PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()),
                      salt, 9, 1003, EVP_sha1(), 16, derived_key);

    unsigned char iv[16];
    std::memset(iv, ' ', 16);

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) return "";

    std::vector<unsigned char> decrypted(payload.size() + 16);
    int out_len1 = 0;
    int out_len2 = 0;

    if (EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), nullptr, derived_key, iv) &&
        EVP_DecryptUpdate(ctx, decrypted.data(), &out_len1,
                           reinterpret_cast<const unsigned char*>(payload.data()), static_cast<int>(payload.size())) &&
        EVP_DecryptFinal_ex(ctx, decrypted.data() + out_len1, &out_len2)) {
        
        EVP_CIPHER_CTX_free(ctx);
        int total_len = out_len1 + out_len2;
        if (total_len <= 0) return "";

        std::string dec_str(reinterpret_cast<char*>(decrypted.data()), total_len);
        if (!isPrintableAscii(dec_str)) {
            return "";
        }
        return dec_str;
    }

    EVP_CIPHER_CTX_free(ctx);
    return "";
}

std::string CookieExtractor::autoExtractCookies() {
    try {
        struct BrowserTarget {
            std::string name;
            std::string keychain_service;
            std::string db_rel_path;
        };

        std::vector<BrowserTarget> targets = {
            {"Arc", "Arc Safe Storage", "/Library/Application Support/Arc/User Data/Default/Cookies"},
            {"Chrome", "Chrome Safe Storage", "/Library/Application Support/Google/Chrome/Default/Cookies"},
            {"Brave", "Brave Safe Storage", "/Library/Application Support/BraveSoftware/Brave-Browser/Default/Cookies"}
        };

        static const std::unordered_set<std::string> valid_auth_cookie_names = {
            "SAPISID",
            "__Secure-3PAPISID",
            "__Secure-1PAPISID",
            "APISID",
            "SSID",
            "HSID",
            "SID",
            "LOGIN_INFO",
            "VISITOR_INFO1_LIVE"
        };

        const char* env_home = std::getenv("HOME");
        std::string home = env_home ? env_home : "";
        if (home.empty()) return "";

        for (const auto& target : targets) {
            std::error_code ec;
            std::string db_path = home + target.db_rel_path;
            if (!fs::exists(db_path, ec) || ec) continue;

            std::string cmd = "security find-generic-password -w -s \"" + target.keychain_service + "\" 2>/dev/null";
            std::string pass = "";
            
            std::array<char, 256> buffer;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
            if (pipe) {
                while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
                    pass += buffer.data();
                }
                while (!pass.empty() && (pass.back() == '\n' || pass.back() == '\r')) {
                    pass.pop_back();
                }
            }

            if (pass.empty()) continue;

            std::string tmp_db = "/tmp/ymcli_cookie_copy.db";
            fs::copy_file(db_path, tmp_db, fs::copy_options::overwrite_existing, ec);
            if (ec) continue;

            sqlite3* db = nullptr;
            if (sqlite3_open(tmp_db.c_str(), &db) != SQLITE_OK) {
                fs::remove(tmp_db, ec);
                continue;
            }

            const char* sql = "SELECT name, encrypted_value FROM cookies WHERE host_key LIKE '%youtube.com%'";
            sqlite3_stmt* stmt = nullptr;
            std::string result_cookies;

            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                bool found_sapisid = false;

                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                    const void* blob = sqlite3_column_blob(stmt, 1);
                    int blob_bytes = sqlite3_column_bytes(stmt, 1);

                    if (!name || !blob || blob_bytes == 0) continue;
                    std::string cookie_name = name;

                    // Only extract known text auth cookies
                    if (valid_auth_cookie_names.find(cookie_name) == valid_auth_cookie_names.end()) {
                        continue;
                    }

                    std::string enc_val(reinterpret_cast<const char*>(blob), blob_bytes);
                    std::string dec_val = decryptChromeValue(enc_val, pass);

                    if (!dec_val.empty()) {
                        if (!result_cookies.empty()) result_cookies += "; ";
                        result_cookies += cookie_name + "=" + dec_val;

                        if (cookie_name == "SAPISID" || cookie_name == "__Secure-3PAPISID") {
                            found_sapisid = true;
                        }
                    }
                }
                sqlite3_finalize(stmt);

                if (found_sapisid && !result_cookies.empty()) {
                    sqlite3_close(db);
                    fs::remove(tmp_db, ec);
                    return result_cookies;
                }
            }
            sqlite3_close(db);
            fs::remove(tmp_db, ec);
        }
    } catch (...) {}

    return "";
}

} // namespace ymcli
