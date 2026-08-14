if (WITH_ASM AND NOT XMRIG_ARM AND NOT XMRIG_RISCV AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(XMRIG_ASM_LIBRARY "")
    set(XMRIG_ASM_SOURCES
        src/crypto/common/Assembly.h
        src/crypto/common/Assembly.cpp
        )

    add_definitions(/DXMRIG_FEATURE_ASM)
else()
    set(XMRIG_ASM_SOURCES "")
    set(XMRIG_ASM_LIBRARY "")

    remove_definitions(/DXMRIG_FEATURE_ASM)
endif()
