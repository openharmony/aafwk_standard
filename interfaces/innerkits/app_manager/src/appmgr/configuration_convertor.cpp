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

#include "configuration_convertor.h"

#include "configuration.h"

namespace OHOS::AppExecFwk {
constexpr float DPI_BASE = 160.0;

Global::Resource::ColorMode ConvertColorMode(std::string colormode)
{
    auto resolution = Global::Resource::ColorMode::COLOR_MODE_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::ColorMode>> resolutions = {
        { "dark", Global::Resource::ColorMode::DARK },
        { "light", Global::Resource::ColorMode::LIGHT },
    };

    for (const auto& [tempColorMode, value] : resolutions) {
        if (tempColorMode == colormode) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

Global::Resource::Direction ConvertDirection(int32_t height, int32_t width)
{
    return height >= width ? Global::Resource::Direction::DIRECTION_VERTICAL :
        Global::Resource::Direction::DIRECTION_HORIZONTAL;
}

Global::Resource::Direction ConvertDirection(std::string direction)
{
    auto resolution = Global::Resource::Direction::DIRECTION_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::Direction>> resolutions = {
        { "vertical", Global::Resource::Direction::DIRECTION_VERTICAL },
        { "horizontal", Global::Resource::Direction::DIRECTION_HORIZONTAL },
    };

    for (const auto& [tempDirection, value] : resolutions) {
        if (tempDirection == direction) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

Global::Resource::ScreenDensity ConvertDensity(float density)
{
    static const std::vector<std::pair<float, Global::Resource::ScreenDensity>> resolutions = {
        { 0.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET },
        { 120.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_SDPI },
        { 160.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_MDPI },
        { 240.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_LDPI },
        { 320.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_XLDPI },
        { 480.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_XXLDPI },
        { 640.0, Global::Resource::ScreenDensity::SCREEN_DENSITY_XXXLDPI },
    };

    float deviceDpi = density * DPI_BASE;
    auto resolution = Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET;
    for (const auto& [dpi, value] : resolutions) {
        resolution = value;
        if (deviceDpi <= dpi) {
            break;
        }
    }
    return resolution;
}

Global::Resource::ScreenDensity ConvertDensity(std::string density)
{
    auto resolution = Global::Resource::ScreenDensity::SCREEN_DENSITY_NOT_SET;

    static const std::vector<std::pair<std::string, Global::Resource::ScreenDensity>> resolutions = {
        { "sdpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_SDPI },
        { "mdpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_MDPI },
        { "ldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_LDPI },
        { "xldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_XLDPI },
        { "xxldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_XXLDPI },
        { "xxxldpi", Global::Resource::ScreenDensity::SCREEN_DENSITY_XXXLDPI },
    };

    for (const auto& [tempdensity, value] : resolutions) {
        if (tempdensity == density) {
            resolution = value;
            break;
        }
    }

    return resolution;
}

int32_t ConvertDisplayId(std::string displayId)
{
    if (displayId == ConfigurationInner::EMPTY_STRING) {
        return -1;
    }

    return std::stoi(displayId);
}

std::string GetColorModeStr(int32_t colormode)
{
    std::string ret("no_color_mode");

    switch (colormode) {
        case Global::Resource::ColorMode::DARK:
            ret = ConfigurationInner::COLOR_MODE_DARK;
            break;
        case Global::Resource::ColorMode::LIGHT:
            ret = ConfigurationInner::COLOR_MODE_LIGHT;
            break;
        default:
            break;
    }

    return ret;
}

std::string GetDirectionStr(Global::Resource::Direction direction)
{
    std::string ret("no_direction");

    switch (direction) {
        case Global::Resource::Direction::DIRECTION_VERTICAL:
            ret = ConfigurationInner::DIRECTION_VERTICAL;
            break;
        case Global::Resource::Direction::DIRECTION_HORIZONTAL:
            ret = ConfigurationInner::DIRECTION_HORIZONTAL;
            break;
        default:
            break;
    }

    return ret;
}

std::string GetDirectionStr(int32_t height, int32_t width)
{
    return GetDirectionStr(ConvertDirection(height, width));
}

std::string GetDensityStr(Global::Resource::ScreenDensity density)
{
    std::string ret("no_screen_density");

    static const std::vector<std::pair<Global::Resource::ScreenDensity, std::string>> resolutions = {
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_SDPI, "sdpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_MDPI, "mdpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_LDPI, "ldpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_XLDPI, "xldpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_XXLDPI, "xxldpi" },
        { Global::Resource::ScreenDensity::SCREEN_DENSITY_XXXLDPI, "xxxldpi" },
    };

    for (const auto& [dpi, value] : resolutions) {
        if (dpi == density) {
            return value;
        }
    }

    return ret;
}

std::string GetDensityStr(float density)
{
    return GetDensityStr(ConvertDensity(density));
}
} // namespace OHOS::AppExecFwk