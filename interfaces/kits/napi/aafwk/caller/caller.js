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
class Caller {
    constructor(obj) {
        console.log("Caller::constructor obj is " + typeof obj);
        this.__call_obj__ = obj;
        this.releaseCall = false;
    }

    call(method, data) {
        return new Promise(async (resolve, reject) => {
            console.log("Caller call method [" + method + "]");
            if (typeof method !== 'string' || typeof data !== 'object') {
                console.log("Caller call " + typeof method + " " + typeof data);
                reject(new Error("function input parameter error"));
                return;
            }

            if (method == '' || data == null) {
                console.log("Caller call " + method + ", " + data);
                reject(new Error("function input parameter error"));
                return;
            }

            if (this.releaseCall == true) {
                console.log("Caller call this.callee release");
                reject(new Error("function inner object error"));
                return;
            }

            if (this.__call_obj__.callee == null) {
                console.log("Caller call this.callee is nullptr");
                reject(new Error("function inner object error"));
                return;
            }

            console.log("Caller call msgData rpc.MessageParcel create");
            let msgData = rpc.MessageParcel.create();
            msgData.writeString(method);
            let msgReply = rpc.MessageParcel.create();
            let option = rpc.MessageOption();
            msgData.writeSequenceable(data);

            let retData = undefined;
            try {
                retData = retData = await this.__call_obj__.callee.sendRequest(EVENT_CALL_NOTIFY, msgData, msgReply, option);
                console.log("Caller call msgData rpc.SendRequest called");
                if (retData.errCode != 0) {
                    msgData.reclaim();
                    msgReply.reclaim();
                    console.log("Caller call return errCode " + retData.errCode);
                    reject(new Error("function execution exception"));
                    return;
                }
            } catch (e) {
                console.log("Caller call msgData rpc.sendRequest error " + e);
            }

            try {
                let retval = retData.reply.readInt();
                let str = retData.reply.readString();
                if (retval === REQUEST_SUCCESS && str === 'object') {
                    console.log("Caller call return str " + str);
                    msgData.reclaim();
                    msgReply.reclaim();
                } else {
                    console.log("Caller call retval is [" + retval + "], str [" + str + "]");
                    msgData.reclaim();
                    msgReply.reclaim();
                    reject(new Error("function execution result is abnormal"));
                    return;
                }
            } catch (e) {
                console.log("Caller call msgData SendRequest retval error");
                reject(new Error("function execution result is abnormal"));
                return;
            }

            console.log("Caller call msgData SendRequest end");
            resolve(undefined);
            return;
        });
    }

    callWithResult(method, data) {
        return new Promise(async (resolve, reject) => {
            console.log("Caller callWithResult method [" + method + "]");
            if (typeof method !== 'string' || typeof data !== 'object') {
                console.log("Caller callWithResult " + typeof method + " " + typeof data);
                reject(new Error("function input parameter error"));
                return;
            }

            if (method == '' || data == null) {
                console.log("Caller callWithResult " + method + ", " + data);
                reject(new Error("function input parameter error"));
                return;
            }

            if (this.releaseCall == true) {
                console.log("Caller callWithResult this.callee release");
                reject(new Error("function inner object error"));
                return;
            }

            if (this.__call_obj__.callee == null) {
                console.log("Caller callWithResult this.callee is nullptr");
                reject(new Error("function inner object error"));
                return;
            }

            console.log("Caller callWithResult msgData rpc.MessageParcel create");
            let msgData = rpc.MessageParcel.create();
            msgData.writeString(method);
            let msgReply = rpc.MessageParcel.create();
            let option = rpc.MessageOption();
            msgData.writeSequenceable(data);

            let reply = undefined;
            let retData = undefined;
            try {
                retData = retData = await this.__call_obj__.callee.sendRequest(EVENT_CALL_NOTIFY, msgData, msgReply, option);
                console.log("Caller callWithResult msgData rpc.SendRequest called");
                if (retData.errCode != 0) {
                    msgData.reclaim();
                    msgReply.reclaim();
                    console.log("Caller callWithResult return errCode " + retData.errCode);
                    reject(new Error("function execution exception"));
                    return;
                }
            } catch (e) {
                console.log("Caller call msgData rpc.MessageParcel error " + e);
            }

            try {
                let retval = retData.reply.readInt();
                let str = retData.reply.readString();
                if (retval === REQUEST_SUCCESS && str === 'object') {
                    console.log("Caller callWithResult return str " + str);
                    msgData.reclaim();
                    reply = retData.reply;
                } else {
                    console.log("Caller callWithResult retval is [" + retval + "], str [" + str + "]");
                    msgData.reclaim();
                    msgReply.reclaim();
                    reject(new Error("function execution result is abnormal"));
                    return;
                }
            } catch (e) {
                console.log("Caller callWithResult msgData SendRequest retval error");
                reject(new Error("function execution result is abnormal"));
                return;
            }

            console.log("Caller callWithResult msgData SendRequest end");
            resolve(reply);
            return;
        });
    }

    release() {
        console.log("Caller release js called.");
        if (this.releaseCall == true) {
            console.log("Caller release remoteObj releaseState is true");
            throw new Error("caller release call remoteObj is released");
            return;
        }

        if (this.__call_obj__.callee == null) {
            console.log("Caller release call remoteObj is released");
            throw new Error("caller release call remoteObj is released");
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
            throw new Error("caller onRelease call remoteObj is released");
            return;
        }

        this.__call_obj__.onRelease(callback);
    }
}

export default Caller