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

import { AsyncCallback } from './../basic';
import bundle from './../@ohos.bundle';

/**
 * @name InstallParam
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
export interface InstallParam {
  userId: number;
  installFlag: number;
  isKeepData: boolean;
}

/**
 * @name InstallStatus
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
export interface InstallStatus {
  status: bundle.InstallErrorCode;
  /**
   * The install result string message.
   *
   * @default -
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   */
  statusMessage: string;
}

/**
 * @name BundleInstaller
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
export interface BundleInstaller {
  /**
   * Install an application in a HAP.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param bundleFilePaths Indicates the paths of the HAP.
   * @param InstallParam Indicates the userId and whether keep data.
   * @return InstallStatus
   */
  install(bundleFilePaths: Array<string>, param: InstallParam, callback: AsyncCallback<InstallStatus>): void;

  /**
   * Uninstall an application.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param bundleName Indicates the bundle name.
   * @param InstallParam Indicates the userId and whether keep data.
   * @return InstallStatus
   */
  uninstall(bundleName: string, param: InstallParam, callback: AsyncCallback<InstallStatus>): void;
}