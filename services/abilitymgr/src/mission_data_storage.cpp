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

#include "mission_data_storage.h"

#include <cctype>

#include "file_util.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace AAFwk {
MissionDataStorage::MissionDataStorage(int userId)
{
    userId_ = userId;
}

MissionDataStorage::~MissionDataStorage()
{}

void MissionDataStorage::SetEventHandler(const std::shared_ptr<AppExecFwk::EventHandler> &handler)
{
    handler_ = handler;
}

bool MissionDataStorage::LoadAllMissionInfo(std::list<InnerMissionInfo> &missionInfoList)
{
    std::vector<std::string> fileNameVec;
    std::string dirPath = GetMissionDataDirPath();
    OHOS::HiviewDFX::FileUtil::GetDirFiles(dirPath, fileNameVec);

    for (auto fileName : fileNameVec) {
        if (!CheckFileNameValid(fileName)) {
            HILOG_ERROR("load mission info: file name %{public}s invalid.", fileName.c_str());
            continue;
        }

        std::string content;
        bool loadFile = OHOS::HiviewDFX::FileUtil::LoadStringFromFile(fileName, content);
        if (!loadFile) {
            HILOG_ERROR("load string from file %{public}s failed.", fileName.c_str());
            continue;
        }

        InnerMissionInfo misssionInfo;
        misssionInfo.FromJsonStr(content);
        missionInfoList.push_back(misssionInfo);
    }
    return true;
}

void MissionDataStorage::SaveMissionInfo(const InnerMissionInfo &missionInfo)
{
    std::string filePath = GetMissionDataFilePath(missionInfo.missionInfo.id);
    std::string dirPath = OHOS::HiviewDFX::FileUtil::ExtractFilePath(filePath);
    if (!OHOS::HiviewDFX::FileUtil::FileExists(dirPath)) {
        bool createDir = OHOS::HiviewDFX::FileUtil::ForceCreateDirectory(dirPath);
        if (!createDir) {
            HILOG_ERROR("create dir %{public}s failed.", dirPath.c_str());
            return;
        }
    }

    std::string jsonStr = missionInfo.ToJsonStr();
    bool saveMissionFile = OHOS::HiviewDFX::FileUtil::SaveStringToFile(filePath, jsonStr, true);
    if (!saveMissionFile) {
        HILOG_ERROR("save mission file %{public}s failed.", filePath.c_str());
    }
}

void MissionDataStorage::DeleteMissionInfo(int missionId)
{
    std::string filePath = GetMissionDataFilePath(missionId);
    bool removeMissionFile = OHOS::HiviewDFX::FileUtil::RemoveFile(filePath);
    if (!removeMissionFile) {
        HILOG_ERROR("remove mission file %{public}s failed.", filePath.c_str());
    }
}

std::string MissionDataStorage::GetMissionDataDirPath()
{
    return TASK_DATA_FILE_BASE_PATH + "/" + std::to_string(userId_) + "/" + MISSION_DATA_FILE_PATH;
}

std::string MissionDataStorage::GetMissionDataFilePath(int missionId)
{
    return GetMissionDataDirPath() + "/"
        + MISSION_JSON_FILE_PREFIX + "_" + std::to_string(missionId) + JSON_FILE_SUFFIX;
}

bool MissionDataStorage::CheckFileNameValid(const std::string &fileName)
{
    std::string fileNameExcludePath = OHOS::HiviewDFX::FileUtil::ExtractFileName(fileName);
    if (fileNameExcludePath.find(MISSION_JSON_FILE_PREFIX) != 0) {
        return false;
    }

    if (fileNameExcludePath.find("_") != MISSION_JSON_FILE_PREFIX.length()) {
        return false;
    }

    if (fileNameExcludePath.find(JSON_FILE_SUFFIX) != fileNameExcludePath.length() - JSON_FILE_SUFFIX.length()) {
        return false;
    }

    int missionIdLength = fileNameExcludePath.find(JSON_FILE_SUFFIX) - fileNameExcludePath.find("_") - 1;
    std::string missionId = fileNameExcludePath.substr(fileNameExcludePath.find("_") + 1, missionIdLength);
    for (auto ch : missionId) {
        if (!isdigit(ch)) {
            return false;
        }
    }

    return true;
}
}  // namespace AAFwk
}  // namespace OHOS
