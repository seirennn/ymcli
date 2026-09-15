#pragma once

#include <string>

namespace ymcli {

std::string getDataDir();
std::string getConfigDir();
bool commandExists(const std::string& cmd);
void ensureDirectory(const std::string& path);

} // namespace ymcli
