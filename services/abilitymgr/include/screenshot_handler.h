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

#ifndef OHOS_AAFWK_SCREEN_SHOT_HANDLER_H
#define OHOS_AAFWK_SCREEN_SHOT_HANDLER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include "nocopyable.h"
#include "screenshot_response.h"
#ifdef SUPPORT_GRAPHICS
#include "window_manager_service_client.h"
#endif

namespace OHOS {
namespace AAFwk {
class ScreenshotHandler {
public:
    ScreenshotHandler();
    virtual ~ScreenshotHandler() = default;

    void StartScreenshot(int32_t missionId, int32_t winId);

    void RemoveImageInfo(int32_t missionId);

#ifdef SUPPORT_GRAPHICS
public:
    OHOS::WMImageInfo GetImageInfo(int32_t missionId);

private:
    std::map<int32_t, OHOS::WMImageInfo> screenShot_;
    sptr<IWindowManagerService> windowMS_;
#endif
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_AAFWK_SCREEN_SHOT_HANDLER_H
