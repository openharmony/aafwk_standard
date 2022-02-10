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

#include "want_receiver_interface.h"
#include "cancel_listener.h"
#include "gmock/gmock.h"

namespace OHOS {
namespace AAFwk {
class MockWantReceiver : public IWantReceiver {
public:
    MOCK_METHOD1(Send, void(const int32_t resultCode));
    MOCK_METHOD0(AsObject, sptr<IRemoteObject>());
    MOCK_METHOD7(PerformReceive, void(const Want &want, int resultCode, const std::string &data,
                                    const WantParams &extras, bool serialized, bool sticky, int sendingUser));
};

class MockCancelListener : public AbilityRuntime::WantAgent::CancelListener {
public:
    MOCK_METHOD1(OnCancelled, void(int resultCode));
};
}  // namespace AAFwk
}  // namespace OHOS
