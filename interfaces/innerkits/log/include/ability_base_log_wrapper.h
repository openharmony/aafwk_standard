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

#ifndef ABILITYBASE_HILOG_WRAPPER_H
#define ABILITYBASE_HILOG_WRAPPER_H

#define ABILITYBASE_CONFIG_HILOG
#ifdef ABILITYBASE_CONFIG_HILOG
#include "hilog/log.h"

#ifdef ABILITYBASE_LOGF
#undef ABILITYBASE_LOGF
#endif

#ifdef ABILITYBASE_LOGE
#undef ABILITYBASE_LOGE
#endif

#ifdef ABILITYBASE_LOGW
#undef ABILITYBASE_LOGW
#endif

#ifdef ABILITYBASE_LOGI
#undef ABILITYBASE_LOGI
#endif

#ifdef ABILITYBASE_LOGD
#undef ABILITYBASE_LOGD
#endif

#ifndef ABILITYBASE_LOG_DOMAIN
#define ABILITYBASE_LOG_DOMAIN 0xD002200
#endif

#ifndef ABILITYBASE_LOG_TAG
#define ABILITYBASE_LOG_TAG "AbilityBase"
#endif

#ifdef ABILITYBASE_LOG_LABEL
#undef ABILITYBASE_LOG_LABEL
#endif

static constexpr OHOS::HiviewDFX::HiLogLabel ABILITYBASE_LOG_LABEL = {
    LOG_CORE, ABILITYBASE_LOG_DOMAIN, ABILITYBASE_LOG_TAG};

#define __FILENAME__ (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define ABILITYBASE_LOGF(fmt, ...)                             \
    (void)OHOS::HiviewDFX::HiLog::Fatal(ABILITYBASE_LOG_LABEL, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,             \
        __FILENAME__,                                          \
        __FUNCTION__,                                          \
        __LINE__,                                              \
        ##__VA_ARGS__)
#define ABILITYBASE_LOGE(fmt, ...)                             \
    (void)OHOS::HiviewDFX::HiLog::Error(ABILITYBASE_LOG_LABEL, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,             \
        __FILENAME__,                                          \
        __FUNCTION__,                                          \
        __LINE__,                                              \
        ##__VA_ARGS__)
#define ABILITYBASE_LOGW(fmt, ...)                            \
    (void)OHOS::HiviewDFX::HiLog::Warn(ABILITYBASE_LOG_LABEL, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,            \
        __FILENAME__,                                         \
        __FUNCTION__,                                         \
        __LINE__,                                             \
        ##__VA_ARGS__)
#define ABILITYBASE_LOGI(fmt, ...)                            \
    (void)OHOS::HiviewDFX::HiLog::Info(ABILITYBASE_LOG_LABEL, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,            \
        __FILENAME__,                                         \
        __FUNCTION__,                                         \
        __LINE__,                                             \
        ##__VA_ARGS__)
#define ABILITYBASE_LOGD(fmt, ...)                             \
    (void)OHOS::HiviewDFX::HiLog::Debug(ABILITYBASE_LOG_LABEL, \
        "[%{public}s(%{public}s:%{public}d)]" fmt,             \
        __FILENAME__,                                          \
        __FUNCTION__,                                          \
        __LINE__,                                              \
        ##__VA_ARGS__)
#else

#define ABILITYBASE_LOGF(...)
#define ABILITYBASE_LOGE(...)
#define ABILITYBASE_LOGW(...)
#define ABILITYBASE_LOGI(...)
#define ABILITYBASE_LOGD(...)
#endif  // ABILITYBASE_CONFIG_HILOG

#endif  // ABILITYBASE_HILOG_WRAPPER_H