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

import abilityManager from './../@ohos.app.abilitymanager';
/**
 * @name This class saves process information about an application
 * @since 3
 * @SysCap appexecfwk
 * @import import app from '@system.app'
 * @permission N/A
 * @devices phone, tablet
 */
 export interface RunningProcessInfo {
  /**
   * The id of the current process
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
  pid: number;

  /**
   * The name of the current process
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
   processName: string;

  /**
   * The list of packaget in the current process
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
   pkgList: Array<string>;

  /**
   * The uid of the current process
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
   uid: number;

  /**
   * the memory level of the current process
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
   lastMemoryLevel: number;

   /**
   * the weight of the current process
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
   weight: number;

   /**
   * the ReasonCode of weight for the current process
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   */
   weightReasonCode: abilityManager.WeightReasonCode;
}