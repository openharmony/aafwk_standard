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

#include "napi_common_configuration.h"
#include "hilog_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AppExecFwk {
EXTERN_C_START

bool InnerWrapConfigurationString(
    napi_env env, napi_value jsObject, const std::string &key, const std::string &value)
{
    if (!value.empty()) {
        HILOG_INFO("%{public}s called. key=%{public}s, value=%{public}s", __func__, key.c_str(), value.c_str());
        napi_value jsValue = WrapStringToJS(env, value);
        if (jsValue != nullptr) {
            NAPI_CALL_BASE(env, napi_set_named_property(env, jsObject, key.c_str(), jsValue), false);
            return true;
        }
    }
    return false;
}

napi_value WrapConfiguration(napi_env env, const AppExecFwk::Configuration &configuration)
{
    HILOG_INFO("%{public}s called, config size %{public}d", __func__, static_cast<int>(configuration.GetItemSize()));
    napi_value jsObject = nullptr;
    NAPI_CALL(env, napi_create_object(env, &jsObject));

    std::vector<std::string> keys = OHOS::AppExecFwk::ConfigurationInner::SystemConfigurationKeyStore;
    for (auto const &key : keys) {
        std::string value = configuration.GetItem(key);
        if (value.empty()) {
            HILOG_INFO("value is empty");
            continue;
        }

        std::size_t pos = key.rfind(DOT_STRING);
        if (pos != std::string::npos) {
            InnerWrapConfigurationString(env, jsObject, key.substr(pos+1), value);
        } else {
            InnerWrapConfigurationString(env, jsObject, key, value);
        }
    }
    return jsObject;
}

bool UnwrapConfiguration(napi_env env, napi_value param, Configuration &config)
{
    HILOG_INFO("%{public}s called.", __func__);

    if (!IsTypeForNapiValue(env, param, napi_object)) {
        HILOG_INFO("%{public}s called. Params is invalid.", __func__);
        return false;
    }

    std::string language {""};
    if (UnwrapStringByPropertyName(env, param, "language", language)) {
        HILOG_DEBUG("The parsed language part %{public}s", language.c_str());
        if (!config.AddItem(GlobalConfigurationKey::SYSTEM_LANGUAGE, language)) {
            HILOG_ERROR("language Parsing failed");
            return false;
        }
    }

    return true;
}
EXTERN_C_END
}  // namespace AppExecFwk
}  // namespace OHOS
