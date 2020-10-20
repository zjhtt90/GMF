#include "D3DVideoRender.h"
#include "../MediaBuffer.h"
#include "../../Common/LogManager.h"


CD3DVideoRender::CD3DVideoRender() 
	: m_pDirect3D9(NULL), m_pDirecr3DDevice(NULL), m_pDirect3DSurface(NULL), m_hWnd(0x00000000)
{
}

CD3DVideoRender::CD3DVideoRender(const std::string &name)
	:CVideoRender(name), m_pDirect3D9(NULL), m_pDirecr3DDevice(NULL), m_pDirect3DSurface(NULL), m_hWnd(0x00000000)
{

}

CD3DVideoRender::~CD3DVideoRender()
{
}

int CD3DVideoRender::Open()
{
	int res = MEDIA_ERR_NONE;

	res = CVideoRender::Open();

	if(m_pHwnd == NULL)
	{
		LOG_ERR("Invalided HWND!");
		return MEDIA_ERR_INVALIDE_PARAME;
	}

	m_hWnd = (HWND)m_pHwnd;

	GetClientRect(m_hWnd, &m_viewRect);

	m_pDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION);
	if(m_pDirect3D9 == NULL)
		return MEDIA_ERR_INVALIDE_PARAME;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	res = m_pDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
		m_hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pDirecr3DDevice);
	if(res != D3D_OK)
		return MEDIA_ERR_INVALIDE_PARAME;

	res = m_pDirecr3DDevice->CreateOffscreenPlainSurface(m_width, m_height, 
		(D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'),D3DPOOL_DEFAULT, &m_pDirect3DSurface, NULL);
	if(res != D3D_OK)
		return MEDIA_ERR_INVALIDE_PARAME;

	return res;
}

void CD3DVideoRender::Close()
{
	if(m_pDirect3DSurface != NULL)
	{
		m_pDirect3DSurface->Release();
		m_pDirect3DSurface = NULL;
	}

	if(m_pDirecr3DDevice != NULL)
	{
		m_pDirecr3DDevice->Release();
		m_pDirecr3DDevice = NULL;
	}

	if(m_pDirect3D9 != NULL)
	{
		m_pDirect3D9->Release();
		m_pDirect3D9 = NULL;
	}
}

void CD3DVideoRender::DrainInputBuffer(TRACKID id, CMediaBuffer *buffer)
{
	if(id == 0)
		RenderFrame(buffer->GetData(), buffer->GetDataSize());
}

int CD3DVideoRender::RenderFrame(BYTE *pData, size_t size)
{
	HRESULT lRes;
	int i;

	if(m_pDirect3DSurface == NULL)
		return MEDIA_ERR_INVALIDE_PARAME;

	D3DLOCKED_RECT d3d_rect;
	lRes = m_pDirect3DSurface->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
	if(lRes != D3D_OK)
		return MEDIA_ERR_INVALIDE_PARAME;
	BYTE *pDst = (BYTE*)d3d_rect.pBits;
	int strid = d3d_rect.Pitch;
	for(i=0;i<m_height;i++)
	{
		memcpy(pDst+i*strid, pData+i*m_width, m_width);
	}
	for(i=0;i<m_height/2;i++)
	{
		memcpy(pDst+strid*m_height+i*strid/2, 
			pData+m_width*m_height+m_width*m_height/4+i*m_width/2, m_width/2);
	}
	for(i=0;i<m_height/2;i++)
	{
		memcpy(pDst+strid*m_height+strid*m_height/4+i*strid/2,
			pData+m_width*m_height+i*m_width/2, m_width/2);
	}
	m_pDirect3DSurface->UnlockRect();

	m_pDirecr3DDevice->BeginScene();

	IDirect3DSurface9 *pBackBuffer = NULL;
	lRes = m_pDirecr3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer);
	if(lRes != D3D_OK)
	{
		m_pDirecr3DDevice->EndScene();
		return -1;
	}

	m_pDirecr3DDevice->StretchRect(m_pDirect3DSurface, NULL, pBackBuffer, &m_viewRect,D3DTEXF_LINEAR);
	m_pDirecr3DDevice->EndScene();

	m_pDirecr3DDevice->Present(NULL, NULL, NULL, NULL);

	return MEDIA_ERR_NONE;
}