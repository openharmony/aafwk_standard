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
#ifndef OHOS_AAFWK_WANT_PARAMS_WRAPPER_H
#define OHOS_AAFWK_WANT_PARAMS_WRAPPER_H
#include "ohos/aafwk/base/base_def.h"
#include "ohos/aafwk/base/base_object.h"
#include "ohos/aafwk/content/want_params.h"
namespace OHOS {
namespace AAFwk {
class WantParams;
INTERFACE(IWantParams, a75b9db6 - 9813 - 4371 - 8848 - d2966ce6ec68)
{
    inline static IWantParams *Query(IInterface * object)
    {
        if (object == nullptr) {
            return nullptr;
        }
        return static_cast<IWantParams *>(object->Query(g_IID_IWantParams));
    }

    virtual ErrCode GetValue(WantParams & value) = 0;
};
class WantParamWrapper final : public Object, public IWantParams {
public:
    inline WantParamWrapper(const WantParams &value) : wantParams_(value)
    {}

    inline ~WantParamWrapper()
    {}

    IINTERFACE_DECL();

    ErrCode GetValue(WantParams &value) override;

    bool Equals(IObject &other) override;

    std::string ToString() override;

    static sptr<IWantParams> Box(const WantParams &value);

    static WantParams Unbox(IWantParams *object);

    static bool ValidateStr(const std::string &str);

    static sptr<IWantParams> Parse(const std::string &str);

    static WantParams ParseWantParams(const std::string &str);

    static constexpr char SIGNATURE = 'W';

private:
    WantParams wantParams_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_WANT_PARAMS_WRAPPER_H