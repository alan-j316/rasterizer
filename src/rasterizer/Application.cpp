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
        io.ConfigDpiScaleFonts = true;

        // Customization
        float mainScale   = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());
        ImGuiStyle& style = ImGui::GetStyle();
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();
        // style.FontSizeBase = 16.0f;           // Default to 13.0f when unspecified, which allows crisp legacy font
        // for non-4K screens.
        style.FrameRounding = 3.0f;
        style.GrabRounding  = 3.0f;

        // Setup scaling
        style.ScaleAllSizes(mainScale); // Bake a fixed style scale. (until we have a solution for dynamic style
                                        // scaling, changing this requires resetting Style + calling this again)
        style.FontScaleDpi =
            mainScale; // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically
                       // overrides this for every window depending on the current monitor)


        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForSDLGPU(_window);
        ImGui_ImplSDLGPU3_InitInfo imguiInitInfo = {};
        imguiInitInfo.Device                     = _device;
        imguiInitInfo.ColorTargetFormat          = SDL_GetGPUSwapchainTextureFormat(_device, _window);
        imguiInitInfo.MSAASamples                = SDL_GPU_SAMPLECOUNT_1;      // Only used in multi-viewports mode.
        imguiInitInfo.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR; // Only used in multi-viewports mode.
        imguiInitInfo.PresentMode          = SDL_GPU_PRESENTMODE_VSYNC;
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


    void Application::renderImgui() noexcept
    {

        const ImGuiIO& io = ImGui::GetIO();
        // Start the Dear ImGui frame
        ImGui_ImplSDLGPU3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();
        bool showDemoWindow    = true;
        bool showAnotherWindow = true;
        ImVec4 clearColor{ 0, 0, 0, 0 };

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its
        // code to learn more about Dear ImGui!).
        if (showDemoWindow)
        {
            ImGui::ShowDemoWindow(&showDemoWindow);
        }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f     = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");        // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &showDemoWindow); // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &showAnotherWindow);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*) &showAnotherWindow); // Edit 3 floats representing a color

            if (ImGui::Button(
                    "Button")) // Buttons return true when clicked (most widgets return true when edited/activated)
            {
                counter++;
            }
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (showAnotherWindow)
        {
            ImGui::Begin("Another Window",
                         &showAnotherWindow); // Pass a pointer to our bool variable (the window will have a closing
                                              // button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
            {
                showAnotherWindow = false;
            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        ImDrawData* draw_data   = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);

        SDL_GPUCommandBuffer* command_buffer = SDL_AcquireGPUCommandBuffer(_device); // Acquire a GPU command buffer

        SDL_GPUTexture* swapchain_texture;
        SDL_WaitAndAcquireGPUSwapchainTexture(command_buffer, _window, &swapchain_texture, nullptr,
                                              nullptr); // Acquire a swapchain texture

        if (swapchain_texture != nullptr && !is_minimized)
        {
            // This is mandatory: call ImGui_ImplSDLGPU3_PrepareDrawData() to upload the vertex/index buffer!
            ImGui_ImplSDLGPU3_PrepareDrawData(draw_data, command_buffer);

            // Setup and start a render pass
            SDL_GPUColorTargetInfo target_info = {};
            target_info.texture                = swapchain_texture;
            target_info.clear_color            = SDL_FColor{ clearColor.x, clearColor.y, clearColor.z, clearColor.w };
            target_info.load_op                = SDL_GPU_LOADOP_CLEAR;
            target_info.store_op               = SDL_GPU_STOREOP_STORE;
            target_info.mip_level              = 0;
            target_info.layer_or_depth_plane   = 0;
            target_info.cycle                  = false;
            SDL_GPURenderPass* render_pass     = SDL_BeginGPURenderPass(command_buffer, &target_info, 1, nullptr);

            // Render ImGui
            ImGui_ImplSDLGPU3_RenderDrawData(draw_data, command_buffer, render_pass);

            SDL_EndGPURenderPass(render_pass);
        }

        // Submit the command buffer
        SDL_SubmitGPUCommandBuffer(command_buffer);
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
            // renderImgui();
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
