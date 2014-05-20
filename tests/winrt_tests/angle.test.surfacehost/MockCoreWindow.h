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
         ABI::Windows::Foundation::Size *value)
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
         IInspectable **value)
    {
        Assert::Fail(L"Unexpected call to get_AutomationHostProvider");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Bounds(
         ABI::Windows::Foundation::Rect *value)
    {
        mBoundsQueryCount++;
        *value = mBounds;

        return S_OK;
    }

    STDMETHODIMP get_CustomProperties(
         ABI::Windows::Foundation::Collections::IPropertySet **value)
    {
        Assert::Fail(L"Unexpected call to get_CustomProperties");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Dispatcher(
         ABI::Windows::UI::Core::ICoreDispatcher **value)
    {
        Assert::Fail(L"Unexpected call to get_WindowHandle");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_FlowDirection(
         ABI::Windows::UI::Core::CoreWindowFlowDirection *value)
    {
        Assert::Fail(L"Unexpected call to get_FlowDirection");
        return E_NOTIMPL;
    }

    STDMETHODIMP STDMETHODCALLTYPE put_FlowDirection(
         ABI::Windows::UI::Core::CoreWindowFlowDirection value)
    {
        Assert::Fail(L"Unexpected call to put_FlowDirection");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_IsInputEnabled(
         boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_IsInputEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_IsInputEnabled(
         boolean value)
    {
        Assert::Fail(L"Unexpected call to put_IsInputEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_PointerCursor(
         ABI::Windows::UI::Core::ICoreCursor **value)
    {
        Assert::Fail(L"Unexpected call to get_PointerCursor");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_PointerCursor(
          ABI::Windows::UI::Core::ICoreCursor *value)
    {
        Assert::Fail(L"Unexpected call to get_WindowHandle");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_PointerPosition(
         ABI::Windows::Foundation::Point *value)
    {
        Assert::Fail(L"Unexpected call to get_PointerPosition");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Visible(
         boolean *value)
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
         ABI::Windows::System::VirtualKey virtualKey,
         ABI::Windows::UI::Core::CoreVirtualKeyStates *KeyState)
    {
        Assert::Fail(L"Unexpected call to GetAsyncKeyState");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetKeyState(
         ABI::Windows::System::VirtualKey virtualKey,
         ABI::Windows::UI::Core::CoreVirtualKeyStates *KeyState)
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
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CWindowActivatedEventArgs *handler,
         EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_Activated");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_Activated(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_Activated");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_AutomationProviderRequested(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CAutomationProviderRequestedEventArgs *handler,
         EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_AutomationProviderRequested");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_AutomationProviderRequested(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_AutomationProviderRequested");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_CharacterReceived(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CCharacterReceivedEventArgs *handler,
         EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_CharacterReceived");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_CharacterReceived(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_CharacterReceived");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_Closed(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CCoreWindowEventArgs *handler,
         EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_Closed");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_Closed(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_Closed");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_InputEnabled(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CInputEnabledEventArgs *handler,
         EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_InputEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_InputEnabled(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_InputEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_KeyDown(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CKeyEventArgs *handler,
         EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_KeyDown");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_KeyDown(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_KeyDown");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_KeyUp(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CKeyEventArgs *handler,
         EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_KeyUp");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_KeyUp(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_KeyUp");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerCaptureLost(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
         EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerCaptureLost");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerCaptureLost(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerCaptureLost");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerEntered(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
         EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerEntered");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerEntered(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerEntered");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerExited(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
         EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerExited");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerExited(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerExited");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerMoved(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
         EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerMoved");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerMoved(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerMoved");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerPressed(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
         EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerPressed");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerPressed(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerPressed");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerReleased(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
         EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerReleased");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerReleased(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerReleased");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_TouchHitTesting(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CTouchHitTestingEventArgs *handler,
         EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_TouchHitTesting");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_TouchHitTesting(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_TouchHitTesting");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerWheelChanged(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CPointerEventArgs *handler,
         EventRegistrationToken *cookie)
    {
        Assert::Fail(L"Unexpected call to add_PointerWheelChanged");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerWheelChanged(
         EventRegistrationToken cookie)
    {
        Assert::Fail(L"Unexpected call to remove_PointerWheelChanged");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_SizeChanged(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CWindowSizeChangedEventArgs *handler,
         EventRegistrationToken *pCookie)
    {
        mSizeChangeHandlers[mNextSizeChangedEventRegistrationToken] = handler;
        pCookie->value = mNextSizeChangedEventRegistrationToken;
        mNextSizeChangedEventRegistrationToken++;
        return S_OK;
    }

    STDMETHODIMP remove_SizeChanged(
         EventRegistrationToken cookie)
    {
        mSizeChangeHandlers.erase(cookie.value);
        return S_OK;
    }

    STDMETHODIMP add_VisibilityChanged(
          __FITypedEventHandler_2_Windows__CUI__CCore__CCoreWindow_Windows__CUI__CCore__CVisibilityChangedEventArgs *handler,
         EventRegistrationToken *pCookie)
    {
        Assert::Fail(L"Unexpected call to add_VisibilityChanged");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_VisibilityChanged(
         EventRegistrationToken cookie)
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