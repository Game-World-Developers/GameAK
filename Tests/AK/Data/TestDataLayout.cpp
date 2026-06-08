#include <AK/Data/DataLayout.hpp>
#include <cest.h>

namespace {

struct Position {
    float x, y;
};

struct Velocity {
    float vx, vy;
};

using namespace GameAK;
using namespace GameAK::Data;

} // namespace

int main() {
    describe("GameAK::Data::DataLayout<AOS>", {
        it("should construct over a buffer and report zero size", {
            byte buf[4096];
            DataLayout<AOS<Position, Velocity>> layout(buf, sizeof(buf));
            expect(layout.size()).toBe(0UL);
            expect(layout.empty()).toBeTruthy();
            expect(layout.max_elements()).toBe(4096UL / (sizeof(Position) + sizeof(Velocity)));
        });

        it("should allow resize and access elements", {
            byte buf[4096];
            DataLayout<AOS<Position, Velocity>> layout(buf, sizeof(buf));
            layout.resize(5);
            expect(layout.size()).toBe(5UL);

            for (usize i = 0; i < 5; ++i) {
                layout[i].get<Position>().x = static_cast<float>(i);
                layout[i].get<Position>().y = static_cast<float>(i * 2);
                layout[i].get<Velocity>().vx = static_cast<float>(i * 3);
                layout[i].get<Velocity>().vy = static_cast<float>(i * 4);
            }

            for (usize i = 0; i < 5; ++i) {
                auto ref = layout[i];
                expect(ref.get<Position>().x).toBe(static_cast<float>(i));
                expect(ref.get<Position>().y).toBe(static_cast<float>(i * 2));
                expect(ref.get<Velocity>().vx).toBe(static_cast<float>(i * 3));
                expect(ref.get<Velocity>().vy).toBe(static_cast<float>(i * 4));
            }
        });

        it("should iterate via range-for", {
            byte buf[4096];
            DataLayout<AOS<Position, Velocity>> layout(buf, sizeof(buf));
            layout.resize(3);

            layout[0].get<Position>().x = 10.0f;
            layout[1].get<Position>().x = 20.0f;
            layout[2].get<Position>().x = 30.0f;

            float sum = 0.0f;
            for (auto ref : layout) {
                sum += ref.get<Position>().x;
            }
            expect(sum).toBe(60.0f);
        });

        it("should clear and reset", {
            byte buf[4096];
            DataLayout<AOS<Position, Velocity>> layout(buf, sizeof(buf));
            layout.resize(10);
            expect(layout.size()).toBe(10UL);
            layout.clear();
            expect(layout.size()).toBe(0UL);
            expect(layout.empty()).toBeTruthy();
        });
    });

    describe("GameAK::Data::DataLayout<SOA>", {
        it("should construct, resize and access elements", {
            byte buf[4096];
            DataLayout<SOA<Position, Velocity>> layout(buf, sizeof(buf));
            expect(layout.max_elements()).toBe(4096UL / (sizeof(Position) + sizeof(Velocity)));

            layout.resize(4);
            expect(layout.size()).toBe(4UL);

            layout[0].get<Position>().x = 1.0f;
            layout[0].get<Position>().y = 2.0f;
            layout[0].get<Velocity>().vx = 3.0f;

            auto ref = layout[0];
            expect(ref.get<Position>().x).toBe(1.0f);
            expect(ref.get<Position>().y).toBe(2.0f);
            expect(ref.get<Velocity>().vx).toBe(3.0f);
        });

        it("should provide contiguous slices per component", {
            byte buf[4096];
            DataLayout<SOA<Position, Velocity>> layout(buf, sizeof(buf));
            layout.resize(8);

            for (usize i = 0; i < 8; ++i) {
                layout[i].get<Position>().x = static_cast<float>(i);
                layout[i].get<Position>().y = static_cast<float>(i * 10);
            }

            auto positions = layout.slice<Position>();
            expect(positions.size()).toBe(8UL);
            for (usize i = 0; i < 8; ++i) {
                expect(positions[i].x).toBe(static_cast<float>(i));
                expect(positions[i].y).toBe(static_cast<float>(i * 10));
            }
        });

        it("should iterate via range-for", {
            byte buf[4096];
            DataLayout<SOA<Position, Velocity>> layout(buf, sizeof(buf));
            layout.resize(3);

            layout[0].get<Velocity>().vy = 100.0f;
            layout[1].get<Velocity>().vy = 200.0f;
            layout[2].get<Velocity>().vy = 300.0f;

            float total = 0.0f;
            for (auto ref : layout) {
                total += ref.get<Velocity>().vy;
            }
            expect(total).toBe(600.0f);
        });

        it("should correctly separate component arrays in memory", {
            // Verify that different components don't overlap
            byte buf[4096];
            DataLayout<SOA<Position, Velocity>> layout(buf, sizeof(buf));
            layout.resize(2);

            layout[0].get<Position>().x  = 111.0f;
            layout[0].get<Velocity>().vx = 222.0f;

            layout[1].get<Position>().x  = 333.0f;
            layout[1].get<Velocity>().vx = 444.0f;

            expect(layout[0].get<Position>().x).toBe(111.0f);
            expect(layout[0].get<Velocity>().vx).toBe(222.0f);
            expect(layout[1].get<Position>().x).toBe(333.0f);
            expect(layout[1].get<Velocity>().vx).toBe(444.0f);
        });
    });

    describe("GameAK::Data::DataLayout<AOSOA>", {
        it("should construct with explicit chunk size", {
            byte buf[8192];
            DataLayout<AOSOA<Position, Velocity>> layout(buf, sizeof(buf), 8);
            expect(layout.chunk_size()).toBe(8UL);
            expect(layout.chunk_count()).toBeGreaterThan(0UL);
        });

        it("should construct with ExecutionProfile", {
            byte buf[8192];
            Backend::ExecutionProfile profile{32, 64, false};
            DataLayout<AOSOA<Position, Velocity>> layout(buf, sizeof(buf), profile);
            expect(layout.chunk_size()).toBe(32UL);
        });

        it("should default to chunk_size=8 when ExecutionProfile simd_width is 0", {
            byte buf[8192];
            Backend::ExecutionProfile profile{0, 64, false};
            DataLayout<AOSOA<Position, Velocity>> layout(buf, sizeof(buf), profile);
            expect(layout.chunk_size()).toBe(8UL);
        });

        it("should resize and access elements across chunk boundaries", {
            byte buf[8192];
            DataLayout<AOSOA<Position, Velocity>> layout(buf, sizeof(buf), 4);
            expect(layout.max_elements()).toBeGreaterThan(8UL);

            layout.resize(10);
            expect(layout.size()).toBe(10UL);

            // Write across two chunks (chunk_size=4, so first 4 in chunk 0, next 4 in chunk 1, last 2 in chunk 2)
            for (usize i = 0; i < 10; ++i) {
                layout[i].get<Position>().x = static_cast<float>(i * 10);
                layout[i].get<Velocity>().vx = static_cast<float>(i * 20);
            }

            for (usize i = 0; i < 10; ++i) {
                auto ref = layout[i];
                expect(ref.get<Position>().x).toBe(static_cast<float>(i * 10));
                expect(ref.get<Velocity>().vx).toBe(static_cast<float>(i * 20));
            }
        });

        it("should provide per-chunk slices", {
            byte buf[8192];
            DataLayout<AOSOA<Position, Velocity>> layout(buf, sizeof(buf), 4);
            layout.resize(10);

            for (usize i = 0; i < 10; ++i) {
                layout[i].get<Position>().y = static_cast<float>(i);
            }

            // Chunk 0: elements 0..3, Chunk 1: elements 4..7, Chunk 2: elements 8..9
            auto chunk0 = layout.chunk_slice<Position>(0);
            expect(chunk0.size()).toBe(4UL);
            expect(chunk0[0].y).toBe(0.0f);
            expect(chunk0[3].y).toBe(3.0f);

            auto chunk1 = layout.chunk_slice<Position>(1);
            expect(chunk1.size()).toBe(4UL);
            expect(chunk1[0].y).toBe(4.0f);
            expect(chunk1[3].y).toBe(7.0f);

            auto chunk2 = layout.chunk_slice<Position>(2);
            expect(chunk2.size()).toBe(2UL);
            expect(chunk2[0].y).toBe(8.0f);
            expect(chunk2[1].y).toBe(9.0f);
        });

        it("should iterate via range-for", {
            byte buf[8192];
            DataLayout<AOSOA<Position, Velocity>> layout(buf, sizeof(buf), 4);
            layout.resize(6);

            layout[0].get<Velocity>().vy = 1.0f;
            layout[1].get<Velocity>().vy = 2.0f;
            layout[2].get<Velocity>().vy = 3.0f;
            layout[3].get<Velocity>().vy = 4.0f;
            layout[4].get<Velocity>().vy = 5.0f;
            layout[5].get<Velocity>().vy = 6.0f;

            float sum = 0.0f;
            for (auto ref : layout) {
                sum += ref.get<Velocity>().vy;
            }
            expect(sum).toBe(21.0f);
        });

        it("should handle clear and reset", {
            byte buf[8192];
            DataLayout<AOSOA<Position, Velocity>> layout(buf, sizeof(buf), 8);
            layout.resize(20);
            expect(layout.size()).toBe(20UL);
            layout.clear();
            expect(layout.size()).toBe(0UL);
            expect(layout.empty()).toBeTruthy();
        });

        it("should handle chunk_size=0 as chunk_size=1", {
            byte buf[4096];
            DataLayout<AOSOA<Position>> layout(buf, sizeof(buf), 0);
            expect(layout.chunk_size()).toBe(1UL);
        });

        it("should be usable with a single component type", {
            byte buf[4096];
            DataLayout<AOSOA<Position>> layout(buf, sizeof(buf), 16);
            layout.resize(10);
            layout[5].get<Position>().x = 42.0f;
            expect(layout[5].get<Position>().x).toBe(42.0f);
        });
    });

    describe("GameAK::Data::LayoutPolicy traits", {
        it("should detect AOS policy", {
            using AOS_IF = AOS<int, float>;
            expect(LayoutTraits<AOS_IF>::kIsAOS).toBeTruthy();
            expect(LayoutTraits<AOS_IF>::kIsSOA).toBeFalsy();
            expect(LayoutTraits<AOS_IF>::kIsAOSOA).toBeFalsy();
        });

        it("should detect SOA policy", {
            using SOA_IF = SOA<int, float>;
            expect(LayoutTraits<SOA_IF>::kIsSOA).toBeTruthy();
            expect(LayoutTraits<SOA_IF>::kIsAOS).toBeFalsy();
            expect(LayoutTraits<SOA_IF>::kIsAOSOA).toBeFalsy();
        });

        it("should detect AOSOA policy", {
            using AOSOA_IF = AOSOA<int, float>;
            expect(LayoutTraits<AOSOA_IF>::kIsAOSOA).toBeTruthy();
            expect(LayoutTraits<AOSOA_IF>::kIsAOS).toBeFalsy();
            expect(LayoutTraits<AOSOA_IF>::kIsSOA).toBeFalsy();
        });
    });

    describe("GameAK::Data::ElementRef", {
        it("should allow read/write access to components", {
            Position p{10.0f, 20.0f};
            Velocity v{30.0f, 40.0f};
            ElementRef<Position, Velocity> ref(&p, &v);

            expect(ref.get<Position>().x).toBe(10.0f);
            expect(ref.get<Position>().y).toBe(20.0f);
            expect(ref.get<Velocity>().vx).toBe(30.0f);
            expect(ref.get<Velocity>().vy).toBe(40.0f);

            ref.get<Position>().x = 99.0f;
            expect(p.x).toBe(99.0f);
        });
    });

    describe("GameAK::Data::Span", {
        it("should provide array-like access", {
            float data[] = {1.0f, 2.0f, 3.0f};
            Span<float> span{data, 3};
            expect(span.size()).toBe(3UL);
            expect(span.empty()).toBeFalsy();
            expect(span[1]).toBe(2.0f);
            span[1] = 99.0f;
            expect(data[1]).toBe(99.0f);
        });

        it("should be iterable", {
            int data[] = {10, 20, 30};
            Span<int> span{data, 3};
            int sum = 0;
            for (auto v : span) {
                sum += v;
            }
            expect(sum).toBe(60);
        });

        it("should report empty correctly", {
            Span<int> empty{nullptr, 0};
            expect(empty.empty()).toBeTruthy();
            expect(empty.size()).toBe(0UL);
        });
    });

    return 0;
}
