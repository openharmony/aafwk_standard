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

#include "pending_want_key.h"
#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {

PendingWantKey::PendingWantKey()
{}

void PendingWantKey::SetType(const int32_t type)
{
    type_ = type;
}

void PendingWantKey::SetBundleName(const std::string &bundleName)
{
    bundleName_ = bundleName;
}

void PendingWantKey::SetRequestWho(const std::string &requestWho)
{
    requestWho_ = requestWho;
}

void PendingWantKey::SetRequestCode(int32_t requestCode)
{
    requestCode_ = requestCode;
}

void PendingWantKey::SetRequestWant(const Want &requestWant)
{
    requestWant_ = requestWant;
}

void PendingWantKey::SetRequestResolvedType(const std::string &requestResolvedType)
{
    requestResolvedType_ = requestResolvedType;
}

void PendingWantKey::SetAllWantsInfos(const std::vector<WantsInfo> &allWantsInfos)
{
    allWantsInfos_ = allWantsInfos;
}

void PendingWantKey::SetFlags(int32_t flags)
{
    flags_ = flags;
}

void PendingWantKey::SetCode(int32_t code)
{
    code_ = code;
}

void PendingWantKey::SetUserId(int32_t userId)
{
    userId_ = userId;
}

int32_t PendingWantKey::GetType()
{
    return type_;
}

std::string &PendingWantKey::GetBundleName()
{
    return bundleName_;
}

std::string &PendingWantKey::GetRequestWho()
{
    return requestWho_;
}

int32_t PendingWantKey::GetRequestCode()
{
    return requestCode_;
}

Want &PendingWantKey::GetRequestWant()
{
    return requestWant_;
}

std::string &PendingWantKey::GetRequestResolvedType()
{
    return requestResolvedType_;
}

std::vector<WantsInfo> &PendingWantKey::GetAllWantsInfos()
{
    return allWantsInfos_;
}

int32_t PendingWantKey::GetFlags()
{
    return flags_;
}

int32_t PendingWantKey::GetCode()
{
    return code_;
}

int32_t PendingWantKey::GetUserId()
{
    return userId_;
}

}  // namespace AAFwk
}  // namespace OHOS