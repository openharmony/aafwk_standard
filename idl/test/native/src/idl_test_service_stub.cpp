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

#include "idl_test_service_stub.h"

namespace OHOS {
int IdlTestServiceStub::OnRemoteRequest(uint32_t code, MessageParcel& data,
    MessageParcel& reply, MessageOption& option)
{
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        return ERR_TRANSACTION_FAILED;
    }
    switch (code) {
        case COMMAND_TEST_INT_TRANSACTION: {
            int _data = data.ReadInt32();
            int result;
            ErrCode ec = TestIntTransaction(_data, result);
            reply.WriteInt32(ec);
            if (SUCCEEDED(ec)) {
                reply.WriteInt32(result);
            }
            return ERR_NONE;
        }
        case COMMAND_TEST_STRING_TRANSACTION: {
            std::string _data = Str16ToStr8(data.ReadString16());
            ErrCode ec = TestStringTransaction(_data);
            reply.WriteInt32(ec);
            return ERR_NONE;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return ERR_TRANSACTION_FAILED;
}
} // namespace OHOS
