/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_AAFWK_SCREEN_SHOT_RESPONSE_H
#define OHOS_AAFWK_SCREEN_SHOT_RESPONSE_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <condition_variable>
#include "nocopyable.h"
#ifdef SUPPORT_GRAPHICS
#include "../wmclient/wm_common.h"
#endif

namespace OHOS {
namespace AAFwk {
class ScreenShotResponse {
public:
    ScreenShotResponse() = default;
    virtual ~ScreenShotResponse() = default;

#ifdef SUPPORT_GRAPHICS
public:
    void OnWindowShot(const OHOS::WMImageInfo &info);
    OHOS::WMImageInfo GetImageInfo();

private:
    std::shared_ptr<OHOS::WMImageInfo> info_;
#endif

private:
    static constexpr int TIME_OUT = 200 * 1000;
    std::mutex mutex_;
    std::condition_variable condition_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_SCREEN_SHOT_RESPONSE_H
