/**
 * @file
 * @brief
 * Configuration settings for the library.
 *
 * @author
 * Takaaki Sato
 *
 * @copyright @parblock
 * (c) 2023, Demiquartz <info@demiquartz.jp>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * @endparblock
 */
#include <shared_mutex>
#include "config.hpp"
#include "version.hpp"

namespace Starlight::Core::Config {

static_assert(Version::Major >= 0 && Version::Major <= 0x3ff);
static_assert(Version::Minor >= 0 && Version::Minor <= 0x3ff);
static_assert(Version::Patch >= 0 && Version::Patch <= 0xfff);

static std::shared_mutex appMutex;
static std::string       appName  = "Starlight";
static std::uint16_t     appMajor = Version::Major;
static std::uint16_t     appMinor = Version::Minor;
static std::uint16_t     appPatch = Version::Patch;

std::string GetAppName(void) {
    std::shared_lock lock(appMutex);
    return appName;
}

std::uint16_t GetAppMajor(void) {
    std::shared_lock lock(appMutex);
    return appMajor;
}

std::uint16_t GetAppMinor(void) {
    std::shared_lock lock(appMutex);
    return appMinor;
}

std::uint16_t GetAppPatch(void) {
    std::shared_lock lock(appMutex);
    return appPatch;
}

void SetAppInfo(const AppInfo& info) {
    std::unique_lock lock(appMutex);
    appName  = info.name;
    appMajor = info.major;
    appMinor = info.minor;
    appPatch = info.patch;
}

} // namespace Starlight::Core::Config
