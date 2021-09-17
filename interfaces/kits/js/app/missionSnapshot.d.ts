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

import { ElementName } from '../bundle/elementName';
import { image } from '../@ohos.multimedia.image';

/**
 * @name This class represents a mission snapshot.
 * @since 7
 * @SysCap appexecfwk
 * @import import MissionSnapshot from 'app/missionSnapshot'
 * @devices phone, tablet
 * @systemapi hide this for inner system use
 */
export interface MissionSnapshot {
    /**
     * @default The top ability in this mission snapshot
     */
    topAbility: ElementName;

    /**
     * @default The entity of snapshot
     */
    snapshot: image.PixelMap;
}
