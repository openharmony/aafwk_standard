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
import { Context } from './app/context';
import { Want } from './ability/want';
import { StartAbilityParameter, StartAbilityForResultParameter } from './ability/startabilityparameter';
import { AbilityResult, StartAbilityResult } from './ability/abilityresult';

/**
 * A Feature Ability represents an ability with a UI and is designed to interact with users.
 * @name featureAbility
 * @since 3
 * @sysCap AAFwk
 * @devices phone, tablet
 * @permission N/A
 */
declare namespace featureAbility {
  /**
   * Checks whether the main window of this ability has window focus.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @param -
   * @return Returns true if this ability currently has window focus; returns false otherwise.
   */
  function hasWindowFocus(): Promise<boolean>;
  function hasWindowFocus(callback: AsyncCallback<boolean>): void;

  /**
   * Destroys the current ability.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @param -
   * @return -
   */
  function terminateAbility(): Promise<void>;
  function terminateAbility(callback: AsyncCallback<void>): void;

  /**
   * Obtain the want sended from the source ability.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @param parameter Indicates the ability to start.
   * @return -
   */
  function getWant(callback: AsyncCallback<Want>): void;
  function getWant(): Promise<Want>;

  /**
   * Starts a new ability.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @param parameter Indicates the ability to start.
   * @return -
   */
  function startAbility(parameter: StartAbilityParameter, callback: AsyncCallback<number>): void;
  function startAbility(parameter: StartAbilityParameter): Promise<number>;

  /**
   * Starts a new ability, onAbilityResult will be called after this new ability is terminated.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @param parameter Indicates the ability to start.
   * @return -
   */
  function startAbilityForResult(parameter: StartAbilityForResultParameter,
    resultCallback: AsyncCallback<StartAbilityResult>, callback: AsyncCallback<number>): void;
  function startAbilityForResult(parameter: StartAbilityForResultParameter,
    resultCallback: AsyncCallback<StartAbilityResult>): Promise<number>;

   /**
   * Sets the result code and data to be returned by this Feature ability to the caller.
   * @devices phone, tablet
   * @since 3
   * @sysCap AAFwk
   * @param AbilityResult Indicates the result code returned after the ability is destroyed.
   *           You can define the result code to identify an error.
   * @return -
   */
  function finishWithResult(result: AbilityResult, callback: AsyncCallback<void>): void;
  function finishWithResult(result: AbilityResult): Promise<void>;
}
export default featureAbility;