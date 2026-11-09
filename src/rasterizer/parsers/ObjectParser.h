#pragma once
/**
 * @file ObjectParser.h
 * @author Alan Abraham P Kochumon
 * @date Created on: June 16, 2026
 *
 * @brief Parser for Obj/Wavefront files.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include "rasterizer/Mesh.h"

#include <charconv>
#include <cstdlib>
#include <falcon/Falcon.h>
#include <fast_float/fast_float.h>
#include <fstream>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>


namespace rasterizer
{
    class ObjectParser
    {
    public:
        /**
         * @brief Parse a .obj file into a @ref demo::Mesh object.
         * @param filename The name of the file to parse.
         * @return A mesh object.
         */
        Mesh parse(const std::string& filename)
        {

            Mesh mesh;
            std::ifstream modelStream(filename);


            if (!modelStream.is_open())
            {
                std::cout << "Error opening file! Returning a default mesh.\n";
            }

            std::cout << "Parsing Mesh: " << filename << "...\n";
            std::string line;
            while (std::getline(modelStream, line))
            {
                if (line.starts_with("vt"))
                {
                    // TODO: Texture coordinates
                }
                else if (line.starts_with("vn"))
                {
                    // TODO: Vertex normals
                }
                else if (line.starts_with("v"))
                {
                    // Vertex
                    auto vertexIterator = std::views::split(line, ' ') | std::views::drop(1);
                    flcn::Vec3<float> vertexData;
                    size_t index = 0;
                    for (const auto token : vertexIterator)
                    {
                        float vertex{};
                        const auto [ptr, ec] =
                            fast_float::from_chars(token.data(), token.data() + token.size(), vertex);
                        // Update the mesh's current min and max value
                        mesh.minVertexValue[index] = std::min(mesh.minVertexValue[index], vertex);
                        mesh.maxVertexValue[index] = std::max(mesh.maxVertexValue[index], vertex);
                        if (ec != std::errc())
                        {
                            std::cout << "There was an error while parsing the vertex data.\n";
                        }
                        vertexData[index++] = vertex;
                    }
                    mesh.vertices.push_back(vertexData);
                }
                else if (line.starts_with("f"))
                {
                    // Vertex/Texture/Normal
                    // Only takes the face index for now
                    auto d1 = line | std::views::split(' ') | std::views::drop(1);

                    // TODO: Figure out a way to reduce this temporary heap allocations
                    std::vector<int> temp;
                    for (auto d : d1)
                    {
                        // Note: Only taking the face indices for now
                        auto firstIndexIt =
                            std::string_view(d.begin(), d.end()) | std::views::split('/') | std::views::take(1);
                        for (auto idx : firstIndexIt)
                        {
                            int data{}; // Fix: maybe uninitialized
                            std::from_chars(idx.data(), idx.data() + idx.size(), data);
                            temp.push_back(data);
                            // std::cout << std::string_view(idx.begin(), idx.end()) << " ";
                        }
                    }
                    // TODO: Support concave shapes triangulation
                    for (size_t i = 2; i < temp.size(); ++i)
                    {
                        // Note: -1 is subtracted since obj file uses 1-based indexing
                        // Clang on Linux throws class template deduction failed error without typename passed-in
                        auto vec = flcn::Vec3I(temp[0] - 1, temp[i - 1] - 1, temp[i] - 1);
                        mesh.indices.push_back(vec);

                        // TODO: Remove after testing
                        // Create random color per face
                        const auto r        = static_cast<uint8_t>(std::rand() % 255);
                        const auto g        = static_cast<uint8_t>(std::rand() % 255);
                        const auto b        = static_cast<uint8_t>(std::rand() % 255);
                        const auto colorVec = flcn::Vec3UB{ r, g, b };
                        mesh.colors.push_back(colorVec);
                    }
                }
            }


            std::cout << "Mesh Parsed: " << filename << '\n';
            return mesh;
        }
    };

} // namespace rasterizer
