
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

#ifndef MOCK_FOUNDATION_AAFWK_MOCK_DATA_OBS_MANAGER_ONCHANGE_CALLBACK_STUB_H
#define MOCK_FOUNDATION_AAFWK_MOCK_DATA_OBS_MANAGER_ONCHANGE_CALLBACK_STUB_H

#include <gmock/gmock.h>
#include <map>
#include "data_ability_observer_stub.h"
#include "semaphore_ex.h"

namespace OHOS {
namespace AAFwk {
class MockDataObsManagerOnChangeCallBack : public DataAbilityObserverStub {
public:
    MOCK_METHOD0(OnChange, void());

    void Wait()
    {
        sem_.Wait();
    }

    int Post()
    {
        sem_.Post();
        return 0;
    }

    void PostVoid()
    {
        sem_.Post();
    }

private:
    Semaphore sem_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // MOCK_FOUNDATION_AAFWK_MOCK_DATA_OBS_MANAGER_ONCHANGE_CALLBACK_STUB_H