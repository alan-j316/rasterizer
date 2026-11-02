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


#include "application/Mesh.h"
#include "application/Renderer.h"
#include "parsers/ObjectParser.h"

#include <SDL3/SDL.h>
#include <chrono>
#include <filesystem>
#include <string>

namespace demo
{
    class Application
    {
        SDL_Window* _window{ nullptr };
        bool _isRunning{ false };
        int _width{ 1280 }, _height{ 720 }, _colorChannels{ 4 }; // TODO: Update to use ctd.
        Renderer _renderer;


    public:
        Application(const std::string_view appName, const std::string_view version, const std::string_view identifier)
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


        void pollEvent()
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

        void run()
        {
            // auto modelPath = std::filesystem::path("models") / "utah_teapot.obj";
            // auto modelPath = std::filesystem::path("models") / "suzanne.obj";
            auto modelPath = std::filesystem::path("models") / "Cube.obj";
            ObjectParser parser;
            auto mesh = parser.parse(modelPath.string());

            _isRunning = true;


            SDL_Surface* renderingSurface = SDL_GetWindowSurface(_window);

            _renderer.clearScreen();
            // _renderer.render(mesh, 0.0f);
            // _renderer.renderTriangleWireframe(fgm::Vec2F{400, 400}, fgm::Vec2F{700, 100}, fgm::Vec2F{100, 100});
            _renderer.renderTriangleWireframe(fgm::Vec2F{100, 400}, fgm::Vec2F{400, 400}, fgm::Vec2F{10, 10} );
            _renderer.renderTriangle( fgm::Vec3F{400, 400, 1}, fgm::Vec3F{100, 400, 1}, fgm::Vec3F{10, 10, 1} );

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
    };
} // namespace demo
