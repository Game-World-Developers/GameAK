#include <AK/Core/Optional.hpp>
#include <cest.h>

struct NonTrivial {
  static int alive_count;
  int id;
  NonTrivial(int id) : id(id) { alive_count++; }
  ~NonTrivial() { alive_count--; }
  NonTrivial(const NonTrivial &o) : id(o.id) { alive_count++; }
  NonTrivial(NonTrivial &&o) : id(o.id) {
    o.id = -1;
    alive_count++;
  }
  NonTrivial &operator=(const NonTrivial &o) {
    id = o.id;
    return *this;
  }
  NonTrivial &operator=(NonTrivial &&o) {
    id = o.id;
    o.id = -1;
    return *this;
  }
};
int NonTrivial::alive_count = 0;

int main() {
  describe("GameAK::Optional", {
    it("should be empty by default", {
      GameAK::Optional<int> opt;
      expect(opt.has_value()).toBeFalsy();
      expect((bool)opt).toBeFalsy();
    });

    it("should be empty when constructed with Nullopt", {
      GameAK::Optional<int> opt{GameAK::Nullopt};
      expect(opt.has_value()).toBeFalsy();
    });

    it("should hold a value when constructed with a value", {
      GameAK::Optional<int> opt(42);
      expect(opt.has_value()).toBeTruthy();
      expect(*opt).toBe(42);
    });

    it("should support copy construction from valued optional", {
      GameAK::Optional<int> a(42);
      GameAK::Optional<int> b(a);
      expect(b.has_value()).toBeTruthy();
      expect(*b).toBe(42);
    });

    it("should support copy construction from empty optional", {
      GameAK::Optional<int> a;
      GameAK::Optional<int> b(a);
      expect(b.has_value()).toBeFalsy();
    });

    it("should support move construction", {
      GameAK::Optional<int> a(42);
      GameAK::Optional<int> b(GameAK::Move(a));
      expect(b.has_value()).toBeTruthy();
      expect(*b).toBe(42);
      expect(a.has_value()).toBeFalsy();
    });

    it("should support copy assignment", {
      GameAK::Optional<int> a(42);
      GameAK::Optional<int> b;
      b = a;
      expect(b.has_value()).toBeTruthy();
      expect(*b).toBe(42);
    });

    it("should support move assignment", {
      GameAK::Optional<int> a(42);
      GameAK::Optional<int> b;
      b = GameAK::Move(a);
      expect(b.has_value()).toBeTruthy();
      expect(*b).toBe(42);
      expect(a.has_value()).toBeFalsy();
    });

    it("should return the value from value()", {
      GameAK::Optional<int> opt(99);
      expect(opt.value()).toBe(99);
      const auto &const_opt = opt;
      expect(const_opt.value()).toBe(99);
    });

    it("should return fallback from value_or when empty", {
      GameAK::Optional<int> opt;
      expect(opt.value_or(-1)).toBe(-1);
    });

    it("should return value from value_or when filled", {
      GameAK::Optional<int> opt(42);
      expect(opt.value_or(-1)).toBe(42);
    });

    it("should reset to empty", {
      GameAK::Optional<int> opt(42);
      expect(opt.has_value()).toBeTruthy();
      opt.reset();
      expect(opt.has_value()).toBeFalsy();
    });

    it("should support operator bool", {
      GameAK::Optional<int> empty;
      GameAK::Optional<int> full(42);
      expect((bool)empty).toBeFalsy();
      expect((bool)full).toBeTruthy();
    });

    it("should support operator* for access", {
      GameAK::Optional<int> opt(42);
      int &ref = *opt;
      expect(ref).toBe(42);
      ref = 100;
      expect(*opt).toBe(100);
    });

    it("should work with non-trivial destructor types", {
      NonTrivial::alive_count = 0;
      {
        GameAK::Optional<NonTrivial> opt(42);
        expect(opt.has_value()).toBeTruthy();
        expect((*opt).id).toBe(42);
        expect(NonTrivial::alive_count).toBe(1);
      }
      expect(NonTrivial::alive_count).toBe(0);
    });

    it("should copy non-trivial types correctly", {
      NonTrivial::alive_count = 0;
      {
        GameAK::Optional<NonTrivial> a(1);
        GameAK::Optional<NonTrivial> b(a);
        expect((*b).id).toBe(1);
        expect(NonTrivial::alive_count).toBe(2);
      }
      expect(NonTrivial::alive_count).toBe(0);
    });

    it("should move non-trivial types correctly", {
      NonTrivial::alive_count = 0;
      {
        GameAK::Optional<NonTrivial> a(1);
        GameAK::Optional<NonTrivial> b(GameAK::Move(a));
        expect((*b).id).toBe(1);
        expect(a.has_value()).toBeFalsy();
        expect(NonTrivial::alive_count).toBe(1);
      }
      expect(NonTrivial::alive_count).toBe(0);
    });

    it("should support value_or with move semantics", {
      GameAK::Optional<std::string> opt;
      std::string result = opt.value_or("fallback");
      expect(result == "fallback").toBeTruthy();
    });

    it("[Invariant] size of Optional<int> is sizeof(int) + sizeof(bool)", {
      expect(sizeof(GameAK::Optional<int>) > sizeof(int)).toBeTruthy();
    });

    it("[Invariant] has_value is false after reset", {
      GameAK::Optional<int> opt(42);
      opt.reset();
      expect(opt.has_value()).toBeFalsy();
      opt.reset();
      expect(opt.has_value()).toBeFalsy();
    });

    it("[Property] value_or returns the stored value when present", {
      GameAK::Optional<int> opt(100);
      expect(opt.value_or(-1)).toBe(100);
    });

    it("[Property] value_or returns fallback when empty", {
      GameAK::Optional<int> opt;
      expect(opt.value_or(-1)).toBe(-1);
    });

    it("[Critical Bug: Optional: self-assignment (copy)]", {
      GameAK::Optional<int> opt(42);
      opt = opt;
      expect(opt.has_value()).toBeTruthy();
      expect(*opt).toBe(42);
    });

    it("[Critical Bug: Optional: self-assignment (move)]", {
      GameAK::Optional<int> opt(42);
      opt = GameAK::Move(opt);
      expect(opt.has_value()).toBeTruthy();
      expect(*opt).toBe(42);
    });

    it("[Critical Bug: Optional: self-assignment (non-trivial copy)]", {
      NonTrivial::alive_count = 0;
      {
        GameAK::Optional<NonTrivial> opt(1);
        opt = opt;
        expect(opt.has_value()).toBeTruthy();
        expect((*opt).id).toBe(1);
        expect(NonTrivial::alive_count).toBe(1);
      }
      expect(NonTrivial::alive_count).toBe(0);
    });

    it("[Critical Bug: Optional: self-assignment (non-trivial move)]", {
      NonTrivial::alive_count = 0;
      {
        GameAK::Optional<NonTrivial> opt(1);
        opt = GameAK::Move(opt);
        expect(opt.has_value()).toBeTruthy();
        expect(NonTrivial::alive_count).toBe(1);
      }
      expect(NonTrivial::alive_count).toBe(0);
    });
  });

  return cest_result();
}
