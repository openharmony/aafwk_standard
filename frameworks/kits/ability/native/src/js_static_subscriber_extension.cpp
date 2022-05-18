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

#include "js_static_subscriber_extension.h"

#include "ability_info.h"
#include "ability_handler.h"
#include "hilog_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_static_subscriber_extension_context.h"
#include "napi_common_want.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_remote_object.h"
#include "static_subscriber_stub_imp.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
}

using namespace OHOS::AppExecFwk;
JsStaticSubscriberExtension* JsStaticSubscriberExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsStaticSubscriberExtension(static_cast<JsRuntime&>(*runtime));
}

JsStaticSubscriberExtension::JsStaticSubscriberExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsStaticSubscriberExtension::~JsStaticSubscriberExtension() = default;

void JsStaticSubscriberExtension::Init(const std::shared_ptr<AbilityLocalRecord>& record,
    const std::shared_ptr<OHOSApplication>& application,
    std::shared_ptr<AbilityHandler>& handler,
    const sptr<IRemoteObject>& token)
{
    StaticSubscriberExtension::Init(record, application, handler, token);
    if (Extension::abilityInfo_->srcEntrance.empty()) {
        HILOG_ERROR("%{public}s abilityInfo srcEntrance is empty", __func__);
        return;
    }
    std::string srcPath(Extension::abilityInfo_->moduleName + "/");
    srcPath.append(Extension::abilityInfo_->srcEntrance);
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    HILOG_INFO("JsStaticSubscriberExtension::Init moduleName:%{public}s,srcPath:%{public}s.",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto& engine = jsRuntime_.GetNativeEngine();

    jsObj_ = jsRuntime_.LoadModule(moduleName, srcPath);
    if (jsObj_ == nullptr) {
        HILOG_ERROR("Failed to get jsObj_");
        return;
    }
    HILOG_INFO("JsStaticSubscriberExtension::Init ConvertNativeValueTo.");
    NativeObject* obj = ConvertNativeValueTo<NativeObject>(jsObj_->Get());
    if (obj == nullptr) {
        HILOG_ERROR("Failed to get JsStaticSubscriberExtension object");
        return;
    }

    auto context = GetContext();
    if (context == nullptr) {
        HILOG_ERROR("Failed to get context");
        return;
    }
    HILOG_INFO("JsStaticSubscriberExtension::Init CreateJsStaticSubscriberExtensionContext.");
    NativeValue* contextObj = CreateJsStaticSubscriberExtensionContext(engine, context);
    auto shellContextRef = jsRuntime_.LoadSystemModule("application.StaticSubscriberExtensionContext",
        &contextObj, ARGC_ONE);
    contextObj = shellContextRef->Get();
    HILOG_INFO("JsStaticSubscriberExtension::Init Bind.");
    context->Bind(jsRuntime_, shellContextRef.release());
    HILOG_INFO("JsStaticSubscriberExtension::SetProperty.");
    obj->SetProperty("context", contextObj);

    auto nativeObj = ConvertNativeValueTo<NativeObject>(contextObj);
    if (nativeObj == nullptr) {
        HILOG_ERROR("Failed to get static subscriber extension native object");
        return;
    }

    HILOG_INFO("Set static subscriber extension context");

    nativeObj->SetNativePointer(new std::weak_ptr<AbilityRuntime::Context>(context),
        [](NativeEngine*, void* data, void*) {
            HILOG_INFO("Finalizer for weak_ptr static subscriber extension context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::Context>*>(data);
        }, nullptr);

    HILOG_INFO("JsStaticSubscriberExtension::Init end.");
}

void JsStaticSubscriberExtension::OnStart(const AAFwk::Want& want)
{
    Extension::OnStart(want);
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
}

void JsStaticSubscriberExtension::OnStop()
{
    Extension::OnStop();
    HILOG_INFO("%{public}s end.", __func__);
}

sptr<IRemoteObject> JsStaticSubscriberExtension::OnConnect(const AAFwk::Want& want)
{
    Extension::OnConnect(want);
    HILOG_INFO("%{public}s begin.", __func__);
    sptr<StaticSubscriberStubImp> remoteObject = new (std::nothrow) StaticSubscriberStubImp(
        std::static_pointer_cast<JsStaticSubscriberExtension>(shared_from_this()));
    HILOG_INFO("%{public}s end. ", __func__);
    return remoteObject->AsObject();
}

void JsStaticSubscriberExtension::OnDisconnect(const AAFwk::Want& want)
{
    Extension::OnDisconnect(want);
    HILOG_INFO("%{public}s begin.", __func__);
    HILOG_INFO("%{public}s end.", __func__);
}

void JsStaticSubscriberExtension::OnReceiveEvent(std::shared_ptr<EventFwk::CommonEventData> data)
{
    HILOG_INFO("%{public}s begin.", __func__);

    if (handler_ == nullptr) {
        return;
    }
    auto task = [this, data]() {
        if (data == nullptr) {
            HILOG_ERROR("OnReceiveEvent common event data == nullptr");
            return;
        }
        StaticSubscriberExtension::OnReceiveEvent(data);
        if (!jsObj_) {
            HILOG_ERROR("Not found StaticSubscriberExtension.js");
            return;
        }
        HandleScope handleScope(jsRuntime_);
        NativeEngine& nativeEngine = jsRuntime_.GetNativeEngine();
        NativeValue* jCommonEventData = nativeEngine.CreateObject();
        NativeObject* commonEventData = ConvertNativeValueTo<NativeObject>(jCommonEventData);
        Want want = data->GetWant();
        commonEventData->SetProperty("event",
            nativeEngine.CreateString(want.GetAction().c_str(), want.GetAction().size()));
        commonEventData->SetProperty("bundleName",
            nativeEngine.CreateString(want.GetBundle().c_str(), want.GetBundle().size()));
        commonEventData->SetProperty("code", nativeEngine.CreateNumber(data->GetCode()));
        commonEventData->SetProperty("data",
            nativeEngine.CreateString(data->GetData().c_str(), data->GetData().size()));
        napi_value napiParams = AppExecFwk::WrapWantParams(
            reinterpret_cast<napi_env>(&nativeEngine), want.GetParams());
        NativeValue* nativeParams = reinterpret_cast<NativeValue*>(napiParams);
        commonEventData->SetProperty("parameters", nativeParams);

        NativeValue* argv[] = {jCommonEventData};

        NativeValue* value = jsObj_->Get();
        NativeObject* obj = ConvertNativeValueTo<NativeObject>(value);
        if (obj == nullptr) {
            HILOG_ERROR("Failed to get StaticSubscriberExtension object");
            return;
        }

        NativeValue* method = obj->GetProperty("onReceiveEvent");
        if (method == nullptr) {
            HILOG_ERROR("Failed to get onReceiveEvent from StaticSubscriberExtension object");
            return;
        }
        nativeEngine.CallFunction(value, method, argv, ARGC_ONE);
        HILOG_INFO("JsStaticSubscriberExtension js receive event called.");
    };
    handler_->PostTask(task);
}
} // namespace AbilityRuntime
} // namespace OHOS