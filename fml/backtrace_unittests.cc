// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "backtrace.h"

#include "gtest/gtest.h"
#include "logging.h"

namespace fml {
namespace testing {

static void BacktracePrinter() {
  abort();
}

TEST(BacktraceTest, BacktraceHere) {
  BacktracePrinter();
}

}  // namespace testing
}  // namespace fml
