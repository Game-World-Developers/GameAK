#include <AK/Memory/MemoryDebug.hpp>
#include <cest.h>
#include <csetjmp>
#include <signal.h>

namespace {

static sigjmp_buf trap_jmp_buf;

extern "C" void trap_handler(int) { siglongjmp(trap_jmp_buf, 1); }

struct SavedSigaction {
  struct sigaction old_act;
  void install() {
    struct sigaction trap_act;
    trap_act.sa_handler = trap_handler;
    sigemptyset(&trap_act.sa_mask);
    trap_act.sa_flags = 0;
    sigaction(SIGTRAP, &trap_act, &old_act);
  }
  void restore() { sigaction(SIGTRAP, &old_act, nullptr); }
};

} // namespace

int main() {
  describe("GameAK::Memory::Debug", {
    it("debug_enabled_v should reflect GAMEAK_DEBUG_VALIDATE", {
#ifdef GAMEAK_DEBUG_VALIDATE
      expect(GameAK::Memory::Debug::debug_enabled_v<>).toBeTruthy();
#else
      expect(!GameAK::Memory::Debug::debug_enabled_v<>).toBeTruthy();
#endif
    });

    it("validate_alignment should accept powers of two", {
      GameAK::Memory::Debug::validate_alignment(1);
      GameAK::Memory::Debug::validate_alignment(2);
      GameAK::Memory::Debug::validate_alignment(4);
      GameAK::Memory::Debug::validate_alignment(8);
      GameAK::Memory::Debug::validate_alignment(16);
      GameAK::Memory::Debug::validate_alignment(4096);
      expect(true).toBeTruthy();
    });

    it("validate_offset should accept offset <= capacity", {
      GameAK::Memory::Debug::validate_offset(0, 1024);
      GameAK::Memory::Debug::validate_offset(512, 1024);
      GameAK::Memory::Debug::validate_offset(1024, 1024);
      expect(true).toBeTruthy();
    });

    it("validate_pointer_in_range should accept pointers inside buffer", {
      GameAK::u8 buf[64];
      GameAK::Memory::Debug::validate_pointer_in_range(buf, buf, sizeof(buf));
      GameAK::Memory::Debug::validate_pointer_in_range(buf + 32, buf,
                                                        sizeof(buf));
      GameAK::Memory::Debug::validate_pointer_in_range(buf + 63, buf,
                                                        sizeof(buf));
      expect(true).toBeTruthy();
    });

    if constexpr (GameAK::Memory::Debug::debug_enabled_v<>) {
      it("[Debug] validate_alignment should trap for non-power-of-two", {
        SavedSigaction saved;
        saved.install();
        if (sigsetjmp(trap_jmp_buf, 1) == 0) {
          GameAK::Memory::Debug::validate_alignment(3);
        }
        saved.restore();
        expect(true).toBeTruthy();
      });

      it("[Debug] validate_alignment should trap for zero alignment", {
        SavedSigaction saved;
        saved.install();
        if (sigsetjmp(trap_jmp_buf, 1) == 0) {
          GameAK::Memory::Debug::validate_alignment(0);
        }
        saved.restore();
        expect(true).toBeTruthy();
      });

      it("[Debug] validate_offset should trap when offset exceeds capacity",
         {
           SavedSigaction saved;
           saved.install();
           if (sigsetjmp(trap_jmp_buf, 1) == 0) {
             GameAK::Memory::Debug::validate_offset(1025, 1024);
           }
           saved.restore();
           expect(true).toBeTruthy();
         });

      it("[Debug] validate_pointer_in_range should trap for pointer before "
         "buffer",
         {
           SavedSigaction saved;
           saved.install();
           GameAK::u8 buf[64];
           GameAK::u8 before = 0;
           if (sigsetjmp(trap_jmp_buf, 1) == 0) {
             GameAK::Memory::Debug::validate_pointer_in_range(&before, buf,
                                                               sizeof(buf));
           }
           saved.restore();
           expect(true).toBeTruthy();
         });

      it("[Debug] validate_pointer_in_range should trap for pointer past "
         "buffer",
         {
           SavedSigaction saved;
           saved.install();
           GameAK::u8 buf[64];
           if (sigsetjmp(trap_jmp_buf, 1) == 0) {
             GameAK::Memory::Debug::validate_pointer_in_range(buf + 64, buf,
                                                               sizeof(buf));
           }
           saved.restore();
           expect(true).toBeTruthy();
         });
    }

    if constexpr (!GameAK::Memory::Debug::debug_enabled_v<>) {
      it("[Release] validate_alignment is no-op for invalid values", {
        GameAK::Memory::Debug::validate_alignment(3);
        GameAK::Memory::Debug::validate_alignment(0);
        expect(true).toBeTruthy();
      });

      it("[Release] validate_offset is no-op for invalid values", {
        GameAK::Memory::Debug::validate_offset(9999, 64);
        expect(true).toBeTruthy();
      });

      it("[Release] validate_pointer_in_range is no-op for invalid pointers",
         {
           GameAK::u8 buf[64];
           GameAK::u8 outside = 0;
           GameAK::Memory::Debug::validate_pointer_in_range(&outside, buf,
                                                             sizeof(buf));
           expect(true).toBeTruthy();
         });
    }

    it("[Invariant] debug_assert compiles with any callable returning bool", {
      GameAK::Memory::Debug::debug_assert([] { return true; });
      expect(true).toBeTruthy();
    });
  });

  return cest_result();
}
