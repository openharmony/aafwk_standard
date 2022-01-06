/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_JS_WINDOW_UTILS_H
#define OHOS_JS_WINDOW_UTILS_H
#include "native_engine/native_engine.h"
#include "native_engine/native_value.h"
#include "window.h"
#include "window_option.h"
#include "wm_common.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr size_t ARGC_ONE = 1;
    constexpr size_t ARGC_TWO = 2;
    constexpr int32_t INDEX_ONE = 1;
    constexpr int32_t INDEX_TWO = 2;
}

    NativeValue* CreateJsWindowPropertiesObject(NativeEngine& engine, sptr<Rosen::Window>& window);
}
}
#endif