/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <gtest/gtest.h>

#define private public
#define protected public
#include "idl_common.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Idl::TestCommon;

namespace OHOS {
namespace Idl {
namespace UnitTest {
class JsCodeEmitterStubTest : public testing::Test, public IdlCommon {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void JsCodeEmitterStubTest::SetUpTestCase()
{}

void JsCodeEmitterStubTest::TearDownTestCase()
{}

void JsCodeEmitterStubTest::SetUp()
{}

void JsCodeEmitterStubTest::TearDown()
{}
}  // namespace UnitTest
}  // namespace Idl
}  // namespace OHOS
