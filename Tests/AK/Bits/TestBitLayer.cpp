#include <AK/Core/Bits/BitLayer.hpp>
#include <cest.h>

namespace {

using DefaultLayer = GameAK::Bits::BitLayer<GameAK::Backend::ScalarBuiltinsBackend>;

static bool call_test(const DefaultLayer &bl, GameAK::usize layer_idx,
                      GameAK::usize bit) {
  return (bl.layer(layer_idx).test)(bit);
}

} // namespace

int main() {
  describe("GameAK::Bits::BitLayer (ScalarBuiltinsBackend)", {
    it("should report correct layer count and words per layer", {
      GameAK::u64 storage[6] = {};
      DefaultLayer bl(storage, 3, 2);

      expect(bl.layers() == 3).toBeTruthy();
      expect(bl.words_per_layer() == 2).toBeTruthy();
    });

    it("should provide writable layer views", {
      GameAK::u64 storage[4] = {};
      DefaultLayer bl(storage, 2, 2);

      bl.layer(0).set(5);
      expect(call_test(bl, 0, 5)).toBeTruthy();
    });

    it("should provide readable const layer views", {
      GameAK::u64 storage[4] = {};
      DefaultLayer bl(storage, 2, 2);

      bl.layer(0).set(42);

      const auto &cbl = bl;
      expect(call_test(cbl, 0, 42)).toBeTruthy();
    });

    it("[Property] layer(index) set(x) => test(x) == true for all bits in "
       "range",
       {
         GameAK::u64 storage[8] = {};
         DefaultLayer bl(storage, 2, 4);

         for (GameAK::usize i = 0; i < 256; ++i) {
           bl.layer(0).set(i);
           expect(call_test(bl, 0, i)).toBeTruthy();
         }
       });

    it("[Property] clear(x) => test(x) == false after set(x)", {
      GameAK::u64 storage[4] = {};
      DefaultLayer bl(storage, 2, 2);

      for (GameAK::usize i = 0; i < 128; ++i) {
        bl.layer(0).set(i);
        bl.layer(0).clear(i);
        expect(call_test(bl, 0, i)).toBeFalsy();
      }
    });

    it("should clear all bits in a specific layer", {
      GameAK::u64 storage[4] = {};
      DefaultLayer bl(storage, 2, 2);

      bl.layer(0).set(10);
      bl.layer(0).set(50);
      bl.layer(0).set(100);
      bl.clear_layer(0);

      expect(call_test(bl, 0, 10)).toBeFalsy();
      expect(call_test(bl, 0, 50)).toBeFalsy();
      expect(call_test(bl, 0, 100)).toBeFalsy();
    });

    it("[Property] clear_layer does not affect other layers", {
      GameAK::u64 storage[4] = {};
      DefaultLayer bl(storage, 2, 2);

      bl.layer(0).set(10);
      bl.layer(1).set(20);
      bl.clear_layer(0);

      expect(call_test(bl, 0, 10)).toBeFalsy();
      expect(call_test(bl, 1, 20)).toBeTruthy();
    });

    it("should clear all bits across all layers", {
      GameAK::u64 storage[6] = {};
      DefaultLayer bl(storage, 3, 2);

      bl.layer(0).set(10);
      bl.layer(1).set(20);
      bl.layer(2).set(30);
      bl.clear_all();

      expect(call_test(bl, 0, 10)).toBeFalsy();
      expect(call_test(bl, 1, 20)).toBeFalsy();
      expect(call_test(bl, 2, 30)).toBeFalsy();
    });

    it("and_layers should compute bitwise AND of two layers into destination", {
      GameAK::u64 storage[6] = {};
      DefaultLayer bl(storage, 3, 2);

      bl.layer(0).set(2);
      bl.layer(0).set(3);
      bl.layer(0).set(64);
      bl.layer(0).set(65);

      bl.layer(1).set(1);
      bl.layer(1).set(3);
      bl.layer(1).set(64);
      bl.layer(1).set(66);

      bl.and_layers(2, 0, 1);

      expect(storage[4] == 0b1000UL).toBeTruthy();
      expect(storage[5] == 0b0001UL).toBeTruthy();
    });

    it("or_layers should compute bitwise OR of two layers into destination", {
      GameAK::u64 storage[6] = {};
      DefaultLayer bl(storage, 3, 2);

      bl.layer(0).set(2);
      bl.layer(0).set(3);
      bl.layer(0).set(64);
      bl.layer(0).set(65);

      bl.layer(1).set(1);
      bl.layer(1).set(3);
      bl.layer(1).set(64);
      bl.layer(1).set(66);

      bl.or_layers(2, 0, 1);

      expect(storage[4] == 0b1110UL).toBeTruthy();
      expect(storage[5] == 0b0111UL).toBeTruthy();
    });

    it("xor_layers should compute bitwise XOR of two layers into destination", {
      GameAK::u64 storage[6] = {};
      DefaultLayer bl(storage, 3, 2);

      bl.layer(0).set(2);
      bl.layer(0).set(3);
      bl.layer(0).set(64);
      bl.layer(0).set(65);

      bl.layer(1).set(1);
      bl.layer(1).set(3);
      bl.layer(1).set(64);
      bl.layer(1).set(66);

      bl.xor_layers(2, 0, 1);

      expect(storage[4] == 0b0110UL).toBeTruthy();
      expect(storage[5] == 0b0110UL).toBeTruthy();
    });

    it("[Property] and_layers with dst == a is safe", {
      GameAK::u64 storage[6] = {};
      DefaultLayer bl(storage, 3, 2);

      bl.layer(0).set(2);
      bl.layer(0).set(3);
      bl.layer(0).set(64);
      bl.layer(0).set(65);

      bl.layer(1).set(1);
      bl.layer(1).set(3);
      bl.layer(1).set(64);
      bl.layer(1).set(66);

      bl.and_layers(0, 0, 1);

      expect(call_test(bl, 0, 3)).toBeTruthy();
      expect(call_test(bl, 0, 64)).toBeTruthy();
      expect(call_test(bl, 0, 2)).toBeFalsy();
      expect(call_test(bl, 0, 65)).toBeFalsy();
    });

    it("[Property] and_layers with dst == b is safe", {
      GameAK::u64 storage[6] = {};
      DefaultLayer bl(storage, 3, 2);

      bl.layer(0).set(2);
      bl.layer(0).set(3);
      bl.layer(0).set(64);
      bl.layer(0).set(65);

      bl.layer(1).set(1);
      bl.layer(1).set(3);
      bl.layer(1).set(64);
      bl.layer(1).set(66);

      bl.and_layers(1, 0, 1);

      expect(call_test(bl, 1, 3)).toBeTruthy();
      expect(call_test(bl, 1, 64)).toBeTruthy();
      expect(call_test(bl, 1, 1)).toBeFalsy();
      expect(call_test(bl, 1, 66)).toBeFalsy();
    });

    it("[Property] xor_layers with a == b yields zero", {
      GameAK::u64 storage[6] = {};
      DefaultLayer bl(storage, 3, 2);

      bl.layer(0).set(3);
      bl.layer(0).set(64);

      bl.xor_layers(2, 0, 0);

      expect(call_test(bl, 2, 3)).toBeFalsy();
      expect(call_test(bl, 2, 64)).toBeFalsy();
    });

    it("[Property] or_layers with a == b yields a", {
      GameAK::u64 storage[6] = {};
      DefaultLayer bl(storage, 3, 2);

      bl.layer(0).set(3);
      bl.layer(0).set(64);

      bl.or_layers(2, 0, 0);

      expect(call_test(bl, 2, 3)).toBeTruthy();
      expect(call_test(bl, 2, 64)).toBeTruthy();
    });

    it("should handle word boundaries across multi-word layers", {
      GameAK::u64 storage[12] = {};
      DefaultLayer bl(storage, 2, 6);

      bl.layer(0).set(63);
      bl.layer(0).set(64);
      bl.layer(0).set(191);
      bl.layer(0).set(192);

      expect(call_test(bl, 0, 63)).toBeTruthy();
      expect(call_test(bl, 0, 64)).toBeTruthy();
      expect(call_test(bl, 0, 191)).toBeTruthy();
      expect(call_test(bl, 0, 192)).toBeTruthy();
    });

    it("[Edge] single-word layers", {
      GameAK::u64 storage[3] = {};
      DefaultLayer bl(storage, 3, 1);

      bl.layer(0).set(0);
      bl.layer(1).set(63);
      bl.layer(2).set(31);

      expect(call_test(bl, 0, 0)).toBeTruthy();
      expect(call_test(bl, 1, 63)).toBeTruthy();
      expect(call_test(bl, 2, 31)).toBeTruthy();
    });

    it("[Edge] single layer", {
      GameAK::u64 storage[4] = {};
      DefaultLayer bl(storage, 1, 4);

      bl.layer(0).set(0);
      bl.layer(0).set(255);

      expect(call_test(bl, 0, 0)).toBeTruthy();
      expect(call_test(bl, 0, 255)).toBeTruthy();
    });

    it("[Edge] many layers", {
      GameAK::u64 storage[10] = {};
      DefaultLayer bl(storage, 10, 1);

      for (GameAK::usize i = 0; i < 10; ++i) {
        bl.layer(i).set(i * 6);
      }

      for (GameAK::usize i = 0; i < 10; ++i) {
        expect(call_test(bl, i, i * 6)).toBeTruthy();
      }
    });

    it("should compute popcount for a layer", {
      GameAK::u64 storage[4] = {};
      DefaultLayer bl(storage, 2, 2);

      bl.layer(0).set(0);
      bl.layer(0).set(2);
      bl.layer(0).set(5);
      bl.layer(0).set(63);
      bl.layer(0).set(64);
      bl.layer(0).set(100);

      expect(bl.popcount_layer(0) == 6).toBeTruthy();
    });

    it("[Invariant] layers are independent — no bit leakage", {
      GameAK::u64 storage[12] = {};
      DefaultLayer bl(storage, 4, 3);

      for (GameAK::usize i = 0; i < 192; ++i) {
        bl.layer(0).set(i);
      }

      for (GameAK::usize i = 0; i < 192; ++i) {
        expect(call_test(bl, 0, i)).toBeTruthy();
      }

      for (GameAK::usize layer = 1; layer < 4; ++layer) {
        for (GameAK::usize i = 0; i < 192; ++i) {
          expect(call_test(bl, layer, i)).toBeFalsy();
        }
      }
    });

    it("[Stress] repeated layer operations across many cycles", {
      GameAK::u64 storage[12] = {};
      DefaultLayer bl(storage, 3, 4);

      for (GameAK::usize i = 0; i < 10000; ++i) {
        bl.clear_all();
        bl.layer(0).set(i % 256);
        bl.layer(1).set((i + 37) % 256);
        bl.layer(2).set((i + 89) % 256);
        bl.or_layers(0, 1, 2);
        bl.and_layers(1, 0, 2);
        bl.xor_layers(2, 0, 1);
      }

      expect(bl.layers() == 3).toBeTruthy();
      expect(bl.words_per_layer() == 4).toBeTruthy();
    });
  });

  return cest_result();
}
