#ifndef MAIN_FRAME_HPP
#define MAIN_FRAME_HPP

#include "wx.hpp"
#include <string>

class WorldView;

class MainFrame : public wxFrame {
public:
	MainFrame();
	~MainFrame();

	void OnUpdateInputProperty(wxCommandEvent& event);

	void OnSelectPen(wxCommandEvent& event);
	void OnSelectErasor(wxCommandEvent& event);
	void OnNewWorld(wxCommandEvent& event);

	void OnRenderModeWire(wxCommandEvent& event);
	void OnRenderModeHalo(wxCommandEvent& event);

	void setInfoText(const std::string& iInfo);
protected:
	wxMenu* buildFileMenu();
	wxMenu* buildViewMenu();
	wxMenu* buildToolsMenu();
private:
	DECLARE_EVENT_TABLE()

	wxPanel* panel;

	WorldView* mWorldView;
	wxTextCtrl* mInputProperty;
};

#endif