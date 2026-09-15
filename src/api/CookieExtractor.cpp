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
#include <unistd.h>

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
        
        // Modern Chromium on macOS prefixes decrypted cookies with a 32-byte HMAC signature
        if (total_len > 32) {
            std::string dec_str(reinterpret_cast<char*>(decrypted.data() + 32), total_len - 32);
            if (isPrintableAscii(dec_str)) {
                return dec_str;
            }
        }

        // Fallback for earlier Chromium versions without 32-byte prefix
        if (total_len > 0) {
            std::string dec_str(reinterpret_cast<char*>(decrypted.data()), total_len);
            if (isPrintableAscii(dec_str)) {
                return dec_str;
            }
        }
        return "";
    }

    EVP_CIPHER_CTX_free(ctx);
    return "";
}

std::string CookieExtractor::autoExtractCookies(std::string* out_browser) {
    try {
        static const std::unordered_set<std::string> valid_auth_cookie_names = {
            "SAPISID",
            "__Secure-3PAPISID",
            "__Secure-1PAPISID",
            "APISID",
            "SSID",
            "HSID",
            "SID",
            "__Secure-1PSID",
            "__Secure-3PSID",
            "__Secure-1PSIDTS",
            "__Secure-3PSIDTS",
            "SIDCC",
            "__Secure-1PSIDCC",
            "__Secure-3PSIDCC",
            "LOGIN_INFO",
            "VISITOR_INFO1_LIVE",
            "PREF",
            "YSC"
        };

        const char* env_home = std::getenv("HOME");
        std::string home = env_home ? env_home : "";
        if (home.empty()) return "";

        std::error_code ec;
        std::string pid_str = std::to_string(getpid());

        // 1. Check Gecko-based browsers first (Firefox, Zen, LibreWolf, Waterfox)
        struct GeckoTarget {
            std::string name;
            std::string path;
        };
        std::vector<GeckoTarget> gecko_targets = {
            {"Firefox", home + "/Library/Application Support/Firefox/Profiles"},
            {"Zen", home + "/Library/Application Support/zen/Profiles"},
            {"LibreWolf", home + "/Library/Application Support/librewolf/Profiles"},
            {"Waterfox", home + "/Library/Application Support/Waterfox/Profiles"},
            {"Firefox (Linux)", home + "/.mozilla/firefox"}
        };

        for (const auto& target : gecko_targets) {
            if (!fs::exists(target.path, ec) || ec) continue;

            for (const auto& entry : fs::directory_iterator(target.path, ec)) {
                if (!entry.is_directory()) continue;
                std::string cookie_db = entry.path().string() + "/cookies.sqlite";
                if (!fs::exists(cookie_db, ec) || ec) continue;

                std::string tmp_db = "/tmp/ymcli_gecko_" + pid_str + ".db";
                std::string tmp_wal = tmp_db + "-wal";
                fs::copy_file(cookie_db, tmp_db, fs::copy_options::overwrite_existing, ec);
                if (ec) continue;

                std::string cookie_wal = cookie_db + "-wal";
                if (fs::exists(cookie_wal, ec)) {
                    fs::copy_file(cookie_wal, tmp_wal, fs::copy_options::overwrite_existing, ec);
                }

                sqlite3* db = nullptr;
                if (sqlite3_open_v2(tmp_db.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK) {
                    sqlite3_stmt* stmt = nullptr;
                    const char* sql = "SELECT name, value, host FROM moz_cookies WHERE host LIKE '%youtube.com%' OR host LIKE '%google.com%'";
                    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                        std::unordered_map<std::string, std::string> cookies_map;
                        bool found_auth = false;

                        while (sqlite3_step(stmt) == SQLITE_ROW) {
                            const char* name_p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                            const char* val_p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                            const char* host_p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                            if (!name_p || !val_p) continue;
                            std::string name = name_p;
                            std::string val = val_p;
                            std::string host = host_p ? host_p : "";
                            if (valid_auth_cookie_names.find(name) != valid_auth_cookie_names.end() && isPrintableAscii(val)) {
                                bool is_yt = (host.find("youtube.com") != std::string::npos);
                                if (is_yt || cookies_map.find(name) == cookies_map.end()) {
                                    cookies_map[name] = val;
                                }
                                if (name == "SAPISID" || name == "__Secure-3PAPISID") {
                                    found_auth = true;
                                }
                            }
                        }
                        sqlite3_finalize(stmt);

                        if (found_auth && !cookies_map.empty()) {
                            std::string result_cookies;
                            for (const auto& [name, val] : cookies_map) {
                                if (!result_cookies.empty()) result_cookies += "; ";
                                result_cookies += name + "=" + val;
                            }
                            sqlite3_close(db);
                            fs::remove(tmp_db, ec);
                            fs::remove(tmp_wal, ec);
                            if (out_browser) *out_browser = target.name;
                            return result_cookies;
                        }
                    }
                    sqlite3_close(db);
                }
                fs::remove(tmp_db, ec);
                fs::remove(tmp_wal, ec);
            }
        }

        // 2. Check Chromium-based browsers (Arc, Google Chrome, Brave, Comet, Edge, Chromium)
        struct ChromeTarget {
            std::string name;
            std::string keychain;
            std::string dir;
        };
        std::vector<ChromeTarget> chrome_targets = {
            {"Arc", "Arc Safe Storage", home + "/Library/Application Support/Arc/User Data/Default"},
            {"Google Chrome", "Chrome Safe Storage", home + "/Library/Application Support/Google/Chrome/Default"},
            {"Brave", "Brave Safe Storage", home + "/Library/Application Support/BraveSoftware/Brave-Browser/Default"},
            {"Comet", "Comet Safe Storage", home + "/Library/Application Support/Comet/Default"},
            {"Microsoft Edge", "Microsoft Edge Safe Storage", home + "/Library/Application Support/Microsoft Edge/Default"},
            {"Chromium", "Chromium Safe Storage", home + "/Library/Application Support/Chromium/Default"},
            {"Vivaldi", "Vivaldi Safe Storage", home + "/Library/Application Support/Vivaldi/Default"}
        };

        for (const auto& target : chrome_targets) {
            std::string db_path = target.dir + "/Network/Cookies";
            if (!fs::exists(db_path, ec) || ec) {
                db_path = target.dir + "/Cookies";
                if (!fs::exists(db_path, ec) || ec) continue;
            }

            std::string cmd = "security find-generic-password -w -s \"" + target.keychain + "\" 2>/dev/null";
            std::string pass;
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

            std::string tmp_db = "/tmp/ymcli_chrome_" + pid_str + ".db";
            std::string tmp_wal = tmp_db + "-wal";
            fs::copy_file(db_path, tmp_db, fs::copy_options::overwrite_existing, ec);
            if (ec) continue;

            std::string wal_path = db_path + "-wal";
            if (fs::exists(wal_path, ec)) {
                fs::copy_file(wal_path, tmp_wal, fs::copy_options::overwrite_existing, ec);
            }

            sqlite3* db = nullptr;
            if (sqlite3_open_v2(tmp_db.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) == SQLITE_OK) {
                sqlite3_stmt* stmt = nullptr;
                const char* sql = "SELECT name, encrypted_value, host_key FROM cookies WHERE host_key LIKE '%youtube.com%' OR host_key LIKE '%google.com%'";
                if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
                    std::unordered_map<std::string, std::string> cookies_map;
                    bool found_auth = false;

                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        const char* name_p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                        const void* blob = sqlite3_column_blob(stmt, 1);
                        int blob_bytes = sqlite3_column_bytes(stmt, 1);
                        const char* host_p = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
                        if (!name_p || !blob || blob_bytes == 0) continue;

                        std::string name = name_p;
                        if (valid_auth_cookie_names.find(name) == valid_auth_cookie_names.end()) continue;

                        std::string host = host_p ? host_p : "";
                        std::string enc_val(reinterpret_cast<const char*>(blob), blob_bytes);
                        std::string dec_val = decryptChromeValue(enc_val, pass);
                        if (!dec_val.empty()) {
                            bool is_yt = (host.find("youtube.com") != std::string::npos);
                            if (is_yt || cookies_map.find(name) == cookies_map.end()) {
                                cookies_map[name] = dec_val;
                            }
                            if (name == "SAPISID" || name == "__Secure-3PAPISID") {
                                found_auth = true;
                            }
                        }
                    }
                    sqlite3_finalize(stmt);

                    if (found_auth && !cookies_map.empty()) {
                        std::string result_cookies;
                        for (const auto& [name, val] : cookies_map) {
                            if (!result_cookies.empty()) result_cookies += "; ";
                            result_cookies += name + "=" + val;
                        }
                        sqlite3_close(db);
                        fs::remove(tmp_db, ec);
                        fs::remove(tmp_wal, ec);
                        if (out_browser) *out_browser = target.name;
                        return result_cookies;
                    }
                }
                sqlite3_close(db);
            }
            fs::remove(tmp_db, ec);
            fs::remove(tmp_wal, ec);
        }
    } catch (...) {}

    return "";
}

} // namespace ymcli
