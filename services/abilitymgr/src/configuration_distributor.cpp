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

#include "configuration_distributor.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
ConfigurationDistributor::ConfigurationDistributor()
{}

ConfigurationDistributor::~ConfigurationDistributor()
{
    if (!observerList_.empty()) {
        for (auto &iter : observerList_) {
            iter.reset();
        }
    }
}

void ConfigurationDistributor::Atach(const std::weak_ptr<ConfigurationHolder> &ob)
{
    std::lock_guard<std::mutex> lock(configLock_);
    if (ob.expired()) {
        HILOG_INFO("Atach ob is null");
        return;
    }
    auto ability = ob.lock();
    int id = ability->GetId();
    HILOG_INFO("Atach ob id[%{public}d]", id);
    observerList_.emplace_back(ob);

    // Init ability thread configurtion object
    ability->UpdateConfiguration(config_);
}

void ConfigurationDistributor::Detach(const std::weak_ptr<ConfigurationHolder> &ob)
{
    std::lock_guard<std::mutex> lock(configLock_);
    if (ob.expired()) {
        HILOG_INFO("Detach ob is null");
        return;
    }

    int id = ob.lock()->GetId();
    HILOG_INFO("Detach ob id[%{public}d]", id);

    auto removeById = [id](const std::weak_ptr<ConfigurationHolder>& ob) -> bool {
        auto observer = ob.lock();
        int currentId = -1;
        if (observer) {
            currentId = observer->GetId();
        }
        HILOG_INFO("current Compare id is [%{public}d]", currentId);
        return currentId == id;
    };
    observerList_.remove_if(removeById);
}

void ConfigurationDistributor::UpdateConfiguration(const AppExecFwk::Configuration &newConfig)
{
    std::lock_guard<std::mutex> lock(configLock_);
    for (const auto &iter : observerList_) {
        auto observer = iter.lock();
        if (observer) {
            HILOG_INFO("UpdateConfiguration To [%{public}d]", observer->GetId());
            observer->UpdateConfiguration(newConfig);
        }
    }
    HILOG_INFO("notify done");
}

void ConfigurationDistributor::InitConfiguration(const AppExecFwk::Configuration &newConfig)
{
    std::lock_guard<std::mutex> lock(configLock_);
    config_ = newConfig;
}
} // namespace AAFwk
} // namespace OHOS