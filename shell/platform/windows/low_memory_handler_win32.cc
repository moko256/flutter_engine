// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/low_memory_handler_win32.h"

#include <Windows.h>

namespace flutter {

LowMemoryHandlerWin32::LowMemoryHandlerWin32(std::function<void()> callback) {
  HANDLE handle =
      CreateMemoryResourceNotification(LowMemoryResourceNotification);

  low_memory_watcher = std::make_unique<EventWatcherWin32>(callback, handle);
}

}  // namespace flutter
