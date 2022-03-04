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
var rpc = requireNapi("rpc")

const EVENT_CALL_NOTIFY = 1;
const REQUEST_SUCCESS = 0;
const REQUEST_FAILED = 1;
class Caller {
    constructor(obj) {
        console.log("Caller::constructor obj is " + typeof obj);
        this.__call_obj__ = obj;
        this.releaseCall = false;
    }

    async call(method, data) {
        console.log("Caller call method [" + method + "]");
        if (typeof method !== 'string' || typeof data !== 'object') {
            console.log("Caller call " + typeof method + " " + typeof data);
            throw new Error("function input parameter error");
            return;
        }

        if (method == '' || data == null) {
            console.log("Caller call " + method + ", " + data);
            throw new Error("function input parameter error");
            return;
        }

        if (this.releaseCall) {
            console.log("Caller call this.callee release");
            throw new Error("Function inner object error");
            return;
        }

        if (this.__call_obj__.callee == null) {
            console.log("Caller call this.callee is nullptr");
            throw new Error("Function inner object error");
            return;
        }

        let msgData = rpc.MessageParcel.create();
        msgData.writeString(method);
        let msgReply = rpc.MessageParcel.create();
        let option = rpc.MessageOption();
        msgData.writeSequenceable(data);

        let status = await this.__call_obj__.callee.sendRequest(EVENT_CALL_NOTIFY, msgData, msgReply, option);
        if (!status) {
            console.log("Caller call return status " + status);
            throw new Error("Function execution exception");
            return ;
        }

        let retval = msgReply.readInt();
        let str = msgReply.readString();
        if (retval === REQUEST_SUCCESS && str === 'object') {
            console.log("Caller call return str " + str);
        } else {
            console.log("Caller call retval is [" + retval + "], str [" + str + "]");
            msgData.reclaim();
            msgReply.reclaim();
            throw new Error("Function execution result is abnormal");
        }

        console.log("Caller call msgData SendRequest end");
        return;
    }

    async callWithResult(method, data) {
        console.log("Caller callWithResult method [" + method + "]");
        if (typeof method !== 'string' || typeof data !== 'object') {
            console.log("Caller callWithResult " + typeof method + ", " + typeof data);
            return undefined;
        }

        if (method == '' || data == null) {
            console.log("Caller callWithResult " + method + ", " + data);
            return undefined;
        }

        if (this.releaseCall) {
            console.log("Caller callWithResult this.callee release");
            return undefined;
        }

        if (this.__call_obj__.callee == null) {
            console.log("Caller callWithResult this.callee is nullptr");
            return undefined;
        }

        let msgData = rpc.MessageParcel.create();
        let msgReply = rpc.MessageParcel.create();
        let option = rpc.MessageOption();
        let reply = undefined;
        msgData.writeString(method);
        msgData.writeSequenceable(data);
        let status = await this.__call_obj__.callee.sendRequest(EVENT_CALL_NOTIFY, msgData, msgReply, option);
        if (!status) {
            console.log("Caller callWithResult return data " + status);
            return reply;
        }

        let retval = msgReply.readInt();
        let str = msgReply.readString();
        if (retval === REQUEST_SUCCESS && str === 'object') {
            reply = msgReply;
            console.log("Caller callWithResult return data " + str);
        } else {
            console.log("Caller callWithResult retval is [" + retval + "], str [" + str + "]");
            msgData.reclaim();
            msgReply.reclaim();
        }

        console.log("Caller callWithResult sendMsg return");
        return reply;
    }

    release() {
        console.log("Caller release js called.");
        if (this.releaseCall == true) {
            console.log("Caller release remoteObj releaseState is true");
            throw new Error("Caller release call remoteObj is released");
            return;
        }

        if (this.__call_obj__.callee == null) {
            console.log("Caller release call remoteObj is released");
            throw new Error("Caller release call remoteObj is released");
            return;
        }

        this.releaseCall = true;
        this.__call_obj__.release();
    }

    onRelease(callback) {
        console.log("Caller onRelease jscallback called.");
        if (typeof callback !== 'function') {
            console.log("Caller onRelease " + typeof callback);
            throw new Error("function input parameter error");
            return;
        }

        if (this.releaseCall == true) {
            console.log("Caller onRelease remoteObj releaseState is true");
            throw new Error("Caller onRelease call remoteObj is released");
            return;
        }

        this.__call_obj__.onRelease(callback);
    }
}

export default Caller