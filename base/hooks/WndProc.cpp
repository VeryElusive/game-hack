#include "../core/hooks.h"
#include "../utils/render.h"
#include "../core/menu rework/menu.h"
#include "../core/framework/framework.h"
#include "../context.h"
#include <intrin.h>

LRESULT CALLBACK Hooks::hkWndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( uMsg == WM_KEYDOWN && wParam == VK_INSERT )
	{
		Menu::m_bOpened = !Menu::m_bOpened;
		MenuFramework::m_bOpen = !MenuFramework::m_bOpen;
	}

	

	//if ( Menu::Opened && ( uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONUP || uMsg == WM_MOUSEMOVE ) )
	//	return false;

	if ( Menu::m_bOpened ) {
		if ( uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEHWHEEL )
			Inputsys::scroll += ( float )GET_WHEEL_DELTA_WPARAM( wParam ) / ( float )WHEEL_DELTA;

		switch ( uMsg ) {
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
			return 0;

		default:
			break;
		}
	}

	// return input controls to the game
	return CallWindowProc( pOldWndProc, hWnd, uMsg, wParam, lParam );
}