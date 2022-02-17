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
#include "local_call_record.h"

#include "hilog_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
int64_t LocalCallRecord::callRecordId = 0;
LocalCallRecord::LocalCallRecord(const AppExecFwk::ElementName &elementName)
{
    recordId_ = callRecordId++;
    elementName_ = elementName;
}

LocalCallRecord::~LocalCallRecord()
{
    if (remoteObject_ && callRecipient_) {
        remoteObject_->RemoveDeathRecipient(callRecipient_);
    }
}

void LocalCallRecord::SetRemoteObject(const sptr<IRemoteObject> &call)
{
    if (!call) {
        HILOG_ERROR("remote object is nullptr");
        return;
    }

    remoteObject_ = call;

    if (!callRecipient_) {
        callRecipient_ =
            new CallRecipient(std::bind(&LocalCallRecord::OnCallStubDied, this, std::placeholders::_1));
    }

    remoteObject_->AddDeathRecipient(callRecipient_);
    HILOG_DEBUG("SetRemoteObject complete.");
}

void LocalCallRecord::AddCaller(const std::shared_ptr<CallerCallBack> &callback)
{
    callers_.emplace_back(callback);
}

bool LocalCallRecord::RemoveCaller(const std::shared_ptr<CallerCallBack> &callback)
{
    if (callers_.empty()) {
        HILOG_ERROR("this caller vector is empty.");
        return false;
    }

    auto iter = std::find(callers_.begin(), callers_.end(), callback);
    if (iter != callers_.end()) {
        callback->InvokeOnRelease(ON_RELEASE);
        callers_.erase(iter);
        return true;
    }

    HILOG_ERROR("this caller callback can't find.");
    return false;
}

void LocalCallRecord::OnCallStubDied(const wptr<IRemoteObject> &remote)
{
    HILOG_DEBUG("OnCallStubDied.");
    for (auto &callBack:callers_) {
        if (callBack) {
            HILOG_ERROR("invoke caller's OnRelease.");
            callBack->InvokeOnRelease(ON_DIED);
        }
    }
}

void LocalCallRecord::InvokeCallBack() const
{
    if (!remoteObject_) {
        HILOG_ERROR("remote object is nullptr, can't callback.");
        return;
    }

    for (auto &callBack:callers_) {
        if (callBack && !callBack->IsCallBack()) {
            callBack->InvokeCallBack(remoteObject_);
        }
    }
    HILOG_DEBUG("finish callback with remote object.");
}

sptr<IRemoteObject> LocalCallRecord::GetRemoteObject() const
{
    return remoteObject_;
}

AppExecFwk::ElementName LocalCallRecord::GetElementName() const
{
    return elementName_;
}

bool LocalCallRecord::IsExistCallBack() const
{
    return (callers_.size() > 0);
}

int LocalCallRecord::GetRecordId()
{
    return recordId_;
}

std::vector<std::shared_ptr<CallerCallBack>> LocalCallRecord::GetCallers()
{
    return callers_;
}
}  // namespace AbilityRuntime
}  // namespace OHOS