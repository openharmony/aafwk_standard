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

#ifndef OHOS_APPEXECFWK_MOCK_ABILITY_THREAD_FOR_ABILITY_ON_CONFIGURATION_UPDATE_H
#define OHOS_APPEXECFWK_MOCK_ABILITY_THREAD_FOR_ABILITY_ON_CONFIGURATION_UPDATE_H

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ability.h"

namespace OHOS {
namespace AppExecFwk {

class AbilityTest : public Ability {
public:
    AbilityTest() = default;
    virtual ~AbilityTest() = default;

    MOCK_METHOD1(OnConfigurationUpdated, void(const Configuration &));

    static std::shared_ptr<AbilityTest> GetInstance()
    {
        return abilitTest_;
    };

    static void InitInstance()
    {
        AbilityTest::abilitTest_ = std::make_shared<AbilityTest>();
    };

    static void UnInitInstance()
    {
        AbilityTest::abilitTest_ = nullptr;
    };

    static std::shared_ptr<AbilityTest> abilitTest_;
};

std::shared_ptr<AbilityTest> AbilityTest::abilitTest_ = nullptr;

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APPEXECFWK_MOCK_ABILITY_THREAD_FOR_ABILITY_ON_CONFIGURATION_UPDATE_H
