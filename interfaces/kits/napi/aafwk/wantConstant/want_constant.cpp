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
#include "want_constant.h"

#include <cstring>
#include <uv.h>
#include <vector>

#include "hilog_wrapper.h"
#include "securec.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief WantConstantInit NAPI module registration.
 *
 * @param env The environment that the Node-API call is invoked under.
 * @param exports An empty object via the exports parameter as a convenience.
 *
 * @return The return value from Init is treated as the exports object for the module.
 */
napi_value WantConstantInit(napi_env env, napi_value exports)
{
    HILOG_INFO("%{public}s,called1", __func__);
    napi_value action = nullptr;
    napi_value entity = nullptr;
    napi_value Flags = nullptr;
    napi_create_object(env, &action);
    napi_create_object(env, &entity);
    napi_create_object(env, &Flags);

    SetNamedProperty(env, action, "ohos.want.action.home", "ACTION_HOME");
    SetNamedProperty(env, action, "ohos.want.action.dial", "ACTION_DIAL");
    SetNamedProperty(env, action, "ohos.want.action.search", "ACTION_SEARCH");
    SetNamedProperty(env, action, "ohos.settings.wireless", "ACTION_WIRELESS_SETTINGS");
    SetNamedProperty(env, action, "ohos.settings.manage.applications", "ACTION_MANAGE_APPLICATIONS_SETTINGS");
    SetNamedProperty(env, action, "ohos.settings.application.details", "ACTION_APPLICATION_DETAILS_SETTINGS");
    SetNamedProperty(env, action, "ohos.want.action.setAlarm", "ACTION_SET_ALARM");
    SetNamedProperty(env, action, "ohos.want.action.showAlarms", "ACTION_SHOW_ALARMS");
    SetNamedProperty(env, action, "ohos.want.action.snoozeAlarm", "ACTION_SNOOZE_ALARM");
    SetNamedProperty(env, action, "ohos.want.action.dismissAlarm", "ACTION_DISMISS_ALARM");
    SetNamedProperty(env, action, "ohos.want.action.dismissTimer", "ACTION_DISMISS_TIMER");
    SetNamedProperty(env, action, "ohos.want.action.sendSms", "ACTION_SEND_SMS");
    SetNamedProperty(env, action, "ohos.want.action.choose", "ACTION_CHOOSE");
    SetNamedProperty(env, action, "ohos.want.action.imageCapture", "ACTION_IMAGE_CAPTURE");
    SetNamedProperty(env, action, "ohos.want.action.videoCapture", "ACTION_VIDEO_CAPTURE");
    SetNamedProperty(env, action, "ohos.want.action.select", "ACTION_SELECT");
    SetNamedProperty(env, action, "ohos.want.action.sendData", "ACTION_SEND_DATA");
    SetNamedProperty(env, action, "ohos.want.action.sendMultipleData", "ACTION_SEND_MULTIPLE_DATA");
    SetNamedProperty(env, action, "ohos.want.action.scanMediaFile", "ACTION_SCAN_MEDIA_FILE");
    SetNamedProperty(env, action, "ohos.want.action.viewData", "ACTION_VIEW_DATA");
    SetNamedProperty(env, action, "ohos.want.action.editData", "ACTION_EDIT_DATA");
    SetNamedProperty(env, action, "ability.want.params.INTENT", "INTENT_PARAMS_INTENT");
    SetNamedProperty(env, action, "ability.want.params.TITLE", "INTENT_PARAMS_TITLE");
    SetNamedProperty(env, action, "ohos.action.fileSelect", "ACTION_FILE_SELECT");
    SetNamedProperty(env, action, "ability.params.stream", "PARAMS_STREAM");
    SetNamedProperty(env, action, "ohos.account.appAccount.action.oauth", "ACTION_APP_ACCOUNT_OAUTH");

    SetNamedProperty(env, entity, "entity.system.default", "ENTITY_DEFAULT");
    SetNamedProperty(env, entity, "entity.system.home", "ENTITY_HOME");
    SetNamedProperty(env, entity, "entity.system.voice", "ENTITY_VOICE");
    SetNamedProperty(env, entity, "entity.system.browsable", "ENTITY_BROWSABLE");
    SetNamedProperty(env, entity, "entity.system.video", "ENTITY_VIDEO");

    SetNamedProperty(env, Flags, 0x00000001, "FLAG_AUTH_READ_URI_PERMISSION");
    SetNamedProperty(env, Flags, 0x00000002, "FLAG_AUTH_WRITE_URI_PERMISSION");
    SetNamedProperty(env, Flags, 0x00000004, "FLAG_ABILITY_FORWARD_RESULT");
    SetNamedProperty(env, Flags, 0x00000008, "FLAG_ABILITY_CONTINUATION");
    SetNamedProperty(env, Flags, 0x00000010, "FLAG_NOT_OHOS_COMPONENT");
    SetNamedProperty(env, Flags, 0x00000020, "FLAG_ABILITY_FORM_ENABLED");
    SetNamedProperty(env, Flags, 0x00000040, "FLAG_AUTH_PERSISTABLE_URI_PERMISSION");
    SetNamedProperty(env, Flags, 0x00000080, "FLAG_AUTH_PREFIX_URI_PERMISSION");
    SetNamedProperty(env, Flags, 0x00000100, "FLAG_ABILITYSLICE_MULTI_DEVICE");
    SetNamedProperty(env, Flags, 0x00000200, "FLAG_START_FOREGROUND_ABILITY");
    SetNamedProperty(env, Flags, 0x00000400, "FLAG_ABILITY_CONTINUATION_REVERSIBLE");
    SetNamedProperty(env, Flags, 0x00000800, "FLAG_INSTALL_ON_DEMAND");
    SetNamedProperty(env, Flags, 0x80000000, "FLAG_INSTALL_WITH_BACKGROUND_MODE");
    SetNamedProperty(env, Flags, 0x00008000, "FLAG_ABILITY_CLEAR_MISSION");
    SetNamedProperty(env, Flags, 0x10000000, "FLAG_ABILITY_NEW_MISSION");
    SetNamedProperty(env, Flags, 0x20000000, "FLAG_ABILITY_MISSION_TOP");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("Action", action),
        DECLARE_NAPI_PROPERTY("Entity", entity),
        DECLARE_NAPI_PROPERTY("Flags", Flags),
    };
    napi_define_properties(env, exports, sizeof(exportFuncs) / sizeof(*exportFuncs), exportFuncs);

    return exports;
}

void SetNamedProperty(napi_env env, napi_value dstObj, const char *objName, const char *propName)
{
    napi_value prop = nullptr;
    napi_create_string_utf8(env, objName, NAPI_AUTO_LENGTH, &prop);
    napi_set_named_property(env, dstObj, propName, prop);
}

void SetNamedProperty(napi_env env, napi_value dstObj, const int32_t objValue, const char *propName)
{
    napi_value prop = nullptr;
    napi_create_int32(env, objValue, &prop);
    napi_set_named_property(env, dstObj, propName, prop);
}

napi_value ActionConstructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    return jsthis;
}
}  // namespace AppExecFwk
}  // namespace OHOS