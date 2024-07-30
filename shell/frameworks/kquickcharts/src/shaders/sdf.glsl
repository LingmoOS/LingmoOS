// SPDX-FileCopyrightText: 2019 Arjen Hiemstra <ahiemstra@heimr.nl>
// SPDX-FileCopyrightText: 2017 Inigo Quilez
//
// SPDX-License-Identifier: MIT
//
// This file is based on
// https://iquilezles.org/www/articles/distfunctions2d/distfunctions2d.htm

// A maximum point count to be used for sdf_polygon input arrays.
// Unfortunately even function inputs require a fixed size at declaration time
// for arrays, unless we were to use OpenGL 4.5.
// Since the polygon is most likely to be defined in a uniform, this should be
// at least less than MAX_FRAGMENT_UNIFORM_COMPONENTS / 2 (since we need vec2).
#define SDF_POLYGON_MAX_POINT_COUNT 400

// A constant for pi
const lowp float pi = 3.1415926535897932384626433832795;

/*********************************
    Shapes
*********************************/

// Distance field for a circle.
//
// \param point A point on the distance field.
// \param radius The radius of the circle.
//
// \return The signed distance from point to the circle. If negative, point is
//         inside the circle.
lowp float sdf_circle(in lowp vec2 point, in lowp float radius)
{
    return length(point) - radius;
}

// Distance field for a triangle.
//
// \param point A point on the distance field.
// \param p0 The first vertex of the triangle.
// \param p0 The second vertex of the triangle.
// \param p0 The third vertex of the triangle.
//
// \note The ordering of the three vertices does not matter.
//
// \return The signed distance from point to triangle. If negative, point is
//         inside the triangle.
lowp float sdf_triangle(in lowp vec2 point, in lowp vec2 p0, in lowp vec2 p1, in lowp vec2 p2)
{
    lowp vec2 e0 = p1 - p0;
    lowp vec2 e1 = p2 - p1;
    lowp vec2 e2 = p0 - p2;

    lowp vec2 v0 = point - p0;
    lowp vec2 v1 = point - p1;
    lowp vec2 v2 = point - p2;

    lowp vec2 pq0 = v0 - e0 * clamp( dot(v0, e0) / dot(e0, e0), 0.0, 1.0 );
    lowp vec2 pq1 = v1 - e1 * clamp( dot(v1, e1) / dot(e1, e1), 0.0, 1.0 );
    lowp vec2 pq2 = v2 - e2 * clamp( dot(v2, e2) / dot(e2, e2), 0.0, 1.0 );

    lowp float s = sign( e0.x*e2.y - e0.y*e2.x );
    lowp vec2 d = min(min(vec2(dot(pq0,pq0), s*(v0.x*e0.y-v0.y*e0.x)),
                          vec2(dot(pq1,pq1), s*(v1.x*e1.y-v1.y*e1.x))),
                          vec2(dot(pq2,pq2), s*(v2.x*e2.y-v2.y*e2.x)));

    return -sqrt(d.x)*sign(d.y);
}

#ifndef API_ES2
// Distance field for an arbitrary polygon.
//
// \param point A point on the distance field.
// \param vertices An array of points that make up the polygon.
// \param count The amount of points to use for the polygon.
//
// \note points should be an array of vec2 of size SDF_POLYGON_MAX_POINT_COUNT.
//       Use count to indicate how many items of that array should be used.
//
// \return The signed distance from point to triangle. If negative, point is
//         inside the triangle.
lowp float sdf_polygon(in lowp vec2 point, in lowp vec2[SDF_POLYGON_MAX_POINT_COUNT] vertices, in lowp int count)
{
    lowp float d = dot(point - vertices[0], point - vertices[0]);
    lowp float s = 1.0;
    for (int i = 0, j = count - 1; i < count && i < SDF_POLYGON_MAX_POINT_COUNT; j = i, i++)
    {
        lowp vec2 e = vertices[j] - vertices[i];
        lowp vec2 w = point - vertices[i];
        lowp float h = clamp( dot(w, e) / dot(e, e), 0.0, 1.0 );
        lowp vec2 b = w - e * h;
        d = min(d, dot(b, b));

        bvec3 c = bvec3(point.y >= vertices[i].y, point.y < vertices[j].y, e.x * w.y > e.y * w.x);
        if(all(c) || all(not(c))) s *= -1.0;
    }
    return s * sqrt(d);
}
#endif

// Distance field for a rectangle.
//
// \param point A point on the distance field.
// \param rect A vec2 with the size of the rectangle.
//
// \return The signed distance from point to rectangle. If negative, point is
//         inside the rectangle.
lowp float sdf_rectangle(in lowp vec2 point, in lowp vec2 rect)
{
    lowp vec2 d = abs(point) - rect;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

// Distance field for a torus segment.
//
// \param point A point on the distance field.
// \param start The start angle in radians of the segment.
// \param end The end angle in radians of the segment.
// \param inner_radius The inner radius of the torus.
// \param outer_radius The outer radius of the torus.
//
// \return The signed distance from point to the torus segment. If negative,
//         point is inside the segment.
lowp float sdf_torus_segment(in lowp vec2 point, in lowp float start, in lowp float end, in lowp float inner_radius, in lowp float outer_radius)
{
    start = clamp(start, end - 2.0 * pi, end);
    end = clamp(end, start, start + 2.0 * pi);

    lowp float angle = (end - start) / 2.0;
    lowp float rotation = (start + end) / 2.0;

    lowp vec2 rotated = point * mat2(cos(rotation), -sin(rotation), sin(rotation), cos(rotation));
    lowp vec2 c = vec2(sin(angle), cos(angle));

    rotated.x = abs(rotated.x);

    lowp float t = (outer_radius - inner_radius) / 2.0;
    lowp float l = abs(length(rotated) - (inner_radius + t)) - t;

    lowp float m = length(rotated - c * clamp(dot(rotated, c), inner_radius, outer_radius));
    return max(l, m * sign(c.y * rotated.x - c.x * rotated.y));
}

/*********************
    Operators
*********************/

// Convert a distance field to an annular (hollow) distance field.
//
// \param sdf The result of an sdf shape to convert.
// \param thickness The thickness of the resulting shape.
//
// \return The value of sdf modified to an annular shape.
lowp float sdf_annular(in lowp float sdf, in lowp float thickness)
{
    return abs(sdf) - thickness;
}

// Union two sdf shapes together.
//
// \param sdf1 The first sdf shape.
// \param sdf2 The second sdf shape.
//
// \return The union of sdf1 and sdf2, that is, the distance to both sdf1 and
//         sdf2.
lowp float sdf_union(in lowp float sdf1, in lowp float sdf2)
{
    return min(sdf1, sdf2);
}

// Subtract two sdf shapes.
//
// \param sdf1 The first sdf shape.
// \param sdf2 The second sdf shape.
//
// \return sdf1 with sdf2 subtracted from it.
lowp float sdf_subtract(in lowp float sdf1, in lowp float sdf2)
{
    return max(sdf1, -sdf2);
}

// Intersect two sdf shapes.
//
// \param sdf1 The first sdf shape.
// \param sdf2 The second sdf shape.
//
// \return The intersection between sdf1 and sdf2, that is, the area where both
//         sdf1 and sdf2 provide the same distance value.
lowp float sdf_intersect(in lowp float sdf1, in lowp float sdf2)
{
    return max(sdf1, sdf2);
}

// Smoothly intersect two sdf shapes.
//
// \param sdf1 The first sdf shape.
// \param sdf2 The second sdf shape.
// \param smoothing The amount of smoothing to apply.
//
// \return A smoothed version of the intersect operation.
lowp float sdf_intersect_smooth(in lowp float sdf1, in lowp float sdf2, in lowp float smoothing)
{
    lowp float h = clamp(0.5 - 0.5 * (sdf1 - sdf2) / smoothing, 0.0, 1.0);
    return mix(sdf1, sdf2, h) + smoothing * h * (1.0 - h);
}

// Round an sdf shape.
//
// \param sdf The sdf shape to round.
// \param amount The amount of rounding to apply.
//
// \return The rounded shape of sdf.
//         Note that rounding happens by basically selecting an isoline of sdf,
//         therefore, the resulting shape may be larger than the input shape.
lowp float sdf_round(in lowp float sdf, in lowp float amount)
{
    return sdf - amount;
}

// Convert an sdf shape to an outline of its shape.
//
// \param sdf The sdf shape to turn into an outline.
//
// \return The outline of sdf.
lowp float sdf_outline(in lowp float sdf)
{
    return abs(sdf);
}

/********************
    Convenience
********************/

// A constant to represent a "null" value of an sdf.
// Since 0 is a point exactly on the outline of an sdf shape, and negative
// values are inside the shape, this uses a very large positive constant to
// indicate a value that is really far away from the actual sdf shape.
const lowp float sdf_null = 99999.0;

// A constant for a default level of smoothing when rendering an sdf.
const lowp float sdf_default_smoothing = 0.625;

/**
 * Render an sdf shape alpha-blended onto an existing color.
 *
 * \param sdf The sdf shape to render.
 * \param fwidthSdf fwidth() of the sdf. This is a separte argument because
 *        fwidth() cannot be used in conditional contexts.
 * \param sourceColor The source color to render on top of.
 * \param sdfColor The color to use for rendering the sdf shape.
 * \param alpha The alpha to use for blending.
 * \param smoothing The amount of smoothing to apply to the sdf.
 *
 * \return sourceColor with the sdf shape rendered on top.
 */
lowp vec4 sdf_render(in lowp float sdf, in lowp float fwidthSdf, in lowp vec4 sourceColor, in lowp vec4 sdfColor, in lowp float alpha, in lowp float smoothing)
{
    lowp float g = smoothing * fwidthSdf;
    return mix(sourceColor, sdfColor, alpha * (1.0 - smoothstep(-g, g, sdf)));
}

/**
 * Render an sdf shape.
 *
 * This is an overload of sdf_render(float, vec4, vec4, float, float) that uses
 * 1.0 for blending value and sdf_default_smoothing for smoothing.
 */
lowp vec4 sdf_render(in lowp float sdf, in lowp vec4 sourceColor, in lowp vec4 sdfColor)
{
    return sdf_render(sdf, fwidth(sdf), sourceColor, sdfColor, 1.0, sdf_default_smoothing);
}

/**
 * Render an sdf shape.
 *
 * This is an overload of sdf_render(float, vec4, vec4, float, float) that uses
 * 1.0 for blending value but allows specifying a smoothing amount.
 */
lowp vec4 sdf_render(in lowp float sdf, in lowp vec4 sourceColor, in lowp vec4 sdfColor, in lowp float smoothing)
{
    return sdf_render(sdf, fwidth(sdf), sourceColor, sdfColor, 1.0, smoothing);
}
