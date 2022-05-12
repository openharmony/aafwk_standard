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

#ifndef OHOS_AAFWK_DISTRIBUTE_REQ_PARAM_H
#define OHOS_AAFWK_DISTRIBUTE_REQ_PARAM_H

#include <string>

#include "distribute_constants.h"
#include "element_name.h"

namespace OHOS {
namespace AppExecFwk {
class DistributeReqParam : public ElementName {
public:
    DistributeReqParam() = default;
    ~DistributeReqParam() = default;

    inline std::string GetAction() const
    {
        return action_;
    }

    inline void SetAction(const std::string &action)
    {
        action_ = action;
    }

    inline std::vector<std::string> GetEntities() const
    {
        return entities_;
    }

    inline void SetEntities(const std::vector<std::string> &entities)
    {
        entities_ = entities;
    }

    inline int GetDeviceType() const
    {
        return deviceType_;
    }

    inline void SetDeviceType(const int deviceType)
    {
        deviceType_ = deviceType;
    }

    inline int GetFlag() const
    {
        return flag_;
    }
    inline void SetFlag(const int flag)
    {
        flag_ = flag;
    }

    inline std::string GetData() const
    {
        return data_;
    }

    inline void SetData(const std::string &data)
    {
        data_ = data;
    }

    inline std::string GetUrl() const
    {
        return url_;
    }

    void SetUrl(const std::string url)
    {
        url_ = url;
    }

    inline std::string GetNetworkId() const
    {
        return netWorkId_;
    }

    inline void SetNetWorkId(const std::string &netWorkId)
    {
        netWorkId_ = netWorkId;
    }

    inline std::string GetType() const
    {
        return type_;
    }

    inline void SetType(const std::string &type)
    {
        type_ = type;
    }

    inline std::string GetExtraParams() const
    {
        return extraParams_;
    }

    inline void SetExtraParams(const std::string &extraParams)
    {
        extraParams_ = extraParams;
    }
private:
    std::vector<std::string> entities_;
    std::string action_;
    std::string netWorkId_;
    int deviceType_ = DistributeConstants::DEVICE_TYPE_DEFAULT;
    std::string data_;
    int flag_ = 0;
    std::string url_;
    std::string type_;
    std::string extraParams_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_AAFWK_DISTRIBUTE_REQ_PARAM_H
