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

#ifndef FOUNDATION_OHOS_ABILITYRUNTIME_JS_FILE_ASSET_PROVIDER_H
#define FOUNDATION_OHOS_ABILITYRUNTIME_JS_FILE_ASSET_PROVIDER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "asset_manager.h"

namespace OHOS::AbilityRuntime {
class JsFileAssetProvider final : public AssetProvider {
public:
    JsFileAssetProvider() = default;
    ~JsFileAssetProvider() = default;

    bool Initialize(const std::string& packagePath, const std::vector<std::string>& assetBasePaths);

    bool IsValid() const override;

    std::unique_ptr<AssetMapping> GetAsMapping(const std::string& assetName) const override;

    std::string GetAssetPath(const std::string& assetName) override;

    void GetAssetList(const std::string& path, std::vector<std::string>& assetList) override;

private:
    mutable std::mutex mutex_;
    std::string packagePath_;
    std::vector<std::string> assetBasePaths_;
};
} // namespace OHOS::AbilityRuntime

#endif // FOUNDATION_OHOS_ABILITYRUNTIME_JS_FILE_ASSET_PROVIDER_H
