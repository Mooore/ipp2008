/*******************************************************************************
 * ICP - Dáma klient 2008:
 * -----------------------------------------------------------------------------
 *     Černý Lukáš          <xcerny37@stud.fit.vutbr.cz>
 *     Dvořák Miroslav      <xdvora11@stud.fit.vutbr.cz>
 *     Mrázek Petr          <xmraze03@stud.fit.vutbr.cz>
 *     Oujeský Miroslav     <xoujes00@stud.fit.vutbr.cz>
 ***************************************************************************//**
 * @file LobbyWindow.cpp
 *
 * Implementace okna se seznamem her.
 * 
 * @version $Rev: 189 $
 ******************************************************************************/

// -*- C++ -*- generated by wxGlade 0.6.3 on Wed Apr 16 18:38:49 2008

#include "LobbyWindow.h"
#include "MenuWindow.h"
#include "gameWindow.h"
#include "cl_config.h"
#include <iostream>
#include <sstream>

#ifndef TIXML_USE_TICPP
#define TIXML_USE_TICPP
#endif
#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#include "ticpp.h"
// begin wxGlade: ::extracode

// end wxGlade


/**
 * Konstruktor 
 * 
 * @param parent
 * @param id
 * @param title
 * @param pos
 * @param size
 * @param style
 * 
 */
LobbyWindow::LobbyWindow(wxWindow* parent, int id, const wxString& title, const wxPoint& pos, const wxSize& size, long style):
	wxFrame(parent, id, title, pos, size, wxDEFAULT_FRAME_STYLE)
{
	// begin wxGlade: LobbyWindow::LobbyWindow
	listGames = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER);
	chatBox = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_BESTWRAP);
	listPlayers = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxSUNKEN_BORDER);
	buttonInvite = new wxButton(this, wxID_ANY, _("Pozvat"));
	buttonPlay = new wxButton(this, wxID_ANY, _("Hrát"));
	buttonRefresh = new wxButton(this, wxID_ANY, _("Obnovit"));
	textChat = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	buttonSend = new wxButton(this, wxID_ANY, _("Odeslat"));

	Connect(buttonInvite->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LobbyWindow::clickInvite));
	Connect(buttonPlay->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LobbyWindow::clickPlay));
	Connect(buttonSend->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LobbyWindow::clickSend));
	Connect(buttonRefresh->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(LobbyWindow::clickRefresh));

	//Connect(SOCKET_ID,wxEVT_SOCKET, wxSocketEventHandler(LobbyWindow::onSocketEvent));

	// napojení odentrování v políčku pro chat na odeslání zprávy 
	Connect(textChat->GetId(), wxEVT_COMMAND_TEXT_ENTER, wxCommandEventHandler(LobbyWindow::clickSend));

	Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, wxCloseEventHandler(LobbyWindow::onCloseWindow));

	set_properties();
	do_layout();
	// end wxGlade

	// registrace pro příjem zpráv týkajících se sítě
	clSocket * sock = clSocket::Instance();
	sock->RegisterHandler(this);
	// zpráva serveru, že je možné přijímat chat
	sock->SendRequest(std::string("<chatstart />"));
	
	// obnovení seznamu hráčů a her
	refresh();
	
	// napojení eventů pro seznam hráčů
	Connect(listPlayers->GetId(), wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(LobbyWindow::onPlayersActivate));
	Connect(listPlayers->GetId(), wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(LobbyWindow::onPlayersSelect));
	
	// napojení eventů pro seznam her
    Connect(listGames->GetId(), wxEVT_COMMAND_LIST_ITEM_ACTIVATED, wxListEventHandler(LobbyWindow::onGamesActivate));
    Connect(listGames->GetId(), wxEVT_COMMAND_LIST_ITEM_SELECTED, wxListEventHandler(LobbyWindow::onGamesSelect));
	
	// napojení eventu na časovač
	Connect(wxID_ANY, wxEVT_TIMER, wxTimerEventHandler(LobbyWindow::onTimer));
	// spuštění časovače
	timer.SetOwner(this, wxID_ANY);
	timer.Start(10 * 1000, false);
}

/**
 * Event handler pro socket
 * 
 * @param zprava	řeťezec se zprácou
 */
int LobbyWindow::Receive(std::string zprava)
{
	std::string pom_str;
	std::string pom_str2;
	wxString retezec;
	ticpp::Document doc;
	ticpp::Element *root_element;
	
	try 
	{
		doc.Parse(zprava);
		root_element = doc.FirstChildElement();	// ziskáni prvniho elementu
		pom_str = root_element->Value();	// ziskáni hodnoty prvního elementu
	}
	catch( ticpp::Exception& ex )
	{
		return false;
	}
	if(pom_str == "chat")
	{
		try
		{
			pom_str = root_element->GetAttribute("from");
			pom_str2 = root_element->GetText();
			retezec = wxString(pom_str.c_str(), wxConvUTF8) + _T(": ")+ wxString(pom_str2.c_str(), wxConvUTF8) + _T("\n");
			chatBox->AppendText(retezec);
		}
		catch( ticpp::Exception& ex )
		{
			return 0;// jednalo se o chat, ale zpráva byla poškozena
		}
		return 1;
	}
	else if (pom_str == "playerslist")
	{
		try
		{
		    setPlayerList(root_element);
		}
		catch (ticpp::Exception& ex)
		{
			return 0;
		}
		return 1;
	}
	else if (pom_str == "gameslist")
	{
		try
		{
		    setGamesList(root_element);
		}
		catch (ticpp::Exception& ex)
		{
			return 0;
		}
		return 1;
	}
	else if (pom_str == "duel")
	{
		try
		{
			std::string accept = root_element->GetAttribute("accept");
		
			wxString with(root_element->GetAttribute("with").c_str(), wxConvUTF8);
		
			if (accept == "false")
			{
				wxMessageBox(_("Hráč '") + with + _("' odmítl Vaši výzvu."), _("Výzva"), wxICON_INFORMATION);
			}
			else if (accept == "true")
			{
				wxMessageBox(_("Hráč '") + with + _("' přijal Vaši výzvu."), _("Registrace"), wxICON_INFORMATION);
			}
			else
			{
				acceptMatch(root_element);
			}
			return 1;
		}
		catch (ticpp::Exception& ex)
		{
			return 0;
		}
	}
	else if(pom_str == "savedgame")
	{
		try
		{
			wxString with(root_element->GetAttribute("with").c_str(), wxConvUTF8);
		 	wxString white(root_element->FirstChildElement("white")->GetAttribute("nickname").c_str(), wxConvUTF8);
		 	wxString black(root_element->FirstChildElement("black")->GetAttribute("nickname").c_str(), wxConvUTF8);
		 	std::string state =  root_element->FirstChildElement("gamestate")->GetText();
//		 	wxWindow* parent, int id, const wxString& title, bool netgame,const wxString& _with,const wxString& _bily ,const wxString& _cerny, std::string & stav, const wxPoint& pos, const wxSize& size, long style
			// vytvoření okna hry
			gameWindow* game = new gameWindow(GetParent(), wxID_ANY, wxEmptyString, true, with, white, black, state);
			game->Show();
		 	return 1;
		}
		catch (ticpp::Exception& ex)
		{
			return 0;
		}
	}
	return 0; // nezpracováno
}

void LobbyWindow::set_properties()
{
	SetTitle(clSocket::Instance()->getHostname());

	// nastavení maximální délky zprávy
	textChat->SetMaxLength(200);
	textChat->SetFocus();
	
	// nastavení hlaviček sloupců v seznamech
	listPlayers->InsertColumn(0, _("Hráči"));
	listGames->InsertColumn(0, _("Protihráč"));
}


void LobbyWindow::do_layout()
{
	// begin wxGlade: LobbyWindow::do_layout
	wxBoxSizer* sizer_10 = new wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* grid_sizer_2 = new wxFlexGridSizer(2, 3, 10, 10);
	wxBoxSizer* sizer_11 = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer_12 = new wxBoxSizer(wxVERTICAL);
	grid_sizer_2->Add(listGames, 1, wxEXPAND|wxFIXED_MINSIZE, 0);
	// nastavení minimální velikosti chatboxu
	chatBox->SetMinSize(wxSize(200, 300));
	grid_sizer_2->Add(chatBox, 0, wxEXPAND, 0);
	sizer_12->Add(listPlayers, 1, wxEXPAND, 0);
	sizer_12->Add(buttonInvite, 0, wxTOP|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 10);
	grid_sizer_2->Add(sizer_12, 1, wxEXPAND, 0);
	sizer_11->Add(buttonPlay, 0, 0, 0);
	sizer_11->Add(buttonRefresh, 0, wxLEFT, 10);
	grid_sizer_2->Add(sizer_11, 1, wxEXPAND, 0);
	grid_sizer_2->Add(textChat, 0, wxEXPAND, 0);
	grid_sizer_2->Add(buttonSend, 0, wxALIGN_CENTER_HORIZONTAL, 0);
	grid_sizer_2->AddGrowableRow(0);
	grid_sizer_2->AddGrowableCol(0);
	grid_sizer_2->AddGrowableCol(1);
	sizer_10->Add(grid_sizer_2, 1, wxALL|wxEXPAND, 10);
	SetSizer(sizer_10);
	sizer_10->Fit(this);
	sizer_10->SetSizeHints(this);
	Layout();
	Centre();
	// end wxGlade
}

/**
 * Event handler pro kliknutí na tlačítko pro pozvání do hry
 * @param event
 */
void LobbyWindow::clickInvite(wxCommandEvent &event)
{
    if (selectedPlayer != _(""))
    {
        invite(selectedPlayer);
    }
}

/**
 * Event handler pro kliknutí na tlačítko pro pozvání do hry
 * @param event
 */
void LobbyWindow::clickRefresh(wxCommandEvent &event)
{
    refresh();
}

/**
 * Event handler pro kliknutí na tlačítko pro spuštění hry
 * @param event
 */
void LobbyWindow::clickPlay(wxCommandEvent &event)
{
    if (selectedGame != _(""))
    {
    	clSocket * sock = clSocket::Instance();
    	sock->SendRequest(std::string("<getgame with=\""+ std::string(selectedGame.mb_str()) +"\" />"));
    }
//	<getgame with="hrac" />
}

/**
 * Event handler pro kliknutí na tlačítko pro odeslání chatu
 * @param event
 */
void LobbyWindow::clickSend(wxCommandEvent &event)
{
	clSocket * sock = clSocket::Instance();
	// odeslání zprávy
	wxString text = textChat->GetValue();
	text.Replace(_T("<"),_T("&lt;"));
	text.Replace(_T(">"),_T("&gt;"));
	std::string zprava = std::string(text.mb_str());	

	if(!zprava.empty())
		sock->SendRequest(std::string("<chat from=\""+  dama::ClientConfig::instance()->getUser() +"\" >" + zprava + "</chat>"));
	textChat->ChangeValue(wxEmptyString);
}

void LobbyWindow::onCloseWindow(wxCloseEvent &event)
{
	/// @TODO odhlášení

	((MenuWindow*)GetParent())->enableLogin();

	// odregistrování příjemu zpráv týkajících se sítě
	clSocket * sock = clSocket::Instance();
	sock->UnregisterHandler(this);

	Destroy();
}

/**
 * Provede obnovení seznamu hráčů a rozehraných her
 */
void LobbyWindow::refresh()
{
    clSocket* sock = clSocket::Instance();
    
    // posleme request na seznam hracu na serveru
    sock->SendRequest(std::string("<playerslist />"));
    
    sock->SendRequest(std::string("<getgames />"));
    
}

/**
 * Naplní seznam hráčů z xml elementu
 * 
 * @param element Element se seznamem hráčů
 */
void LobbyWindow::setPlayerList(ticpp::Element* element)
{
    // vyprazdnime seznam
    listPlayers->DeleteAllItems();
    
    // neni vybran zadny hrac
    selectedPlayer = _("");
    
    // projdeme xml
    ticpp::Iterator<ticpp::Element> child("player");  
    
    for ( child = child.begin(element); child != child.end(); child++ ) 
    {
        std::string name = child->GetText(false);
        
        // vytvorime novou polozku seznamu
        wxListItem item;
        item.SetText(wxString(name.c_str(), wxConvUTF8));
        item.SetColumn(0);
    
        // vlozime ji do seznamu
        listPlayers->InsertItem(item);
    }
}

/**
 * Event handler pro aktivaci položky v seznamu hráčů
 * 
 * @param event
 */
void LobbyWindow::onPlayersActivate(wxListEvent &event)
{
    invite(event.GetText());
}

/**
 * Event handler pro výběr položky v seznamu hráčů
 * 
 * @param event
 */
void LobbyWindow::onPlayersSelect(wxListEvent &event)
{
   selectedPlayer = event.GetText(); 
}

/**
 * Pozvání hráče
 * 
 * @param name Jméno hráče
 */
void LobbyWindow::invite(wxString name)
{
    std::string ownName = dama::ClientConfig::instance()->getUser();
    
    clSocket* sock = clSocket::Instance();
    
    sock->SendRequest("<duel player=\"" + ownName + "\" with=\"" + std::string(name.mb_str()) + "\" />");
}

/**
 * Event handler pro časované refreshování seznamu hráčů
 * 
 * @param event
 */
void LobbyWindow::onTimer(wxTimerEvent &event)
{
    refresh();
}

/**
 * Zpracování výzvy od jiného hráče
 * 
 * @param element Prichozi xml s vyzvou
 */
void LobbyWindow::acceptMatch(ticpp::Element* element)
{
    std::string player = element->GetAttribute("player");
    std::string with = element->GetAttribute("with");
    
    wxMessageDialog* dialog = new wxMessageDialog(this,
                  _("Hráč '") + wxString(player.c_str(), wxConvUTF8) + _("' Vás vyzval ke hře.\nPřejete si výzvu přijmout?"), 
                  _("Výzva"), wxYES|wxNO);
    int answer = dialog->ShowModal();
    dialog->Destroy();
    
    std::string accept;
    
    if (answer == wxID_NO)
    {
        accept = "false";
    }
    else
    {
        accept = "true";
    }
    
    clSocket* sock = clSocket::Instance();
    sock->SendRequest("<duel player=\"" + player + "\" with=\"" + with + "\" accept=\"" + accept + "\" />");
}
    
/**
 * Event handler pro aktivaci položky v seznamu her
 * 
 * @param event
 */
void LobbyWindow::onGamesActivate(wxListEvent &event)
{
    //invite(event.GetText());
}

/**
 * Event handler pro výběr položky v seznamu her
 * 
 * @param event
 */
void LobbyWindow::onGamesSelect(wxListEvent &event)
{
   selectedGame = event.GetText(); 
}

/**
 * Naplní seznam hráčů z xml elementu
 * 
 * @param element Element se seznamem hráčů
 */
void LobbyWindow::setGamesList(ticpp::Element* element)
{
    // vyprazdnime seznam
    listGames->DeleteAllItems();
    
    // neni vybran zadny hrac
    selectedGame = _("");
    
    // projdeme xml
    ticpp::Iterator<ticpp::Element> child("gamewith");  
    
    for ( child = child.begin(element); child != child.end(); child++ ) 
    {
        std::string name = child->GetText(false);
        
        // vytvorime novou polozku seznamu
        wxListItem item;
        item.SetText(wxString(name.c_str(), wxConvUTF8));
        item.SetColumn(0);
    
        // vlozime ji do seznamu
        listGames->InsertItem(item);
    }
}
