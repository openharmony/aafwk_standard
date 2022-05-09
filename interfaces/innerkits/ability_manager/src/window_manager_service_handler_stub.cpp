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

#ifdef SUPPORT_GRAPHICS
#include "window_manager_service_handler_stub.h"

#include "ability_manager_errors.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
WindowManagerServiceHandlerStub::WindowManagerServiceHandlerStub()
{
    Init();
}

WindowManagerServiceHandlerStub::~WindowManagerServiceHandlerStub()
{
    requestFuncMap_.clear();
}

void WindowManagerServiceHandlerStub::Init()
{
    requestFuncMap_[ON_NOTIFY_WINDOW_TRANSITION] = &WindowManagerServiceHandlerStub::NotifyWindowTransitionInner;
    requestFuncMap_[ON_GET_FOCUS_ABILITY] = &WindowManagerServiceHandlerStub::GetFocusWindowInner;
    requestFuncMap_[ON_COLD_STARTING_WINDOW] = &WindowManagerServiceHandlerStub::StartingWindowCold;
    requestFuncMap_[ON_HOT_STARTING_WINDOW] = &WindowManagerServiceHandlerStub::StartingWindowHot;
    requestFuncMap_[ON_CANCLE_STARTING_WINDOW] = &WindowManagerServiceHandlerStub::CancelStartingWindowInner;
}

int WindowManagerServiceHandlerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IWindowManagerServiceHandler::GetDescriptor()) {
        HILOG_ERROR("InterfaceToken not equal IWindowManagerServiceHandler's descriptor.");
        return ERR_AAFWK_PARCEL_FAIL;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    HILOG_WARN("default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int WindowManagerServiceHandlerStub::NotifyWindowTransitionInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    sptr<AbilityTransitionInfo> fromInfo(data.ReadParcelable<AbilityTransitionInfo>());
    if (!fromInfo) {
        HILOG_ERROR("To read fromInfo failed.");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    sptr<AbilityTransitionInfo> toInfo(data.ReadParcelable<AbilityTransitionInfo>());
    if (!toInfo) {
        HILOG_ERROR("To read toInfo failed.");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    NotifyWindowTransition(fromInfo, toInfo);
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::GetFocusWindowInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    sptr<IRemoteObject> abilityToken = nullptr;
    int32_t ret = GetFocusWindow(abilityToken);
    if (!reply.WriteInt32(ret)) {
        HILOG_ERROR("To write result failed.");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    if (abilityToken) {
        if (!reply.WriteBool(true)) {
            HILOG_ERROR("To write true failed.");
            return ERR_AAFWK_PARCEL_FAIL;
        }
        if (!reply.WriteObject(abilityToken)) {
            HILOG_ERROR("To write abilityToken failed.");
            return ERR_AAFWK_PARCEL_FAIL;
        }
    } else {
        if (!reply.WriteBool(false)) {
            HILOG_ERROR("To write false failed.");
            return ERR_AAFWK_PARCEL_FAIL;
        }
    }
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::StartingWindowCold(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    sptr<AbilityTransitionInfo> info(data.ReadParcelable<AbilityTransitionInfo>());
    if (!info) {
        HILOG_ERROR("To read info failed.");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    sptr<Media::PixelMap> pixelMap(data.ReadParcelable<Media::PixelMap>());
    if (!pixelMap) {
        HILOG_ERROR("To read pixelMap failed.");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    auto bgColor = data.ReadUint32();
    StartingWindow(info, pixelMap, bgColor);
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::StartingWindowHot(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    sptr<AbilityTransitionInfo> info(data.ReadParcelable<AbilityTransitionInfo>());
    if (!info) {
        HILOG_ERROR("To read info failed.");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    sptr<Media::PixelMap> pixelMap(data.ReadParcelable<Media::PixelMap>());
    if (!pixelMap) {
        HILOG_ERROR("To read pixelMap failed.");
        return ERR_AAFWK_PARCEL_FAIL;
    }
    StartingWindow(info, pixelMap);
    return ERR_OK;
}

int WindowManagerServiceHandlerStub::CancelStartingWindowInner(MessageParcel &data, MessageParcel &reply)
{
    HILOG_DEBUG("%{public}s is called.", __func__);
    sptr<IRemoteObject> abilityToken = nullptr;
    if (data.ReadBool()) {
        HILOG_DEBUG("abilityToken is valid.");
        abilityToken = data.ReadObject<IRemoteObject>();
    }
    CancelStartingWindow(abilityToken);
    return ERR_OK;
}
}  // namespace AAFwk
}  // namespace OHOS
#endif
