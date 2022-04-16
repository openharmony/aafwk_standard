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

#ifndef OHOS_IDL_AUTOPTR_H
#define OHOS_IDL_AUTOPTR_H

namespace OHOS {
namespace Idl {

template<class T>
class AutoPtr {
public:
    inline AutoPtr()
        : mPtr(nullptr)
    {}

    AutoPtr(
        /* [in] */ T* other);

    AutoPtr(
        /* [in] */ const AutoPtr<T>& other);

    AutoPtr(
        /* [in] */ AutoPtr<T>&& other);

    ~AutoPtr();

    AutoPtr& operator=(
        /* [in] */ T* other);

    AutoPtr& operator=(
        /* [in] */ const AutoPtr<T>& other);

    AutoPtr& operator=(
        /* [in] */ AutoPtr<T>&& other);

    void MoveTo(
        /* [out] */ T** other);

    inline operator T*() const;

    inline T** operator&();

    inline T* operator->() const;

    inline T& operator*() const;

    inline T* Get() const;

    inline bool operator==(
        /* [in] */ T* other) const;

    inline bool operator==(
        /* [in] */ const AutoPtr<T>& other) const;

    inline bool operator!=(
        /* [in] */ T* other) const;

    inline bool operator!=(
        /* [in] */ const AutoPtr<T>& other) const;

    inline bool operator>(
        /* [in] */ T* other) const;

    inline bool operator>(
        /* [in] */ const AutoPtr<T>& other) const;

    inline bool operator<(
        /* [in] */ T* other) const;

    inline bool operator<(
        /* [in] */ const AutoPtr<T>& other) const;

    inline bool operator<=(
        /* [in] */ T* other) const;

    inline bool operator<=(
        /* [in] */ const AutoPtr<T>& other) const;

    inline bool operator>=(
        /* [in] */ T* other) const;

    inline bool operator>=(
        /* [in] */ const AutoPtr<T>& other) const;

private:
    T* mPtr;
};

template<class T>
AutoPtr<T>::AutoPtr(
    /* [in] */ T* other)
    : mPtr(other)
{
    if (mPtr != nullptr) {
        mPtr->AddRef();
    }
}

template<class T>
AutoPtr<T>::AutoPtr(
    /* [in] */ const AutoPtr<T>& other)
    : mPtr(other.mPtr)
{
    if (mPtr != nullptr) {
        mPtr->AddRef();
    }
}

template<class T>
AutoPtr<T>::AutoPtr(
    /* [in] */ AutoPtr<T>&& other)
    : mPtr(other.mPtr)
{
    other.mPtr = nullptr;
}

template<class T>
AutoPtr<T>::~AutoPtr()
{
    if (mPtr != nullptr) {
        mPtr->Release();
    }
}

template<class T>
AutoPtr<T>& AutoPtr<T>::operator=(
    /* [in] */ T* other)
{
    if (mPtr == other) return *this;

    if (other != nullptr) {
        other->AddRef();
    }
    if (mPtr != nullptr) {
        mPtr->Release();
    }
    mPtr = other;
    return *this;
}

template<class T>
AutoPtr<T>& AutoPtr<T>::operator=(
    /* [in] */ const AutoPtr<T>& other)
{
    if (mPtr == other.mPtr) return *this;

    if (other.mPtr != nullptr) {
        other.mPtr->AddRef();
    }
    if (mPtr != nullptr) {
        mPtr->Release();
    }
    mPtr = other.mPtr;
    return *this;
}

template<class T>
AutoPtr<T>& AutoPtr<T>::operator=(
    /* [in] */ AutoPtr<T>&& other)
{
    if (mPtr != nullptr) {
        mPtr->Release();
    }
    mPtr = other.mPtr;
    other.mPtr = nullptr;
    return *this;
}

template<class T>
void AutoPtr<T>::MoveTo(
    /* [out] */ T** other)
{
    if (other != nullptr) {
        *other = mPtr;
        mPtr = nullptr;
    }
}

template<class T>
AutoPtr<T>::operator T*() const
{
    return mPtr;
}

template<class T>
T** AutoPtr<T>::operator&()
{
    return &mPtr;
}

template<class T>
T* AutoPtr<T>::operator->() const
{
    return mPtr;
}

template<class T>
T& AutoPtr<T>::operator*() const
{
    return *mPtr;
}

template<class T>
T* AutoPtr<T>::Get() const
{
    return mPtr;
}

template<class T>
bool AutoPtr<T>::operator==(
    /* [in] */ T* other) const
{
    return mPtr == other;
}

template<class T>
bool AutoPtr<T>::operator==(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr == other.mPtr;
}

template<class T>
bool AutoPtr<T>::operator!=(
    /* [in] */ T* other) const
{
    return mPtr != other;
}

template<class T>
bool AutoPtr<T>::operator!=(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr != other.mPtr;
}

template<class T>
bool AutoPtr<T>::operator>(
    /* [in] */ T* other) const
{
    return mPtr > other;
}

template<class T>
bool AutoPtr<T>::operator>(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr > other.mPtr;
}

template<class T>
bool AutoPtr<T>::operator<(
    /* [in] */ T* other) const
{
    return mPtr < other;
}

template<class T>
bool AutoPtr<T>::operator<(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr < other.mPtr;
}

template<class T>
bool AutoPtr<T>::operator<=(
    /* [in] */ T* other) const
{
    return mPtr <= other;
}

template<class T>
bool AutoPtr<T>::operator<=(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr <= other.mPtr;
}

template<class T>
bool AutoPtr<T>::operator>=(
    /* [in] */ T* other) const
{
    return mPtr >= other;
}

template<class T>
bool AutoPtr<T>::operator>=(
    /* [in] */ const AutoPtr<T>& other) const
{
    return mPtr >= other.mPtr;
}

}
}

#endif // OHOS_IDL_AUTOPTR_H
