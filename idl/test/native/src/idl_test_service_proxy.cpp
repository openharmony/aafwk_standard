/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "idl_test_service_proxy.h"

namespace OHOS {
ErrCode IdlTestServiceProxy::TestIntTransaction(int _data, int& result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERR_INVALID_VALUE;
    }

    data.WriteInt32(_data);

    if (Remote() == nullptr) {
        return ERR_INVALID_VALUE;
    }
    int32_t st = Remote()->SendRequest(COMMAND_TEST_INT_TRANSACTION, data, reply, option);
    if (st != ERR_NONE) {
        return st;
    }

    ErrCode ec = reply.ReadInt32();
    if (FAILED(ec)) {
        return ec;
    }

    result = reply.ReadInt32();
    return ERR_OK;
}

ErrCode IdlTestServiceProxy::TestStringTransaction(const std::string& _data)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERR_INVALID_VALUE;
    }

    data.WriteString16(Str8ToStr16(_data));

    if (Remote() == nullptr) {
        return ERR_INVALID_VALUE;
    }
    int32_t st = Remote()->SendRequest(COMMAND_TEST_STRING_TRANSACTION, data, reply, option);
    if (st != ERR_NONE) {
        return st;
    }

    ErrCode ec = reply.ReadInt32();
    if (FAILED(ec)) {
        return ec;
    }

    return ERR_OK;
}
} // namespace OHOS
