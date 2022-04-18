/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef FOUNDATION_ABILITYRUNTIME_OHOS_JS_SERVICE_EXTENSION_H
#define FOUNDATION_ABILITYRUNTIME_OHOS_JS_SERVICE_EXTENSION_H

#include "configuration.h"
#include "service_extension.h"

class NativeReference;
class NativeValue;

namespace OHOS {
namespace AbilityRuntime {
class ServiceExtension;
class JsRuntime;
/**
 * @brief Basic service components.
 */
class JsServiceExtension : public ServiceExtension,
                           public std::enable_shared_from_this<JsServiceExtension> {
public:
    JsServiceExtension(JsRuntime& jsRuntime);
    virtual ~JsServiceExtension() override;

    /**
     * @brief Create JsServiceExtension.
     *
     * @param runtime The runtime.
     * @return The JsServiceExtension instance.
     */
    static JsServiceExtension* Create(const std::unique_ptr<Runtime>& runtime);

    /**
     * @brief Init the extension.
     *
     * @param record the extension record.
     * @param application the application info.
     * @param handler the extension handler.
     * @param token the remote token.
     */
    virtual void Init(const std::shared_ptr<AppExecFwk::AbilityLocalRecord> &record,
        const std::shared_ptr<AppExecFwk::OHOSApplication> &application,
        std::shared_ptr<AppExecFwk::AbilityHandler> &handler,
        const sptr<IRemoteObject> &token) override;

    /**
     * @brief Called when this extension is started. You must override this function if you want to perform some
     *        initialization operations during extension startup.
     *
     * This function can be called only once in the entire lifecycle of an extension.
     * @param Want Indicates the {@link Want} structure containing startup information about the extension.
     */
    virtual void OnStart(const AAFwk::Want &want) override;

    /**
     * @brief Called when this Service extension is connected for the first time.
     *
     * You can override this function to implement your own processing logic.
     *
     * @param want Indicates the {@link Want} structure containing connection information about the Service extension.
     * @return Returns a pointer to the <b>sid</b> of the connected Service extension.
     */
    virtual sptr<IRemoteObject> OnConnect(const AAFwk::Want &want) override;

    /**
     * @brief Called when all abilities connected to this Service extension are disconnected.
     *
     * You can override this function to implement your own processing logic.
     *
     */
    virtual void OnDisconnect(const AAFwk::Want &want) override;

    /**
     * @brief Called back when Service is started.
     * This method can be called only by Service. You can use the StartAbility(ohos.aafwk.content.Want) method to start
     * Service. Then the system calls back the current method to use the transferred want parameter to execute its own
     * logic.
     *
     * @param want Indicates the want of Service to start.
     * @param restart Indicates the startup mode. The value true indicates that Service is restarted after being
     * destroyed, and the value false indicates a normal startup.
     * @param startId Indicates the number of times the Service extension has been started. The startId is incremented
     * by 1 every time the extension is started. For example, if the extension has been started for six times, the
     * value of startId is 6.
     */
    virtual void OnCommand(const AAFwk::Want &want, bool restart, int startId) override;

    /**
     * @brief Called when this extension enters the <b>STATE_STOP</b> state.
     *
     * The extension in the <b>STATE_STOP</b> is being destroyed.
     * You can override this function to implement your own processing logic.
     */
    virtual void OnStop() override;

    /**
     * @brief Called when the system configuration is updated.
     *
     * @param configuration Indicates the updated configuration information.
     */
    void OnConfigurationUpdated(const AppExecFwk::Configuration& configuration) override;

    /**
     * @brief Called when extension need dump info.
     *
     * @param params The params from service.
     * @param info The dump info to show.
     */
    virtual void Dump(const std::vector<std::string> &params, std::vector<std::string> &info) override;

private:
    NativeValue* CallObjectMethod(const char* name, NativeValue* const* argv = nullptr, size_t argc = 0);

    void GetSrcPath(std::string &srcPath);

    JsRuntime& jsRuntime_;
    std::unique_ptr<NativeReference> jsObj_;
    std::shared_ptr<NativeReference> shellContextRef_ = nullptr;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // FOUNDATION_ABILITYRUNTIME_OHOS_JS_SERVICE_EXTENSION_H