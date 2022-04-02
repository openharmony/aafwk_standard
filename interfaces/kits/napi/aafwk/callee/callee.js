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
var rpc = requireNapi("rpc")

const EVENT_CALL_NOTIFY = 1;
const REQUEST_SUCCESS = 0;
const REQUEST_FAILED = 1;

class Callee extends rpc.RemoteObject {
    constructor(des) {
        if (typeof des === 'string') {
            super(des);
            this.callList = new Map();
            console.log("Callee constructor is OK " + typeof des);
        } else {
            console.log("Callee constructor error, des is " + typeof des);
            return null;
        }
    }

    onRemoteRequest(code, data, reply, option) {
        console.log("Callee onRemoteRequest code [" + typeof code + " " + code + "]");
        if (typeof code !== 'number' || typeof data !== 'object' ||
            typeof reply !== 'object' || typeof option !== 'object') {
            console.log("Callee onRemoteRequest error, code is [" +
                typeof code + "], data is [" + typeof data + "], reply is [" +
                typeof reply + "], option is [" + typeof option + "]");
            return false;
        }

        console.log("Callee onRemoteRequest code proc");
        if (code == EVENT_CALL_NOTIFY) {
            if (this.callList == null) {
                console.log("Callee onRemoteRequest error, this.callList is nullptr");
                return false;
            }

            let method = data.readString();
            console.log("Callee onRemoteRequest method [" + method + "]");
            let func = this.callList.get(method);
            if (typeof func !== 'function') {
                console.log("Callee onRemoteRequest error, get func is " + typeof func);
                return false;
            }

            let result = func(data);
            if (typeof result === 'object' && result != null) {
                reply.writeInt(REQUEST_SUCCESS);
                reply.writeString(typeof result);
                reply.writeSequenceable(result);
                console.log("Callee onRemoteRequest code proc Packed data");
            } else {
                reply.writeInt(REQUEST_FAILED);
                reply.writeString(typeof result);
                console.log("Callee onRemoteRequest error, retval is " + REQUEST_FAILED + ", type is " + typeof result);
            }

        } else {
            console.log("Callee onRemoteRequest error, code is " + code);
            return false;
        }

        console.log("Callee onRemoteRequest code proc success");
        return true;
    }

    on(method, callback) {
        if (typeof method !== 'string' || method == "" || typeof callback !== 'function') {
            console.log("Callee on error, method is [" + typeof method + "], typeof callback [" + typeof callback + "]");
            throw new Error("function input parameter error");
            return;
        }

        if (this.callList == null) {
            console.log("Callee on error, this.callList is nullptr");
            throw new Error("Function inner container error");
            return;
        }

        if (this.callList.has(method)) {
            console.log("Callee on error, this.callList not found " + method);
            throw new Error("function is registered");
            return;
        }

        this.callList.set(method, callback);
        console.log("Callee on method [" + method + "]");
    }

    off(method) {
        if (typeof method !== 'string' || method == "") {
            console.log("Callee off error, method is [" + typeof method + "]");
            throw new Error("function input parameter error");
            return;
        }

        if (this.callList == null) {
            console.log("Callee off error, this.callList is null");
            throw new Error("Function inner container error");
            return;
        }

        if (!this.callList.has(method)) {
            console.log("Callee off error, this.callList not found " + method);
            throw new Error("function not registered");
            return;
        }

        this.callList.delete(method);
        console.log("Callee off method [" + method + "]");
    }
}

export default Callee