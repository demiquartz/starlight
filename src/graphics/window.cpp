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
#include <mutex>
#include <GLFW/glfw3.h>
#include "window.hpp"

namespace Starlight::Graphics {

static std::once_flag initialized;

struct Window::Impl {
    GLFWwindow*    window;
    ResizeCallback resize;
    Impl(const std::string& title, std::size_t width, std::size_t height, bool visible) {
        std::call_once(initialized, [] {
            if (glfwInit() && std::atexit(glfwTerminate)) glfwTerminate();
        });
        glfwDefaultWindowHints();
        glfwWindowHint(GLFW_RESIZABLE,                     GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API,                   GLFW_NO_API);
        window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
        if (!window) {
            const char* description;
            glfwGetError(&description);
            throw std::runtime_error(description);
        }
        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
            auto self = static_cast<Impl*>(glfwGetWindowUserPointer(window));
            if (self && self->resize) self->resize(width, height);
        });
    }
    ~Impl() {
        glfwDestroyWindow(window);
    }
};

Window::Window(const std::string& title, std::size_t width, std::size_t height, bool visible) :
pImpl(std::make_unique<Impl>(title, width, height, visible)) {
}

Window::~Window() {
}

void Window::SetResizeCallback(const ResizeCallback& resize) {
    pImpl->resize = resize;
}

} // namespace Starlight::Graphics
