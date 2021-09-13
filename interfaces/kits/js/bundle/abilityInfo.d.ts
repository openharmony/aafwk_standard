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
import { ApplicationInfo } from './applicationInfo';
import { CustomizeData } from './customizeData'
import bundle from './../@ohos.bundle';

/**
 * @name AbilityInfo
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
export interface AbilityInfo {
  readonly bundleName: string;
  readonly name: string;
  readonly label: string;
  readonly description: string;
  readonly icon: string;
  readonly labelId: number;
  readonly descriptionId: number;
  readonly iconId: number;
  readonly moduleName: string;
  readonly process: string;
  readonly targetAbility: string;
  readonly backgroundModes: number;
  readonly isVisible: boolean;
  readonly formEnabled: boolean;
  readonly type: bundle.AbilityType;
  readonly subType: bundle.AbilitySubType;
  readonly orientation: bundle.DisplayOrientation;
  readonly launchMode: bundle.LaunchMode;
  readonly permissions: Array<String>;
  readonly deviceTypes: Array<String>;
  readonly deviceCapabilities: Array<string>;
  readonly readPermission: string;
  readonly writePermission: string;
  readonly applicationInfo: ApplicationInfo;
  readonly formEntity: number;
  readonly minFormHeight: number;
  readonly defaultFormHeight: number;
  readonly minFormWidth: number;
  readonly defaultFormWidth: number;
  readonly uri: string;
  customizeData: Map<string, Array<CustomizeData>>;
}
