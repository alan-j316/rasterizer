#pragma once
/**
 * @file Mesh.h
 * @author Alan Abraham P Kochumon
 * @date Created on: June 16, 2026
 *
 * @brief Defines a mesh object with vertices, indices, uv coordinates etc.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include <falcon/Falcon.h>
#include <vector>


namespace rasterizer
{
    class Mesh
    {
    public:
        std::vector<flcn::Vec3F> vertices{};
        // std::vector<flcn::Vector3<int>> colors{};
        std::vector<flcn::Vec3I> indices{};
        std::vector<flcn::Vec3<uint8_t>> colors{};
        flcn::Vec3F minVertexValue{ flcn::constants::INFINITY_F, flcn::constants::INFINITY_F,
                                    flcn::constants::INFINITY_F },
            maxVertexValue{ -flcn::constants::INFINITY_F, -flcn::constants::INFINITY_F, -flcn::constants::INFINITY_F };
    };
} // namespace demo
