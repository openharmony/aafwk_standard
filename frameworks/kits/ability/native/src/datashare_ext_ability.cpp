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

#include "datashare_ext_ability.h"

#include "ability_loader.h"
#include "connection_manager.h"
#include "hilog_wrapper.h"
#include "js_datashare_ext_ability.h"
#include "runtime.h"
#include "datashare_ext_ability_context.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
DataShareExtAbility* DataShareExtAbility::Create(const std::unique_ptr<Runtime>& runtime)
{
    if (!runtime) {
        return new DataShareExtAbility();
    }
    HILOG_INFO("DataShareExtAbility::Create runtime");
    switch (runtime->GetLanguage()) {
        case Runtime::Language::JS:
            return JsDataShareExtAbility::Create(runtime);

        default:
            return new DataShareExtAbility();
    }
}

void DataShareExtAbility::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    ExtensionBase<DataShareExtAbilityContext>::Init(record, application, handler, token);
    HILOG_INFO("DataShareExtAbility begin init context");
}

std::shared_ptr<DataShareExtAbilityContext> DataShareExtAbility::CreateAndInitContext(
    const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application,
    std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    std::shared_ptr<DataShareExtAbilityContext> context =
        ExtensionBase<DataShareExtAbilityContext>::CreateAndInitContext(record, application, handler, token);
    if (record == nullptr) {
        HILOG_ERROR("DataShareExtAbility::CreateAndInitContext record is nullptr");
        return context;
    }

    auto abilityInfo = record->GetAbilityInfo();
    context->SetAbilityInfo(abilityInfo);
    context->InitHapModuleInfo(abilityInfo);
    auto appContext = Context::GetApplicationContext();
    context->SetApplicationInfo(appContext->GetApplicationInfo());
    context->SetResourceManager(appContext->GetResourceManager());
    return context;
}

std::vector<std::string> DataShareExtAbility::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::vector<std::string> ret;
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

int DataShareExtAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return 0;
}

int DataShareExtAbility::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return 0;
}

int DataShareExtAbility::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return 0;
}

int DataShareExtAbility::Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
    const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return 0;
}

int DataShareExtAbility::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return 0;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> DataShareExtAbility::Query(const Uri &uri,
    std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> ret;
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

std::string DataShareExtAbility::GetType(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return "";
}

int DataShareExtAbility::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return 0;
}

bool DataShareExtAbility::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

bool DataShareExtAbility::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

bool DataShareExtAbility::NotifyChange(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

Uri DataShareExtAbility::NormalizeUri(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Uri urivalue("");
    HILOG_INFO("%{public}s end.", __func__);
    return urivalue;
}

Uri DataShareExtAbility::DenormalizeUri(const Uri &uri)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Uri urivalue("");
    HILOG_INFO("%{public}s end.", __func__);
    return urivalue;
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> DataShareExtAbility::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    HILOG_INFO("%{public}s begin.", __func__);
    std::vector<std::shared_ptr<DataAbilityResult>> results;
    HILOG_INFO("%{public}s end.", __func__);
    return results;
}
} // namespace AbilityRuntime
} // namespace OHOS