#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

namespace RESANA {

    class Log {
    public:
        static void Init();

        static std::shared_ptr<spdlog::logger> &GetCoreLogger() { return sCoreLogger; }

    private:
        static std::shared_ptr<spdlog::logger> sCoreLogger;
    };

} // RESANA

// Core log macros
#define RS_CORE_TRACE(...)          ::RESANA::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define RS_CORE_INFO(...)           ::RESANA::Log::GetCoreLogger()->info(__VA_ARGS__)
#define RS_CORE_WARN(...)           ::RESANA::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define RS_CORE_ERROR(...)          ::RESANA::Log::GetCoreLogger()->error(__VA_ARGS__)
#define RS_CORE_CRITICAL(...)       ::RESANA::Log::GetCoreLogger()->critical(__VA_ARGS__)
