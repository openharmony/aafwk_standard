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

#ifndef OHOS_AAFWK_STACK_SETTING_H
#define OHOS_AAFWK_STACK_SETTING_H

#include "ability_window_configuration.h"
#include "parcel.h"

#include <map>

namespace OHOS {
namespace AAFwk {
enum STACK_ID {
    LAUNCHER_MISSION_STACK_ID = 0,
    DEFAULT_MISSION_STACK_ID,
    FLOATING_MISSION_STACK_ID,
    SPLIT_SCREEN_MISSION_STACK_ID
};
/**
 * @struct StackSetting
 * StackSetting contains default setting for mission stack.
 */
struct StackSetting final : public Parcelable {

    int32_t userId = 0;  // stack 's user id ,default value is 0
    // multi window mode key , default in an undefined
    STACK_ID stackId = DEFAULT_MISSION_STACK_ID;
    int32_t maxHoldMission = -1;  // default unlimited.
    bool isSyncVisual = false;    // default don'r support synchronous visualization when multiwindow

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static StackSetting *Unmarshalling(Parcel &parcel);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_STACK_SETTING_H