/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
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

// #include "ohos/zidl_test_service_proxy.h"----> #include "zidl_test_service_proxy.h"
#include "zidl_test_service_proxy.h"

// namespace ohos {---> namespace OHOS {
namespace OHOS {
ErrCode ZidlTestServiceProxy::TestIntTransaction(
    /* [in] */ int _data,
    /* [out] */ int& result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    // uint32_t def = 5;
    // uint32_t rApiVersion = 6;
    // uint32_t apiVersion = system::GetUintParameter("hw_sc.build.os.apiversion", def);
    // if (apiVersion >= rApiVersion) {
    //     data.UpdateDataVersion(remote);
    // }
    data.WriteInt32(_data);

    int32_t st = Remote()->SendRequest(COMMAND_TEST_INT_TRANSACTION, data, reply, option);
    if (st != ERR_NONE) {
        // HILOG_WARN("TestIntTransaction failed, error code is %d", st);
        return st;
    }

    ErrCode ec = reply.ReadInt32();
    if (FAILED(ec)) {
        return ec;
    }

    result = reply.ReadInt32();
    return ERR_OK;
}

ErrCode ZidlTestServiceProxy::TestStringTransaction(
    /* [in] */ const std::string& _data)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    // uint32_t def = 5;
    // uint32_t rApiVersion = 6;
    // uint32_t apiVersion = system::GetUintParameter("hw_sc.build.os.apiversion", def);
    // if (apiVersion >= rApiVersion) {
    //     data.UpdateDataVersion(remote);
    // }
    data.WriteString16(Str8ToStr16(_data));

    int32_t st = Remote()->SendRequest(COMMAND_TEST_STRING_TRANSACTION, data, reply, option);
    if (st != ERR_NONE) {
        // HILOG_WARN("TestStringTransaction failed, error code is %d", st);
        return st;
    }

    ErrCode ec = reply.ReadInt32();
    if (FAILED(ec)) {
        return ec;
    }

    return ERR_OK;
}
} // OHOS
