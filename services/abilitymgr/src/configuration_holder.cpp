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
#include "configuration_holder.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
void ConfigurationHolder::Init(const std::shared_ptr<DummyConfiguration> &config)
{
    baseConfiguration_ = config;
}

void ConfigurationHolder::UpdateConfiguration(const std::shared_ptr<DummyConfiguration> config)
{
    if (GetChildSize() == 0 && GetParent()) {
        HILOG_INFO("%{public}s child update configuration at the time of activing.", __func__);
        return;
    }
    auto configChanges = baseConfiguration_ ? baseConfiguration_->Differ(config) : CHANGE_CONFIG_ALL_CHANGED;

    if (configChanges > 0) {
        // update base configuration immediately
        baseConfiguration_ = config;
    }

    DealUpdateConfiguration();
}

std::shared_ptr<DummyConfiguration> ConfigurationHolder::GetConfiguration() const
{
    return baseConfiguration_;
}

void ConfigurationHolder::DealUpdateConfiguration()
{
    for (unsigned int index = 0; index < GetChildSize(); index++) {
        auto child = FindChild(index);
        if (child) {
            child->UpdateConfiguration(baseConfiguration_);
        }
    }
}

bool ConfigurationHolder::ProcessConfigurationChange()
{
    HILOG_INFO("%{public}s", __func__);
    auto parent = GetParent();
    auto targetConfig = parent ? parent->GetConfiguration() : baseConfiguration_;
    if (targetConfig == nullptr && baseConfiguration_ == nullptr) {
        HILOG_DEBUG("targetConfig and baseConfiguration_ is nullptr, no change.");
        return false;
    }

    return ProcessConfigurationChangeInner(targetConfig);
}

bool ConfigurationHolder::ForceProcessConfigurationChange(const std::shared_ptr<DummyConfiguration> &config)
{
    return ProcessConfigurationChangeInner(config);
}

bool ConfigurationHolder::ProcessConfigurationChangeInner(const std::shared_ptr<DummyConfiguration> &config)
{
    auto configChanges = baseConfiguration_ ? baseConfiguration_->Differ(config) : CHANGE_CONFIG_ALL_CHANGED;
    if (configChanges > 0) {
        if (baseConfiguration_) {
            baseConfiguration_ = config;
            return OnConfigurationChanged(*(baseConfiguration_.get()), configChanges);
        }
        // update base configuration immediately
        baseConfiguration_ = config;
        HILOG_DEBUG("have changes.");
    }
    HILOG_DEBUG("there is no change.");
    return false;
}

bool ConfigurationHolder::OnConfigurationChanged(const DummyConfiguration &config, unsigned int configChanges)
{
    return false;
}
}  // namespace AAFwk
}  // namespace OHOS