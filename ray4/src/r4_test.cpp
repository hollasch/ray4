#include <format>
#include <string>
#include <catch2/catch_test_macros.hpp>
#include "r4_color.h"
#include "r4_vector.h"
#include "r4_point.h"
#include "r4_ray.h"



//__________________________________________________________________________________________________

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

//__________________________________________________________________________________________________

TEST_CASE("Color tests", "[color]") {
    auto c1 = Color(1,2,3);
    auto c2 = Color(9,8,7);
    auto c3 = c1;

    SECTION("Color equality") {
        REQUIRE(c1 == c1);
        REQUIRE_FALSE(c1 == c2);

        REQUIRE(c1 != c2);
        REQUIRE_FALSE(c1 == c2);

        auto c1r = c1;        // Differ in red only
        c1r.r = 100;
        REQUIRE(c1 != c1r);
        REQUIRE_FALSE(c1 == c1r);

        auto c1g = c1;        // Differ in green only
        c1g.g = 100;
        REQUIRE(c1 != c1g);
        REQUIRE_FALSE(c1 == c1g);

        auto c1b = c1;        // Differ in blue only
        c1b.b = 100;
        REQUIRE(c1 != c1b);
        REQUIRE_FALSE(c1 == c1b);
    }

    SECTION("Color scalar scale") {
         auto c5 = c1;
         c5 *= 2.0;
         CHECK(c5 == Color(2,4,6));

         CHECK(c1*3.0 == Color(3,6,9));
         CHECK(3.0*c1 == Color(3,6,9));
    }

    SECTION("Color-color multiply") {
        CHECK(c1 * 2 == Color(2,4,6));
        CHECK(2 * c1 == Color(2,4,6));

        auto c5 = c1;
        c5 *= Color(6,3,2);
        CHECK(c5 == Color(6,6,6));

        CHECK(c1 * Color(6,3,2) == Color(6,6,6));
        CHECK(Color(6,3,2) * c1 == Color(6,6,6));
    }

    SECTION("Color-color addition") {
        auto a = Color(1,2,3);
        a += Color(7,7,7);
        CHECK(a == Color(8,9,10));
    }

    SECTION("Color clamp") {
        auto a = Color(-10, 0.5, 10);

        CHECK(a.clamp(-5,5) == Color(-5, 0.5, 5));
        CHECK(a.clamp(0,1) == Color(0, 0.5, 1));
    }
}

//__________________________________________________________________________________________________

TEST_CASE("Vector tests", "[vector4]") {
    auto v1 = Vector4(1,2,3,4);
    auto v2 = Vector4(9,8,7,6);

    SECTION("Vector equality") {
        REQUIRE(v1 == v1);
        REQUIRE_FALSE(v1 != v1);

        REQUIRE(v1 != v2);
        REQUIRE_FALSE(v1 == v2);

        // Vectors differing in a single component.
        REQUIRE_FALSE(Vector4(1,2,3,4) == Vector4(0,2,3,4));
        REQUIRE_FALSE(Vector4(1,2,3,4) == Vector4(1,0,3,4));
        REQUIRE_FALSE(Vector4(1,2,3,4) == Vector4(1,2,0,4));
        REQUIRE_FALSE(Vector4(1,2,3,4) == Vector4(1,2,3,0));

        REQUIRE(Vector4(1,2,3,4) != Vector4(0,2,3,4));
        REQUIRE(Vector4(1,2,3,4) != Vector4(1,0,3,4));
        REQUIRE(Vector4(1,2,3,4) != Vector4(1,2,0,4));
        REQUIRE(Vector4(1,2,3,4) != Vector4(1,2,3,0));
    }

    SECTION("Vector negation") {
        CHECK(-Vector4(1,-2,3,-4) == Vector4(-1,2,-3,4));
    }

    SECTION("Vector addition") {
        CHECK(Vector4(1,2,3,4) + Vector4(5,5,5,5) == Vector4(6,7,8,9));
    }

    SECTION("Vector subtraction") {
        CHECK(Vector4(5,5,5,5) - Vector4(1,2,3,4) == Vector4(4,3,2,1));
    }

    SECTION("Vector scalar multiply") {
        CHECK(2 * Vector4(1,2,3,4) == Vector4(2,4,6,8));
        CHECK(Vector4(1,2,3,4) * 2 == Vector4(2,4,6,8));
        auto v = Vector4(1,2,3,4);
        v *= 2;
        CHECK(v == Vector4(2,4,6,8));
    }

    SECTION("Vector scalar division") {
        CHECK(Vector4(2,4,6,8) / 2 == Vector4(1,2,3,4));

        auto v = Vector4(2,4,6,8);
        v /= 2;
        CHECK(v == Vector4(1,2,3,4));
    }

    SECTION("Vector length and squared length") {
        CHECK(Vector4(0,0,0,0).normSquared() == 0);
        CHECK(Vector4(2,0,0,0).normSquared() == 4);
        CHECK(Vector4(0,2,0,0).normSquared() == 4);
        CHECK(Vector4(0,0,2,0).normSquared() == 4);
        CHECK(Vector4(0,0,0,2).normSquared() == 4);
        CHECK(Vector4(1,2,3,4).normSquared() == 30);

        CHECK(Vector4(0,0,0,0).norm() == 0);
        CHECK(Vector4(2,0,0,0).norm() == 2);
        CHECK(Vector4(0,2,0,0).norm() == 2);
        CHECK(Vector4(0,0,2,0).norm() == 2);
        CHECK(Vector4(0,0,0,2).norm() == 2);
    }

    SECTION("Vector normalization") {
        Vector4 v;

        v = Vector4(0,0,0,0);
        CHECK_FALSE(v.normalize());
        CHECK(v == Vector4(0,0,0,0));

        v = Vector4(2,2,2,2);
        CHECK(v.normalize());
        CHECK(v == Vector4(0.5, 0.5, 0.5, 0.5));
    }

    SECTION("Vector dot product") {
        CHECK(dot(Vector4(1,2,3,4), Vector4(0,0,0,0)) ==  0.0);
        CHECK(dot(Vector4(1,2,3,4), Vector4(4,3,2,1)) == 20.0);
    }

    SECTION("Vector cross product") {
        CHECK(cross(Vector4(0,0,0,0), Vector4(0,0,0,0), Vector4(0,0,0,0)) == Vector4( 0, 0, 0, 0));
        CHECK(cross(Vector4(0,0,0,1), Vector4(0,0,0,2), Vector4(1,1,1,1)) == Vector4( 0, 0, 0, 0));
        CHECK(cross(Vector4(0,0,1,0), Vector4(0,0,2,0), Vector4(1,1,1,1)) == Vector4( 0, 0, 0, 0));
        CHECK(cross(Vector4(0,1,0,0), Vector4(0,2,0,0), Vector4(1,1,1,1)) == Vector4( 0, 0, 0, 0));
        CHECK(cross(Vector4(1,0,0,0), Vector4(2,0,0,0), Vector4(1,1,1,1)) == Vector4( 0, 0, 0, 0));

        CHECK(cross(Vector4(0,0,0,1), Vector4(0,0,1,0), Vector4(0,1,0,0)) == Vector4(-1, 0, 0, 0));
        CHECK(cross(Vector4(0,0,1,0), Vector4(0,1,0,0), Vector4(1,0,0,0)) == Vector4( 0, 0, 0, 1));
        CHECK(cross(Vector4(0,1,0,0), Vector4(1,0,0,0), Vector4(0,0,0,1)) == Vector4( 0, 0,-1, 0));
        CHECK(cross(Vector4(1,0,0,0), Vector4(0,0,0,1), Vector4(0,0,1,0)) == Vector4( 0, 1, 0, 0));
    }
}

//__________________________________________________________________________________________________

TEST_CASE("Point tests", "[point4]") {
    SECTION("Point equality") {
        REQUIRE(Point4(1,2,3,4) == Point4(1,2,3,4));
        REQUIRE_FALSE(Point4(1,2,3,4) == Point4(9,8,7,6));

        REQUIRE(Point4(1,2,3,4) != Point4(9,8,7,6));
        REQUIRE_FALSE(Point4(1,2,3,4) != Point4(1,2,3,4));

        // Points differing in a single component.
        REQUIRE_FALSE(Point4(1,2,3,4) == Point4(0,2,3,4));
        REQUIRE_FALSE(Point4(1,2,3,4) == Point4(1,0,3,4));
        REQUIRE_FALSE(Point4(1,2,3,4) == Point4(1,2,0,4));
        REQUIRE_FALSE(Point4(1,2,3,4) == Point4(1,2,3,0));

        REQUIRE(Point4(1,2,3,4) != Point4(0,2,3,4));
        REQUIRE(Point4(1,2,3,4) != Point4(1,0,3,4));
        REQUIRE(Point4(1,2,3,4) != Point4(1,2,0,4));
        REQUIRE(Point4(1,2,3,4) != Point4(1,2,3,0));
    }

    SECTION("Point plus equals vector") {
        auto p = Point4(1,2,3,4);
        p += Vector4(2,3,4,5);
        CHECK(p == Point4(3,5,7,9));
    }

    SECTION("Point minus equals vector") {
        auto p = Point4(3,5,7,9);
        p -= Vector4(2,3,4,5);
        CHECK(p == Point4(1,2,3,4));
    }

    SECTION("Point to vector") {
        CHECK(Point4(1,2,3,4).toVector() == Vector4(1,2,3,4));
    }

    SECTION("Point-point subtraction") {
        CHECK(Point4(10,10,10,10) - Point4(1,2,3,4) == Vector4(9,8,7,6));
    }

    SECTION("Point-vector addition") {
        CHECK(Point4(10,10,10,10) + Vector4(4,5,6,7) == Point4(14,15,16,17));
        CHECK(Vector4(4,5,6,7) + Point4(10,10,10,10) == Point4(14,15,16,17));
    }

    SECTION("Point-vector subtraction") {
        CHECK(Point4(10,10,10,10) - Vector4(4,5,6,7) == Point4(6,5,4,3));
    }
}

//__________________________________________________________________________________________________

TEST_CASE("Ray tests", "[ray]") {
    SECTION("Ray equality") {
        REQUIRE(Ray4(Point4(1,2,3,4),Vector4(6,7,8,9)) == Ray4(Point4(1,2,3,4),Vector4(6,7,8,9)));
        REQUIRE_FALSE(Ray4(Point4(1,2,3,4),Vector4(6,7,8,9)) == Ray4(Point4(1,2,3,4),Vector4(0,7,8,9)));
        REQUIRE_FALSE(Ray4(Point4(1,2,3,4),Vector4(6,7,8,9)) == Ray4(Point4(0,2,3,4),Vector4(6,7,8,9)));

        REQUIRE(Ray4(Point4(1,2,3,4),Vector4(6,7,8,9)) != Ray4(Point4(1,2,3,4),Vector4(0,7,8,9)));
        REQUIRE(Ray4(Point4(1,2,3,4),Vector4(6,7,8,9)) != Ray4(Point4(0,2,3,4),Vector4(6,7,8,9)));
        REQUIRE_FALSE(Ray4(Point4(1,2,3,4),Vector4(6,7,8,9)) != Ray4(Point4(1,2,3,4),Vector4(6,7,8,9)));
    }

    SECTION("Ray evaluation") {
        Ray4 r {Point4(0,0,0,0), Vector4(1,2,3,4)};

        CHECK(r( 0.0) == Point4(0,0,0,0));
        CHECK(r( 1.0) == Point4(1,2,3,4));
        CHECK(r( 2.0) == Point4(2,4,6,8));
        CHECK(r(-1.0) == Point4(-1,-2,-3,-4));
    }
}
