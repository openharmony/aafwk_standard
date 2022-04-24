/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "startability_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "ability_manager_client.h"
#include "ability_record.h"
#include "parcel.h"
#include "want.h"

using namespace OHOS::AAFwk;
using namespace OHOS::AppExecFwk;

namespace OHOS {
    sptr<Token> GetFuzzAbilityToken()
    {
        sptr<Token> token = nullptr;

        AbilityRequest abilityRequest;
        abilityRequest.appInfo.bundleName = "com.example.fuzzTest";
        abilityRequest.abilityInfo.name = "MainAbility";
        abilityRequest.abilityInfo.type = AbilityType::DATA;
        std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
        if (abilityRecord) {
            token = abilityRecord->GetToken();
        }

        return token;
    }

    bool DoSomethingInterestingWithMyAPI(const uint8_t* data, size_t size)
    {
        auto abilityMgr = AbilityManagerClient::GetInstance();
        if (!abilityMgr) {
            return false;
        }

        // get token and connectCallback
        sptr<IRemoteObject> token = GetFuzzAbilityToken();
        if (!token) {
            std::cout << "Get ability token failed." << std::endl;
            return false;
        }

        // fuzz for want
        Parcel wantParcel;
        Want *want = nullptr;
        if (wantParcel.WriteBuffer(data, size)) {
            want = Want::Unmarshalling(wantParcel);
            if (want) {
                abilityMgr->StartAbility(*want);
            }
        }

        // fuzz for abilitysetting
        Parcel settingParcel;
        AbilityStartSetting *setting = nullptr;
        if (settingParcel.WriteBuffer(data, size)) {
            setting = AbilityStartSetting::Unmarshalling(settingParcel);
            if (want && setting) {
                abilityMgr->StartAbility(*want, *setting, token);
            }
        }

        // fuzz for startoptions
        Parcel optionParcel;
        StartOptions *options = nullptr;
        if (optionParcel.WriteBuffer(data, size)) {
            options = StartOptions::Unmarshalling(optionParcel);
            if (want && options) {
                abilityMgr->StartAbility(*want, *options, token);
            }
        }

        if (want) {
            delete want;
            want = nullptr;
        }
        if (setting) {
            delete setting;
            setting = nullptr;
        }
        if (options) {
            delete options;
            options = nullptr;
        }

        return true;
    }
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DoSomethingInterestingWithMyAPI(data, size);
    return 0;
}

