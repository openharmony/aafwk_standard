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
import { AsyncCallback } from './.basic';
import { ApplicationInfo } from './bundle/applicationinfo';
import { BundleInfo } from './bundle/bundleinfo';
import { AbilityInfo } from './bundle/abilityinfo';
import { Want } from './ability/want';
import { BundleInstaller } from './bundle/bundleinstaller';
import permission from './@ohos.security.permission';

/**
 * @name BundleFlag
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
export declare interface BundleFlag {
  getBundleDefault: boolean;
  getBundleWithAbilities: boolean;
  getAbilityInfoWithPermission: boolean;
  getAbilityInfoWithApplication: boolean;
  getApplicationInfoWithPermission: boolean;
  getBundleWithRequestedPermission: boolean;
  getAllApplicationInfo: boolean;
}

export declare interface QueryParameter {
  flags?: number;
  userId?: string;
}

/**
 * bundle.
 * @name bundle
 * @since 3
 * @sysCap appexecfwk
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace bundle {
  export enum ModuleUpdateFlag {
    FLAG_MODULE_UPGRADE_CHECK = 0,
    FLAG_MODULE_UPGRADE_INSTALL = 1,
    FLAG_MODULE_UPGRADE_INSTALL_WITH_CONFIG_WINDOWS = 2,
  }

  export enum FormType {
    JAVA = 0,
    JS = 1,
  }

  export enum ColorMode {
    AUTO_MODE = -1,
    DARK_MODE = 0,
    LIGHT_MODE = 1,
  }

  export enum GrantStatus {
    PERMISSION_DENIED = -1,
    PERMISSION_GRANTED = 0,
  }

  export enum ModuleRemoveFlag {
    FLAG_MODULE_NOT_USED_BY_FORM = 0,
    FLAG_MODULE_USED_BY_FORM = 1,
    FLAG_MODULE_NOT_USED_BY_SHORTCUT = 2,
    FLAG_MODULE_USED_BY_SHORTCUT = 3,
  }

  export enum SignatureCompareResult {
    SIGNATURE_MATCHED = 0,
    SIGNATURE_NOT_MATCHED = 1,
    SIGNATURE_UNKNOWN_BUNDLE = 2,
  }

  export enum ShortcutExistence {
    SHORTCUT_EXISTENCE_EXISTS = 0,
    SHORTCUT_EXISTENCE_NOT_EXISTS = 1,
    SHORTCUT_EXISTENCE_UNKNOW = 2,
  }

  export enum QueryShortCutFlag {
    QUERY_SHORTCUT_HOME = 0,
  }

  /**
   * @name AbilityType
   * @since 3
   * @SysCap BMS
   * @import NA
   * @permission NA
   * @devices phone, tablet
   */
  export enum AbilityType {
    UNKNOWN,
    PAGE,
    SERVICE,
    DATA,
  }

  /**
   * @name AbilitySubType
   * @since 3
   * @SysCap BMS
   * @import NA
   * @permission NA
   * @devices phone, tablet
   */
  export enum AbilitySubType {
    UNSPECIFIED = 0,
    CA = 1,
  }

  /**
   * @name DisplayOrientation
   * @since 3
   * @SysCap BMS
   * @import NA
   * @permission NA
   * @devices phone, tablet
   */
  export enum DisplayOrientation {
    UNSPECIFIED,
    LANDSCAPE,
    PORTRAIT,
    FOLLOWRECENT,
  }

  /**
   * @name LaunchMode
   * @since 3
   * @SysCap BMS
   * @import NA
   * @permission NA
   * @devices phone, tablet
   */
  export enum LaunchMode {
      SINGLETON = 0,
      STANDARD = 1,
  }

  export enum InstallErrorCode{
    SUCCESS = 0,
    STATUS_INSTALL_FAILURE = 1,
    STATUS_INSTALL_FAILURE_ABORTED = 2,
    STATUS_INSTALL_FAILURE_INVALID = 3,
    STATUS_INSTALL_FAILURE_CONFLICT = 4,
    STATUS_INSTALL_FAILURE_STORAGE = 5,
    STATUS_INSTALL_FAILURE_INCOMPATIBLE = 6,
    STATUS_UNINSTALL_FAILURE = 7,
    STATUS_UNINSTALL_FAILURE_BLOCKED = 8,
    STATUS_UNINSTALL_FAILURE_ABORTED = 9,
    STATUS_UNINSTALL_FAILURE_CONFLICT = 10,
    STATUS_INSTALL_FAILURE_DOWNLOAD_TIMEOUT = 0x0B,
    STATUS_INSTALL_FAILURE_DOWNLOAD_FAILED = 0x0C,
    STATUS_ABILITY_NOT_FOUND = 0x40,
    STATUS_BMS_SERVICE_ERROR = 0x41
  }

  /**
   * Obtains BundleInfo based on a given bundle name.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param bundleName Indicates the bundle name.
   * @param callback Specified callback method.
   */
  function getBundleInfo(bundleName: string, flags: number, callback: AsyncCallback<BundleInfo>) : void;

  /**
   * Obtains BundleInfo based on a given bundle name.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param bundleName Indicates the bundle name.
   * @return BundleInfo.
   */
  function getBundleInfo(bundleName: string, flags: number) : Promise<BundleInfo>;

  /**
   * Obtains Bundle installer to install or uninstall hap.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @return BundleInstaller.
   */
  function getBundleInstaller(callback: AsyncCallback<BundleInstaller>): void;

  /**
   * Obtains Bundle installer to install or uninstall hap.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @return BundleInstaller.
   */
  function getBundleInstaller(): Promise<BundleInstaller>;

  /**
   * Obtains the ApplicationInfo based on a given application name.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param bundleName Indicates the application name.
   * @param callback Specified callback method.
   */
  function getApplicationInfo(bundleName: string, flags: number, userId: number, callback: AsyncCallback<ApplicationInfo>) : void;

  /**
   * Obtains the ApplicationInfo based on a given application name.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param bundleName Indicates the application name.
   * @return ApplicationInfo.
   */
  function getApplicationInfo(bundleName: string, flags: number, userId: number) : Promise<ApplicationInfo>;

  /**
   * Query the AbilityInfo by the given Want.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param want Indicates the Want for the ability to be queried.
   * @param callback Specified callback method.
   */
  function queryAbilityByWant(want: Want, params: QueryParameter, callback: AsyncCallback<Array<AbilityInfo>>): void;

  /**
   * Query the AbilityInfo by the given Want.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param want Indicates the Want for the ability to be queried.
   * @return AbilityInfo.
   */
  function queryAbilityByWant(want: Want, params: QueryParameter): Promise<AbilityInfo>;

  /**
   * Obtains BundleInfo of all bundles available in the system.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param callback Specified callback method.
   */
  function getBundleInfos(flags: BundleFlag, callback: AsyncCallback<Array<BundleInfo>>) : void;

  /**
   * Obtains BundleInfo of all bundles available in the system.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @return Array of BundleInfo.
   */
  function getBundleInfos(flags: BundleFlag) : Promise<Array<BundleInfo>>;

  /**
   * Obtains information about all installed applications.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param callback Specified callback method.
   */
  function getApplicationInfos(flags: number, userId: number, callback: AsyncCallback<Array<ApplicationInfo>>) : void;

  /**
   * Obtains information about all installed applications.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param NA
   * @return Array of ApplicationInfo.
   */
  function getApplicationInfos(flags: number, userId: number) : Promise<Array<ApplicationInfo>>;

  /**
   * Obtains information about a bundle contained in a HAP.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param hapFilePath Indicates the path of the HAP.
   * @param callback Specified callback method.
   */
  function getBundleArchiveInfo(hapFilePath: string, flags: number, callback: AsyncCallback<BundleInfo>) : void;

  /**
   * Obtains information about a bundle contained in a HAP.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param hapFilePath Indicates the path of the HAP.
   * @return BundleInfo.
   */
  function getBundleArchiveInfo(hapFilePath: string, flags: number) : Promise<BundleInfo>;

  /**
   * Obtains detailed information about a specified permission.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param permissionName Indicates the name of the permission.
   * @param callback Specified callback method.
   */
  function getPermissionDef(permissionName: string, callback: AsyncCallback<permission.PermissionDef>) : void;
  /**
   * Obtains detailed information about a specified permission.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param permissionName Indicates the name of the permission.
   * @return PermissionDef.
   */
  function getPermissionDef(permissionName: string) : Promise<permission.PermissionDef>;
}

export default bundle;