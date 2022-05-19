#include "Log.h"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace RESANA {

    std::shared_ptr<spdlog::logger> Log::sCoreLogger;

    void Log::Init() {
        spdlog::set_pattern("%^%T %n[%l]: %v%$");
        sCoreLogger = spdlog::stderr_color_mt("RESANA");
        sCoreLogger->set_level(spdlog::level::trace);
    }

} // RESANA