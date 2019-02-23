/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Version: v2017.2.6  Build: 6581
  Copyright (c) 2006-2018 Audiokinetic Inc.
*******************************************************************************/

#include "stdafx.h"
#include "SimpleTextRenderer.h"

#include "DX/DirectXHelper.h"

using namespace D2D1;
using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;

SimpleTextRenderer* SimpleTextRenderer::s_instance = nullptr;

SimpleTextRenderer::SimpleTextRenderer(const std::shared_ptr<DX::DeviceResources>& deviceResources)
	: m_deviceResources(deviceResources)
	, m_renderNeeded(true)
{
	s_instance = this;

	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
		L"Segoe UI",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		42.0f,
		L"en-US",
		&m_textFormat[TextType_Reg]
		)
		);

	DX::ThrowIfFailed(
		m_textFormat[TextType_Reg]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)
		);

	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
		L"Segoe UI",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		72.0f,
		L"en-US",
		&m_textFormat[TextType_Title]
		)
		);

	DX::ThrowIfFailed(
		m_textFormat[TextType_Title]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)
		);

	DX::ThrowIfFailed(
		m_deviceResources->GetDWriteFactory()->CreateTextFormat(
		L"Segoe UI",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		24.0f,
		L"en-US",
		&m_textFormat[TextType_Text]
		)
		);

	DX::ThrowIfFailed(
		m_textFormat[TextType_Text]->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING)
		);

	CreateDeviceDependentResources();
}

void SimpleTextRenderer::CreateDeviceDependentResources()
{
	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(
		ColorF(ColorF::Black),
		&m_textBrush[TextColor_Normal]
		)
		);

	DX::ThrowIfFailed(
		m_deviceResources->GetD2DDeviceContext()->CreateSolidColorBrush(
		ColorF(ColorF::DeepPink),
		&m_textBrush[TextColor_Selected]
		)
		);
}

void SimpleTextRenderer::ReleaseDeviceDependentResources()
{
	m_textBrush[TextColor_Normal].Reset();
	m_textBrush[TextColor_Selected].Reset();
}

void SimpleTextRenderer::BeginDrawing()
{
	m_deviceResources->GetD2DDeviceContext()->BeginDraw();

	m_deviceResources->GetD2DDeviceContext()->Clear( ColorF( ColorF::White ) );

}
void SimpleTextRenderer::DrawText( WCHAR * in_szText, int in_eTextType, int in_eColor, int in_X, int in_Y )
{
	auto rect = D2D1_RECT_F();
	rect.bottom = 768.0f;
	rect.top = (FLOAT) in_Y;
	rect.left = (FLOAT) in_X;
	rect.right = 1366.0f;

	m_deviceResources->GetD2DDeviceContext()->DrawText( in_szText, wcslen( in_szText ), m_textFormat[ in_eTextType ].Get(), rect, m_textBrush[ in_eColor ].Get() );
}

void SimpleTextRenderer::DoneDrawing()
{
	// Ignore D2DERR_RECREATE_TARGET. This error indicates that the device
	// is lost. It will be handled during the next call to Present.
	HRESULT hr = m_deviceResources->GetD2DDeviceContext()->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
	{
		DX::ThrowIfFailed(hr);
	}

	m_renderNeeded = false;
}

void SimpleTextRenderer::SaveInternalState(IPropertySet^ state)
{
}

void SimpleTextRenderer::LoadInternalState(IPropertySet^ state)
{
}
