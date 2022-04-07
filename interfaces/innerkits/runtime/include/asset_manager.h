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

#ifndef FOUNDATION_OHOS_ABILITYRUNTIME_ASSET_MANAGER_H
#define FOUNDATION_OHOS_ABILITYRUNTIME_ASSET_MANAGER_H

#include <string>

namespace OHOS::AbilityRuntime {
class Asset {
public:
    Asset() = default;
    ~Asset() = default;

    virtual size_t GetSize() const = 0;
    virtual const uint8_t* GetData() const = 0;
};

class AssetMapping {
public:
    AssetMapping(std::unique_ptr<uint8_t[]> data, size_t size) : data_(std::move(data)), size_(size) {}
    ~AssetMapping() = default;

    size_t GetSize() const
    {
        return size_;
    }

    const uint8_t* GetMapping() const
    {
        return data_.get();
    }

private:
    std::unique_ptr<uint8_t[]> data_ = nullptr;
    size_t size_ = 0;
};

class AssetProvider {
public:
    virtual bool IsValid() const = 0;

    virtual std::unique_ptr<AssetMapping> GetAsMapping(const std::string& assetName) const = 0;

    virtual std::string GetAssetPath(const std::string& assetName) = 0;

    virtual void GetAssetList(const std::string& path, std::vector<std::string>& assetList) = 0;
};

class AssetManager {
public:
    AssetManager() = default;
    ~AssetManager() = default;

    virtual void PushFront(std::shared_ptr<AssetProvider> provider) = 0;

    virtual void PushBack(std::shared_ptr<AssetProvider> provider) = 0;

    virtual std::shared_ptr<Asset> GetAsset(const std::string& assetName) = 0;

    virtual std::string GetAssetPath(const std::string& assetName) = 0;

    virtual void GetAssetList(const std::string& path, std::vector<std::string>& assetList) const = 0;
};
} // namespace OHOS::AbilityRuntime

#endif // FOUNDATION_OHOS_ABILITYRUNTIME_ASSET_MANAGER_H
