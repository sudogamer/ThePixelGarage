#include "header/SudoApp.h"

SudoApp::SudoApp()
	:
	wnd( 800,600,"SudoGamer Window" )
{}

int SudoApp::Go()
{
	while( true )
	{
		// process all messages pending, but to not block for new messages
		if( const auto ecode = Window::ProcessMessages() )
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		DoFrame();
	}
}

void SudoApp::DoFrame()
{
	const double c = sin( timer.Peek() ) / 2.0f + 0.5f;
	wnd.Gfx().ClearBuffer( c,c,1.0f );
	wnd.Gfx().Draw();
	wnd.Gfx().EndFrame();
}