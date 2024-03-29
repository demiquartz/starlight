/**
 * @file
 * @brief
 * Entry point of the Starlight application.
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
#include <thread>
#include "core/window.hpp"

int main(int argc, char** argv) {
    auto window = Starlight::Core::CreateSharedWindow("Starlight", 1280, 720, true);
    while (!window->ShouldClose()) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(10ms);
        window->PollEvents();
    }
    return EXIT_SUCCESS;
}
