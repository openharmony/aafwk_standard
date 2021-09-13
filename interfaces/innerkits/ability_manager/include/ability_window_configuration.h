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
#ifndef OHOS_AAFWK_ABILITY_WINDOW_CONFIGURATION_H
#define OHOS_AAFWK_ABILITY_WINDOW_CONFIGURATION_H

namespace OHOS {
namespace AAFwk {
enum AbilityWindowConfiguration {
    /**
     * Indicates that the Page ability is in an undefined window display mode.
     */
    MULTI_WINDOW_DISPLAY_UNDEFINED = 0,
    /**
     * Indicates that the Page ability is in the fullscreen display mode.
     */
    MULTI_WINDOW_DISPLAY_FULLSCREEN = 1,
    /**
     * Indicates that the Page ability is displayed in the primary window when it is in split-screen mode.
     *     The primary window, which triggers the split-screen mode, refers to the top window in the
     *     top/bottom split-screen mode or the left window in the left/right screen mode.
     */
    MULTI_WINDOW_DISPLAY_PRIMARY = 100,
    /**
     * Indicates that the Page ability is displayed in the secondary window when it is in split-screen mode.
     *     The secondary window refers to the bottom window in the top/bottom split-screen mode or
     *     the right window in the left/right screen mode.
     */
    MULTI_WINDOW_DISPLAY_SECONDARY = 101,
    /**
     * Indicates that the Page ability is in floating window display mode.
     */
    MULTI_WINDOW_DISPLAY_FLOATING = 102
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_ABILITY_WINDOW_CONFIGURATION_H