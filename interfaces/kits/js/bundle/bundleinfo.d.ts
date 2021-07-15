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
import { AbilityInfo } from './abilityinfo';
import { ApplicationInfo } from './applicationinfo';
import { HapModuleInfo } from './hapmoduleinfo';

/**
 * @name UsedScene
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
export interface UsedScene {
  abilities: Array<string>;
  when: string;
}

/**
 * @name ReqPermissionDetail
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
export interface ReqPermissionDetail {
  name: string;
  reason: string;
  usedScene: UsedScene;
}

/**
 * @name BundleInfo
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
export interface BundleInfo {
  readonly name: string;
  readonly type: string;
  readonly appId: string;
  readonly uid: number;
  readonly installTime: number;
  readonly updateTime: number;
  readonly appInfo: ApplicationInfo;
  readonly abilityInfos: Array<AbilityInfo>;
  readonly reqPermissions: Array<string>;
  readonly reqPermissionDetails: Array<ReqPermissionDetail>;
  readonly vendor: string;
  readonly versionCode: number;
  readonly versionName: string;
  readonly compatibleVersion: number;
  readonly targetVersion: number;
  readonly isCompressNativeLibs: boolean;
  readonly hapModuleInfos: Array<HapModuleInfo>;
  readonly entryModuleName: string;
  readonly cpuAbi: string;
  readonly isSilentInstallation: string;
  readonly minCompatibleVersionCode: number;
  readonly entryInstallationFree: boolean;
}