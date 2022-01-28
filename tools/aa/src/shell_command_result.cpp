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

#include "shell_command_result.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool ShellCommandResult::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteInt32(exitCode)) {
        HILOG_ERROR("Failed to write exitCode");
        return false;
    }

    if (!parcel.WriteString(stdResult)) {
        HILOG_ERROR("Failed to write stdResult");
        return false;
    }

    return true;
}

bool ShellCommandResult::ReadFromParcel(Parcel &parcel)
{
    exitCode = parcel.ReadInt32();
    stdResult = parcel.ReadString();
    return true;
}

ShellCommandResult *ShellCommandResult::Unmarshalling(Parcel &parcel)
{
    ShellCommandResult *result = new (std::nothrow) ShellCommandResult();
    if (result && !result->ReadFromParcel(parcel)) {
        delete result;
        result = nullptr;
    }
    return result;
}
}  // namespace AAFwk
}  // namespace OHOS