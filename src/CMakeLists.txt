add_executable(Rasterizer)
set_target_properties(Rasterizer PROPERTIES CXX_STANDARD 20)
set_target_properties(Rasterizer PROPERTIES CXX_STANDARD_REQUIRED ON)
set_target_properties(Rasterizer PROPERTIES CXX_EXTENSIONS OFF)

target_sources(
        Rasterizer
        PRIVATE
        Main.cpp
        rasterizer/Renderer.cpp
        rasterizer/Application.cpp

        PRIVATE
        FILE_SET HEADERS
        BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
        FILES
        rasterizer/Renderer.h
        rasterizer/parsers/ObjectParser.h
        rasterizer/Mesh.h
)

target_include_directories(
        Rasterizer
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)


target_link_libraries(
        Rasterizer
        PRIVATE
        Falcon::Falcon
        SDL3::SDL3
        fast_float
        fastgltf::fastgltf
        # Rasterizer-StrictWarnings # TODO: Add back after fixing warnings
        # Rasterizer-ProjectSanitizers # TODO: Add back after fixing warnings
)
AddCompilerFlag(Rasterizer PRIVATE "AUTO")

# Copy models directory to the build directory
add_custom_command(
        TARGET Rasterizer
        POST_BUILD
        # Create the directory
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Rasterizer>/models"
        # Copy files into the target directory
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/models"
        "$<TARGET_FILE_DIR:Rasterizer>/models"

        COMMENT "Copying models to build directory"
)