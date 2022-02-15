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

#include "start_options.h"

namespace OHOS {
namespace AAFwk {
StartOptions::StartOptions(const StartOptions &other)
{
    windowMode_ = other.windowMode_;
    displayId_ = other.displayId_;
}

StartOptions &StartOptions::operator=(const StartOptions &other)
{
    if (this != &other) {
        windowMode_ = other.windowMode_;
        displayId_ = other.displayId_;
    }
    return *this;
}

bool StartOptions::ReadFromParcel(Parcel &parcel)
{
    SetWindowMode(parcel.ReadInt32());
    SetDisplayID(parcel.ReadInt32());
    return true;
}

StartOptions *StartOptions::Unmarshalling(Parcel &parcel)
{
    StartOptions *option = new (std::nothrow) StartOptions();
    if (option == nullptr) {
        return nullptr;
    }

    if (!option->ReadFromParcel(parcel)) {
        delete option;
        option = nullptr;
    }

    return option;
}

bool StartOptions::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt32(GetWindowMode());
    parcel.WriteInt32(GetDisplayID());
    return true;
}

void StartOptions::SetWindowMode(int32_t windowMode)
{
    windowMode_ = windowMode;
}

int32_t StartOptions::GetWindowMode() const
{
    return windowMode_;
}

void StartOptions::SetDisplayID(int32_t id)
{
    displayId_ = id;
}

int32_t StartOptions::GetDisplayID() const
{
    return displayId_;
}
}  // namespace AAFwk
}  // namespace OHOS
