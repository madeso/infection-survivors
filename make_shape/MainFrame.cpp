#include "make_shape/MainFrame.hpp"
#include "make_shape/WorldView.hpp"
#include "make_shape/World.hpp"

enum {
	FILE_PROPERTY = wxID_HIGHEST +1,
	RED_PROPERTY,
	GREEN_PROPERTY,
	BLUE_PROPERTY,
	ALPHA_PROPERTY
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
	ID_ASPECT_1_1,
	ID_ASPECT_14_9,
	ID_ASPECT_16_9,
	ID_ASPECT_1_33,
	ID_ASPECT_1_85,
	ID_ASPECT_2_35,

	ID_ASPECT_4_3,
	ID_ASPECT_5_4,

	// TOOL
	ID_EXPORT_FILE,
	ID_EXPORT_TEX
};

unsigned int GetResolutionFromIndex(int index) {
	switch(index) {
		case 0: return 256;
		case 1: return 512;
		case 2: return 1024;
		case 3:
		default: return 4096;
	}
}

MainFrame::MainFrame() : wxFrame(NULL, wxID_ANY, "Shape Maker" ), mWorldView(0), mFileProperty(0), mRedSlider(0), mGreenSlider(0), mBlueSlider(0) {
	panel = new wxPanel(this);
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	const int space = 2;

	mWorldView = WorldView::build(panel, this);
	sizer->Add( mWorldView, 1, wxEXPAND | wxALL, space);

	wxPanel* bottomPanel = new wxPanel(panel);
	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add( bottomPanel, 0, wxEXPAND | wxALL, space);

	mRedSlider = new wxSlider(bottomPanel, RED_PROPERTY, 125, 0, 255);
	mRedSlider->SetToolTip("Red color");
	bottomSizer->Add( mRedSlider, 0, wxEXPAND | wxALL, space);

	mGreenSlider = new wxSlider(bottomPanel, GREEN_PROPERTY, 125, 0, 255);
	mGreenSlider->SetToolTip("Green color");
	bottomSizer->Add( mGreenSlider, 0, wxEXPAND | wxALL, space);

	mBlueSlider = new wxSlider(bottomPanel, BLUE_PROPERTY, 255, 0, 255);
	mBlueSlider->SetToolTip("Blue color");
	bottomSizer->Add( mBlueSlider, 0, wxEXPAND | wxALL, space);

	mAlphaSlider = new wxSlider(bottomPanel, ALPHA_PROPERTY, 255, 0, 255);
	mAlphaSlider->SetToolTip("Alpha");
	bottomSizer->Add( mAlphaSlider, 0, wxEXPAND | wxALL, space);

	mFileProperty = new wxTextCtrl(bottomPanel, FILE_PROPERTY, "");
	mFileProperty->SetToolTip("Texture file");
	bottomSizer->Add( mFileProperty, 1, wxEXPAND | wxALL, space);

	const unsigned int resCount = 4;
	const wxString res[resCount] = {
		wxString("256"),
		wxString("512"),
		wxString("1024"),
		wxString("4096")
	};
	mWidth = new wxComboBox(bottomPanel, -1, res[0], wxDefaultPosition, wxDefaultSize, resCount, res, wxCB_DROPDOWN|wxCB_READONLY);
	mWidth->SetToolTip("Width");
	bottomSizer->Add( mWidth, 0, wxEXPAND | wxALL, space);
	mHeight = new wxComboBox(bottomPanel, -1, res[0], wxDefaultPosition, wxDefaultSize, resCount, res, wxCB_DROPDOWN|wxCB_READONLY);
	mHeight->SetToolTip("Height");
	bottomSizer->Add( mHeight, 0, wxEXPAND | wxALL, space);

	wxMenuBar* menuBar = new wxMenuBar;
	menuBar->Append(buildFileMenu(), "&File");
	menuBar->Append(buildAspectMenu(), "&Aspect");
	//menuBar->Append(buildToolsMenu(), "&Tools");
	//menuBar->Append(buildViewMenu(), "&View");
	//menuBar->Append(buildCameraMenu(), "&Camera");
	//menuBar->Append(buildWindowMenu(), "&Window");
	//menuBar->Append(buildHelpMenu(), "&Help");
	SetMenuBar(menuBar);

	bottomSizer->SetSizeHints(bottomPanel);
	sizer->SetSizeHints(panel);

	bottomPanel->SetSizerAndFit(bottomSizer);
	panel->SetSizerAndFit(sizer);
	
	bottomSizer->Layout();
	sizer->Layout();
	panel->Layout();
	Layout();
	
	SetMinSize(sizer->GetMinSize());

	sendColor();
}

MainFrame::~MainFrame() {
}
void MainFrame::setInfoText(const std::string& iInfo) {
}

void MainFrame::sendColor() {
	const int red = mRedSlider->GetValue();
	const int green = mGreenSlider->GetValue();
	const int blue = mBlueSlider->GetValue();
	const int alpha = mAlphaSlider->GetValue();
	const ::lunarlady::real max = 255;
	mWorldView->getWorld().setColor( red/max, green/max, blue/max, alpha/max );
	Refresh();
}

void MainFrame::OnUpdateColor(wxScrollEvent& event){
	sendColor();
}

void MainFrame::OnExportFile(wxCommandEvent& event) {
	wxFileDialog file(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "2d object files (*.obj2)|*.obj2", wxSAVE);
	if( file.ShowModal() == wxID_OK ) {
		unsigned int width = GetResolutionFromIndex(mWidth->GetSelection());
		unsigned int height = GetResolutionFromIndex(mHeight->GetSelection());
		mWorldView->getWorld().exportToFile( file.GetPath().c_str(), mFileProperty->GetValue().c_str(), height, width);
	}
}
void MainFrame::OnExportTexture(wxCommandEvent& event) {
	wxFileDialog file(this, wxFileSelectorPromptStr, wxEmptyString, wxEmptyString, "Texture files (*.png, *.jpeg, *.bmp, *.tif *.raw)|*.png;*.jpeg;*.bmp;*.tif;*.jpg;*.tga;*.raw", wxSAVE);
	if( file.ShowModal() == wxID_OK ) {
		unsigned int width = GetResolutionFromIndex(mWidth->GetSelection());
		unsigned int height = GetResolutionFromIndex(mHeight->GetSelection());
		mWorldView->getWorld().saveImage(file.GetPath().c_str(), width, height);
	}
}
void MainFrame::OnNewWorld(wxCommandEvent& event) {
	mWorldView->newWorld();
	sendColor();
}

void MainFrame::OnAspect_1_1(wxCommandEvent& event) {
	mWorldView->getWorld().setAspect(1.0);
}
void MainFrame::OnAspect_14_9(wxCommandEvent& event) {
	mWorldView->getWorld().setAspect(14.0 / 9.0);
}
void MainFrame::OnAspect_16_9(wxCommandEvent& event) {
	mWorldView->getWorld().setAspect(16.0 / 9.0);
}
void MainFrame::OnAspect_1_33(wxCommandEvent& event) {
	mWorldView->getWorld().setAspect(1.33);
}
void MainFrame::OnAspect_1_85(wxCommandEvent& event) {
	mWorldView->getWorld().setAspect(1.82);
}
void MainFrame::OnAspect_2_35(wxCommandEvent& event) {
	mWorldView->getWorld().setAspect(2.35);
}

void MainFrame::OnAspect_4_3(wxCommandEvent& event) {
	mWorldView->getWorld().setAspect(4.0 / 3.0);
}
void MainFrame::OnAspect_5_4(wxCommandEvent& event) {
	mWorldView->getWorld().setAspect(5.0 / 4.0);
}

wxMenu* MainFrame::buildFileMenu() {
	wxMenu* menu = new wxMenu;
	menu->Append(ID_NEW, "&New\tCtrl-N", "Creates a new world");
	menu->Append(ID_OPEN, "&Open\tCtrl-O", "Open a previously saved level");
	menu->AppendSeparator();
	menu->Append(ID_SAVE, "&Save\tCtrl-S", "Save the world");
	menu->Append(ID_SAVEAS, "Save as", "Save the world as a new file and continue using that");
	menu->Append(ID_SAVECOPY, "Save copy", "Save a backup of the world");
	menu->Append(ID_REVERT, "&Revert", "Discard changes made to the file");
	menu->AppendSeparator();
	menu->Append(ID_EXPORT_FILE, "Export 2d &object", "Export as a 2d object");
	menu->Append(ID_EXPORT_TEX, "Export &texture image", "Export as a image");
	menu->AppendSeparator();
	menu->Append(ID_EXIT, "E&xit\tAlt-F4", "Exit the Shape Maker");
	return menu;
}

wxMenu* MainFrame::buildAspectMenu() {
	wxMenu* menu = new wxMenu;

	menu->Append(ID_ASPECT_1_1, "1:1");
	menu->AppendSeparator();
	menu->Append(ID_ASPECT_4_3, "4:3");
	menu->Append(ID_ASPECT_5_4, "5:4");
	menu->AppendSeparator();
	menu->Append(ID_ASPECT_14_9, "14:9");
	menu->Append(ID_ASPECT_16_9, "16:9");
	menu->AppendSeparator();
	menu->Append(ID_ASPECT_1_33, "1.33");
	menu->Append(ID_ASPECT_1_85, "1.85");
	menu->Append(ID_ASPECT_2_35, "2.35");
	return menu;
}

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
	//EVT_TEXT(FILE_PROPERTY, MainFrame::OnUpdateInputProperty)

	EVT_COMMAND_SCROLL(RED_PROPERTY, MainFrame::OnUpdateColor)
	EVT_COMMAND_SCROLL(GREEN_PROPERTY, MainFrame::OnUpdateColor)
	EVT_COMMAND_SCROLL(BLUE_PROPERTY, MainFrame::OnUpdateColor)
	EVT_COMMAND_SCROLL(ALPHA_PROPERTY, MainFrame::OnUpdateColor)

	// file
	EVT_MENU(ID_NEW, MainFrame::OnNewWorld)

	// aspect
	EVT_MENU(ID_ASPECT_1_1, MainFrame::OnAspect_1_1)
	EVT_MENU(ID_ASPECT_14_9, MainFrame::OnAspect_14_9)
	EVT_MENU(ID_ASPECT_16_9, MainFrame::OnAspect_16_9)
	EVT_MENU(ID_ASPECT_1_33, MainFrame::OnAspect_1_33)
	EVT_MENU(ID_ASPECT_1_85, MainFrame::OnAspect_1_85)
	EVT_MENU(ID_ASPECT_2_35, MainFrame::OnAspect_2_35)

	EVT_MENU(ID_ASPECT_4_3,	OnAspect_4_3)
	EVT_MENU(ID_ASPECT_5_4, OnAspect_5_4)

	// tool
	EVT_MENU(ID_EXPORT_FILE, MainFrame::OnExportFile)
	EVT_MENU(ID_EXPORT_TEX, MainFrame::OnExportTexture)
	
	
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