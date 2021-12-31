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

#ifndef OHOS_ZIDLTESTSERVICESTUB_H
#define OHOS_ZIDLTESTSERVICESTUB_H

#include "izidl_test_service.h"
#include <iremote_stub.h>

namespace OHOS {
class ZidlTestServiceStub : public IRemoteStub<IZidlTestService> {
public:
    int OnRemoteRequest(
        /* [in] */ uint32_t code,
        /* [in] */ MessageParcel& data,
        /* [out] */ MessageParcel& reply,
        /* [in] */ MessageOption& option) override;

private:
    static constexpr int COMMAND_TEST_INT_TRANSACTION = MIN_TRANSACTION_ID + 0;
    static constexpr int COMMAND_TEST_STRING_TRANSACTION = MIN_TRANSACTION_ID + 1;
};
} // namespace OHOS
#endif // OHOS_ZIDLTESTSERVICESTUB_H

