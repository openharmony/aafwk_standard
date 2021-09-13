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
import { AsyncCallback } from './basic';
import { ApplicationInfo } from './bundle/applicationInfo';
import { BundleInfo } from './bundle/bundleInfo';
import { AbilityInfo } from './bundle/abilityInfo';
import { Want } from './ability/want';
import { BundleInstaller } from './bundle/bundleInstaller';
import { ElementName } from './bundle/elementName';
import { ShortcutInfo } from './bundle/shortcutInfo';
import { ModuleUsageRecord } from './bundle/moduleUsageRecord';
import permission from './@ohos.security.permission';

/**
 * @name BundleFlag
 * @since 3
 * @SysCap BMS
 * @import NA
 * @permission NA
 * @devices phone, tablet
 */
type PermissionEvent = 'permissionChange' | 'anyPermissionChange';

/**
 * bundle.
 * @name bundle
 * @since 3
 * @sysCap appexecfwk
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace bundle {

  enum BundleFlag {
    GET_BUNDLE_DEFAULT = 0x00000000,
    GET_BUNDLE_WITH_ABILITIES = 0x00000001,
    GET_APPLICATION_INFO_WITH_PERMISSION = 0x00000008,
  }

  export enum GrantStatus {
    PERMISSION_DENIED = -1,
    PERMISSION_GRANTED = 0,
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
   function getBundleInfo(bundleName: string, bundelFlags: number, callback: AsyncCallback<BundleInfo>): void;

  /**
   * Obtains Bundle installer to install or uninstall hap.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @return BundleInstaller.
   */
  function getBundleInstaller(callback: AsyncCallback<BundleInstaller>): void;
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
  function getApplicationInfo(bundleName: string, bundelFlags: number, userId: number, callback: AsyncCallback<ApplicationInfo>) : void;
  function getApplicationInfo(bundleName: string, bundelFlags: number, userId?: number) : Promise<ApplicationInfo>;

  function checkPermission(bundleName: string, permission: string, callback: AsyncCallback<GrantStatus>): void;
  function checkPermission(bundleName: string, permission: string): Promise<GrantStatus>;

  /**
   * Obtains BundleInfo of all bundles available in the system.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param callback Specified callback method.
   */
  function getBundleInfos(bundelFlag: BundleFlag, callback: AsyncCallback<Array<BundleInfo>>) : void;
  function getBundleInfos(bundelFlag: BundleFlag) : Promise<Array<BundleInfo>>;

  // void registerAllPermissionsChanged(IRemoteObject callback) throws RemoteException;
  function on(type: PermissionEvent, callback: AsyncCallback<number>): void;
  function on(type: PermissionEvent, uids: Array<number>, callback: AsyncCallback<number>): void;

  // void unregisterPermissionsChanged(IRemoteObject callback) throws RemoteException;
  function off(type: PermissionEvent, callback: AsyncCallback<number>):void;
  function off(type: PermissionEvent, uids: Array<number>, callback: AsyncCallback<number>): void;

  /**
   * Obtains information about all installed applications.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param callback Specified callback method.
   */
  function getApplicationInfos(bundelFlags: number, userId: number, callback: AsyncCallback<Array<ApplicationInfo>>) : void;
  function getApplicationInfos(bundelFlags: number, callback: AsyncCallback<Array<ApplicationInfo>>) : void;
  function getApplicationInfos(bundelFlags: number, userId?: number) : Promise<Array<ApplicationInfo>>;

  /**
   * Obtains information about a bundle contained in a HAP.
   *
   * @devices phone, tablet
   * @since 3
   * @SysCap BMS
   * @param hapFilePath Indicates the path of the HAP.
   * @param callback Specified callback method.
   */
  function getBundleArchiveInfo(hapFilePath: string, bundelFlags: number, callback: AsyncCallback<BundleInfo>) : void
  function getBundleArchiveInfo(hapFilePath: string, bundelFlags: number) : Promise<BundleInfo>;

  // List<ShortcutInfo> getShortcutInfos(String bundleName) throws RemoteException;
  function getShortcutInfos(bundleName: string, callback: AsyncCallback<Array<ShortcutInfo>>): void;
  function getShortcutInfos(bundleName: string): Promise<Array<ShortcutInfo>>;

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
  function getPermissionDef(permissionName: string) : Promise<permission.PermissionDef>;

  function getHomeShortcutInfos(callback: AsyncCallback<Array<ShortcutInfo>>): void;
  function getHomeShortcutInfos(): Promise<void>;

  // List<ModuleUsageRecord> getModuleUsageRecords(int maxNum) throws RemoteException, IllegalArgumentException;
  function getModuleUsageRecords(maxNum: number, callback: AsyncCallback<Array<ModuleUsageRecord>>): void;
  function getModuleUsageRecords(maxNum: number): Promise<Array<ModuleUsageRecord>>;
}

export default bundle;
