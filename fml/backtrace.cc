// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/backtrace.h"

#include <csignal>
#include <sstream>

#include "flutter/fml/build_config.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/paths.h"
#include "third_party/abseil-cpp/absl/debugging/stacktrace.h"
#include "third_party/abseil-cpp/absl/debugging/symbolize.h"

#ifdef FML_OS_WIN
#include <Windows.h>
#include <crtdbg.h>
#include <debugapi.h>
#else  // FML_OS_WIN
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#endif  // FML_OS_WIN

namespace fml {

static std::string kKUnknownFrameName = "Unknown";

static std::string DemangleSymbolName(const std::string& mangled) {
#if FML_OS_WIN
  return mangled;
#else
  if (mangled == kKUnknownFrameName) {
    return kKUnknownFrameName;
  }

  int status = 0;
  size_t length = 0;
  char* demangled = __cxxabiv1::__cxa_demangle(
      mangled.data(),  // mangled name
      nullptr,         // output buffer (malloc-ed if nullptr)
      &length,         // demangled length
      &status);

  if (demangled == nullptr || status != 0) {
    return mangled;
  }

  auto demangled_string = std::string{demangled, length};
  free(demangled);
  return demangled_string;
#endif  // FML_OS_WIN
}

static std::string GetSymbolName(void* symbol) {
  char name[1024];
  if (!absl::Symbolize(symbol, name, sizeof(name))) {
    return kKUnknownFrameName;
  }

  return DemangleSymbolName({name});
}

std::string BacktraceHere(size_t offset) {
  constexpr size_t kMaxFrames = 256;
  void* symbols[kMaxFrames];
  const auto available_frames =
      absl::GetStackTrace(symbols, kMaxFrames, offset);
  if (available_frames <= 0) {
    return "";
  }

  std::stringstream stream;
  for (int i = 0; i < available_frames; i++) {
    stream << "Frame " << i << ": " << symbols[i] << " "
           << GetSymbolName(symbols[i]) << std::endl;
  }
  return stream.str();
}

static size_t kKnownSignalHandlers[] = {
    SIGABRT,  // abort program
    SIGFPE,   // floating-point exception
    SIGTERM,  // software termination signal
    SIGSEGV,  // segmentation violation
#if !FML_OS_WIN
    SIGBUS,   // bus error
    SIGSYS,   // non-existent system call invoked
    SIGPIPE,  // write on a pipe with no reader
    SIGALRM,  // real-time timer expired
#endif  // !FML_OS_WIN
};

static std::string SignalNameToString(int signal) {
  switch (signal) {
    case SIGABRT:
      return "SIGABRT";
    case SIGFPE:
      return "SIGFPE";
    case SIGSEGV:
      return "SIGSEGV";
    case SIGTERM:
      return "SIGTERM";
#if !FML_OS_WIN
    case SIGBUS:
      return "SIGBUS";
    case SIGSYS:
      return "SIGSYS";
    case SIGPIPE:
      return "SIGPIPE";
    case SIGALRM:
      return "SIGALRM";
#endif  // !FML_OS_WIN
  };
  return std::to_string(signal);
}

static void ToggleSignalHandlers(bool set);

static void SignalHandler(int signal) {
  // We are a crash signal handler. This can only happen once. Since we don't
  // want to catch crashes while we are generating the crash reports, disable
  // all set signal handlers to their default values before reporting the crash
  // and re-raising the signal.
  ToggleSignalHandlers(false);

  FML_LOG(ERROR) << "Caught signal " << SignalNameToString(signal)
                 << " during program execution." << std::endl
                 << BacktraceHere(3);

  ::raise(signal);
}

static void ToggleSignalHandlers(bool set) {
  for (size_t i = 0; i < sizeof(kKnownSignalHandlers) / sizeof(size_t); ++i) {
    auto signal_name = kKnownSignalHandlers[i];
    auto handler = set ? &SignalHandler : SIG_DFL;

    if (::signal(signal_name, handler) == SIG_ERR) {
      FML_LOG(ERROR) << "Could not attach signal handler for " << signal_name;
    }
  }
}

void InstallCrashHandler() {
#if FML_OS_WIN
  if (!IsDebuggerPresent()) {
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
  }
#endif
  auto exe_path = fml::paths::GetExecutablePath();
  if (exe_path.first) {
    absl::InitializeSymbolizer(exe_path.second.c_str());
  }
  ToggleSignalHandlers(true);
}

bool IsCrashHandlingSupported() {
  return true;
}

}  // namespace fml
