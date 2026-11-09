#pragma once
/**
 * @file Application.h
 * @author Alan Abraham P Kochumon
 * @date Created on: June 13, 2026
 *
 * @brief Manages the entire application core and running loop.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include "Mesh.h"
#include "parsers/ObjectParser.h"
#include "rasterizer/Renderer.h"

#include <SDL3/SDL.h>

namespace rasterizer
{
    class Application
    {
    public:
        Application(std::string_view appName, std::string_view version, std::string_view identifier);
        void run() noexcept;

    private:
        void pollEvent() noexcept;

    private:
        SDL_Window* _window{ nullptr };
        SDL_GPUDevice* _device;

        bool _isRunning{ false };
        int _width{ 1280 }, _height{ 720 }, _colorChannels{ 4 }; // TODO: Update to use ctd.
        Renderer _renderer;
    };

} // namespace rasterizer
