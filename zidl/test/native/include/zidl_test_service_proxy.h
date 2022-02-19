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

#ifndef OHOS_ZIDLTESTSERVICEPROXY_H
#define OHOS_ZIDLTESTSERVICEPROXY_H

#include <iremote_proxy.h>

#include "izidl_test_service.h"

namespace OHOS {
class ZidlTestServiceProxy : public IRemoteProxy<IZidlTestService> {
public:
    explicit ZidlTestServiceProxy(
        /* [in] */ const sptr<IRemoteObject>& remote)
        : IRemoteProxy<IZidlTestService>(remote)
    {}

    virtual ~ZidlTestServiceProxy()
    {}

    ErrCode TestIntTransaction(
        /* [in] */ int _data,
        /* [out] */ int& result) override;

    ErrCode TestStringTransaction(
        /* [in] */ const std::string& _data) override;

private:
    static constexpr int COMMAND_TEST_INT_TRANSACTION = MIN_TRANSACTION_ID + 0;
    static constexpr int COMMAND_TEST_STRING_TRANSACTION = MIN_TRANSACTION_ID + 1;

    static inline BrokerDelegator<ZidlTestServiceProxy> delegator_;
};
} // namespace OHOS
#endif // OHOS_ZIDLTESTSERVICEPROXY_H

