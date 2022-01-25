/*
* Copyright (c) 2021 Huawei Device Co., Ltd.
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

import { AsyncCallback, Callback } from './basic';

declare namespace windowStage {

  enum WindowStageEventType {
    VISIBLE = 1,
    FOCUSED,
    UNFOCUSED,
    INVISIBLE,
  }

  /**
   * WindowStage
   * @devices tv, phone, tablet, wearable, liteWearable.
   */
  interface WindowStage {
    /**
     * Get main window of the stage.
     * @since 8
     */
    getMainWindow(): Promise<Window>;
    /**
     * Loads content
     * @param path  path Path of the page to which the content will be loaded
     * @param storage storage The data object shared within the content instance loaded by the window
     * @devices tv, phone, tablet, wearable, car
     * @since 8
     */
    loadContent(path: string, storage: ContenStorage, callback: AsyncCallback<void>): void;
    /**
     * Loads content
     * @param path path of the page to which the content will be loaded
     * @devices tv, phone, tablet, wearable, car
     * @since 8
     */
    loadContent(path: string, callback: AsyncCallback<void>): void;
    /**
     * Loads content
     * @param path path of the page to which the content will be loaded
     * @devices tv, phone, tablet, wearable, car
     * @since 8
     */
    loadContent(path: string, storage?: ContenStorage): Promise<void>;
    /**
     * get the windowmode of current window
     * @devices tv, phone, tablet, wearable, car
     * @systemapi
     * @since 8
     */
    getWindowMode(callback: AsyncCallback<WindowMode>): void;
    /**
     * get the windowmode of current window
     * @devices tv, phone, tablet, wearable, car
     * @systemapi
     * @since 8
     */
    getWindowMode(): Promise<WindowMode>;
    /**
     * window stage event callback on.
     * @since 8
     */
    on(eventType: 'windowStageEvent', callback: Callback<WindowStageEventType>): void;
    /**
     * window stage event callback off.
     * @since 8
     */
    off(eventType: 'windowStageEvent', callback: Callback<WindowStageEventType>): void;
    /**
     * window stage event callback off.
     * @since 8
     */
    off(eventType: 'windowStageEvent'): void;
  }
}

export default windowStage;
