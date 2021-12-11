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

#ifndef MOCK_ABILITY_THREAD_FOR_DATA_OBSERVER_H
#define MOCK_ABILITY_THREAD_FOR_DATA_OBSERVER_H

#include <iostream>
#include <memory>

#include "gtest/gtest.h"

#include "mock_ability_test.h"
#include "ohos_application.h"
#include "ability_thread.h"
#include "event_runner.h"
#include "ability_info.h"
#include "ability_local_record.h"
#include <iostream> //
namespace OHOS {
namespace AppExecFwk {
sptr<AppExecFwk::AbilityThread> MockCreateAbilityThread()
{
    sptr<AppExecFwk::AbilityThread> abilitythread(new (std::nothrow) AppExecFwk::AbilityThread());
    if (abilitythread == nullptr) {
        GTEST_LOG_(INFO) << "mock_ability_thread_for_data_observer:mock abilityhreadptr is  nullptr called";
    }

    std::shared_ptr<OHOSApplication> application = std::make_shared<OHOSApplication>();
    if (application == nullptr) {
        GTEST_LOG_(INFO) << "mock_ability_thread_for_data_observer:mock applicationptr is  nullptr called";
    }

    std::shared_ptr<AbilityInfo> info = std::make_shared<AbilityInfo>();
    if (info == nullptr) {
        GTEST_LOG_(INFO) << "mock_ability_thread_for_data_observer:mock AbilityInfo::info is  nullptr called";
    }

    info->name = std::string("MockAbilityTest");
    info->type = AbilityType::DATA;
    info->isNativeAbility = true;
    std::cout << "info->name : " << (*info).name <<std::endl;

    sptr<IRemoteObject> token = sptr<IRemoteObject>(new AbilityThread());

    std::shared_ptr<AbilityLocalRecord> abilityRecord = std::make_shared<AbilityLocalRecord>(info, token);
    std::shared_ptr<EventRunner> mainRunner = EventRunner::Create(info->name);

    abilitythread->Attach(application, abilityRecord, mainRunner);

    return abilitythread;
}
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // MOCK_ABILITY_THREAD_FOR_DATA_OBSERVER_H