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
#include <vector>
#include <uv.h>
#include "securec.h"
#include "hilog_wrapper.h"

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
    napi_create_object(env, &action);
    napi_create_object(env, &entity);

    SetNamedProperty(env, action, "action.system.home", "ACTION_HOME");
    SetNamedProperty(env, action, "action.system.play", "ACTION_PLAY");
    SetNamedProperty(env, action, "action.bundle.add", "ACTION_BUNDLE_ADD");
    SetNamedProperty(env, action, "action.bundle.remove", "ACTION_BUNDLE_REMOVE");
    SetNamedProperty(env, action, "action.bundle.update", "ACTION_BUNDLE_UPDATE");
    SetNamedProperty(env, action, "ability.intent.ORDER_TAXI", "ACTION_ORDER_TAXI");
    SetNamedProperty(env, action, "ability.intent.QUERY_TRAFFIC_RESTRICTION", "ACTION_QUERY_TRAFFIC_RESTRICTION");
    SetNamedProperty(env, action, "ability.intent.PLAN_ROUTE", "ACTION_PLAN_ROUTE");
    SetNamedProperty(env, action, "ability.intent.BOOK_FLIGHT", "ACTION_BOOK_FLIGHT");
    SetNamedProperty(env, action, "ability.intent.BOOK_TRAIN_TICKET", "ACTION_BOOK_TRAIN_TICKET");
    SetNamedProperty(env, action, "ability.intent.BOOK_HOTEL", "ACTION_BOOK_HOTEL");
    SetNamedProperty(env, action, "ability.intent.QUERY_TRAVELLING_GUIDELINE", "ACTION_QUERY_TRAVELLING_GUIDELINE");
    SetNamedProperty(env, action, "ability.intent.QUERY_POI_INFO", "ACTION_QUERY_POI_INFO");
    SetNamedProperty(env, action, "ability.intent.QUERY_CONSTELLATION_FORTUNE", "ACTION_QUERY_CONSTELLATION_FORTUNE");
    SetNamedProperty(env, action, "ability.intent.QUERY_ALMANC", "ACTION_QUERY_ALMANC");
    SetNamedProperty(env, action, "ability.intent.QUERY_WEATHER", "ACTION_QUERY_WEATHER");
    SetNamedProperty(env, action, "ability.intent.QUERY_ENCYCLOPEDIA", "ACTION_QUERY_ENCYCLOPEDIA");
    SetNamedProperty(env, action, "ability.intent.QUERY_RECIPE", "ACTION_QUERY_RECIPE");
    SetNamedProperty(env, action, "ability.intent.BUY_TAKEOUT", "ACTION_BUY_TAKEOUT");
    SetNamedProperty(env, action, "ability.intent.TRANSLATE_TEXT", "ACTION_TRANSLATE_TEXT");
    SetNamedProperty(env, action, "ability.intent.BUY", "ACTION_BUY");
    SetNamedProperty(env, action, "ability.intent.QUERY_LOGISTICS_INFO", "ACTION_QUERY_LOGISTICS_INFO");
    SetNamedProperty(env, action, "ability.intent.SEND_LOGISTICS", "ACTION_SEND_LOGISTICS");
    SetNamedProperty(env, action, "ability.intent.QUERY_SPORTS_INFO", "ACTION_QUERY_SPORTS_INFO");
    SetNamedProperty(env, action, "ability.intent.QUERY_NEWS", "ACTION_QUERY_NEWS");
    SetNamedProperty(env, action, "ability.intent.QUERY_JOKE", "ACTION_QUERY_JOKE");
    SetNamedProperty(env, action, "ability.intent.WATCH_VIDEO_CLIPS", "ACTION_WATCH_VIDEO_CLIPS");
    SetNamedProperty(env, action, "ability.intent.QUERY_STOCK_INFO", "ACTION_QUERY_STOCK_INFO");
    SetNamedProperty(env, action, "ability.intent.LOCALE_CHANGED", "ACTION_LOCALE_CHANGED");

    SetNamedProperty(env, entity, "entity.system.home", "ENTITY_HOME");
    SetNamedProperty(env, entity, "entity.system.video", "ENTITY_VIDEO");

    napi_property_descriptor exportFuncs[] = {
        DECLARE_NAPI_PROPERTY("Action", action),
        DECLARE_NAPI_PROPERTY("Entity", entity),
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

napi_value ActionConstructor(napi_env env, napi_callback_info info)
{
    napi_value jsthis = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
    return jsthis;
}
}  // namespace AppExecFwk
}  // namespace OHOS