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

#ifndef OHOS_AAFWK_START_OPTIONS_H
#define OHOS_AAFWK_START_OPTIONS_H

#include <string>

#include "ability_window_configuration.h"
#include "parcel.h"
namespace OHOS {
namespace AAFwk {
class StartOptions final : public Parcelable, public std::enable_shared_from_this<StartOptions> {
public:
    const int32_t DEFAULT_DISPLAY_ID {0};

    StartOptions() = default;
    ~StartOptions() = default;
    StartOptions(const StartOptions &other);
    StartOptions &operator=(const StartOptions &other);

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static StartOptions *Unmarshalling(Parcel &parcel);

    void SetWindowMode(int32_t windowMode);
    int32_t GetWindowMode() const;

    void SetDisplayID(int32_t displayId);
    int32_t GetDisplayID() const;
private:
    int32_t windowMode_ = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_UNDEFINED;
    int32_t displayId_ = DEFAULT_DISPLAY_ID;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_START_OPTIONS_H
