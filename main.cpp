#include <iostream>
#include <stdint.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <dispatch/dispatch.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

// Поиск базового адреса GameAssembly в памяти iOS (обход ASLR)
uintptr_t GetGameAssemblyBase() {
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const char* name = _dyld_get_image_name(i); // Исправлено название функции
        if (name && strstr(name, "GameAssembly")) {
            return (uintptr_t)_dyld_get_image_header(i); // Исправлено название функции
        }
    }
    return 0;
}

// Безопасная перезапись памяти под архитектуру ARM64
void WriteArm64Patch(uintptr_t destination) {
    long page_size = sysconf(_SC_PAGESIZE);
    uintptr_t start = destination & ~(page_size - 1);
    uintptr_t end = (destination + 4 + page_size - 1) & ~(page_size - 1);
    
    // Снимаем защиту памяти
    mprotect((void*)start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC);
    
    // Инструкции ARM64:
    // mov w0, #0 (возвращает 0, что соответствует HitPart::Head)
    // ret (возврат из функции)
    uint32_t patchBytes[2] = {
        0x52800000, 
        0xD65F03C0  
    };
    
    memcpy((void*)destination, patchBytes, sizeof(patchBytes));
    
    // Возвращаем защиту памяти обратно
    mprotect((void*)start, end - start, PROT_READ | PROT_EXEC);
}

__attribute__((constructor)) void InitMod() {
    std::cout << "[Mod] Free Fire MagicHead Active!" << std::endl;
    
    // Задержка 4 секунды, чтобы игра успела полностью загрузить GameAssembly
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(4.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        uintptr_t baseAddr = GetGameAssemblyBase();
        
        if (baseAddr != 0) {
            // Точный адрес функции GetPartByCollider из твоего dump.cs: 0x253D668
            uintptr_t targetAddr = baseAddr + 0x253D668;
            
            WriteArm64Patch(targetAddr);
            std::cout << "[Mod] Successfully patched target at: " << (void*)targetAddr << std::endl;
        } else {
            std::cout << "[Mod] Error: GameAssembly base not found!" << std::endl;
        }
    });
}
