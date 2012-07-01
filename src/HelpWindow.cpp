/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file HelpWindow.cpp
 * 
 * Implementace okna s nápovědou.
 *
 * @version $Rev: 153 $
 ******************************************************************************/

// -*- C++ -*- generated by wxGlade 0.6.3 on Sun Apr 20 23:10:55 2008

#include "HelpWindow.h"
#include "cl_config.h"

// begin wxGlade: ::extracode

// end wxGlade


HelpWindow::HelpWindow(wxWindow* parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
    wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
    // begin wxGlade: HelpWindow::HelpWindow
    htmlWindow = new wxHtmlWindow(this, wxID_ANY);

    set_properties();
    do_layout();
    // end wxGlade
    
    
    // načtení souboru s nápovědou
    wxString helpFile(dama::ClientConfig::instance()->getHelpPath().c_str(), wxConvUTF8);
    
    htmlWindow->LoadPage(helpFile);

}


void HelpWindow::set_properties()
{
    // begin wxGlade: HelpWindow::set_properties
    SetTitle(wxT("Nápověda"));
    // end wxGlade
}


void HelpWindow::do_layout()
{
    // begin wxGlade: HelpWindow::do_layout
    wxBoxSizer* sizer_1 = new wxBoxSizer(wxVERTICAL);
    sizer_1->Add(htmlWindow, 1, wxEXPAND, 0);
    sizer_1->SetMinSize(780, 580);
    SetSizer(sizer_1);
    sizer_1->Fit(this);
    Layout();
    // end wxGlade
}
