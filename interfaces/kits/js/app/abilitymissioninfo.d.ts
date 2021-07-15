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
import { ElementName }  from '../bundle/elementname';

/**
 * @name Task stack information corresponding to ability
 * @since 3
 * @SysCap appexecfwk
 * @import import AbilityMissionInfo from '@ohos.AbilityMissionInfo'
 * @permission N/A
 * @devices phone, tablet
 */
export interface AbilityMissionInfo {
  /**
   * Unique identification of task stack information corresponding to ability
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
  id: number;

  /**
   * The component launched as the first ability in the task stack
   * This can be considered the "application" of this task stack
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
  baseAbility: ElementName;

  /**
   * The ability component at the top of the history stack of the task
   * This is what the user is currently doing
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
  topAbility: ElementName;

  /**
   * The corresponding ability description information in the task stack
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
  missionDescription: MissionDescriptionInfo;
}

/**
 * @name Task stack description information
 * @since 3
 * @SysCap appexecfwk
 * @import import app from '@system.app'
 * @permission N/A
 * @devices phone, tablet
 */
export interface MissionDescriptionInfo {
  /**
   * The label of the specified ability in the task stack
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
  label: string;

  /**
   * The path to the icon that specifies the ability in the task stack
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
  iconPath: string;
}