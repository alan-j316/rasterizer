#pragma once
/**
 * @file Renderer.h
 * @author Alan Abraham P Kochumon
 * @date Created on: June 13, 2026
 *
 * @brief Manages application rendering.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */


#include "Mesh.h"

#include <bit>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fgm/Mat>
#include <fgm/Vec>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

namespace demo
{
    constexpr auto EPSILON = 1e-6;

    struct BarycentricCoordinates
    {
        float alpha, beta, gamma;
    };

    struct BoundingBox
    {
        size_t top, left, right, bottom;
    };

    class Renderer
    {
    public:
        uint8_t* frameBuffer;
        float* depthBuffer;
        int width, height, colorChannels;
        int clearColor = 0x000000;

        /**
         * @brief Construct a renderer with the given size.
         * @param w The width of the render.
         * @param h The height of the render.
         * @param numColorChannels The number of color channels the renderer support. Eg: 4 -> RGBA
         */
        Renderer(const int w, const int h, const int numColorChannels)
            : frameBuffer(new uint8_t[static_cast<size_t>(w * h * numColorChannels)]),
              depthBuffer(new float[static_cast<size_t>(w * h)]),
              width(w),
              height(h),
              colorChannels(numColorChannels)
        { clearScreen(); }

        void renderLine(float ax, float ay, float bx, float by, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
        {
            for (float t = 0.0f; t <= 1.0f; t += 0.002f)
            {
                auto x            = static_cast<int>(std::round(ax + t * (bx - ax)));
                auto y            = static_cast<int>(std::round(ay + t * (by - ay)));
                auto bufferOffset = colorChannels * (width * y + x);
                if constexpr (std::endian::native == std::endian::little)
                {
                    frameBuffer[bufferOffset + 0] = b;
                    frameBuffer[bufferOffset + 1] = g;
                    frameBuffer[bufferOffset + 2] = r;
                    frameBuffer[bufferOffset + 3] = a;
                }
                else if constexpr (std::endian::native == std::endian::big)
                {
                    frameBuffer[bufferOffset + 0] = r;
                    frameBuffer[bufferOffset + 1] = g;
                    frameBuffer[bufferOffset + 2] = b;
                    frameBuffer[bufferOffset + 3] = a;
                }
            }
        }

        template <typename T>
        void renderTriangleWireframe(const fgm::Vec2<T>& v0, const fgm::Vec2<T>& v1, const fgm::Vec2<T>& v2)
        {
            renderLine(v0.x(), v0.y(), v1.x(), v1.y(), 0xff, 0xff, 0xff, 0xff);
            renderLine(v1.x(), v1.y(), v2.x(), v2.y(), 0xff, 0xff, 0xff, 0xff);
            renderLine(v2.x(), v2.y(), v0.x(), v0.y(), 0xff, 0xff, 0xff, 0xff);
        }


        /**
         * @brief Update the renderer's clear color.
         * @note Must call @ref clearScreen to clear the buffer.
         *
         * @warning The endianness of the clear color is not managed and the user must specify the clear color bits
         *          following the endianness of target CPU.
         *
         * @param color The new clear color.
         */
        void setClearColor(const int color) { clearColor = color; }

        /**
         * @brief Clear the FrameBuffer with @ref clearColor.
         */
        void clearScreen() const
        {
            std::memset(frameBuffer, clearColor, static_cast<size_t>(width * height * colorChannels));
            // Clear the depth buffer
            std::memset(depthBuffer, 0, static_cast<size_t>(width * height * 4));
        }

        ~Renderer()
        {
            delete[] frameBuffer;
            delete[] depthBuffer;
        }

        /**
         * @brief Compute the edge cross of 2 vertices and a point.
         *
         * @tparam T The numeric type of vertices.
         *
         * @param vert0 The first vertex.
         * @param vert1 The second vertex.
         * @param point The point.
         * @return A positive number if the point is above the edge formed by @p vert0 and @p vert1,
         *         negative number if the point is below the edge and 0 if the point is on the edge.
         */
        template <typename T>
        T edgeCross(const fgm::Vec2<T>& vert0, const fgm::Vec2<T>& vert1, const fgm::Vec2<T>& point)
        {
            // We are assuming that the edges start from vert0
            const auto edge0 = vert1 - vert0;
            const auto edge1 = point - vert0;
            return edge0.cross(edge1);
        }


        BarycentricCoordinates computeBaryCentricCoordinates(const fgm::Vec2F& v0, const fgm::Vec2F& v1,
                                                             const fgm::Vec2F& v2, const fgm::Vec2F& point)
        {
            // Division by two is omitted since that is common in smaller triangles area too
            const auto triArea   = (v1 - v0).cross(v2 - v1);
            const auto alphaArea = (v2 - v1).cross(point - v1);
            const auto betaArea  = (v0 - v2).cross(point - v2);
            const auto gammaArea = (v1 - v0).cross(point - v0);

            return BarycentricCoordinates{ .alpha = alphaArea / triArea,
                                           .beta  = betaArea / triArea,
                                           .gamma = gammaArea / triArea };
        }

        BarycentricCoordinates computeBaryCentricCoordinates(const fgm::Vec2F& v0, const fgm::Vec2F& v1,
                                                             const fgm::Vec2F& v2, const fgm::Vec2F& point,
                                                             const float triArea)
        {
            const auto alphaArea = 0.5f * (v2 - v1).cross(point - v1);
            const auto betaArea  = 0.5f * (v0 - v2).cross(point - v2);
            const auto gammaArea = 0.5f * (v1 - v0).cross(point - v0);

            return BarycentricCoordinates{ .alpha = alphaArea / triArea,
                                           .beta  = betaArea / triArea,
                                           .gamma = gammaArea / triArea };
        }


        /**
         * @brief Compute if the edge formed by @p v0 and @p v1 is a left or top edge.
         * @param v0 The first vertex of the edge.
         * @param v1 The second vertex of the edge.
         * @return `true` if the edge is a top or left edge else `false`.
         */
        bool isTopLeftEdge(const fgm::Vec2<float>& v0, const fgm::Vec2<float>& v1)
        {
            const auto edge = v1 - v0;
            // Top Left if edge's x coordinate is positive (negative is bottom edge due to clockwise convention used)
            // and if y coordinate is 0
            const auto isTopEdge = edge.x() >= EPSILON && std::abs(edge.y()) < EPSILON;
            // Left Edge if y coordinate is >=0
            const auto isLeftEdge = edge.y() >= 1e-5;

            return isTopEdge && isLeftEdge;
        }

        // bufferOffset must be a multiple of 4 since we need to put 4 channels per bufferOffset
        inline void putPixel(size_t bufferOffset, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff)
        {
            if constexpr (std::endian::native == std::endian::big)
            {
                // ARGB
                frameBuffer[bufferOffset]     = r;
                frameBuffer[bufferOffset + 1] = g;
                frameBuffer[bufferOffset + 2] = b;
                frameBuffer[bufferOffset + 3] = a;
            }
            else if constexpr (std::endian::native == std::endian::little)
            {
                // BGRA
                frameBuffer[bufferOffset]     = b;
                frameBuffer[bufferOffset + 1] = g;
                frameBuffer[bufferOffset + 2] = r;
                frameBuffer[bufferOffset + 3] = a;
            }
        }

        // minValue-> Lowest vertex value
        // maxValue -> Highest vertex value
        template <typename T>
        fgm::Vec2<T> toScreenSpace(const fgm::Vec3<T>& vec) const
        {
            return fgm::Vec2<T>{
                width - vec.x() * width,
                height - vec.y() * height,
            };
        }


        template <typename T>
        static fgm::Vec3<T> toNDC(const fgm::Vec3<T>& vec, const fgm::Vec3F& minValueVec, const fgm::Vec3F& maxValueVec)
        {
            return fgm::Vec3<T>{ static_cast<T>((vec.x() - minValueVec.x()) / (maxValueVec.x() - minValueVec.x())),
                                 static_cast<T>((vec.y() - minValueVec.y()) / (maxValueVec.y() - minValueVec.y())),
                                 static_cast<T>((vec.z() - minValueVec.z()) / (maxValueVec.z() - minValueVec.z())) };
        }

        inline BoundingBox computeBoundingBox(const fgm::Vec2F& v0, const fgm::Vec2F& v1, const fgm::Vec2F& v2)
        {
            return BoundingBox{
                .top    = static_cast<size_t>(std::min({ v0.y(), v1.y(), v2.y() })),
                .left   = static_cast<size_t>(std::min({ v0.x(), v1.x(), v2.x() })),
                .right  = static_cast<size_t>(std::max({ v0.x(), v1.x(), v2.x() })),
                .bottom = static_cast<size_t>(std::max({ v0.y(), v1.y(), v2.y() })),
            };
        }


        // TODO: Add docs
        template <typename T>
        void renderTriangle(const fgm::Vec3<T>& v0, const fgm::Vec3<T>& v1, const fgm::Vec3<T>& v2,
                            const uint8_t r = 0xff, const uint8_t g = 0xff, const uint8_t b = 0xff,
                            const uint8_t a = 0xff)
        {
            const auto projV0 = toScreenSpace(v0);
            const auto projV1 = toScreenSpace(v1);
            const auto projV2 = toScreenSpace(v2);

            // Calculate the bounding box of the triangle
            // (minX, minY)--------
            //       |____________|
            //       |\ \ \ \ \ \/|
            //       | \ \ \ \ \/ |
            //       |  \ \ \ \/  |
            //       |   \ \ \/   |
            //       |    \ \/    |
            //       |     \/     |
            //       --------(maxX, maxY)
            auto bb = computeBoundingBox(projV0, projV1, projV2);

            // Clip against screen bounds
            bb.left = std::max(bb.left, 0ull);
            bb.top = std::max(bb.top, 0ull);
            bb.right = std::min(bb.right, static_cast<size_t>(width - 1));
            bb.bottom = std::min(bb.bottom, static_cast<size_t>(height - 1));


            const auto triArea = (projV1 - projV0).cross(projV2 - projV1) * 0.5f;

            // Backface culling
            if (triArea < 0.0f) // Triangle is inverted or less than 1 px
            {
                return;
            }

            // Compute top left for each edge (vertex pair)
            const auto isTopLeft0 = isTopLeftEdge(projV0, projV1);
            const auto isTopLeft1 = isTopLeftEdge(projV1, projV2);
            const auto isTopLeft2 = isTopLeftEdge(projV2, projV0);

            const auto bias0 = isTopLeft0 * EPSILON;
            const auto bias1 = isTopLeft1 * EPSILON;
            const auto bias2 = isTopLeft2 * EPSILON;



            // Looping in reverse order for better cache locality
            for (auto y = bb.top; y <= bb.bottom; ++y)
            {
                for (auto x = bb.left; x <= bb.right; ++x)
                {
                    const auto offset      = y * static_cast<size_t>(width) + x;
                    const auto colorOffset = static_cast<size_t>(colorChannels) * offset;
                    const auto point       = fgm::Vec2(static_cast<float>(x), static_cast<float>(y));

                    if (offset > static_cast<size_t>(width * height))
                    {
                        continue;
                    }

                    const auto [alpha, beta, gamma] =
                        computeBaryCentricCoordinates(projV0, projV1, projV2, point, triArea);

                    const auto currentPixelDepth = alpha * v0.z() + beta * v1.z() + gamma * v2.z();

                    // A comparison of anything other than 0.0f, like 1e-5 or 1e-10 will cause visual glitches
                    const bool eC0 = edgeCross(projV0, projV1, point) - bias0 >= 0.0f;
                    const bool eC1 = edgeCross(projV1, projV2, point) - bias1 >= 0.0f;
                    const bool eC2 = edgeCross(projV2, projV0, point) - bias2 >= 0.0f;

                    // Edge cross && Depth buffer-based culling
                    if (eC0 && eC1 && eC2 && currentPixelDepth > depthBuffer[offset])
                    {
                        // Color Rendering
                        putPixel(colorOffset, r, g, b, a);

                        // Update depth buffer
                        depthBuffer[offset] = currentPixelDepth;

                        // Depth buffer rendering
                        // const auto depthColor = static_cast<uint8_t>(depthBuffer[offset] * 255);
                        // putPixel(colorOffset, depthColor, depthColor, depthColor);
                    }
                }
            }
        }

        fgm::Vec3F project(const fgm::Vec3F& vertex, const float focalLength)
        {
            fgm::Vec3F vec{};
            const auto factor = focalLength / vertex.z();

            vec.x() = vertex.x() * factor;
            vec.y() = vertex.y() * factor;
            vec.z() = vertex.z();

            return vec;
        }

        // Vertex Shader
        inline fgm::Vec3F perVertex(const fgm::Vec3F& vertex, const fgm::Vec3F& minVertexValues,
                                    const fgm::Vec3F& maxVertexValues, const float rotationDeg)
        {
            const auto vec = fgm::Mat4F::makeRotationY(rotationDeg) * fgm::Vec4(vertex, 1.0f);
            return toNDC(vec.swizzle<fgm::axis::X, fgm::axis::Y, fgm::axis::Z>(), minVertexValues, maxVertexValues);
        }


        /**
         * @brief Renders a mesh to the current @p frameBuffer.
         *
         * @note Blitting is not performed by the renderer, the user will need to blit the framebuffer
         *       to the target surface.
         *
         * @param mesh The mesh to render.
         */
        void render(const Mesh& mesh, [[maybe_unused]] const float deg)
        {
            std::vector<fgm::Vec3<float>> vertices{ mesh.vertices.size() };
            // const auto focalLength = 3.0f;

            std::ranges::transform(mesh.vertices, std::inserter(vertices, vertices.begin()),
                                   [mesh, this, deg](const fgm::Vec3<float> vertex) {
                                       return perVertex(vertex, mesh.minVertexValue, mesh.maxVertexValue, deg);
                                   });

            // std::ranges::transform(mesh.vertices, std::inserter(vertices, vertices.begin()),
            //                [this, mesh](const fgm::Vec3<float> vertex) {
            //                    const auto vec = project(vertex, 3.0f);
            //                    const auto proj = toNDC(vec, mesh.minVertexValue, mesh.maxVertexValue);
            //                    // const auto vec = rotateY(vertex, deg);
            //                    return proj;
            //                });

            [[maybe_unused]] size_t i = 0;
            for (const auto& index : mesh.indices)
            {
                const auto i0 = static_cast<size_t>(index.x());
                const auto i1 = static_cast<size_t>(index.y());
                const auto i2 = static_cast<size_t>(index.z());
                // FIXME: Test code
                const auto faceColor = mesh.colors[i++];
                renderTriangle(vertices[i0], vertices[i1], vertices[i2], faceColor.r(), faceColor.g(), faceColor.b(),
                               0xff);
                // renderTriangle(vertices[i0], vertices[i1], vertices[i2], 0xff, 0xff, 0xff, 0xff);
                // renderTriangleWireframe(vertices[i0], vertices[i1], vertices[i2]);
            }
        }
    };
} // namespace demo
