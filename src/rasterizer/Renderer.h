#pragma once
/**
 * @file Renderer.h
 * @author Alan Abraham P Kochumon
 * @date Created on: November 09, 2026
 *
 * @brief Software Rasterizer.
 *
 * @copyright Copyright (c) 2026 Alan Abraham P Kochumon
 */

#include "rasterizer/parsers/ObjectParser.h"

#include <cstddef>
#include <cstdint>
#include <falcon/Falcon.h>

namespace rasterizer
{
    inline constexpr auto EPSILON = 1e-6;

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


        /**
         * Render a single line using interpolation.
         * @param ax Starting x value.
         * @param ay Starting y-value.
         * @param bx End x-value.
         * @param by End y-value.
         * @param r  Line color's red-channel.
         * @param g  Line color's green-channel.
         * @param b  Line color's blue-channel.
         * @param a  Line color's alpha-channel.
         */
        void renderLine(float ax, float ay, float bx, float by, uint8_t r, uint8_t g, uint8_t b,
                        uint8_t a) const noexcept;

        /**
         * Render a triangulated wireframe.
         * @tparam T The numeric type of vertices.
         * @param v0 The first vertex of the triangle.
         * @param v1 The second vertex of the triangle.
         * @param v2 The last vertex of the triangle.
         */
        template <typename T>
        void renderTriangleWireframe(const flcn::Vec2<T>& v0, const flcn::Vec2<T>& v1, const flcn::Vec2<T>& v2);


        /**
         * Update the renderer's clear color.
         * @note Must call @ref clearScreen to clear the buffer.
         *
         * @warning The endianness of the clear color is not managed and the user must specify the clear color bits
         *          following the endianness of target CPU.
         *
         * @param color The new clear color.
         */
        inline void setClearColor(const int color) noexcept { clearColor = color; }


        /**
         * @brief Clear the FrameBuffer with @ref clearColor.
         */
        void clearScreen() const noexcept;

        ~Renderer() noexcept;

        /**
         * @brief Compute the edge cross of 2 vertices and a point.
         *
         * @tparam T The numeric type of vertices.
         *
         * @param v0 The first vertex.
         * @param v1 The second vertex.
         * @param p The point.
         * @return A positive number if the point is above the edge formed by @p vert0 and @p vert1,
         *         negative number if the point is below the edge and 0 if the point is on the edge.
         */
        template <typename T>
        static T edgeCross(const flcn::CVec2<T>& v0, const flcn::CVec2<T>& v1, const flcn::CVec2<T>& p);


        /**
         * Compute the barycenter coordinate of @p point.
         * @param v0 The first vertex of the triangle.
         * @param v1 The second vertex of the triangle.
         * @param v2 The last vertex of the triangle.
         * @param point The point to compute the barycentric coordinates of.
         * @return The barycentric coordinate of @ref point.
         */
        BarycentricCoordinates computeBaryCentricCoordinates(const flcn::Vec2F& v0, const flcn::Vec2F& v1,
                                                             const flcn::Vec2F& v2,
                                                             const flcn::Vec2F& point) const noexcept;

        /**
         * Compute the barycenter coordinate of @p point.
         * @param v0 The first vertex of the triangle.
         * @param v1 The second vertex of the triangle.
         * @param v2 The last vertex of the triangle.
         * @param point The point to compute the barycentric coordinates of.
         * @param triArea The area of the enclosing triangle.
         * @return The barycentric coordinate of @ref point.
         */
        static BarycentricCoordinates computeBaryCentricCoordinates(const flcn::CVec2F& v0, const flcn::CVec2F& v1,
                                                                    const flcn::CVec2F& v2, const flcn::CVec2F& point,
                                                                    const float triArea) noexcept;


        /**
         * @brief Compute if the edge formed by @p v0 and @p v1 is a left or top edge.
         * @param v0 The first vertex of the edge.
         * @param v1 The second vertex of the edge.
         * @return `true` if the edge is a top or left edge else `false`.
         */
        static bool isTopLeftEdge(const flcn::CVec2F& v0, const flcn::CVec2F& v1) noexcept;


        /**
         * Put the color formed by r, g, b, and a into the framebuffer.
         * @param bufferOffset The offset into the frame buffer. Must be a multiple of 4 since we need to put
         *                     4 channel per buffer offset.
         * @param r The pixel's red-channel.
         * @param g The pixel's green-channel.
         * @param b The pixel's blue-channel.
         * @param a The pixel's alpha-channel.
         */
        void putPixel(size_t bufferOffset, uint8_t r, uint8_t g, uint8_t b, uint8_t a = 0xff) const noexcept;

        // minValue-> Lowest vertex value
        // // maxValue -> Highest vertex value
        // template <typename T>
        // flcn::Vec2<T> toScreenSpace(const flcn::Vec3<T>& vec) const
        // {
        //     // Convert from -1 to 1 to 0 to 2
        //     // Then from 0 to 2 to 0 to 1
        //     // Finally multiply it screen width and height
        //     return flcn::Vec2<T>{
        //         (vec.x() + 1) * 0.5f * width,
        //         (vec.y() + 1) * 0.5f * height,
        //     };
        // }


        /**
         * Compute approximate the bounding box of the triangle.
         * @param v0 The first vertex of the triangle.
         * @param v1 The second vertex of the triangle.
         * @param v2 The last vertex of the triangle.
         * @return A bounding box that can enclose the given triangle.
         */
        static BoundingBox computeBoundingBox(const flcn::CVec2F& v0, const flcn::CVec2F& v1,
                                              const flcn::CVec2F& v2) noexcept;


        /**
         * Render the given triangle into the frame buffer.
         * @tparam T The numeric type of the vertices.
         * @param v0 The first vertex of the triangle.
         * @param v1 The second vertex of the triangle.
         * @param v2 The last vertex of the triangle.
         * @param r The red-channel of the triangle.
         * @param g The green-channel of the triangle.
         * @param b The blue-channel of the triangle.
         * @param a The alpha-channel of the triangle.
         */
        template <typename T>
        void renderTriangle(const flcn::Vec3<T>& v0, const flcn::Vec3<T>& v1, const flcn::Vec3<T>& v2,
                            uint8_t r = 0xff, uint8_t g = 0xff, uint8_t b = 0xff,
                            uint8_t a = 0xff) const noexcept;


        // TODO: REMOVE TEMPORARY METHOD
        inline flcn::Mat4F makeOrtho(float top, float bottom, float left, float right, float near, float far)
        {
            return flcn::Mat4F(2 / (right - left), 0, 0, -(right + left) / (right - left), 0, 2 / (top - bottom), 0,
                               -(top + bottom) / (top - bottom), 0, 0, -2 / (far - near), -(far + near) / (far - near),
                               0, 0, 0, 1);
        }

        // Vertex Shader
        flcn::Vec3F perVertex(const flcn::Vec3F& vertex, [[maybe_unused]] const float rotationDeg)
        {
            const auto proj = makeOrtho(-1, 1, -1, 1, 0, 1);
            // Projection to NDC
            auto vec = proj * flcn::Vec4(vertex, 1.0f);
            // To screen space/viewport space
            vec.x() = (1 - vec.x()) * width * 0.5f;
            vec.y() = (vec.y() + 1) * height * 0.5f;
            return vec.swizzle<flcn::axis::X, flcn::axis::Y, flcn::axis::Z>();
        }


        /**
         * @brief Renders a mesh to the current @p frameBuffer.
         *
         * @note Blitting is not performed by the renderer, the user will need to blit the framebuffer
         *       to the target surface.
         *
         * @param mesh The mesh to render.
         */
        void render(const Mesh& mesh, [[maybe_unused]] float deg) noexcept;
    };


} // namespace rasterizer
