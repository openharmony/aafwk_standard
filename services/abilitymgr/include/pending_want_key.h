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

#ifndef OHOS_AAFWK_PENDING_WANT_KEY_H
#define OHOS_AAFWK_PENDING_WANT_KEY_H

#include <vector>
#include <string>

#include "want.h"
#include "ability_record.h"
#include "wants_info.h"

namespace OHOS {
namespace AAFwk {

#define ODD_PRIME_NUMBER (37)

class PendingWantKey {
public:
    PendingWantKey();
    virtual ~PendingWantKey() = default;
    void SetType(const int32_t type);
    void SetBundleName(const std::string &bundleName);
    void SetRequestWho(const std::string &requestWho);
    void SetRequestCode(int32_t requestCode);
    void SetRequestWant(const Want &requestWant);
    void SetRequestResolvedType(const std::string &requestResolvedType);
    void SetAllWantsInfos(const std::vector<WantsInfo> &allWantsInfos);
    void SetFlags(int32_t flags);
    void SetCode(int32_t code);
    void SetUserId(int32_t userId);

    int32_t GetType();
    std::string &GetBundleName();
    std::string &GetRequestWho();
    int32_t GetRequestCode();
    Want &GetRequestWant();
    std::string &GetRequestResolvedType();
    std::vector<WantsInfo> &GetAllWantsInfos();
    int32_t GetFlags();
    int32_t GetCode();
    int32_t GetUserId();

private:
    int32_t type_;
    std::string bundleName_;
    std::string requestWho_;
    int32_t requestCode_;
    Want requestWant_;
    std::string requestResolvedType_;
    std::vector<WantsInfo> allWantsInfos_;
    int32_t flags_;
    int32_t code_;
    int32_t userId_;
};

}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_PENDING_WANT_KEY_H