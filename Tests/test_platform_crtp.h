#pragma once

inline void run_platform_crtp_tests() {
    using namespace gameak::core;

describe("Platform CRTP", {
    it("CpuPlatform memcopy works", {
        GenericCpu cpu;
        char src[64];
        char dst[64] = {};
        for (int i = 0; i < 64; ++i) src[i] = static_cast<char>(i);
        cpu.memcopy(dst, src, 64);
        for (int i = 0; i < 64; ++i)
            expect(dst[i] == src[i]).toBeTruthy();
    });

    it("CpuPlatform memzero works", {
        GenericCpu cpu;
        char buf[64];
        for (int i = 0; i < 64; ++i) buf[i] = static_cast<char>(0xFF);
        cpu.memzero(buf, 64);
        for (int i = 0; i < 64; ++i)
            expect(buf[i] == 0).toBeTruthy();
    });

    it("Platform singleton provides CPU operations", {
        auto& p = Platform::instance();
        char src[16] = {1};
        char dst[16] = {};
        p.cpu.memcopy(dst, src, 16);
        expect(dst[0] == 1).toBeTruthy();
    });

    it("Platform singleton detects environment", {
        auto& p = Platform::instance();
        expect(p.info.compiler != Compiler::Unknown).toBeTruthy();
        expect(p.info.arch != Architecture::Unknown).toBeTruthy();
        expect(p.info.os != OperatingSystem::Unknown).toBeTruthy();
    });

    it("CompilerPlatform barrier compiles and runs", {
        GenericCompiler compiler;
        compiler.compiler_barrier();
    });

    it("OsPlatform debug_output produces output", {
        GenericOs os;
        os.debug_output("CRTP test message");
    });

    it("custom CpuPlatform derived class overrides behavior", {
        struct TestCpu : CpuPlatform<TestCpu> {
            bool called = false;
            void memcopy_impl(void*, const void*, size_t) { called = true; }
            void memzero_impl(void*, size_t) {}
        };
        TestCpu cpu;
        char src[4]{};
        char dst[4]{};
        cpu.memcopy(dst, src, 4);
        expect(cpu.called).toBeTruthy();
    });
});
}
