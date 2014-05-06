//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#pragma once

typedef ABI::Windows::Foundation::__FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CWindowSizeChangedEventArgs_t ISizeChangedEventHandler;

class MockCoreWindowSizeChangeEventArgs : public RuntimeClass <
    ABI::Windows::UI::Core::IWindowSizeChangedEventArgs >
{
public:
    MockCoreWindowSizeChangeEventArgs(ABI::Windows::Foundation::Size changedSize)
    {
        mBounds = changedSize;
    }

    HRESULT STDMETHODCALLTYPE get_Size(
        /* [out][retval] */ __RPC__out ABI::Windows::Foundation::Size *value)
    {
        *value = mBounds;
        return S_OK;
    }

private:
    ABI::Windows::Foundation::Size mBounds;
};

class MockCoreWindow : public RuntimeClass<
    ABI::Windows::UI::Core::ICoreWindow>
{
public:

    MockCoreWindow() :
        mBoundsQueryCount(0),
        mNextSizeChangedEventRegistrationToken(1)
    {
        mBounds = { 0, 0, 800, 600 };
    }

    // MockCoreWindow
    int GetSizeChangeRegistrationCount() { return mSizeChangeHandlers.size(); }
    int GetBoundsQueryCount() { return mBoundsQueryCount; }
    void SetExpectedBounds(ABI::Windows::Foundation::Rect newBounds) { mBounds = newBounds; }
    void UpdateSizeAndSignalChangedEvent(ABI::Windows::Foundation::Rect newBounds)
    {
        SetExpectedBounds(newBounds);

        for(auto& handler : mSizeChangeHandlers)
        {
            if (handler.second != nullptr)
            {
                ABI::Windows::Foundation::Size currentSize;
                currentSize = { (float)newBounds.Width, (float)newBounds.Height };
                ComPtr<MockCoreWindowSizeChangeEventArgs> args = Make<MockCoreWindowSizeChangeEventArgs>(currentSize);
                Assert::IsNotNull(args.Get(), L"Unexpected memory allocation failure for MockCoreWindowSizeChangeEventArgs");
                handler.second->Invoke(this, args.Get());
            }
        }
    }

    // ICoreWindow
    STDMETHODIMP get_AutomationHostProvider(
        /* [out][retval] */ __RPC__deref_out_opt IInspectable **value)
    {
        Assert::Fail(L"Unexpected call to get_AutomationHostProvider");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Bounds(
        /* [out][retval] */ __RPC__out ABI::Windows::Foundation::Rect *value)
    {
        mBoundsQueryCount++;
        *value = mBounds;

        return S_OK;
    }

    STDMETHODIMP get_CustomProperties(
        /* [out][retval] */ __RPC__deref_out_opt ABI::Windows::Foundation::Collections::IPropertySet **value)
    {
        Assert::Fail(L"Unexpected call to get_CustomProperties");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Dispatcher(
        /* [out][retval] */ __RPC__deref_out_opt ABI::Windows::UI::Core::ICoreDispatcher **value)
    {
        Assert::Fail(L"Unexpected call to get_WindowHandle");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_FlowDirection(
        /* [out][retval] */ __RPC__out ABI::Windows::UI::Core::CoreWindowFlowDirection *value)
    {
        Assert::Fail(L"Unexpected call to get_FlowDirection");
        return E_NOTIMPL;
    }

    STDMETHODIMP STDMETHODCALLTYPE put_FlowDirection(
        /* [in] */ ABI::Windows::UI::Core::CoreWindowFlowDirection value)
    {
        Assert::Fail(L"Unexpected call to put_FlowDirection");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_IsInputEnabled(
        /* [out][retval] */ __RPC__out boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_IsInputEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_IsInputEnabled(
        /* [in] */ boolean value)
    {
        Assert::Fail(L"Unexpected call to put_IsInputEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_PointerCursor(
        /* [out][retval] */ __RPC__deref_out_opt ABI::Windows::UI::Core::ICoreCursor **value)
    {
        Assert::Fail(L"Unexpected call to get_PointerCursor");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_PointerCursor(
        /* [in] */ __RPC__in_opt ABI::Windows::UI::Core::ICoreCursor *value)
    {
        Assert::Fail(L"Unexpected call to get_WindowHandle");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_PointerPosition(
        /* [out][retval] */ __RPC__out ABI::Windows::Foundation::Point *value)
    {
        Assert::Fail(L"Unexpected call to get_PointerPosition");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Visible(
        /* [out][retval] */ __RPC__out boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_Visible");
        return E_NOTIMPL;
    }

    STDMETHODIMP Activate(void)
    {
        Assert::Fail(L"Unexpected call to Activate");
        return E_NOTIMPL;
    }

    STDMETHODIMP Close(void)
    {
        Assert::Fail(L"Unexpected call to Close");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetAsyncKeyState(
        /* [in] */ ABI::Windows::System::VirtualKey virtualKey,
        /* [out][retval] */ __RPC__out ABI::Windows::UI::Core::CoreVirtualKeyStates *KeyState)
    {
        Assert::Fail(L"Unexpected call to GetAsyncKeyState");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetKeyState(
        /* [in] */ ABI::Windows::System::VirtualKey virtualKey,
        /* [out][retval] */ __RPC__out ABI::Windows::UI::Core::CoreVirtualKeyStates *KeyState)
    {
        Assert::Fail(L"Unexpected call to GetKeyState");
        return E_NOTIMPL;
    }

    STDMETHODIMP ReleasePointerCapture(void)
    {
        Assert::Fail(L"Unexpected call to ReleasePointerCapture");
        return E_NOTIMPL;
    }

    STDMETHODIMP SetPointerCapture(void)
    {
        Assert::Fail(L"Unexpected call to SetPointerCapture");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_Activated(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CWindowActivatedEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_Activated");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_Activated(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_Activated");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_AutomationProviderRequested(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CAutomationProviderRequestedEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_AutomationProviderRequested");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_AutomationProviderRequested(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_AutomationProviderRequested");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_CharacterReceived(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CCharacterReceivedEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_CharacterReceived");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_CharacterReceived(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_CharacterReceived");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_Closed(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CCoreWindowEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_Closed");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_Closed(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_Closed");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_InputEnabled(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CInputEnabledEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_InputEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_InputEnabled(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_InputEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_KeyDown(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CKeyEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_KeyDown");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_KeyDown(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_KeyDown");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_KeyUp(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CKeyEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_KeyUp");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_KeyUp(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_KeyUp");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerCaptureLost(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerCaptureLost");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerCaptureLost(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerCaptureLost");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerEntered(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerEntered");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerEntered(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerEntered");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerExited(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerExited");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerExited(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerExited");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerMoved(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerMoved");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerMoved(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerMoved");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerPressed(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerPressed");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerPressed(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerPressed");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerReleased(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerReleased");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerReleased(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerReleased");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_TouchHitTesting(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CTouchHitTestingEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_TouchHitTesting");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_TouchHitTesting(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_TouchHitTesting");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerWheelChanged(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerWheelChanged");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerWheelChanged(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerWheelChanged");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_SizeChanged(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CWindowSizeChangedEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *pCookie)
    {
        mSizeChangeHandlers[mNextSizeChangedEventRegistrationToken] = handler;
        pCookie->value = mNextSizeChangedEventRegistrationToken;
        mNextSizeChangedEventRegistrationToken++;
        return S_OK;
    }

    STDMETHODIMP remove_SizeChanged(
        /* [in] */ EventRegistrationToken cookie)
    {
        mSizeChangeHandlers.erase(cookie.value);
        return S_OK;
    }

    STDMETHODIMP add_VisibilityChanged(
        /* [in] */ __RPC__in_opt __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CVisibilityChangedEventArgs *handler,
        /* [out][retval] */ __RPC__out EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_VisibilityChanged");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_VisibilityChanged(
        /* [in] */ EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_VisibilityChanged");
        return E_NOTIMPL;
    }

private:
    int mSizeChangedRegistrationCount;
    int mBoundsQueryCount;
    ABI::Windows::Foundation::Rect mBounds;
    std::map<_int64, ComPtr<ISizeChangedEventHandler> > mSizeChangeHandlers;
    __int64 mNextSizeChangedEventRegistrationToken;
};