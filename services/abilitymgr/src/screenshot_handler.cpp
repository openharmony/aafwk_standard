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

#include "screenshot_handler.h"
#include <chrono>
#include <thread>
#include <sys/mman.h>
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
ScreenshotHandler::ScreenshotHandler()
{
    auto wmsClient = WindowManagerServiceClient::GetInstance();
    if (wmsClient) {
        HILOG_INFO("init window manager service.");
        wmsClient->Init();
        windowMS_ = wmsClient->GetService();
    }
}

void ScreenshotHandler::StartScreenshot(int32_t missionId, int32_t winId)
{
    HILOG_INFO("StartScreenshot");
    if (!windowMS_) {
        HILOG_INFO("wms is nullptr.");
        return;
    }
    auto response = std::make_shared<ScreenShotResponse>();

    auto promise = windowMS_->ShotWindow(winId);
    if (!promise) {
        HILOG_INFO("promise is nullptr.");
        return;
    }

    auto then = [response](const auto &wmsinfo) {
        WMImageInfo wminfo = {
            .wret = wmsinfo.wret,
            .width = wmsinfo.width,
            .height = wmsinfo.height,
            .format = wmsinfo.format,
            .size = wmsinfo.stride * wmsinfo.height,
            .data = nullptr,
        };

        auto data = mmap(nullptr, wminfo.size, PROT_READ, MAP_SHARED, wmsinfo.fd, 0);
        wminfo.data = data;

        response->OnWindowShot(wminfo);

        // 0xffffffff
        uint8_t *errPtr = nullptr;
        errPtr--;
        if (data != errPtr) {
            munmap(data, wminfo.size);
        }
    };
    promise->Then(then);

    auto imageInfo = response->GetImageInfo();
    screenShot_.emplace(missionId, imageInfo);
}

WMImageInfo ScreenshotHandler::GetImageInfo(int32_t missionId)
{
    HILOG_DEBUG("%{public}s begin", __func__);
    WMImageInfo imageInfo;
    auto iter = screenShot_.find(missionId);
    if (iter != screenShot_.end()) {
        imageInfo = iter->second;
    }

    return imageInfo;
}

void ScreenshotHandler::RemoveImageInfo(int32_t missionId)
{
    HILOG_DEBUG("%{public}s begin", __func__);
    screenShot_.erase(missionId);
    HILOG_DEBUG("%{public}zu screenShot_ size", screenShot_.size());
}
}  // namespace AAFwk
}  // namespace OHOS