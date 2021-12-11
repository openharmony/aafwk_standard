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

#ifndef FOUNDATION_AAFWK_SERVICES_TEST_MT_MOCK_APP_MANAGER_CLIENT_H
#define FOUNDATION_AAFWK_SERVICES_TEST_MT_MOCK_APP_MANAGER_CLIENT_H

#include <gmock/gmock.h>

#include "configuration_holder.h"

namespace OHOS {
namespace AppExecFwk {
class MockConfigurationHolder : public AAFwk::ConfigurationHolder {
public:
    MockConfigurationHolder()
    {}
    ~MockConfigurationHolder()
    {}

    std::shared_ptr<ConfigurationHolder> GetParent()
    {
        return nullptr;
    }

    unsigned int GetChildSize()
    {
        return 1;
    }

    std::shared_ptr<ConfigurationHolder> FindChild(unsigned int index)
    {
        return nullptr;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_AAFWK_SERVICES_TEST_MT_MOCK_APP_MGR_CLIENT_H
