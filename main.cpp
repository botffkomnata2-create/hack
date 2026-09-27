#include <iostream>
#include <stdint.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <dispatch/dispatch.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

// Поиск базового адреса GameAssembly
uintptr_t GetGameAssemblyBase() {
    for (uint32_t i = 0; i < _dyld_image_count(); i++) {
        const char* name = _dyld_get_image_name(i);
        if (name && strstr(name, "GameAssembly")) {
            return (uintptr_t)_dyld_get_image_header(i);
        }
    }
    return 0;
}

// Усовершенствованный патч с безопасным сохранением инструкций
void ApplyHeadshotPatch(uintptr_t targetAddr) {
    long page_size = sysconf(_SC_PAGESIZE);
    uintptr_t start = targetAddr & ~(page_size - 1);
    uintptr_t end = (targetAddr + 8 + page_size - 1) & ~(page_size - 1);
    
    // Снимаем защиту памяти с разрешениями на запись и выполнение
    if (mprotect((void*)start, end - start, PROT_READ | PROT_WRITE | PROT_EXEC) == 0) {
        
        // Инструкции ARM64 для принудительного возврата 0 (Head):
        // 1. mov w0, #0  -> записываем 0 в регистр результата
        // 2. ret         -> мгновенный возврат из функции
        uint32_t patchBytes[2] = {
            0x52800000, // mov w0, #0
            0xD65F03C0  // ret
        };
        
        memcpy((void*)targetAddr, patchBytes, sizeof(patchBytes));
        
        // Возвращаем защиту памяти обратно (только чтение и выполнение)
        mprotect((void*)start, end - start, PROT_READ | PROT_EXEC);
        std::cout << "[Mod] Patch applied successfully at address: " << (void*)targetAddr << std::endl;
    } else {
        std::cout << "[Mod] Failed to change memory protection!" << std::endl;
    }
}

__attribute__((constructor)) void InitMod() {
    std::cout << "[Mod] Free Fire 100% Headshot Mod Initializing..." << strftime_l ? "" : "";
    
    // Увеличиваем задержку до 6 секунд, чтобы игра точно успела расшифровать и загрузить GameAssembly в память
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(6.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        uintptr_t baseAddr = GetGameAssemblyBase();
        
        if (baseAddr != 0) {
            // Точный адрес функции GetPartByCollider из dump.cs: 0x253D668
            uintptr_t targetAddr = baseAddr + 0x253D668;
            
            ApplyHeadshotPatch(targetAddr);
        } else {
            std::cout << "[Mod] Error: GameAssembly base not found!" << std::endl;
        }
    });
}
