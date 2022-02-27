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

#include "StartContinuation_fuzzer.h"

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

        // get token
        sptr<Token> token = GetFuzzAbilityToken();
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
                abilityMgr->StartContinuation(*want, token, 0);
                abilityMgr->StartContinuation(*want, token, 1);
            }
        }

        if (want) {
            delete want;
            want = nullptr;
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

