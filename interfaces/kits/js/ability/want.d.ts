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

/**
 * @name WantOptions
 * @since 3
 * @SysCap AAFwk
 * @import
 * @permission N/A
 * @devices phone, tablet
 */
export declare interface WantOptions {
  // indicates the grant to perform read operations on the URI
  authReadUriPermission: boolean;

  // indicates the grant to perform write operations on the URI
  authWriteUriPermission: boolean;

  // support forward intent result to origin ability
  abilityForwardResult: boolean;

  // used for marking the ability start-up is triggered by continuation
  abilityContinuation: boolean;

  // specifies whether a component does not belong to ohos
  notOhosComponent: boolean;

  // specifies whether an ability is started
  abilityFormEnabled: boolean;

  // indicates the grant for possible persisting on the URI.
  authPersistableUriPermission: boolean;

  // indicates the grant for possible persisting on the URI.
  authPrefixUriPermission: boolean;

  // support distributed scheduling system start up multiple devices
  abilitySliceMultiDevice: boolean;

  // indicates that an ability using the service template is started regardless of whether the
  // host application has been started.
  startForegroundAbility: boolean;

  // install the specified ability if it's not installed.
  installOnDemand: boolean;

  // return result to origin ability slice
  abilitySliceForwardResult: boolean;

  // install the specified ability with background mode if it's not installed.
  installWithBackgroundMode: boolean;
}

/**
  * Want is the basic communication component of the system.
  * @name Want
  * @since 3
  * @sysCap AAFwk
  * @devices phone, tablet
  * @permission N/A
  */
export declare interface Want {
  /**
   * device id
   * @default -
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   */
  deviceId?: string;

  /**
   * bundle name
   * @default -
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   */
  bundleName?: string;

  /**
   * ability name
   * @default -
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   */
  abilityName?: string;

  /**
   * The description of a URI in a Want.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @default -
   */
  uri?: string;

  /**
   * The description of the type in this Want.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @default -
   */
  type?: string;

  /**
   * The options of the flags in this Want.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @default -
   */
  options?: WantOptions;

  /**
   * The description of an action in an want.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @default -
   */
  action?: string;

  /**
   * The description of the WantParams object in an Want
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @default -
   */
  parameters?: {[key: string]: any};

  /**
   * The description of a entities in a Want.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @default -
   */
  entities?: Array<string>;
}