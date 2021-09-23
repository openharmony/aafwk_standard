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
 * @name This class saves memory map information about an application.
 * All results are in kB.
 * @since 7
 * @SysCap SystemCapability.Appexecfwk
 * @import import MemoryMapInfo from 'app/memoryMapInfo'
 * @permission N/A
 * @devices phone, tablet, tv, wearable, car
 * @systemapi hide this for inner system use
 */
export interface MemoryMapInfo {
    /**
     * @default The  pss page size for virtual machine heap
     */
    vmPss: number;
    /**
     * @default The private dirty page size for virtual machine heap
     */
    vmPrivateDirty: number;
    /**
     * @default The shared dirty page size for virtual machine heap
     */
    vmSharedDirty: number;
    /**
     * @default The pss page size for native heap
     */
    nativePss: number;
    /**
     * @default The private dirty page size for native heap
     */
    nativePrivateDirty: number;
    /**
     * @default The shared dirty page size for native heap
     */
    nativeSharedDirty: number;
    /**
     * @default The pss page size for others
     */
    otherPss: number;
    /**
     * @default The private dirty page size for others
     */
    otherPrivateDirty: number;
    /**
     * @default The shared dirty page size for others
     */
    otherSharedDirty: number;
}
