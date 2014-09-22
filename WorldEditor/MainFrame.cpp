#include "MainFrame.hpp"
#include "WorldView.hpp"
#include "PenTool.hpp"
#include "ErasorTool.hpp"

enum {
	INPUT_PROPERTY = 100,
};

enum {
	ID_EXIT = wxID_EXIT,
	ID_ABOUT = wxID_ABOUT,
	
	// FILE
	ID_NEW = 2000,
	ID_OPEN,
	ID_SAVE,
	ID_SAVEAS,
	ID_SAVECOPY,
	ID_REVERT,

	// View
	ID_RENDERMODE_WIRE,
	ID_RENDERMODE_HALO,

	// TOOL
	ID_TOOL_PEN,
	ID_TOOL_ERASOR
};

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, "World editor" ), mWorldView(0), mInputProperty(0) {
	panel = new wxPanel(this);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	const int space = 2;

	mWorldView = WorldView::build(panel, this);
	sizer->Add( mWorldView, 1, wxEXPAND | wxALL, space);

	mInputProperty = new wxTextCtrl(panel, INPUT_PROPERTY, "");
	sizer->Add( mInputProperty, 0, wxEXPAND | wxALL, space);

	panel->SetSizer(sizer);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(buildFileMenu(), "&File");
	menuBar->Append(buildViewMenu(), "&View");
	menuBar->Append(buildToolsMenu(), "&Tools");
	//menuBar->Append(buildViewMenu(), "&View");
	//menuBar->Append(buildCameraMenu(), "&Camera");
	//menuBar->Append(buildWindowMenu(), "&Window");
	//menuBar->Append(buildHelpMenu(), "&Help");
	SetMenuBar(menuBar);

	Layout();
	panel->Layout();
	SetMinSize(sizer->GetMinSize());
}

MainFrame::~MainFrame() {
}
void MainFrame::setInfoText(const std::string& iInfo) {
	mInputProperty->SetValue( iInfo.c_str() );
}

void MainFrame::OnUpdateInputProperty(wxCommandEvent& event) {
}
void MainFrame::OnSelectPen(wxCommandEvent& event) {
	mWorldView->selectTool( new PenTool( mWorldView ) );
}
void MainFrame::OnSelectErasor(wxCommandEvent& event) {
	mWorldView->selectTool( new ErasorTool( mWorldView ) );
}
void MainFrame::OnNewWorld(wxCommandEvent& event) {
	mWorldView->newWorld();
}

void MainFrame::OnRenderModeHalo(wxCommandEvent& event) {
	mWorldView->selectRenderMode(RM_HALOED_LINE);
}

void MainFrame::OnRenderModeWire(wxCommandEvent& event) {
	mWorldView->selectRenderMode(RM_WIREFRAME);
}

wxMenu* MainFrame::buildFileMenu() {
	wxMenu* menu = new wxMenu;
	menu->Append(ID_NEW, "&New\tCtrl-N", "Creates a new world");
	menu->Append(ID_OPEN, "&Open\tCtrl-O", "Open a previously saved level");
	menu->AppendSeparator();
	menu->Append(ID_SAVE, "&Save\tCtrl-S", "Save the world");
	menu->Append(ID_SAVEAS, "Save as", "Save the world as a new file and continue using that");
	menu->Append(ID_SAVECOPY, "Save copy", "Save a backup of the world");
	menu->Append(ID_REVERT, "&Revert", "Discard changes made to the level");
	menu->AppendSeparator();
	menu->Append(ID_EXIT, "E&xit\tAlt-F4", "Exit Lunar Lady World Editor");
	return menu;
}

wxMenu* MainFrame::buildToolsMenu() {
	wxMenu* menu = new wxMenu;
	menu->Append(ID_TOOL_PEN, "&Pen tool\tP", "Selects pen tool");
	menu->Append(ID_TOOL_ERASOR, "&Erasor tool\tE", "Selects erasor tool");
	return menu;
}

wxMenu* MainFrame::buildViewMenu() {
	wxMenu* menu = new wxMenu;
	menu->Append(ID_RENDERMODE_WIRE, "Render Mode: Wireframe", "Wireframe");
	menu->Append(ID_RENDERMODE_HALO, "Render Mode: Haloed lines", "Haloed line");
	return menu;
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	EVT_TEXT(INPUT_PROPERTY, MainFrame::OnUpdateInputProperty)

	// file
	EVT_MENU(ID_NEW, MainFrame::OnNewWorld)

	// view
	EVT_MENU(ID_RENDERMODE_WIRE, MainFrame::OnRenderModeWire)
	EVT_MENU(ID_RENDERMODE_HALO, MainFrame::OnRenderModeHalo)

	// tool
	EVT_MENU(ID_TOOL_PEN, MainFrame::OnSelectPen)
	EVT_MENU(ID_TOOL_ERASOR, MainFrame::OnSelectErasor)
	
	
	/*// general
	EVT_MENU(ID_EXIT, MainFrame::OnExit)
	EVT_MENU(ID_ABOUT, MainFrame::OnAbout)
	// view along
	EVT_MENU(ID_VIEW_ALONG_RED_POSITIVE, MainFrame::OnViewAlongRedPositivie)
	EVT_MENU(ID_VIEW_ALONG_GREEN_POSITIVE, MainFrame::OnViewAlongGreenPositivie)
	EVT_MENU(ID_VIEW_ALONG_BLUE_POSITIVE, MainFrame::OnViewAlongBluePositivie)
	EVT_MENU(ID_VIEW_ALONG_RED_NEGATIVE, MainFrame::OnViewAlongRedNegative)
	EVT_MENU(ID_VIEW_ALONG_GREEN_NEGATIVE, MainFrame::OnViewAlongGreenNegative)
	EVT_MENU(ID_VIEW_ALONG_BLUE_NEGATIVE, MainFrame::OnViewAlongBlueNegative)
	//camera move
	EVT_MENU(ID_MOVE_IN, MainFrame::OnViewMoveIn)
	EVT_MENU(ID_MOVE_OUT, MainFrame::OnViewMoveOut)
	EVT_MENU(ID_MOVE_RIGHT, MainFrame::OnViewMoveRight)
	EVT_MENU(ID_MOVE_LEFT, MainFrame::OnViewMoveLeft)
	EVT_MENU(ID_MOVE_UP, MainFrame::OnViewMoveUp)
	EVT_MENU(ID_MOVE_DOWN, MainFrame::OnViewMoveDown)
	//camera load
	EVT_MENU(ID_LOAD_CAMERA_0, MainFrame::OnLoadCamera0)
	EVT_MENU(ID_LOAD_CAMERA_1, MainFrame::OnLoadCamera1)
	EVT_MENU(ID_LOAD_CAMERA_2, MainFrame::OnLoadCamera2)
	EVT_MENU(ID_LOAD_CAMERA_3, MainFrame::OnLoadCamera3)
	EVT_MENU(ID_LOAD_CAMERA_4, MainFrame::OnLoadCamera4)
	EVT_MENU(ID_LOAD_CAMERA_5, MainFrame::OnLoadCamera5)
	EVT_MENU(ID_LOAD_CAMERA_6, MainFrame::OnLoadCamera6)
	EVT_MENU(ID_LOAD_CAMERA_7, MainFrame::OnLoadCamera7)
	EVT_MENU(ID_LOAD_CAMERA_8, MainFrame::OnLoadCamera8)
	EVT_MENU(ID_LOAD_CAMERA_9, MainFrame::OnLoadCamera9)
	// camera save
	EVT_MENU(ID_SAVE_CAMERA_0, MainFrame::OnSaveCamera0)
	EVT_MENU(ID_SAVE_CAMERA_1, MainFrame::OnSaveCamera1)
	EVT_MENU(ID_SAVE_CAMERA_2, MainFrame::OnSaveCamera2)
	EVT_MENU(ID_SAVE_CAMERA_3, MainFrame::OnSaveCamera3)
	EVT_MENU(ID_SAVE_CAMERA_4, MainFrame::OnSaveCamera4)
	EVT_MENU(ID_SAVE_CAMERA_5, MainFrame::OnSaveCamera5)
	EVT_MENU(ID_SAVE_CAMERA_6, MainFrame::OnSaveCamera6)
	EVT_MENU(ID_SAVE_CAMERA_7, MainFrame::OnSaveCamera7)
	EVT_MENU(ID_SAVE_CAMERA_8, MainFrame::OnSaveCamera8)
	EVT_MENU(ID_SAVE_CAMERA_9, MainFrame::OnSaveCamera9)
    EVT_CLOSE(         MainFrame::OnQuit)*/
END_EVENT_TABLE()