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
#ifndef OHOS_AAFWK_CONFIGURATION_HOLDER_H
#define OHOS_AAFWK_CONFIGURATION_HOLDER_H

#include "aafwk_dummy_configuration.h"

#include <memory>

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilityRecord
 * AbilityRecord records ability info and states and used to schedule ability life.
 */
class ConfigurationHolder {
public:
    ConfigurationHolder() = default;
    virtual ~ConfigurationHolder() = default;

    void UpdateConfiguration(const std::shared_ptr<DummyConfiguration> config);
    std::shared_ptr<DummyConfiguration> GetConfiguration() const;
    bool ProcessConfigurationChange();
    bool ForceProcessConfigurationChange(const std::shared_ptr<DummyConfiguration> &config);

protected:
    virtual bool OnConfigurationChanged(const DummyConfiguration &config, unsigned int configChanges);
    virtual std::shared_ptr<ConfigurationHolder> GetParent() = 0;
    virtual unsigned int GetChildSize() = 0;
    virtual std::shared_ptr<ConfigurationHolder> FindChild(unsigned int index) = 0;

    static constexpr unsigned int CHANGE_CONFIG_ALL_CHANGED = 0xFFFFFFFF;
    static constexpr unsigned int CHANGE_CONFIG_NONE = 0x00000000;
    static constexpr unsigned int CHANGE_CONFIG_LOCALE = 0x00000001;
    static constexpr unsigned int CHANGE_CONFIG_LAYOUT = 0x00000002;
    static constexpr unsigned int CHANGE_CONFIG_FONTSIZE = 0x00000004;
    static constexpr unsigned int CHANGE_CONFIG_ORIENTATION = 0x00000008;
    static constexpr unsigned int CHANGE_CONFIG_DENSITY = 0x00000010;

private:
    void DealUpdateConfiguration();
    bool ProcessConfigurationChangeInner(const std::shared_ptr<DummyConfiguration> &config);

private:
    std::shared_ptr<DummyConfiguration> baseConfiguration_ = nullptr;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_CONFIGURATION_HOLDER_H