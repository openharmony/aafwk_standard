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

export interface ModuleUsageRecord {
  readonly bundleName: string;
  readonly appLabelId: number;
  readonly name: string;
  readonly labelId: number;
  readonly descriptionId: number;
  readonly abilityName: string;
  readonly abilityLabelId: number;
  readonly abilityDescriptionId: number;
  readonly abilityIconId: number;
  readonly launchedCount: number;
  readonly lastLaunchTime: number;
  readonly isRemoved: boolean;
  readonly installationFreeSupported: boolean;
}