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

import { ProcessErrCode } from '../@ohos.app.abilityManager.d.ts';

/**
 * @name This class saves process error information
 * @since 7
 * @SysCap appexecfwk
 * @import import app from 'app/processErrorInfo'
 * @devices phone, tablet
 * @systemapi hide this for inner system use
 */
export interface ProcessErrorInfo {
   /**
    * @default Error reason, see { ProcessErrCode } in abilityManager
    */
   errStatus: ProcessErrCode;
   /**
    * @default the name of the error process
    */
   processName: string;
   /**
    * @default the process ID
    */
   pid: number;
   /**
    * @default the user ID
    */
   uid: number;
   /**
    * @default the name of the ability corresponding with the error
    */
   abilityName: string;
   /**
    * @default the error message
    */
   errMsg: string;
   /**
    * @default the error trace, for you to find the original error
    */
   backTrace: string;
}

