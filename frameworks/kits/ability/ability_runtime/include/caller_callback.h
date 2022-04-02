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

#ifndef ABILITY_RUNTIME_CALLER_CALLBACK_H
#define ABILITY_RUNTIME_CALLER_CALLBACK_H

#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {
const std::string ON_RELEASE = "release";
const std::string ON_DIED = "died";
/**
 * @class CallerCallBack
 * CallerCallBack the callback function of caller.
 */
class CallerCallBack : public std::enable_shared_from_this<CallerCallBack> {
public:
    /* Caller's callback object */
    using CallBackClosure = std::function<void(const sptr<IRemoteObject> &)>;
    using OnReleaeClosure = std::function<void(const std::string &)>;

    CallerCallBack() = default;
    virtual ~CallerCallBack() = default;

    void SetCallBack(CallBackClosure callback)
    {
        callback_ = callback;
    };
    void SetOnRelease(OnReleaeClosure onRelease)
    {
        onRelease_ = onRelease;
    };
    void InvokeCallBack(const sptr<IRemoteObject> &remoteObject)
    {
        if (callback_) {
            callback_(remoteObject);
            isCallBack_ = true;
        }
    };
    void InvokeOnRelease(const std::string &key)
    {
        if (onRelease_) {
            onRelease_(key);
        }
    };
    bool IsCallBack()
    {
        return isCallBack_;
    };

private:
    CallBackClosure callback_ = {};
    OnReleaeClosure onRelease_ = {};
    bool isCallBack_ = false;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // ABILITY_RUNTIME_CALLER_CALLBACK_H