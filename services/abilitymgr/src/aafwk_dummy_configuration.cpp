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

#include "aafwk_dummy_configuration.h"

#include "string_ex.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {

DummyConfiguration::DummyConfiguration(const std::string &name) : testInfostr_(name)
{}

bool DummyConfiguration::ReadFromParcel(Parcel &parcel)
{
    testInfostr_ = Str16ToStr8(parcel.ReadString16());
    return true;
}

DummyConfiguration *DummyConfiguration::Unmarshalling(Parcel &parcel)
{
    DummyConfiguration *dummyConfiguration = new (std::nothrow) DummyConfiguration();
    if (dummyConfiguration && !dummyConfiguration->ReadFromParcel(parcel)) {
        delete dummyConfiguration;
        dummyConfiguration = nullptr;
    }
    return dummyConfiguration;
}

bool DummyConfiguration::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString16(Str8ToStr16(testInfostr_))) {
        return false;
    }
    return true;
}

unsigned int DummyConfiguration::Differ(const std::shared_ptr<DummyConfiguration> config) const
{
    // temp test code
    if (config == nullptr || config->GetName() == "none") {
        return 0x00000000;
    }

    std::vector<std::string> changes;
    std::string name = config->GetName();
    std::string::size_type nbegin = 0;
    std::string::size_type nend = 0;
    while (true) {
        nend = name.find_first_of("#", nbegin);
        if (nend == std::string::npos) {
            changes.push_back(name.substr(nbegin, name.length()));
            break;
        }
        changes.push_back(name.substr(nbegin, nend - nbegin));
        nbegin = nend + 1;
    }

    HILOG_ERROR("fail to get AbilityEventHandler");

    unsigned int flag = 0x00000000;
    for (auto item : changes) {
        if (item == "locale") {
            flag |= 0x00000001;
        } else if (item == "layout") {
            flag |= 0x00000002;
        } else if (item == "fontSize") {
            flag |= 0x00000004;
        } else if (item == "orientation") {
            flag |= 0x00000008;
        } else if (item == "density") {
            flag |= 0x00000010;
        } else {
            ;
        }
    }
    return flag;
}

}  // namespace AAFwk
}  // namespace OHOS