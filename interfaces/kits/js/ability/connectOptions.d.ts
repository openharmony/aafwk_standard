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
import rpc from './../@ohos.rpc';
import { ElementName } from '../bundle/elementName';

/**
 * Provides callback methods that are called when a remote ability using the Service template is connected or
 * disconnected.
 *
 * <p>You must override the methods of this interface to implement your processing logic for the connection and
 * disconnection of an ability using the Service template (Service ability for short).</p>
 *
 * @Syscap {@link SystemCapability.Aafwk#ABILITY}
 * @since 1
 */
declare interface ConnectOptions {
  onConnect: (elementName: ElementName, iRemoteObject: IRemoteObject) => void;
  onDisconnect: (elementName: ElementName) => void;
  onFailed: (code: number) => void;
}