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

import { RectBounds } from '../wallpaper';

/**
 * @name This class is the input param for { moveMissionToFirstSplitScreen }.
 * @since 7
 * @SysCap appexecfwk
 * @import import app from 'app/splitMissionInfo'
 * @devices phone, tablet
 * @systemapi hide this for inner system use
 */
export interface SplitMissionInfo {
    /**
     * @default the mission id that you want to move
     */
    missionId: number;
    /**
     * @default whether the target window is the first one
     */
    isFirstWindow: boolean;
    /**
     * @default whether to move the mission to top
     */
    isToTop: boolean;
    /**
     * @default Whether you want an animation when moving the task
     */
    isAnimating: boolean;
    /**
     * @default The bounds of the split window
     */
    initialBounds: RectBounds;
}
