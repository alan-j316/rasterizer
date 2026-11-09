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

#include <SDL3/SDL_gpu.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlgpu3.h>
#include <filesystem>
#include <imgui.h>

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


        /// +---------------------------------------------------------------------------------------+
        /// |                            SDL GPU(USED BY IMGUI)                                     |
        /// +---------------------------------------------------------------------------------------+

        SDL_GPUShaderFormat shaderFormat =
            SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL;

        _device = SDL_CreateGPUDevice(shaderFormat, true, nullptr);
        if (_device == nullptr)
        {
            SDL_Log("Fialed to create GPU Device: %s", SDL_GetError());
        }

        if (!SDL_ClaimWindowForGPUDevice(_device, _window))
        {
            SDL_Log("Failed to claim window: %s", SDL_GetError());
            SDL_DestroyGPUDevice(_device);
        }


        /// +---------------------------------------------------------------------------------------+
        /// |                                   IMGUI                                               |
        /// +---------------------------------------------------------------------------------------+
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // IF using Docking Branch

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForSDLGPU(_window);
        ImGui_ImplSDLGPU3_InitInfo imguiInitInfo = {};
        imguiInitInfo.Device                     = _device;
        imguiInitInfo.ColorTargetFormat          = SDL_GetGPUSwapchainTextureFormat(_device, _window);
        imguiInitInfo.MSAASamples                = SDL_GPU_SAMPLECOUNT_1;            // Only used in multi-viewports mode.
        imguiInitInfo.SwapchainComposition       = SDL_GPU_SWAPCHAINCOMPOSITION_SDR; // Only used in multi-viewports mode.
        imguiInitInfo.PresentMode                = SDL_GPU_PRESENTMODE_VSYNC;
        ImGui_ImplSDLGPU3_Init(&imguiInitInfo);
    }


    void Application::pollEvent() noexcept
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL3_ProcessEvent(&event); // Forward your event to backend
            // (You should discard mouse/keyboard messages in your game/engine when
            // io.WantCaptureMouse/io.WantCaptureKeyboard are set.)
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

            /// +---------------------------------------------------------------------------------------+
            /// |                                 DEAR IMGUI                                            |
            /// +---------------------------------------------------------------------------------------+
            // (After event loop)
            // Start the Dear ImGui frame
            ImGui_ImplSDLGPU3_NewFrame();
            ImGui_ImplSDL3_NewFrame();
            ImGui::NewFrame();
            ImGui::ShowDemoWindow(); // Show demo window! :)

            // Rendering
            // (Your code clears your framebuffer, renders your other stuff etc.)
            ImGui::Render();

            SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(_device); // Acquire a GPU command buffer
            SDL_GPUTexture* swapchainTexture;
            SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, _window, &swapchainTexture, nullptr,
                                                             nullptr); // Acquire a swapchain texture
            ImGui_ImplSDLGPU3_PrepareDrawData(ImGui::GetDrawData(), commandBuffer);

            // Setup and start a render pass
            SDL_GPUColorTargetInfo target_info = {};
            target_info.texture                = swapchainTexture;
            target_info.clear_color            = SDL_FColor{ .r = 0, .g = 0, .b = 0, .a = 1.0f };
            target_info.load_op                = SDL_GPU_LOADOP_CLEAR;
            target_info.store_op               = SDL_GPU_STOREOP_STORE;
            target_info.mip_level              = 0;
            target_info.layer_or_depth_plane   = 0;
            target_info.cycle                  = false;
            SDL_GPURenderPass* render_pass     = SDL_BeginGPURenderPass(commandBuffer, &target_info, 1, nullptr);

            // Render ImGui
            ImGui_ImplSDLGPU3_RenderDrawData(ImGui::GetDrawData(), commandBuffer, render_pass);

            SDL_EndGPURenderPass(render_pass);
        }


        /// +---------------------------------------------------------------------------------------+
        /// |                       DESTROY INITIALIZED STATES                                      |
        /// +---------------------------------------------------------------------------------------+
        ImGui_ImplSDL3_Shutdown();
        ImGui_ImplSDLGPU3_Shutdown();
        ImGui::DestroyContext();
        SDL_DestroyGPUDevice(_device);
        SDL_DestroyWindow(_window);
        SDL_Quit();
    }
} // namespace rasterizer
