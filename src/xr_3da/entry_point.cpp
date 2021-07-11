
#include "stdafx.h"
#include "resource.h"
#if defined(XR_PLATFORM_WINDOWS)
#include "AccessibilityShortcuts.hpp"
#include <shellapi.h>
#include <windows.h>
#elif defined(XR_PLATFORM_LINUX)
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#endif
#include "xrEngine/main.h"
#include "xrEngine/splash.h"
#include "xrCore/CLOptions.h"
#include <SDL.h>

//#define PROFILE_TASK_SYSTEM

#ifdef PROFILE_TASK_SYSTEM
#include "xrCore/Threading/ParallelForEach.hpp"
#endif

// Always request high performance GPU
extern "C"
{
// https://docs.nvidia.com/gameworks/content/technologies/desktop/optimus.htm
XR_EXPORT u32 NvOptimusEnablement = 0x00000001; // NVIDIA Optimus

// https://gpuopen.com/amdpowerxpressrequesthighperformance/
XR_EXPORT u32 AmdPowerXpressRequestHighPerformance = 0x00000001; // PowerXpress or Hybrid Graphics
}

extern CLOption<pstr> fsltx_path;


bool HandleArguments(int argc, char *argv[])
{
    try
    {
        ParseCommandLine(argc, argv);
        CLCheckAllArguments();
    }
    catch (CLOptionMissing &e)
    {
        Msg("Command Line Options error: Missing option %s", e.what());
        return false;
    }
    catch (CLOptionParam &e)
    {
        Msg("Command Line Options error: Missing parameter for option %s", e.what());
        return false;
    }
    catch (std::invalid_argument &e)
    {
        Msg("Command Line Options error: Invalid integer argument %s", e.what());
        return false;
    }
    catch (std::out_of_range &e)
    {
        Msg("Command Line Options error: Invalid integer out of range %s", e.what());
        return false;
    }

    return true;
}

int entry_point(pcstr commandLine)
{

    static CLOption<bool> clhelp("-help", "print this help and exit", false);

    if (clhelp.IsProvided()) {
        CLPrintAllHelp();
        return 0;
    }

    xrDebug::Initialize();
    R_ASSERT3(SDL_Init(SDL_INIT_VIDEO) == 0, "Unable to initialize SDL", SDL_GetError());
    SDL_SetHint(SDL_HINT_WINDOWS_DISABLE_THREAD_NAMING, "0");

    static CLOption<bool> no_splash("-nosplash", "no splash screen", false);
    static CLOption<bool> splash_notop("-splashnotop", "splash no top", false);
    if (!no_splash.OptionValue())
    {
        const bool topmost = !splash_notop.OptionValue();
#ifndef PROFILE_TASK_SYSTEM
        splash::show(topmost);
#endif
    }

    static CLOption<bool> sv_dedicated("-dedicated", "run dedicated server", false);
    GEnv.isDedicatedServer = sv_dedicated.OptionValue();

#ifdef XR_PLATFORM_WINDOWS
    AccessibilityShortcuts shortcuts;
    if (!GEnv.isDedicatedServer)
        shortcuts.Disable();
#endif

#ifdef PROFILE_TASK_SYSTEM
    Core.Initialize("OpenXRay",commandLine, nullptr, false,
                    fsltx_path.IsProvided() ? fsltx_path.OptionValue() : nullptr);

    const auto task = [](const TaskRange<int>&){};

    constexpr int task_count = 1048576;
    constexpr int iterations = 250;
    u64 results[iterations];

    CTimer timer;
    for (int i = 0; i < iterations; ++i)
    {
        timer.Start();
        xr_parallel_for(TaskRange(0, task_count, 1), task);
        results[i] = timer.GetElapsed_ns();
    }

    u64 min = std::numeric_limits<u64>::max();
    u64 average{};
    for (int i = 0; i < iterations; ++i)
    {
        min = std::min(min, results[i]);
        average += results[i] / 1000;
        Log("Time:", results[i]);
    }
    Msg("Time min: %f microseconds", float(min) / 1000.f);
    Msg("Time average: %f microseconds", float(average) / float(iterations));

    const auto result = 0;
#else
    Core.Initialize("OpenXRay",commandLine, nullptr, true,
                    fsltx_path.IsProvided() ? fsltx_path.OptionValue() : nullptr);

    const auto result = RunApplication();
#endif // PROFILE_TASK_SYSTEM

    Core._destroy();

    SDL_Quit();
    return result;
}

#if defined(XR_PLATFORM_WINDOWS)
int StackoverflowFilter(const int exceptionCode)
{
    if (exceptionCode == EXCEPTION_STACK_OVERFLOW)
        return EXCEPTION_EXECUTE_HANDLER;
    return EXCEPTION_CONTINUE_SEARCH;
}

int APIENTRY WinMain(HINSTANCE inst, HINSTANCE prevInst, char* commandLine, int cmdShow)
{
    int result = 0;
    int argc;
    char **argv
    // BugTrap can't handle stack overflow exception, so handle it here
    __try
    {
        argv = CommandLineToArgvW(commandLine, &argc);
        if (HandleArguments(argc, argv))
            result = entry_point(commandLine);
        else
            result = EXIT_FAILURE;
        LocalFree(argv);
    }
    __except (StackoverflowFilter(GetExceptionCode()))
    {
        _resetstkoflw();
        FATAL("stack overflow");
    }
    return result;
}
#elif defined(XR_PLATFORM_LINUX)
int main(int argc, char *argv[])
{
    int result = 0;

    if (!HandleArguments(argc, argv))
        return EXIT_FAILURE;

    try
    {
        char* commandLine = nullptr;
        int i;
        if(argc > 1)
        {
            size_t sum = 0;
            for(i = 1; i < argc; ++i)
                sum += strlen(argv[i]) + strlen(" \0");

            commandLine = (char*)xr_malloc(sum);
            ZeroMemory(commandLine, sum);

            for(i = 1; i < argc; ++i)
            {
                strcat(commandLine, argv[i]);
                strcat(commandLine, " ");
            }
        }
        else
            commandLine = strdup("");

        result = entry_point(commandLine);

        xr_free(commandLine);
    }
    catch (const std::overflow_error& e)
    {
        _resetstkoflw();
        FATAL_F("stack overflow: %s", e.what());
    }
    catch (const std::runtime_error& e)
    {
        FATAL_F("runtime error: %s", e.what());
    }
    catch (const std::exception& e)
    {
        FATAL_F("exception: %s", e.what());
    }
    catch (...)
    {
    // this executes if f() throws std::string or int or any other unrelated type
    }

    return result;
}
#endif
