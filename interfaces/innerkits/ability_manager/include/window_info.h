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

#ifndef OHOS_AAFWK_WINDOW_INFO_H
#define OHOS_AAFWK_WINDOW_INFO_H

// #include "foundation/aafwk/standard/services/common/include/hilog_wrapper.h"
#include "iremote_object.h"
#include "message_parcel.h"

namespace OHOS {
namespace AAFwk {
using DisplayId = uint64_t;

enum class WindowMode : uint32_t {
    WINDOW_MODE_UNDEFINED = 0,
    WINDOW_MODE_FULLSCREEN = 1,
    WINDOW_MODE_SPLIT_PRIMARY = 100,
    WINDOW_MODE_SPLIT_SECONDARY,
    WINDOW_MODE_FLOATING,
    WINDOW_MODE_PIP
};

struct Rect {
    int32_t posX_;
    int32_t posY_;
    uint32_t width_;
    uint32_t height_;

    bool operator==(const Rect& a) const
    {
        return (posX_ == a.posX_ && posY_ == a.posY_ && width_ == a.width_ && height_ == a.height_);
    }

    bool IsInsideOf(const Rect& a) const
    {
        return (posX_ >= a.posX_ && posY_ >= a.posY_ &&
            posX_ + width_ <= a.posX_ + a.width_ && posY_ + height_ <= a.posY_ + a.height_);
    }
};

enum class WindowType : uint32_t {
    APP_WINDOW_BASE = 1,
    APP_MAIN_WINDOW_BASE = APP_WINDOW_BASE,
    WINDOW_TYPE_APP_MAIN_WINDOW = APP_MAIN_WINDOW_BASE,
    APP_MAIN_WINDOW_END,

    APP_SUB_WINDOW_BASE = 1000,
    WINDOW_TYPE_MEDIA = APP_SUB_WINDOW_BASE,
    WINDOW_TYPE_APP_SUB_WINDOW,
    APP_SUB_WINDOW_END,
    APP_WINDOW_END = APP_SUB_WINDOW_END,

    SYSTEM_WINDOW_BASE = 2000,
    BELOW_APP_SYSTEM_WINDOW_BASE = SYSTEM_WINDOW_BASE,
    WINDOW_TYPE_WALLPAPER = SYSTEM_WINDOW_BASE,
    WINDOW_TYPE_DESKTOP,
    BELOW_APP_SYSTEM_WINDOW_END,

    ABOVE_APP_SYSTEM_WINDOW_BASE = 2100,
    WINDOW_TYPE_APP_LAUNCHING = ABOVE_APP_SYSTEM_WINDOW_BASE,
    WINDOW_TYPE_DOCK_SLICE,
    WINDOW_TYPE_INCOMING_CALL,
    WINDOW_TYPE_SEARCHING_BAR,
    WINDOW_TYPE_SYSTEM_ALARM_WINDOW,
    WINDOW_TYPE_INPUT_METHOD_FLOAT,
    WINDOW_TYPE_FLOAT,
    WINDOW_TYPE_TOAST,
    WINDOW_TYPE_STATUS_BAR,
    WINDOW_TYPE_PANEL,
    WINDOW_TYPE_KEYGUARD,
    WINDOW_TYPE_VOLUME_OVERLAY,
    WINDOW_TYPE_NAVIGATION_BAR,
    WINDOW_TYPE_DRAGGING_EFFECT,
    WINDOW_TYPE_POINTER,
    WINDOW_TYPE_LAUNCHER_RECENT,
    WINDOW_TYPE_LAUNCHER_DOCK,
    WINDOW_TYPE_BOOT_ANIMATION,
    WINDOW_TYPE_FREEZE_DISPLAY,
    ABOVE_APP_SYSTEM_WINDOW_END,

    SYSTEM_WINDOW_END = ABOVE_APP_SYSTEM_WINDOW_END,
};

enum class WindowFlag : uint32_t {
    WINDOW_FLAG_DEFAULT,
    WINDOW_FLAG_NEED_AVOID = 1,
    WINDOW_FLAG_PARENT_LIMIT = 1 << 1,
    WINDOW_FLAG_SHOW_WHEN_LOCKED = 1 << 2,
    WINDOW_FLAG_END = 1 << 3,
};

struct WindowTransitionInfo : public Parcelable {
    std::string bundleName_;
    std::string abilityName_;
    WindowMode mode_ = WindowMode::WINDOW_MODE_FULLSCREEN;
    Rect windowRect_ = { 0, 0, 0, 0 };
    sptr<IRemoteObject> abilityToken_ = nullptr;
    DisplayId displayId_ = 0;
    WindowType windowType_ = WindowType::WINDOW_TYPE_APP_MAIN_WINDOW;
    WindowFlag windowFlag_ = WindowFlag::WINDOW_FLAG_DEFAULT;

    virtual bool Marshalling(Parcel& parcel) const override
    {
        if (!parcel.WriteString(bundleName_)) {
            // HILOG_ERROR("Failed to write bundleName");
            return false;
        }

        if (!parcel.WriteString(abilityName_)) {
            // HILOG_ERROR("Failed to write abilityName");
            return false;
        }

        if (!parcel.WriteUint32(static_cast<uint32_t>(mode_))) {
            // HILOG_ERROR("Failed to write window mode.");
            return false;
        }

        if (!parcel.WriteInt32(windowRect_.posX_)) {
            // HILOG_ERROR("Failed to write rect posX.");
            return false;
        }

        if (!parcel.WriteInt32(windowRect_.posY_)) {
            // HILOG_ERROR("Failed to write rect posY.");
            return false;
        }

        if (!parcel.WriteUint32(windowRect_.width_)) {
            // HILOG_ERROR("Failed to write rect width.");
            return false;
        }

        if (!parcel.WriteUint32(windowRect_.height_)) {
            // HILOG_ERROR("Failed to write rect height.");
            return false;
        }

        if (!parcel.WriteRemoteObject(abilityToken_)) {
            // HILOG_ERROR("Failed to write ability token.");
            return false;
        }

        if (!parcel.WriteUint64(displayId_)) {
            // HILOG_ERROR("Failed to write displayId.");
            return false;
        }

        if (!parcel.WriteUint32(static_cast<uint32_t>(windowType_))) {
            // HILOG_ERROR("Failed to write windowType.");
            return false;
        }

        if (!parcel.WriteUint32(static_cast<uint32_t>(windowFlag_))) {
            // HILOG_ERROR("Failed to write windowFlag.");
            return false;
        }

        return true;
    }

    static sptr<WindowTransitionInfo> Unmarshalling(MessageParcel& parcel)
    {
        sptr<WindowTransitionInfo> info;
        info->bundleName_ = parcel.ReadString();
        info->abilityName_ = parcel.ReadString();
        info->mode_ = static_cast<WindowMode>(parcel.ReadUint32());

        int32_t posX = parcel.ReadInt32();
        int32_t posY = parcel.ReadInt32();
        uint32_t width = parcel.ReadUint32();
        uint32_t height = parcel.ReadUint32();
        info->windowRect_ = { posX, posY, width, height };

        info->abilityToken_ = parcel.ReadRemoteObject();
        info->displayId_ = parcel.ReadUint64();
        info->windowType_ = static_cast<WindowType>(parcel.ReadUint32());
        info->windowFlag_ = static_cast<WindowFlag>(parcel.ReadUint32());
        return info;
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_AAFWK_WINDOW_INFO_H