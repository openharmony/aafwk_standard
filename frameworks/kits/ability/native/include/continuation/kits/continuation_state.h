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
#ifndef FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_STATE_H
#define FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_STATE_H
namespace OHOS {
namespace AppExecFwk {
/**
 * Enumerates the migration states of an {@link Ability} that has implemented {@link IAbilityContinuation}
 * and is in a reversible migration process that allows the ability to be migrated back.
 *
 * <p>The following example describes state changes in a migration process:
 * <p>The initial state of {@code FooAbility} on device A is {@link #LOCAL_RUNNING}. If it is migrated to
 * device B in a reversible way, its state on device A becomes {@link #REMOTE_RUNNING} and that on device
 * B becomes {@link #REPLICA_RUNNING}. If it is migrated to device B in a non-reversible way, its states
 * on both devices are {@link #LOCAL_RUNNING}.
 *
 * @Syscap {@link SystemCapability.Aafwk#ABILITY}
 */
enum class ContinuationState {
    // Indicates that the ability is running on the local device.
    LOCAL_RUNNING = 0,

    // Indicates that the ability has been migrated to another device.
    REMOTE_RUNNING = 1,

    // Indicates that the ability was migrated from another device to the current device.
    REPLICA_RUNNING = 2,
};
using DEVICE_CONNECT_STATE = enum ContinuationState;
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // FOUNDATION_APPEXECFWK_OHOS_CONTINUATION_STATE_H