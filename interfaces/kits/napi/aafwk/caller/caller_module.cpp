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

#include "native_engine/native_engine.h"

extern const char _binary_caller_js_start[];
extern const char _binary_caller_js_end[];
extern const char _binary_caller_abc_start[];
extern const char _binary_caller_abc_end[];

extern "C" __attribute__((constructor))
void NAPI_application_Caller_AutoRegister()
{
    auto moduleManager = NativeModuleManager::GetInstance();
    NativeModule newModuleInfo = {
        .name = "application.Caller",
        .fileName = "application/libcaller_napi.so/caller.js",
    };

    moduleManager->Register(&newModuleInfo);
}

extern "C" __attribute__((visibility("default")))
void NAPI_application_Caller_GetJSCode(const char **buf, int *bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_caller_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_caller_js_end - _binary_caller_js_start;
    }
}

extern "C" __attribute__((visibility("default")))
void NAPI_application_Caller_GetABCCode(const char **buf, int *buflen)
{
    if (buf != nullptr) {
        *buf = _binary_caller_abc_start;
    }
    if (buflen != nullptr) {
        *buflen = _binary_caller_abc_end - _binary_caller_abc_start;
    }
}