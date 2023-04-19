#include "App.h"
#include "MainFrame.h"

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
    MainFrame* mainFrame = new MainFrame("Hw List");
    mainFrame->SetClientSize(800, 600);
    mainFrame->Center();
    mainFrame->Show();

    return true;
}
