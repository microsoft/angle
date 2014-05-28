//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#pragma once

class MockSwapChainPanelSizeChangeEventArgs : public RuntimeClass <
    ABI::Windows::UI::Xaml::ISizeChangedEventArgs >
{
public:
    MockSwapChainPanelSizeChangeEventArgs(ABI::Windows::Foundation::Size changedSize)
    {
        mBounds = changedSize;
    }

    HRESULT STDMETHODCALLTYPE get_NewSize(
        ABI::Windows::Foundation::Size *value)
    {
        *value = mBounds;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE  get_PreviousSize(
        ABI::Windows::Foundation::Size *value)
    {
        Assert::Fail(L"Unexpected call to get_PreviousSize");
        return E_NOTIMPL;
    }

private:
    ABI::Windows::Foundation::Size mBounds;
};

class MockSwapChainPanel : public RuntimeClass<
    RuntimeClassFlags<WinRtClassicComMix>,
    ABI::Windows::UI::Xaml::Controls::ISwapChainPanel,
    ABI::Windows::UI::Xaml::IFrameworkElement,
    ABI::Windows::UI::Xaml::IUIElement,
    ISwapChainPanelNative>
{
public:

    MockSwapChainPanel() :
        mBoundsQueryCount(0),
        mNextSizeChangedEventRegistrationToken(1)
    {
        mBounds = { 0, 0, 800, 600 };
    }

    // MockSwapChainPanel
    int GetSizeChangeRegistrationCount() { return static_cast<int>(mSizeChangeHandlers.size()); }
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
                ComPtr<MockSwapChainPanelSizeChangeEventArgs> args = Make<MockSwapChainPanelSizeChangeEventArgs>(currentSize);
                Assert::IsNotNull(args.Get(), L"Unexpected memory allocation failure for MockSwapChainPanelSizeChangeEventArgs");

                ComPtr<ISwapChainPanel> panel = this;
                ComPtr<IInspectable> inspectable;
                panel.As(&inspectable);
                handler.second->Invoke(inspectable.Get(), args.Get());
            }
        }
    }

    // ISwapChainPanelNative
    STDMETHODIMP SetSwapChain(
        IDXGISwapChain *swapChain)
    {
        mSwapChain = swapChain;
        return S_OK;
    }

    // ISwapChainPanel
    STDMETHODIMP get_CompositionScaleX(
        FLOAT *value)
    {
        Assert::Fail(L"Unexpected call to get_CompositionScaleX");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_CompositionScaleY(
        FLOAT *value)
    {
        Assert::Fail(L"Unexpected call to get_CompositionScaleY");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_CompositionScaleChanged(
         __FITypedEventHandler_2_Windows__CUI__CXaml__CControls__CSwapChainPanel_IInspectable *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_CompositionScaleChanged");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_CompositionScaleChanged(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to this_method");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateCoreIndependentInputSource(
         ABI::Windows::UI::Core::CoreInputDeviceTypes deviceTypes,
         ABI::Windows::UI::Core::ICoreInputSourceBase **returnValue)
    {
        Assert::Fail(L"Unexpected call to CreateCoreIndependentInputSource");
        return E_NOTIMPL;
    }

    // IFrameworkElement
    STDMETHODIMP get_Triggers(
         __FIVector_1_Windows__CUI__CXaml__CTriggerBase **value)
    {
        Assert::Fail(L"Unexpected call to get_Triggers");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Resources(
         ABI::Windows::UI::Xaml::IResourceDictionary **value)
    {
        Assert::Fail(L"Unexpected call to get_Resources");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Resources(
         ABI::Windows::UI::Xaml::IResourceDictionary *value)
    {
        Assert::Fail(L"Unexpected call to put_Resources");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Tag(
         IInspectable **value)
    {
        Assert::Fail(L"Unexpected call to get_Tag");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Tag(
         IInspectable *value)
    {
        Assert::Fail(L"Unexpected call to put_Tag");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Language(
         HSTRING *value)
    {
        Assert::Fail(L"Unexpected call to get_Language");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Language(
         __RPC__in HSTRING value)
    {
        Assert::Fail(L"Unexpected call to put_Language");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_ActualWidth(
        DOUBLE *value)
    {
        Assert::Fail(L"Unexpected call to get_ActualWidth");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_ActualHeight(
        DOUBLE *value)
    {
        Assert::Fail(L"Unexpected call to get_ActualHeight");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Width(
        DOUBLE *value)
    {
        Assert::Fail(L"Unexpected call to get_Width");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Width(
         DOUBLE value)
    {
        Assert::Fail(L"Unexpected call to put_Width");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Height(
        DOUBLE *value)
    {
        Assert::Fail(L"Unexpected call to get_Height");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Height(
         DOUBLE value)
    {
        Assert::Fail(L"Unexpected call to put_Height");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_MinWidth(
        DOUBLE *value)
    {
        Assert::Fail(L"Unexpected call to get_MinWidth");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_MinWidth(
         DOUBLE value)
    {
        Assert::Fail(L"Unexpected call to put_MinWidth");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_MaxWidth(
        DOUBLE *value)
    {
        Assert::Fail(L"Unexpected call to get_MaxWidth");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_MaxWidth(
         DOUBLE value)
    {
        Assert::Fail(L"Unexpected call to put_MaxWidth");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_MinHeight(
        DOUBLE *value)
    {
        Assert::Fail(L"Unexpected call to get_MinHeight");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_MinHeight(
         DOUBLE value)
    {
        Assert::Fail(L"Unexpected call to put_MinHeight");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_MaxHeight(
        DOUBLE *value)
    {
        Assert::Fail(L"Unexpected call to get_MaxHeight");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_MaxHeight(
         DOUBLE value)
    {
        Assert::Fail(L"Unexpected call to put_MaxHeight");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_HorizontalAlignment(
        ABI::Windows::UI::Xaml::HorizontalAlignment *value)
    {
        Assert::Fail(L"Unexpected call to get_HorizontalAlignment");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_HorizontalAlignment(
         ABI::Windows::UI::Xaml::HorizontalAlignment value)
    {
        Assert::Fail(L"Unexpected call to put_HorizontalAlignment");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_VerticalAlignment(
        ABI::Windows::UI::Xaml::VerticalAlignment *value)
    {
        Assert::Fail(L"Unexpected call to get_VerticalAlignment");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_VerticalAlignment(
         ABI::Windows::UI::Xaml::VerticalAlignment value)
    {
        Assert::Fail(L"Unexpected call to put_VerticalAlignment");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Margin(
        ABI::Windows::UI::Xaml::Thickness *value)
    {
        Assert::Fail(L"Unexpected call to get_Margin");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Margin(
         ABI::Windows::UI::Xaml::Thickness value)
    {
        Assert::Fail(L"Unexpected call to put_Margin");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Name(
         HSTRING *value)
    {
        Assert::Fail(L"Unexpected call to get_Name");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Name(
         __RPC__in HSTRING value)
    {
        Assert::Fail(L"Unexpected call to put_Name");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_BaseUri(
         ABI::Windows::Foundation::IUriRuntimeClass **value)
    {
        Assert::Fail(L"Unexpected call to get_BaseUri");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_DataContext(
         IInspectable **value)
    {
        Assert::Fail(L"Unexpected call to get_DataContext");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_DataContext(
         IInspectable *value)
    {
        Assert::Fail(L"Unexpected call to put_DataContext");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Style(
         ABI::Windows::UI::Xaml::IStyle **value)
    {
        Assert::Fail(L"Unexpected call to get_Style");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Style(
         ABI::Windows::UI::Xaml::IStyle *value)
    {
        Assert::Fail(L"Unexpected call to put_Style");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Parent(
         ABI::Windows::UI::Xaml::IDependencyObject **value)
    {
        Assert::Fail(L"Unexpected call to get_Parent");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_FlowDirection(
        ABI::Windows::UI::Xaml::FlowDirection *value)
    {
        Assert::Fail(L"Unexpected call to get_FlowDirection");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_FlowDirection(
         ABI::Windows::UI::Xaml::FlowDirection value)
    {
        Assert::Fail(L"Unexpected call to put_FlowDirection");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_Loaded(
         ABI::Windows::UI::Xaml::IRoutedEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_Loaded");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_Loaded(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_Loaded");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_Unloaded(
         ABI::Windows::UI::Xaml::IRoutedEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_Unloaded");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_Unloaded(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_Unloaded");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_SizeChanged(
         ABI::Windows::UI::Xaml::ISizeChangedEventHandler *value,
        EventRegistrationToken *token)
    {
        mSizeChangeHandlers[mNextSizeChangedEventRegistrationToken] = value;
        token->value = mNextSizeChangedEventRegistrationToken;
        mNextSizeChangedEventRegistrationToken++;
        return S_OK;
    }

    STDMETHODIMP remove_SizeChanged(
         EventRegistrationToken token)
    {
        mSizeChangeHandlers.erase(token.value);
        return S_OK;
    }

    STDMETHODIMP add_LayoutUpdated(
         __FIEventHandler_1_IInspectable *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_LayoutUpdated");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_LayoutUpdated(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_LayoutUpdated");
        return E_NOTIMPL;
    }

    STDMETHODIMP FindName(
         __RPC__in HSTRING name,
         IInspectable **returnValue)
    {
        Assert::Fail(L"Unexpected call to FindName");
        return E_NOTIMPL;
    }

    STDMETHODIMP SetBinding(
         ABI::Windows::UI::Xaml::IDependencyProperty *dp,
         ABI::Windows::UI::Xaml::Data::IBindingBase *binding)
    {
        Assert::Fail(L"Unexpected call to SetBinding");
        return E_NOTIMPL;
    }

    // IUIElement
    STDMETHODIMP get_DesiredSize(
        ABI::Windows::Foundation::Size *value)
    {
        Assert::Fail(L"Unexpected call to get_DesiredSize");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_AllowDrop(
        boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_AllowDrop");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_AllowDrop(
         boolean value)
    {
        Assert::Fail(L"Unexpected call to put_AllowDrop");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Opacity(
        DOUBLE *value)
    {
        Assert::Fail(L"Unexpected call to get_Opacity");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Opacity(
         DOUBLE value)
    {
        Assert::Fail(L"Unexpected call to put_Opacity");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Clip(
         ABI::Windows::UI::Xaml::Media::IRectangleGeometry **value)
    {
        Assert::Fail(L"Unexpected call to get_Clip");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Clip(
         ABI::Windows::UI::Xaml::Media::IRectangleGeometry *value)
    {
        Assert::Fail(L"Unexpected call to put_Clip");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_RenderTransform(
         ABI::Windows::UI::Xaml::Media::ITransform **value)
    {
        Assert::Fail(L"Unexpected call to get_RenderTransform");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_RenderTransform(
         ABI::Windows::UI::Xaml::Media::ITransform *value)
    {
        Assert::Fail(L"Unexpected call to put_RenderTransform");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Projection(
         ABI::Windows::UI::Xaml::Media::IProjection **value)
    {
        Assert::Fail(L"Unexpected call to get_Projection");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Projection(
         ABI::Windows::UI::Xaml::Media::IProjection *value)
    {
        Assert::Fail(L"Unexpected call to put_Projection");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_RenderTransformOrigin(
        ABI::Windows::Foundation::Point *value)
    {
        Assert::Fail(L"Unexpected call to get_RenderTransformOrigin");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_RenderTransformOrigin(
         ABI::Windows::Foundation::Point value)
    {
        Assert::Fail(L"Unexpected call to put_RenderTransformOrigin");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_IsHitTestVisible(
        boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_IsHitTestVisible");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_IsHitTestVisible(
         boolean value)
    {
        Assert::Fail(L"Unexpected call to put_IsHitTestVisible");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Visibility(
        ABI::Windows::UI::Xaml::Visibility *value)
    {
        Assert::Fail(L"Unexpected call to get_Visibility");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Visibility(
         ABI::Windows::UI::Xaml::Visibility value)
    {
        Assert::Fail(L"Unexpected call to put_Visibility");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_RenderSize(
        ABI::Windows::Foundation::Size *value)
    {
        mBoundsQueryCount++;
        ABI::Windows::Foundation::Size renderSize;
        renderSize.Width = mBounds.Width;
        renderSize.Height = mBounds.Height;
        *value = renderSize;

        return S_OK;
    }

    STDMETHODIMP get_UseLayoutRounding(
        boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_UseLayoutRounding");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_UseLayoutRounding(
         boolean value)
    {
        Assert::Fail(L"Unexpected call to put_UseLayoutRounding");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_Transitions(
         __FIVector_1_Windows__CUI__CXaml__CMedia__CAnimation__CTransition **value)
    {
        Assert::Fail(L"Unexpected call to get_Transitions");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_Transitions(
         __FIVector_1_Windows__CUI__CXaml__CMedia__CAnimation__CTransition *value)
    {
        Assert::Fail(L"Unexpected call to put_Transitions");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_CacheMode(
         ABI::Windows::UI::Xaml::Media::ICacheMode **value)
    {
        Assert::Fail(L"Unexpected call to get_CacheMode");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_CacheMode(
         ABI::Windows::UI::Xaml::Media::ICacheMode *value)
    {
        Assert::Fail(L"Unexpected call to put_CacheMode");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_IsTapEnabled(
        boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_IsTapEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_IsTapEnabled(
         boolean value)
    {
        Assert::Fail(L"Unexpected call to put_IsTapEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_IsDoubleTapEnabled(
        boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_IsDoubleTapEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_IsDoubleTapEnabled(
         boolean value)
    {
        Assert::Fail(L"Unexpected call to put_IsDoubleTapEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_IsRightTapEnabled(
        boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_IsRightTapEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_IsRightTapEnabled(
         boolean value)
    {
        Assert::Fail(L"Unexpected call to put_IsRightTapEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_IsHoldingEnabled(
        boolean *value)
    {
        Assert::Fail(L"Unexpected call to get_IsHoldingEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_IsHoldingEnabled(
         boolean value)
    {
        Assert::Fail(L"Unexpected call to put_IsHoldingEnabled");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_ManipulationMode(
        ABI::Windows::UI::Xaml::Input::ManipulationModes *value)
    {
        Assert::Fail(L"Unexpected call to get_ManipulationMode");
        return E_NOTIMPL;
    }

    STDMETHODIMP put_ManipulationMode(
         ABI::Windows::UI::Xaml::Input::ManipulationModes value)
    {
        Assert::Fail(L"Unexpected call to put_ManipulationMode");
        return E_NOTIMPL;
    }

    STDMETHODIMP get_PointerCaptures(
         __FIVectorView_1_Windows__CUI__CXaml__CInput__CPointer **value)
    {
        Assert::Fail(L"Unexpected call to get_PointerCaptures");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_KeyUp(
         ABI::Windows::UI::Xaml::Input::IKeyEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_KeyUp");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_KeyUp(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_KeyUp");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_KeyDown(
         ABI::Windows::UI::Xaml::Input::IKeyEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_KeyDown");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_KeyDown(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_KeyDown");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_GotFocus(
         ABI::Windows::UI::Xaml::IRoutedEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_GotFocus");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_GotFocus(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_GotFocus");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_LostFocus(
         ABI::Windows::UI::Xaml::IRoutedEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_LostFocus");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_LostFocus(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_LostFocus");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_DragEnter(
         ABI::Windows::UI::Xaml::IDragEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_DragEnter");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_DragEnter(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_DragEnter");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_DragLeave(
         ABI::Windows::UI::Xaml::IDragEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_DragLeave");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_DragLeave(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_DragLeave");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_DragOver(
         ABI::Windows::UI::Xaml::IDragEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_DragOver");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_DragOver(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_DragOver");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_Drop(
         ABI::Windows::UI::Xaml::IDragEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_Drop");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_Drop(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_Drop");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerPressed(
         ABI::Windows::UI::Xaml::Input::IPointerEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_PointerPressed");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerPressed(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_PointerPressed");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerMoved(
         ABI::Windows::UI::Xaml::Input::IPointerEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_PointerMoved");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerMoved(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_PointerMoved");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerReleased(
         ABI::Windows::UI::Xaml::Input::IPointerEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_PointerReleased");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerReleased(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_PointerReleased");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerEntered(
         ABI::Windows::UI::Xaml::Input::IPointerEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_PointerEntered");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerEntered(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_PointerEntered");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerExited(
         ABI::Windows::UI::Xaml::Input::IPointerEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_PointerExited");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerExited(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_PointerExited");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerCaptureLost(
         ABI::Windows::UI::Xaml::Input::IPointerEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_PointerCaptureLost");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerCaptureLost(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_PointerCaptureLost");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerCanceled(
         ABI::Windows::UI::Xaml::Input::IPointerEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_PointerCanceled");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerCanceled(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_PointerCanceled");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_PointerWheelChanged(
         ABI::Windows::UI::Xaml::Input::IPointerEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_PointerWheelChanged");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_PointerWheelChanged(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_PointerWheelChanged");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_Tapped(
         ABI::Windows::UI::Xaml::Input::ITappedEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_Tapped");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_Tapped(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_Tapped");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_DoubleTapped(
         ABI::Windows::UI::Xaml::Input::IDoubleTappedEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_DoubleTapped");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_DoubleTapped(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_DoubleTapped");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_Holding(
         ABI::Windows::UI::Xaml::Input::IHoldingEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_Holding");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_Holding(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_Holding");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_RightTapped(
         ABI::Windows::UI::Xaml::Input::IRightTappedEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_RightTapped");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_RightTapped(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_RightTapped");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_ManipulationStarting(
         ABI::Windows::UI::Xaml::Input::IManipulationStartingEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_ManipulationStarting");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_ManipulationStarting(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_ManipulationStarting");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_ManipulationInertiaStarting(
         ABI::Windows::UI::Xaml::Input::IManipulationInertiaStartingEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_ManipulationInertiaStarting");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_ManipulationInertiaStarting(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_ManipulationInertiaStarting");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_ManipulationStarted(
         ABI::Windows::UI::Xaml::Input::IManipulationStartedEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_ManipulationStarted");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_ManipulationStarted(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_ManipulationStarted");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_ManipulationDelta(
         ABI::Windows::UI::Xaml::Input::IManipulationDeltaEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_ManipulationDelta");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_ManipulationDelta(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_ManipulationDelta");
        return E_NOTIMPL;
    }

    STDMETHODIMP add_ManipulationCompleted(
         ABI::Windows::UI::Xaml::Input::IManipulationCompletedEventHandler *value,
        EventRegistrationToken *token)
    {
        Assert::Fail(L"Unexpected call to add_ManipulationCompleted");
        return E_NOTIMPL;
    }

    STDMETHODIMP remove_ManipulationCompleted(
         EventRegistrationToken token)
    {
        Assert::Fail(L"Unexpected call to remove_ManipulationCompleted");
        return E_NOTIMPL;
    }

    STDMETHODIMP Measure(
         ABI::Windows::Foundation::Size availableSize)
    {
        Assert::Fail(L"Unexpected call to Measure");
        return E_NOTIMPL;
    }

    STDMETHODIMP Arrange(
         ABI::Windows::Foundation::Rect finalRect)
    {
        Assert::Fail(L"Unexpected call to Arrange");
        return E_NOTIMPL;
    }

    STDMETHODIMP CapturePointer(
         ABI::Windows::UI::Xaml::Input::IPointer *value,
        boolean *returnValue)
    {
        Assert::Fail(L"Unexpected call to CapturePointer");
        return E_NOTIMPL;
    }

    STDMETHODIMP ReleasePointerCapture(
         ABI::Windows::UI::Xaml::Input::IPointer *value)
    {
        Assert::Fail(L"Unexpected call to ReleasePointerCapture");
        return E_NOTIMPL;
    }

    STDMETHODIMP ReleasePointerCaptures(void)
    {
        Assert::Fail(L"Unexpected call to ReleasePointerCaptures");
        return E_NOTIMPL;
    }

    STDMETHODIMP AddHandler(
         ABI::Windows::UI::Xaml::IRoutedEvent *routedEvent,
         IInspectable *handler,
         boolean handledEventsToo)
    {
        Assert::Fail(L"Unexpected call to AddHandler");
        return E_NOTIMPL;
    }

    STDMETHODIMP RemoveHandler(
         ABI::Windows::UI::Xaml::IRoutedEvent *routedEvent,
         IInspectable *handler)
    {
        Assert::Fail(L"Unexpected call to RemoveHandler");
        return E_NOTIMPL;
    }

    STDMETHODIMP TransformToVisual(
         ABI::Windows::UI::Xaml::IUIElement *visual,
         ABI::Windows::UI::Xaml::Media::IGeneralTransform **returnValue)
    {
        Assert::Fail(L"Unexpected call to TransformToVisual");
        return E_NOTIMPL;
    }

    STDMETHODIMP InvalidateMeasure(void)
    {
        Assert::Fail(L"Unexpected call to InvalidateMeasure");
        return E_NOTIMPL;
    }

    STDMETHODIMP InvalidateArrange(void)
    {
        Assert::Fail(L"Unexpected call to InvalidateArrange");
        return E_NOTIMPL;
    }

    STDMETHODIMP UpdateLayout(void)
    {
        Assert::Fail(L"Unexpected call to UpdateLayout");
        return E_NOTIMPL;
    }

private:
    int mSizeChangedRegistrationCount;
    int mBoundsQueryCount;
    ABI::Windows::Foundation::Rect mBounds;
    std::map<_int64, ComPtr<ABI::Windows::UI::Xaml::ISizeChangedEventHandler> > mSizeChangeHandlers;
    __int64 mNextSizeChangedEventRegistrationToken;
    ComPtr<IDXGISwapChain> mSwapChain;
};