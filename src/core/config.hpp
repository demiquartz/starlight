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
#ifndef STARLIGHT_CORE_CONFIG_HPP
#define STARLIGHT_CORE_CONFIG_HPP

#include <string>

namespace Starlight::Core::Config {

/**
 * @brief A structure to hold the application information.
 *
 * This structure contains the name and version information of the application.
 * The version information is divided into major, minor, and patch numbers.
 * The major and minor version numbers are 10-bit values, allowing for a range of 0 to 1023.
 * The patch version number is a 12-bit value, allowing for a range of 0 to 4095.
 */
struct AppInfo final {
    std::string name;       ///< The name of the application.
    std::size_t major : 10; ///< The major version number of the application.
    std::size_t minor : 10; ///< The minor version number of the application.
    std::size_t patch : 12; ///< The patch version number of the application.
};

/**
 * @brief Get the name of the application.
 *
 * This function returns the name of the application.
 *
 * @return The name of the application.
 */
std::string GetAppName(void);

/**
 * @brief Get the major version number of the application.
 *
 * This function returns the major version number of the application.
 * The returned value is guaranteed to be between 0 and 1023.
 *
 * @return The major version number of the application.
 */
std::uint16_t GetAppMajor(void);

/**
 * @brief Get the minor version number of the application.
 *
 * This function returns the minor version number of the application.
 * The returned value is guaranteed to be between 0 and 1023.
 *
 * @return The minor version number of the application.
 */
std::uint16_t GetAppMinor(void);

/**
 * @brief Get the patch version number of the application.
 *
 * This function returns the patch version number of the application.
 * The returned value is guaranteed to be between 0 and 4095.
 *
 * @return The patch version number of the application.
 */
std::uint16_t GetAppPatch(void);

/**
 * @brief Set the application information.
 *
 * This function sets the application information to the given AppInfo structure.
 *
 * @param info The AppInfo structure containing the application information to be set.
 */
void SetAppInfo(const AppInfo& info);

} // namespace Starlight::Core::Config

#endif // STARLIGHT_CORE_CONFIG_HPP
