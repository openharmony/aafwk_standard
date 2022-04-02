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

#include "js_datashare_ext_ability.h"

#include "ability_info.h"
#include "accesstoken_kit.h"
#include "bytrace.h"
#include "dataobs_mgr_client.h"
#include "datashare_stub_impl.h"
#include "hilog_wrapper.h"
#include "ipc_skeleton.h"
#include "js_datashare_ext_ability_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_THREE = 3;
constexpr int INVALID_VALUE = -1;
#if BINDER_IPC_32BIT
const std::string LIB_RDB_PATH = "/system/lib/module/data/librdb.z.so";
const std::string LIB_DATA_ABILITY_PATH = "/system/lib/module/data/libdataability.z.so";
#else
const std::string LIB_RDB_PATH = "/system/lib64/module/data/librdb.z.so";
const std::string LIB_DATA_ABILITY_PATH = "/system/lib64/module/data/libdataability.z.so";
#endif
}

using namespace OHOS::AppExecFwk;
using OHOS::Security::AccessToken::AccessTokenKit;
using DataObsMgrClient = OHOS::AAFwk::DataObsMgrClient;

JsDataShareExtAbility* JsDataShareExtAbility::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsDataShareExtAbility(static_cast<JsRuntime&>(*runtime));
}

JsDataShareExtAbility::JsDataShareExtAbility(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}

JsDataShareExtAbility::~JsDataShareExtAbility()
{
    UnloadLibrary();
}

void JsDataShareExtAbility::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    DataShareExtAbility::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        HILOG_ERROR("Failed to get srcPath");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HILOG_INFO("%{public}s module:%{public}s, srcPath:%{public}s.", __func__, moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }
    HILOG_INFO("JsDataShareExtAbility::Init ConvertNativeValueTo.");
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsDataShareExtAbility object");
        return;
    }

    LoadLibrary();

    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_INFO("JsDataShareExtAbility::Init CreateJsDataShareExtAbilityContext.");
    NativeValue* contextObj = CreateJsDataShareExtAbilityContext(engine, context);
    auto contextRef = jsRuntime_.LoadSystemModule("application.DataShareExtensionAbilityContext",
        &contextObj, ARGC_ONE);
    contextObj = contextRef->Get();
    HILOG_INFO("JsDataShareExtAbility::Init Bind.");
    context->Bind(jsRuntime_, contextRef.release());
    HILOG_INFO("JsDataShareExtAbility::SetProperty.");
    obj->SetProperty("context", contextObj);

    auto nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get datashare extension ability native object");
        return;
    }

    HILOG_INFO("Set datashare extension ability context");

    nativeObj->SetNativePointer(new std::weak_ptr<AbilityRuntime::Context>(context),
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr datashare extension ability context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        }, nullptr);

    HILOG_INFO("JsDataShareExtAbility::Init end.");
}

void JsDataShareExtAbility::LoadLibrary()
{
    libRdbHandle_ = dlopen(LIB_RDB_PATH.c_str(), RTLD_LAZY);
    if (libRdbHandle_ == nullptr) {
        HILOG_ERROR("dlopen failed: %{public}s", dlerror());
    } else {
        rdbValueBucketNewInstance_ = reinterpret_cast<RdbValueBucketNewInstance>(
            dlsym(libRdbHandle_, "NAPI_OHOS_Data_RdbJsKit_ValuesBucketProxy_NewInstance"));
        if (rdbValueBucketNewInstance_ == nullptr) {
            HILOG_ERROR("symbol not found: %{public}s", dlerror());
        }

        rdbResultSetProxyGetNativeObject_ = reinterpret_cast<RdbResultSetProxyGetNativeObject>(
            dlsym(libRdbHandle_, "NAPI_OHOS_Data_RdbJsKit_ResultSetProxy_GetNativeObject"));
        if (rdbResultSetProxyGetNativeObject_ == nullptr) {
            HILOG_ERROR("symbol not found: %{public}s", dlerror());
        }
    }

    libDataAbilityHandle_ = dlopen(LIB_DATA_ABILITY_PATH.c_str(), RTLD_LAZY);
    if (libDataAbilityHandle_ == nullptr) {
        HILOG_ERROR("dlopen failed: %{public}s", dlerror());
    } else {
        dataAbilityPredicatesNewInstance_ = reinterpret_cast<DataAbilityPredicatesNewInstance>(
            dlsym(libDataAbilityHandle_, "NAPI_OHOS_Data_DataAbilityJsKit_DataAbilityPredicatesProxy_NewInstance"));
        if (dataAbilityPredicatesNewInstance_ == nullptr) {
            HILOG_ERROR("symbol not found: %{public}s", dlerror());
        }
    }
}

void JsDataShareExtAbility::UnloadLibrary()
{
    if (libRdbHandle_ != nullptr) {
        dlclose(libRdbHandle_);
    }
    if (libDataAbilityHandle_ != nullptr) {
        dlclose(libDataAbilityHandle_);
    }
}

void JsDataShareExtAbility::OnStart(const AAFwk::Want &want)
{
    HILOG_INFO("%{public}s begin.", __func__);
    Extension::OnStart(want);
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    NativeValue* nativeWant = reinterpret_cast<NativeValue*>(napiWant);
    NativeValue* argv[] = {nativeWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    HILOG_INFO("%{public}s end.", __func__);
}

sptr<IRemoteObject> JsDataShareExtAbility::OnConnect(const AAFwk::Want &want)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    Extension::OnConnect(want);
    sptr<DataShareStubImpl> remoteObject = new (std::nothrow) DataShareStubImpl(
        std::static_pointer_cast<JsDataShareExtAbility>(shared_from_this()),
        reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine()));
    if (remoteObject == nullptr) {
        HILOG_ERROR("%{public}s No memory allocated for DataShareStubImpl", __func__);
        return nullptr;
    }
    HILOG_INFO("%{public}s end. ", __func__);
    return remoteObject->AsObject();
}

NativeValue* JsDataShareExtAbility::CallObjectMethod(const char* name, NativeValue* const* argv, size_t argc)
{
    HILOG_INFO("JsDataShareExtAbility::CallObjectMethod(%{public}s), begin", name);

    if (!jsObj_) {
        HILOG_WARN("Not found DataShareExtAbility.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    auto& nativeEngine = jsRuntime_.GetNativeEngine();

    NativeValue* value = jsObj_->Get();
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get DataShareExtAbility object");
        return nullptr;
    }

    NativeValue* method = obj->GetProperty(name);
    if (method == nullptr) {
        HILOG_ERROR("Failed to get '%{public}s' from DataShareExtAbility object", name);
        return nullptr;
    }
    HILOG_INFO("JsDataShareExtAbility::CallFunction(%{public}s), success", name);
    return handleScope.Escape(nativeEngine.CallFunction(value, method, argv, argc));
}

void JsDataShareExtAbility::GetSrcPath(std::string &srcPath)
{
    if (!Extension::abilityInfo_->isStageBasedModel) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

std::vector<std::string> JsDataShareExtAbility::GetFileTypes(const Uri &uri, const std::string &mimeTypeFilter)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    auto ret = DataShareExtAbility::GetFileTypes(uri, mimeTypeFilter);
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());

    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);
    napi_value napiMimeTypeFilter = nullptr;
    napi_create_string_utf8(env, mimeTypeFilter.c_str(), NAPI_AUTO_LENGTH, &napiMimeTypeFilter);

    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* nativeMimeTypeFilter = reinterpret_cast<NativeValue*>(napiMimeTypeFilter);
    NativeValue* argv[] = {nativeUri, nativeMimeTypeFilter};
    NativeValue* nativeResult = CallObjectMethod("getFileTypes", argv, ARGC_TWO);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call getFileTypes with return null.", __func__);
        return ret;
    }

    if (!OHOS::AppExecFwk::UnwrapArrayStringFromJS(env, reinterpret_cast<napi_value>(nativeResult), ret)) {
        HILOG_ERROR("%{public}s call UnwrapArrayStringFromJS failed", __func__);
        return ret;
    }

    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

int JsDataShareExtAbility::OpenFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto ret = DataShareExtAbility::OpenFile(uri, mode);
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());

    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);
    napi_value napiMode = nullptr;
    napi_create_string_utf8(env, mode.c_str(), NAPI_AUTO_LENGTH, &napiMode);

    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* nativeMode = reinterpret_cast<NativeValue*>(napiMode);
    NativeValue* argv[] = {nativeUri, nativeMode};
    NativeValue* nativeResult = CallObjectMethod("openFile", argv, ARGC_TWO);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call openFile with return null.", __func__);
        return ret;
    }

    ret = OHOS::AppExecFwk::UnwrapInt32FromJS(env, reinterpret_cast<napi_value>(nativeResult));
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

int JsDataShareExtAbility::OpenRawFile(const Uri &uri, const std::string &mode)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto ret = DataShareExtAbility::OpenRawFile(uri, mode);
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());

    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);
    napi_value napiMode = nullptr;
    napi_create_string_utf8(env, mode.c_str(), NAPI_AUTO_LENGTH, &napiMode);

    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* nativeMode = reinterpret_cast<NativeValue*>(napiMode);
    NativeValue* argv[] = {nativeUri, nativeMode};
    NativeValue* nativeResult = CallObjectMethod("openRawFile", argv, ARGC_TWO);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call openRawFile with return null.", __func__);
        return ret;
    }

    ret = OHOS::AppExecFwk::UnwrapInt32FromJS(env, reinterpret_cast<napi_value>(nativeResult));
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

int JsDataShareExtAbility::Insert(const Uri &uri, const NativeRdb::ValuesBucket &value)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = INVALID_VALUE;
    if (!CheckCallingPermission(abilityInfo_->writePermission)) {
        HILOG_ERROR("%{public}s Check calling permission failed.", __func__);
        return ret;
    }

    ret = DataShareExtAbility::Insert(uri, value);
    if (rdbValueBucketNewInstance_ == nullptr) {
        HILOG_ERROR("%{public}s invalid instance of rdb value bucket.", __func__);
        return ret;
    }
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());
    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);
    napi_value napiValue = rdbValueBucketNewInstance_(env, const_cast<OHOS::NativeRdb::ValuesBucket&>(value));
    if (napiValue == nullptr) {
        HILOG_ERROR("%{public}s failed to make new instance of rdbValueBucket.", __func__);
        return ret;
    }

    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* nativeValue = reinterpret_cast<NativeValue*>(napiValue);
    NativeValue* argv[] = {nativeUri, nativeValue};
    NativeValue* nativeResult = CallObjectMethod("insert", argv, ARGC_TWO);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call insert with return null.", __func__);
        return ret;
    }

    ret = OHOS::AppExecFwk::UnwrapInt32FromJS(env, reinterpret_cast<napi_value>(nativeResult));
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

int JsDataShareExtAbility::Update(const Uri &uri, const NativeRdb::ValuesBucket &value,
    const NativeRdb::DataAbilityPredicates &predicates)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = INVALID_VALUE;
    if (!CheckCallingPermission(abilityInfo_->writePermission)) {
        HILOG_ERROR("%{public}s Check calling permission failed.", __func__);
        return ret;
    }

    ret = DataShareExtAbility::Update(uri, value, predicates);
    if (rdbValueBucketNewInstance_ == nullptr) {
        HILOG_ERROR("%{public}s invalid instance of ValuesBucket.", __func__);
        return ret;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());
    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);
    napi_value napiValue = rdbValueBucketNewInstance_(env, const_cast<OHOS::NativeRdb::ValuesBucket&>(value));
    if (napiValue == nullptr) {
        HILOG_ERROR("%{public}s failed to make new instance of rdbValueBucket.", __func__);
        return ret;
    }

    napi_value napiPredicates = MakePredicates(env, predicates);
    if (napiPredicates == nullptr) {
        HILOG_ERROR("%{public}s failed to make new instance of dataAbilityPredicates.", __func__);
        return ret;
    }

    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* nativeValue = reinterpret_cast<NativeValue*>(napiValue);
    NativeValue* nativePredicates = reinterpret_cast<NativeValue*>(napiPredicates);
    NativeValue* argv[] = {nativeUri, nativeValue, nativePredicates};
    NativeValue* nativeResult = CallObjectMethod("update", argv, ARGC_THREE);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call update with return null.", __func__);
        return ret;
    }

    ret = OHOS::AppExecFwk::UnwrapInt32FromJS(env, reinterpret_cast<napi_value>(nativeResult));
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

int JsDataShareExtAbility::Delete(const Uri &uri, const NativeRdb::DataAbilityPredicates &predicates)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = INVALID_VALUE;
    if (!CheckCallingPermission(abilityInfo_->writePermission)) {
        HILOG_ERROR("%{public}s Check calling permission failed.", __func__);
        return ret;
    }

    ret = DataShareExtAbility::Delete(uri, predicates);
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());
    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);

    napi_value napiPredicates = MakePredicates(env, predicates);
    if (napiPredicates == nullptr) {
        HILOG_ERROR("%{public}s failed to make new instance of dataAbilityPredicates.", __func__);
        return ret;
    }

    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* nativePredicates = reinterpret_cast<NativeValue*>(napiPredicates);
    NativeValue* argv[] = {nativeUri, nativePredicates};
    NativeValue* nativeResult = CallObjectMethod("delete", argv, ARGC_TWO);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call delete with return null.", __func__);
        return ret;
    }

    ret = OHOS::AppExecFwk::UnwrapInt32FromJS(env, reinterpret_cast<napi_value>(nativeResult));
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> JsDataShareExtAbility::Query(const Uri &uri,
    std::vector<std::string> &columns, const NativeRdb::DataAbilityPredicates &predicates)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    std::shared_ptr<NativeRdb::AbsSharedResultSet> ret;
    if (!CheckCallingPermission(abilityInfo_->readPermission)) {
        HILOG_ERROR("%{public}s Check calling permission failed.", __func__);
        return ret;
    }

    ret = DataShareExtAbility::Query(uri, columns, predicates);
    if (rdbResultSetProxyGetNativeObject_ == nullptr) {
        HILOG_ERROR("%{public}s invalid instance of RdbResultSet.", __func__);
        return ret;
    }
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());
    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);

    napi_value napiColumns = nullptr;
    napi_create_array(env, &napiColumns);
    bool isArray = false;
    if (napi_is_array(env, napiColumns, &isArray) != napi_ok || !isArray) {
        HILOG_ERROR("JsDataShareExtAbility create array failed");
        return ret;
    }
    int32_t index = 0;
    for (const auto &column : columns) {
        napi_value result = nullptr;
        napi_create_string_utf8(env, column.c_str(), column.length(), &result);
        napi_set_element(env, napiColumns, index++, result);
    }

    napi_value napiPredicates = MakePredicates(env, predicates);
    if (napiPredicates == nullptr) {
        HILOG_ERROR("%{public}s failed to make new instance of dataAbilityPredicates.", __func__);
        return ret;
    }

    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* nativeColumns = reinterpret_cast<NativeValue*>(napiColumns);
    NativeValue* nativePredicates = reinterpret_cast<NativeValue*>(napiPredicates);
    NativeValue* argv[] = {nativeUri, nativeColumns, nativePredicates};
    NativeValue* nativeResult = CallObjectMethod("query", argv, ARGC_THREE);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call query with return null.", __func__);
        return ret;
    }

    auto nativeObject = rdbResultSetProxyGetNativeObject_(env, reinterpret_cast<napi_value>(nativeResult));
    if (nativeObject == nullptr) {
        return ret;
    }

    ret.reset(nativeObject);
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

std::string JsDataShareExtAbility::GetType(const Uri &uri)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    auto ret = DataShareExtAbility::GetType(uri);
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());

    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH,
        &napiUri);
    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* argv[] = {nativeUri};
    NativeValue* nativeResult = CallObjectMethod("getType", argv, ARGC_ONE);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call getType with return null.", __func__);
        return ret;
    }

    ret = OHOS::AppExecFwk::UnwrapStringFromJS(env, reinterpret_cast<napi_value>(nativeResult));
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

int JsDataShareExtAbility::BatchInsert(const Uri &uri, const std::vector<NativeRdb::ValuesBucket> &values)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    int ret = INVALID_VALUE;
    if (!CheckCallingPermission(abilityInfo_->writePermission)) {
        HILOG_ERROR("%{public}s Check calling permission failed.", __func__);
        return ret;
    }

    ret = DataShareExtAbility::BatchInsert(uri, values);
    if (rdbValueBucketNewInstance_ == nullptr) {
        HILOG_ERROR("%{public}s invalid instance of rdb value bucket.", __func__);
        return ret;
    }
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());
    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);

    napi_value napiValues = nullptr;
    napi_create_array(env, &napiValues);
    bool isArray = false;
    if (napi_is_array(env, napiValues, &isArray) != napi_ok || !isArray) {
        HILOG_ERROR("JsDataShareExtAbility create array failed");
        return ret;
    }
    int32_t index = 0;
    for (const auto &value : values) {
        napi_value result = rdbValueBucketNewInstance_(env, const_cast<OHOS::NativeRdb::ValuesBucket&>(value));
        if (result == nullptr) {
            HILOG_ERROR("%{public}s failed to make new instance of rdbValueBucket.", __func__);
            return ret;
        }
        napi_set_element(env, napiValues, index++, result);
    }

    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* nativeValues = reinterpret_cast<NativeValue*>(napiValues);
    NativeValue* argv[] = {nativeUri, nativeValues};
    NativeValue* nativeResult = CallObjectMethod("batchInsert", argv, ARGC_TWO);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call batchInsert with return null.", __func__);
        return ret;
    }

    ret = OHOS::AppExecFwk::UnwrapInt32FromJS(env, reinterpret_cast<napi_value>(nativeResult));
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

bool JsDataShareExtAbility::RegisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    DataShareExtAbility::RegisterObserver(uri, dataObserver);
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->RegisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->RegisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

bool JsDataShareExtAbility::UnregisterObserver(const Uri &uri, const sptr<AAFwk::IDataAbilityObserver> &dataObserver)
{
    HILOG_INFO("%{public}s begin.", __func__);
    DataShareExtAbility::UnregisterObserver(uri, dataObserver);
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->UnregisterObserver(uri, dataObserver);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->UnregisterObserver error return %{public}d", __func__, ret);
        return false;
    }
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

bool JsDataShareExtAbility::NotifyChange(const Uri &uri)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    DataShareExtAbility::NotifyChange(uri);
    auto obsMgrClient = DataObsMgrClient::GetInstance();
    if (obsMgrClient == nullptr) {
        HILOG_ERROR("%{public}s obsMgrClient is nullptr", __func__);
        return false;
    }

    ErrCode ret = obsMgrClient->NotifyChange(uri);
    if (ret != ERR_OK) {
        HILOG_ERROR("%{public}s obsMgrClient->NotifyChange error return %{public}d", __func__, ret);
        return false;
    }
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

Uri JsDataShareExtAbility::NormalizeUri(const Uri &uri)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA,__PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    auto ret = DataShareExtAbility::NormalizeUri(uri);
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());

    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);
    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* argv[] = {nativeUri};
    NativeValue* nativeResult = CallObjectMethod("normalizeUri", argv, ARGC_ONE);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call normalizeUri with return null.", __func__);
        return ret;
    }

    ret = Uri(OHOS::AppExecFwk::UnwrapStringFromJS(env, reinterpret_cast<napi_value>(nativeResult)));
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

Uri JsDataShareExtAbility::DenormalizeUri(const Uri &uri)
{
    BYTRACE_NAME(BYTRACE_TAG_DISTRIBUTEDDATA, __PRETTY_FUNCTION__);
    HILOG_INFO("%{public}s begin.", __func__);
    auto ret = DataShareExtAbility::DenormalizeUri(uri);
    HandleScope handleScope(jsRuntime_);
    napi_env env = reinterpret_cast<napi_env>(&jsRuntime_.GetNativeEngine());

    napi_value napiUri = nullptr;
    napi_create_string_utf8(env, uri.ToString().c_str(), NAPI_AUTO_LENGTH, &napiUri);
    NativeValue* nativeUri = reinterpret_cast<NativeValue*>(napiUri);
    NativeValue* argv[] = {nativeUri};
    NativeValue* nativeResult = CallObjectMethod("denormalizeUri", argv, ARGC_ONE);
    if (nativeResult == nullptr) {
        HILOG_ERROR("%{public}s call denormalizeUri with return null.", __func__);
        return ret;
    }

    ret = Uri(OHOS::AppExecFwk::UnwrapStringFromJS(env, reinterpret_cast<napi_value>(nativeResult)));
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

std::vector<std::shared_ptr<AppExecFwk::DataAbilityResult>> JsDataShareExtAbility::ExecuteBatch(
    const std::vector<std::shared_ptr<AppExecFwk::DataAbilityOperation>> &operations)
{
    HILOG_INFO("%{public}s begin.", __func__);
    auto ret = DataShareExtAbility::ExecuteBatch(operations);
    HILOG_INFO("%{public}s end.", __func__);
    return ret;
}

bool JsDataShareExtAbility::CheckCallingPermission(const std::string &permission)
{
    HILOG_INFO("%{public}s begin, permission:%{public}s", __func__, permission.c_str());
    if (!permission.empty() && AccessTokenKit::VerifyAccessToken(IPCSkeleton::GetCallingTokenID(), permission)
        != AppExecFwk::Constants::PERMISSION_GRANTED) {
        HILOG_ERROR("%{public}s permission not granted.", __func__);
        return false;
    }
    HILOG_INFO("%{public}s end.", __func__);
    return true;
}

napi_value JsDataShareExtAbility::MakePredicates(napi_env env, const NativeRdb::DataAbilityPredicates &predicates)
{
    HILOG_INFO("%{public}s begin.", __func__);
    if (dataAbilityPredicatesNewInstance_ == nullptr) {
        HILOG_ERROR("%{public}s invalid instance of DataAbilityPredicates.", __func__);
        return nullptr;
    }
    OHOS::NativeRdb::DataAbilityPredicates* predicatesPtr = new (std::nothrow) OHOS::NativeRdb::DataAbilityPredicates();
    if (predicatesPtr == nullptr) {
        HILOG_ERROR("%{public}s No memory allocated for predicates", __func__);
        return nullptr;
    }
    *predicatesPtr = predicates;
    napi_value napiPredicates = dataAbilityPredicatesNewInstance_(env, predicatesPtr);
    if (napiPredicates == nullptr) {
        HILOG_ERROR("%{public}s failed to make new instance of dataAbilityPredicates.", __func__);
        delete predicatesPtr;
    }
    HILOG_INFO("%{public}s end.", __func__);
    return napiPredicates;
}
} // namespace AbilityRuntime
} // namespace OHOS