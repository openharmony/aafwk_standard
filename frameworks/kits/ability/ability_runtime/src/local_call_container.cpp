/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "hilog_wrapper.h"
#include "ability_manager_client.h"
#include "local_call_container.h"

namespace OHOS {
namespace AbilityRuntime {
int LocalCallContainer::StartAbilityInner(
    const Want &want, const std::shared_ptr<CallerCallBack> &callback, const sptr<IRemoteObject> &callerToken)
{
    HILOG_DEBUG("start ability by call.");

    if (!callback) {
        HILOG_ERROR("callback is nullptr.");
        return ERR_INVALID_VALUE;
    }

    if (want.GetElement().GetBundleName().empty()
        || want.GetElement().GetAbilityName().empty()) {
        HILOG_ERROR("the element of want is empty.");
        return ERR_INVALID_VALUE;
    }

    if (want.GetElement().GetDeviceID().empty()) {
        HILOG_DEBUG("start ability by call, element:DeviceID is empty");
    }

    HILOG_DEBUG("start ability by call, element:%{public}s", want.GetElement().GetURI().c_str());

    AppExecFwk::ElementName element = want.GetElement();
    std::shared_ptr<LocalCallRecord> localCallRecord;
    if (!GetCallLocalreocrd(element, localCallRecord)) {
        localCallRecord = std::make_shared<LocalCallRecord>(element);
        if (!localCallRecord) {
            HILOG_ERROR("localCallRecord create fail.");
            return ERR_INVALID_VALUE;
        }
        std::string uri = element.GetURI();
        callProxyRecords_.emplace(uri, localCallRecord);
    }

    HILOG_DEBUG("start ability by call, localCallRecord->AddCaller(callback) begin");
    localCallRecord->AddCaller(callback);
    HILOG_DEBUG("start ability by call, localCallRecord->AddCaller(callback) end");

    auto remote = localCallRecord->GetRemoteObject();
    if (!remote) {
        auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
        if (abilityClient == nullptr) {
            HILOG_ERROR("LocalCallContainer::Resolve abilityClient is nullptr");
            return ERR_INVALID_VALUE;
        }
        sptr<IAbilityConnection> connect = iface_cast<IAbilityConnection>(this->AsObject());
        HILOG_DEBUG("start ability by call, abilityClient->StartAbilityByCall call");
        return abilityClient->StartAbilityByCall(want, connect, callerToken);
    }
    // already finish call request.
    HILOG_DEBUG("start ability by call, callback->InvokeCallBack(remote) begin");
    callback->InvokeCallBack(remote);
    HILOG_DEBUG("start ability by call, callback->InvokeCallBack(remote) end");

    return ERR_OK;
}

int LocalCallContainer::Release(const std::shared_ptr<CallerCallBack>& callback)
{
    HILOG_DEBUG("LocalCallContainer::Release begain.");
    auto isExist = [&callback](auto &record) {
        HILOG_DEBUG("LocalCallContainer::Release begain1.");
        return record.second->RemoveCaller(callback);
    };

    auto iter = std::find_if(callProxyRecords_.begin(), callProxyRecords_.end(), isExist);
    if (iter == callProxyRecords_.end()) {
        HILOG_ERROR("release localcallrecord failed.");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<LocalCallRecord> record = iter->second;
    if (!record) {
        HILOG_ERROR("record is nullptr.");
        return ERR_INVALID_VALUE;
    }

    if (record->IsExistCallBack()) {
        // just release callback.
        HILOG_DEBUG("LocalCallContainer::Release begain2.");
        return ERR_OK;
    }

    // notify ams this connect need to release.
    AppExecFwk::ElementName elementName = record->GetElementName() ;
    auto abilityClient = AAFwk::AbilityManagerClient::GetInstance();
    if (abilityClient == nullptr) {
        HILOG_ERROR("LocalCallContainer::Resolve abilityClient is nullptr");
        return ERR_INVALID_VALUE;
    }
    sptr<IAbilityConnection> connect = iface_cast<IAbilityConnection>(this->AsObject());
    if (abilityClient->ReleaseAbility(connect, elementName) != ERR_OK) {
        HILOG_ERROR("ReleaseAbility failed.");
        return ERR_INVALID_VALUE;
    }

    callProxyRecords_.erase(iter);
    HILOG_DEBUG("LocalCallContainer::Release end.");
    return ERR_OK;
}

void LocalCallContainer::DumpCalls(std::vector<std::string> &info) const
{
    HILOG_DEBUG("LocalCallContainer::DumpCalls called.");
    info.emplace_back("          caller connections:");
    for (auto iter = callProxyRecords_.begin(); iter != callProxyRecords_.end(); iter++) {
        std::string tempstr = "            LocalCallRecord";
        tempstr += " ID #" + std::to_string (iter->second->GetRecordId()) + "\n";
        tempstr += "              callee";
        tempstr += " uri[" + iter->first + "]" + "\n";
        tempstr += "              callers #" + std::to_string (iter->second->GetCallers().size());
        bool flag = true;
        for (auto &callBack:iter->second->GetCallers()) {
            if (callBack && !callBack->IsCallBack()) {
                HILOG_INFO("%{public}s call back is not called.", __func__);
                flag = false;
                break;
            }
        }
        if (flag) {
            HILOG_INFO("%{public}s state is REQUESTEND.", __func__);
            tempstr += "  state #REQUESTEND";
        } else {
            HILOG_INFO("%{public}s state is REQUESTING.", __func__);
            tempstr += "  state #REQUESTING";
        }
        info.emplace_back(tempstr);
    }
    return;
}

void LocalCallContainer::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    HILOG_DEBUG("LocalCallContainer::OnAbilityConnectDone start %{public}s .", element.GetURI().c_str());
    if (resultCode != ERR_OK) {
        HILOG_ERROR("OnAbilityConnectDone failed.");
    }
    std::shared_ptr<LocalCallRecord> localCallRecord;
    if (GetCallLocalreocrd(element, localCallRecord)) {
        auto callRecipient = new (std::nothrow) CallRecipient(
            std::bind(&LocalCallContainer::OnCallStubDied, this, std::placeholders::_1));
        localCallRecord->SetRemoteObject(remoteObject, callRecipient);
    }

    if (localCallRecord) {
        localCallRecord->InvokeCallBack();
    }

    HILOG_DEBUG("LocalCallContainer::OnAbilityConnectDone end.");
    return;
}

void LocalCallContainer::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
}

bool LocalCallContainer::GetCallLocalreocrd(
    const AppExecFwk::ElementName &elementName, std::shared_ptr<LocalCallRecord> &localCallRecord)
{
    auto iter = callProxyRecords_.find(elementName.GetURI());
    if (iter != callProxyRecords_.end()) {
        localCallRecord = iter->second;
        return true;
    }
    return false;
}

void LocalCallContainer::OnCallStubDied(const wptr<IRemoteObject> &remote)
{
    auto diedRemote = remote.promote();
    auto isExist = [&diedRemote](auto &record) {
        return record.second->IsSameObject(diedRemote);
    };

    auto iter = std::find_if(callProxyRecords_.begin(), callProxyRecords_.end(), isExist);
    if (iter == callProxyRecords_.end()) {
        HILOG_ERROR("StubDied object not found from localcallrecord.");
        return;
    }

    iter->second->OnCallStubDied(remote);
    callProxyRecords_.erase(iter);
    HILOG_DEBUG("LocalCallContainer::OnCallStubDied end.");
}
}  // namespace AbilityRuntime
}  // namespace OHOS