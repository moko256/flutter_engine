// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/event_watcher_win32.h"

#include <condition_variable>
#include <mutex>

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(EventWatcherWin32Test, WatchEvent) {
  std::mutex barrier;
  std::condition_variable cond;
  bool called = false;

  EventWatcherWin32 watcher([&barrier, &cond, &called]() {
    std::unique_lock<std::mutex> lock(barrier);
    called = true;
    cond.notify_one();
  });

  ::SetEvent(watcher.GetHandle());

  {
    std::unique_lock<std::mutex> lock(barrier);
    cond.wait(lock, [&called]() { return called; });
  }

  ASSERT_TRUE(called);
};

TEST(EventWatcherWin32Test, WatchGivenEvent) {
  std::mutex barrier;
  std::condition_variable cond;
  bool called = false;

  HANDLE event = ::CreateEvent(NULL, TRUE, FALSE, NULL);

  EventWatcherWin32 watcher(
      [&barrier, &cond, &called]() {
        std::unique_lock<std::mutex> lock(barrier);
        called = true;
        cond.notify_one();
      },
      event);

  ::SetEvent(event);

  {
    std::unique_lock<std::mutex> lock(barrier);
    cond.wait(lock, [&called]() { return called; });
  }

  ASSERT_TRUE(called);
};

}  // namespace testing
}  // namespace flutter
