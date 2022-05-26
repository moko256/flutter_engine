// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_LOW_MEMORY_HANDLER_WIN32_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_LOW_MEMORY_HANDLER_WIN32_H_

#include <functional>
#include <memory>

#include "flutter/shell/platform/windows/event_watcher_win32.h"

namespace flutter {

class LowMemoryHandlerWin32 {
 public:
  LowMemoryHandlerWin32(std::function<void()> callback);

 private:
  std::unique_ptr<EventWatcherWin32> low_memory_watcher;

  LowMemoryHandlerWin32(const LowMemoryHandlerWin32&) = delete;
  LowMemoryHandlerWin32& operator=(const LowMemoryHandlerWin32&) = delete;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_LOW_MEMORY_HANDLER_WIN32_H_
