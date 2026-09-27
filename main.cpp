#include <iostream>
#include <stdint.h>
#include <dlfcn.h>
#include <mach-o/dyld.h>
#include <dispatch/dispatch.h>
#include <stdlib.h>

__attribute__((constructor)) void InitMod() {
    // Выводим сообщение в консоль (если есть логгер)
    std::cout << "[Mod Test] DYLIB IS LOADED SUCCESSFULLY!" << std::endl;
    
    // Принудительно крашим игру через 2 секунды после запуска
    dispatch_after(dispatch_time(DISPATCH_TIME_NOW, (int64_t)(2.0 * NSEC_PER_SEC)), dispatch_get_main_queue(), ^{
        // Вызываем аварийное завершение процесса
        abort();
    });
}
