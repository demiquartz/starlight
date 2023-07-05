/**
 * @file
 * @brief
 * Manage the GPU device.
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
#ifndef STARLIGHT_CORE_DEVICE_HPP
#define STARLIGHT_CORE_DEVICE_HPP

#include <memory>

namespace Starlight::Core {

/**
 * @brief Manage the GPU device.
 *
 * This class provides functionality to manage the GPU device for graphics rendering and physics calculations.
 * It encapsulates the platform-specific device management logic.
 *
 * Usage:
 * - Create an instance of Device.
 * - The destructor will automatically clean up the device resources when the object is destroyed.
 *
 * Example:
 * @code
 * Device device;
 * // Use the device...
 * @endcode
 */
class Device final {
public:
    /**
     * @brief Construct a new Device object.
     *
     * This constructor initializes the GPU device.
     * If the device fails to initialize, a `std::runtime_error` exception is thrown.
     *
     * @throw std::runtime_error If the device fails to initialize.
     */
    Device();

    /**
     * @brief Destruct the Device object.
     */
    ~Device();

    // Add your methods and properties here...

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace Starlight::Core

#endif // STARLIGHT_CORE_DEVICE_HPP
