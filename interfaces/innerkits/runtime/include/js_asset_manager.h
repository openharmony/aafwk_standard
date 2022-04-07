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

#ifndef FOUNDATION_OHOS_ABILITYRUNTIME_JS_ASSET_MANAGER_H
#define FOUNDATION_OHOS_ABILITYRUNTIME_JS_ASSET_MANAGER_H

#include <deque>
#include <vector>

#include "asset_manager.h"

namespace OHOS::AbilityRuntime {
class JsAsset final : public Asset {
public:
    explicit JsAsset(std::unique_ptr<AssetMapping> mapping) : mapping_(std::move(mapping)) {}
    ~JsAsset() = default;

    size_t GetSize() const override
    {
        return mapping_ ? mapping_->GetSize() : 0;
    }

    const uint8_t* GetData() const override
    {
        return mapping_ ? mapping_->GetMapping() : nullptr;
    }

private:
    std::unique_ptr<AssetMapping> mapping_;
};

class JsAssetManager final : public AssetManager {
public:
    JsAssetManager() = default;
    ~JsAssetManager() = default;

    void PushFront(std::shared_ptr<AssetProvider> provider) override
    {
        if (!provider || !provider->IsValid()) {
            return;
        }
        providers_.push_front(std::move(provider));
    }

    void PushBack(std::shared_ptr<AssetProvider> provider) override
    {
        if (!provider || !provider->IsValid()) {
            return;
        }
        providers_.push_back(std::move(provider));
    }

    std::shared_ptr<Asset> GetAsset(const std::string& assetName) override;

    std::string GetAssetPath(const std::string& assetName) override;

    void GetAssetList(const std::string& path, std::vector<std::string>& assetList) const override;

private:
    std::deque<std::shared_ptr<AssetProvider>> providers_;
};
} // namespace OHOS::AbilityRuntime

#endif // FOUNDATION_OHOS_ABILITYRUNTIME_JS_ASSET_MANAGER_H
