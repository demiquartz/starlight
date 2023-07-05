/**
 * @file
 * @brief
 * Create and manage a window.
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
#ifndef STARLIGHT_CORE_WINDOW_HPP
#define STARLIGHT_CORE_WINDOW_HPP

#include <functional>
#include <memory>
#include <string>

namespace Starlight::Core {

/**
 * @brief Create and manage a window.
 *
 * This class provides functionality to create and manage a window.
 * It encapsulates the platform-specific window creation and management logic.
 *
 * Usage:
 * - Create an instance of Window by providing the title, width, height, and visibility.
 * - The destructor will automatically clean up the window resources when the object is destroyed.
 *
 * Example:
 * @code
 * Window window("My Window", 1280, 720, true);
 * // Use the window...
 * @endcode
 */
class Window final {
public:
    /**
     * @brief Callback function type for window resize events.
     *
     * This callback function is called when the window is resized.
     * It receives the new width and height of the window as parameters.
     *
     * Signature:
     * @code
     * void resize(std::size_t width, std::size_t height);
     * @endcode
     *
     * @param width  The new width of the window.
     * @param height The new height of the window.
     */
    using ResizeCallback = std::function<void(std::size_t, std::size_t)>;

    /**
     * @brief Construct a new Window object.
     *
     * This constructor creates a window with the specified title, width, and height.
     * The `visible` parameter determines whether the window should be initially visible or not.
     * If the window fails to create, a `std::runtime_error` exception is thrown.
     *
     * @param title   The title of the window.
     * @param width   The width of the window.
     * @param height  The height of the window.
     * @param visible Whether the window should be initially visible or not.
     *
     * @throw std::runtime_error If the window fails to create.
     */
    Window(const std::string& title, std::size_t width, std::size_t height, bool visible);

    /**
     * @brief Destruct the Window object.
     */
    ~Window();

    /**
     * @brief Set the resize callback function.
     *
     * This method sets the callback function to be called when the window is resized.
     * The provided `resize` function will be invoked with the new width and height as parameters.
     *
     * @param resize The resize callback function.
     */
    void SetResizeCallback(const ResizeCallback& resize);

    /**
     * @brief Check if the window should be closed.
     *
     * This method checks if the window should be closed.
     * It returns true if the window should be closed, false otherwise.
     *
     * @return true if the window should be closed, false otherwise.
     */
    bool ShouldClose(void);

    /**
     * @brief Cancel the window close request.
     *
     * This method cancels the window close request.
     * After calling this method, `ShouldClose` method will return false until a new close request is received.
     */
    void CancelClose(void);

    /**
     * @brief Poll for window events.
     *
     * This method polls for window events.
     * All window events that are received are processed immediately.
     */
    void PollEvents(void);

    /**
     * @brief Show the cursor.
     *
     * This method makes the cursor visible.
     */
    void ShowCursor(void);

    /**
     * @brief Hide the cursor.
     *
     * This method makes the cursor invisible.
     */
    void HideCursor(void);

    /**
     * @brief Show the window.
     *
     * This method makes the window visible.
     */
    void ShowWindow(void);

    /**
     * @brief Hide the window.
     *
     * This method makes the window invisible.
     */
    void HideWindow(void);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace Starlight::Core

#endif // STARLIGHT_CORE_WINDOW_HPP
