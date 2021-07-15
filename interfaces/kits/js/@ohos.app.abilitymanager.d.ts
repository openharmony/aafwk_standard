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

import { AbilityMissionInfo } from './app/abilitymissioninfo';
import { RunningProcessInfo } from './app/runningprocessinfo';
import { AsyncCallback } from './.basic';

/**
 * Obtains running process and memory information about an application.
 * @name abilityManager
 * @since 3
 * @sysCap appexecfwk
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace abilityManager {

  export enum WeightReasonCode {
    REASON_UNKNOWN = 0,
    WEIGHT_FOREGROUND = 100,
    WEIGHT_FOREGROUND_SERVICE = 125,
    WEIGHT_VISIBLE = 200,
    WEIGHT_PERCEPTIBLE = 230,
    WEIGHT_SERVICE = 300,
    WEIGHT_TOP_SLEEPING = 325,
    WEIGHT_CANT_SAVE_STATE = 350,
    WEIGHT_CACHED = 400,
    WEIGHT_GONE = 1000,
  }
  /**
   * Obtains information about application processes that are running on the device.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @returns Returns a list of running processes.
   */
  function getAllRunningProcesses(): Promise<Array<RunningProcessInfo>>;

  /**
   * Obtains information about the application process running on the device through callback.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param callback Specified callback method.
   */
  function getAllRunningProcesses(callback: AsyncCallback<Array<RunningProcessInfo>>): void;

  /**
   * Queries information about the running Ability Mission.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @returns Returns the AbilityMissionInfos.
   */
  function queryRunningAbilityMissionInfos(maxNum: number): Promise<Array<AbilityMissionInfo>>;

  /**
   * Queries information about the running Ability Mission through callback.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param callback Specified callback method.
   */
  function queryRunningAbilityMissionInfos(maxNum: number, callback: AsyncCallback<Array<AbilityMissionInfo>>): void;

  /**
   * Queries information about the recent Ability Mission.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @returns Returns the AbilityMissionInfos.
   */
  function queryRecentAbilityMissionInfos(maxNum: number, flag: number): Promise<Array<AbilityMissionInfo>>;

  /**
   * Queries information about the recent Ability Mission.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param callback Specified callback method.
   */
  function queryRecentAbilityMissionInfos(maxNum: number, flag: number, callback: AsyncCallback<Array<AbilityMissionInfo>>): void;

  /**
   * Remove the mission associated with the given mission ID.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param missionId Indicates the mission ID
   * @returns Returns 0 for success, return non-0 for failure.
   */
  function removeMission(missionId: number): Promise<number>;

  /**
   * Remove the mission associated with the given mission ID.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param missionId Indicates the mission ID
   * @param callback Specified callback method
   */
  function removeMission(missionId: number, callback: AsyncCallback<number>): void;

  /**
   * Remove the missions associated with the given array of the mission ID.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param missionIds Indicates the array of the mission ID
   * @returns Returns 0 for success, return non-0 for failure.
   */
  function removeMissions(missionIds: Array<number>): Promise<number>;

  /**
   * Remove the missions associated with the given array of the mission ID.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param missionIds Indicates the array of the mission ID
   * @param callback Specified callback method
   */
  function removeMissions(missionIds: Array<number>, callback: AsyncCallback<number>): void;

  /**
   * Removes all the recent missions
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @returns Returns 0 for success, return non-0 for failure.
   */
  function clearMissions(): Promise<number>;

  /**
   * Removes all the recent missions
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param callback Specified callback method.
   */
  function clearMissions(callback: AsyncCallback<number>): void;

  /**
   * Ask that the mission associated with a given mission ID be moved to the
   * front of the stack.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param missionId Indicates the mission ID
   * @returns Returns 0 for success, return non-0 for failure.
   */
  function moveMissionToTop(missionId: number): Promise<number>;

  /**
   * Ask that the mission associated with a given mission ID be moved to the
   * front of the stack.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param missionId Indicates the mission ID
   * @param callback Specified callback method.
   */
  function moveMissionToTop(missionId: number, callback: AsyncCallback<number>): void;

  /**
   * Kills all background processes associated with a specified bundle.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param bundleName Indicates the bundle name
   * @returns Returns 0 for success, return non-0 for failure.
   */
  function killProcessesByBundleName(bundleName: string): Promise<number>;

  /**
   * Kills all background processes associated with a specified bundle through callback.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap appexecfwk
   * @param bundleName Indicates the bundle name
   * @param callback Specified callback method
  */
  function killProcessesByBundleName(bundleName: string, callback: AsyncCallback<number>): void;
}
export default abilityManager;