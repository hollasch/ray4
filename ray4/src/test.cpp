#include <format>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "r4_color.h"
#include "r4_vector.h"
#include "r4_point.h"

namespace Catch {
    // Color to String
    template<> struct StringMaker<Color> {
        static std::string convert(Color const& value) {
            return std::format("Color<{}, {}, {}>", value.r, value.g, value.b);
        }
    };

    // Vector to String
    template<> struct StringMaker<Vector4> {
        static std::string convert(Vector4 const& value) {
            return std::format("Vector4<{}, {}, {}, {}>", value.x, value.y, value.z, value.w);
        }
    };

    // Point to String
    template<> struct StringMaker<Point4> {
        static std::string convert(Point4 const& value) {
            return std::format("Point4<{}, {}, {}, {}>", value.x, value.y, value.z, value.w);
        }
    };
}

TEST_CASE("Color tests", "[color]") {
    auto c1 = Color(1,2,3);
    auto c2 = Color(9,8,7);
    auto c3 = c1;

    SECTION("Color equality") {
        REQUIRE(c1 == c1);
    }

    SECTION("Color inequality") {
        REQUIRE(c1 != c2);

        auto c1r = c1;        // Differ in red only
        c1r.r = 100;
        REQUIRE(c1 != c1r);

        auto c1g = c1;        // Differ in green only
        c1g.g = 100;
        REQUIRE(c1 != c1g);

        auto c1b = c1;        // Differ in blue only
        c1b.b = 100;
        REQUIRE(c1 != c1b);
    }

    SECTION("Color scalar scale") {
         auto c5 = c1;
         c5 *= 2.0;
         REQUIRE(c5 == Color(2,4,6));

         REQUIRE(c1*3.0 == Color(3,6,9));
         REQUIRE(3.0*c1 == Color(3,6,9));
    }

    SECTION("Color-color multiply") {
        auto c5 = c1;
        c5 *= Color(6,3,2);
        REQUIRE(c5 == Color(6,6,6));

        REQUIRE(c1 * Color(6,3,2) == Color(6,6,6));
        REQUIRE(Color(6,3,2) * c1 == Color(6,6,6));
    }

    SECTION("Color-color addition") {
    }

    SECTION("Color clamp") {
    }
}

TEST_CASE("Vector tests", "[vector4]") {
    auto v1 = Vector4(1,2,3,4);
    auto v2 = Vector4(9,8,7,6);
    REQUIRE (v1 == v1);
    REQUIRE (v1 != v2);
}

TEST_CASE("Point tests", "[point4]") {
}
