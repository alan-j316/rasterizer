/**
 * @file Application.cpp
 * @author Alan Abraham P Kochumon
 * @date Created on: November 09, 2026
 *
 * @brief Implemenat
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include "rasterizer/Application.h"

namespace rasterizer
{
    Application::Application(const std::string_view appName, const std::string_view version,
                             const std::string_view identifier)
        : _renderer(_width, _height, _colorChannels)
    {
        SDL_SetAppMetadata(appName.data(), version.data(), identifier.data());

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            SDL_Log("There was an error initializing SDL application!");
            return;
        }

        // TODO: Make window resizeable
        _window = SDL_CreateWindow(appName.data(), _width, _height, 0);
    }


    void Application::pollEvent() noexcept
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_EVENT_QUIT:
                    _isRunning = false;
                    break;
                default:
                    break;
                    // SDL_Log("[EVENT]: Unhandled"); // TODO: Remove
            }
        }
    }


    void Application::run() noexcept
    {
        // auto modelPath = std::filesystem::path("models") / "utah_teapot.obj";
        auto modelPath = std::filesystem::path("models") / "suzanne.obj";
        // auto modelPath = std::filesystem::path("models") / "Cube.obj";
        ObjectParser parser;
        auto mesh = parser.parse(modelPath.string());

        _isRunning = true;


        SDL_Surface* renderingSurface = SDL_GetWindowSurface(_window);

        _renderer.clearScreen();
        _renderer.render(mesh, 0.0f);

        float rotationAngle = 0.0f;
        while (_isRunning)
        {
            // TODO: Add run loop
            // _renderer.clearScreen();
            // _renderer.render(mesh, rotationAngle);

            SDL_Surface* imageSurface =
                SDL_CreateSurfaceFrom(_width, _height, renderingSurface->format, _renderer.frameBuffer,
                                      _renderer.width * _renderer.colorChannels);

            SDL_BlitSurface(imageSurface, nullptr, renderingSurface, nullptr);
            SDL_UpdateWindowSurface(_window);
            pollEvent();

            rotationAngle += 0.1f;
        }
    }
} // namespace rasterizer
