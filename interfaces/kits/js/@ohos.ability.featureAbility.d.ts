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
import { Want } from './ability/want';
import { StartAbilityParameter } from './ability/startAbilityParameter';
import { AbilityResult } from './ability/abilityResult';
import { Context } from './app/context';
import { ConnectOptions } from './ability/connectOptions';
import { DataAbilityHelper } from './ability/dataAbilityHelper';
import { ProcessInfo } from './app/processInfo';
import { ElementName } from './bundle/elementName';

/**
 * A Feature Ability represents an ability with a UI and is designed to interact with users.
 * @name featureAbility
 * @since 6
 * @sysCap AAFwk
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace featureAbility {
  /**
   * Obtain the want sended from the source ability.
   * @devices phone, tablet
   * @since 6
   * @sysCap AAFwk
   * @return -
   */
  function getWant(callback: AsyncCallback<Want>): void;
  function getWant(): Promise<Want>;

  /**
   * Starts a new ability.
   * @devices phone, tablet
   * @since 6
   * @sysCap AAFwk
   * @param parameter Indicates the ability to start.
   * @return -
   */
  function startAbility(parameter: StartAbilityParameter, callback: AsyncCallback<number>): void;
  function startAbility(parameter: StartAbilityParameter): Promise<number>;

  /**
   * Obtains the application context.
   *
   * @return Returns the application context.
   * @since 6
   */
  function getContext(): Context;

  /**
   * Starts an ability and returns the execution result when the ability is destroyed.
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param parameter Indicates the ability to start.
   * @return -
   */
  function startAbilityForResult(parameter: StartAbilityParameter, callback: AsyncCallback<AbilityResult>): void;
  function startAbilityForResult(parameter: StartAbilityParameter): Promise<AbilityResult>;

  /**
   * Sets the result code and data to be returned by this Page ability to the caller
   * and destroys this Page ability.
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param parameter Indicates the result to return.
   * @return -
   */
  function terminateSelfWithResult(parameter: AbilityResult, callback: AsyncCallback<void>): void;
  function terminateSelfWithResult(parameter: AbilityResult): Promise<void>;

  /**
   * Destroys this Page ability.
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @return -
   */
  function terminateSelf(callback: AsyncCallback<void>): void;

   /**
   * Checks whether the main window of this ability has window focus.
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   */
  function hasWindowFocus(callback: AsyncCallback<boolean>): void;
  function hasWindowFocus(): Promise<boolean>;

  /**
   * Connects an ability to a Service ability.
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param request Indicates the Service ability to connect.
   * @param options Indicates the callback object when the Service ability is connected.
   * @return Returns true if the connection is successful; returns false otherwise.
   */
  function connectAbility(request: Want, options:ConnectOptions): number;

  /**
   * Disconnects an ability from a Service ability.
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param connection Indicates the Service ability to disconnect.
   */
  function disconnectAbility(connection: number, callback:AsyncCallback<void>): void;
  function disconnectAbility(connection: number): Promise<void>;

  /**
   * Obtains the type of this application.
   * @devices phone, tablet
   * @since 7
   * @sysCap appexecfwk
   * @return Returns {@code system} if this application is a system application;
   *         returns {@code normal} if it is released in Ohos AppGallery;
   *         returns {@code other} if it is released by a third-party vendor;
   *         returns an empty string if the query fails.
   */
   function getAppType(callback: AsyncCallback<string>): void;
   function getAppType(): Promise<string>;

  /**
   * Obtains the bundle name of the current ability.
   * @devices phone, tablet
   * @since 7
   * @sysCap appexecfwk
   * @return Returns the bundle name of the current ability.
   */
   function getBundleName(callback: AsyncCallback<string>): void;
   function getBundleName(): Promise<string>;

  /**
   * Obtains the bundle name of the calling ability.
   * @devices phone, tablet
   * @since 7
   * @sysCap appexecfwk
   * @return Returns the bundle name of the current ability.
   */
   function getCallingBundle(callback: AsyncCallback<string>): void;
   function getCallingBundle(): Promise<string>;

  /**
   * Connects an ability to a Service ability.
   * @devices phone, tablet
   * @since 7
   * @sysCap AAFwk
   * @param want Indicates the Service ability to connect.
   * @param connectionCallback Indicates the callback object when the Service ability is connected.
   * @return Returns true if the connection is successful; returns false otherwise.
   */
   function getDataAbilityHelper(URI: string, callback:AsyncCallback<DataAbilityHelper>): void;
   function getDataAbilityHelper(URI: string): Promise<DataAbilityHelper>;

  /**
   * Obtains the element name of the current ability.
   * @devices phone, tablet
   * @since 7
   * @sysCap appexecfwk
   * @return Returns the element name of the current ability.
   */
   function getElementName(callback: AsyncCallback<ElementName>): void;
   function getElementName(): Promise<ElementName>;

  /**
   * Obtains information about the current process, including the process ID and name.
   * @devices phone, tablet
   * @since 7
   * @sysCap appxecfwk
   * @return Returns the process info of the current process.
   */
   function getProcessInfo(callback: AsyncCallback<ProcessInfo>): void;
   function getProcessInfo(): Promise<ProcessInfo>;
}
export default featureAbility;