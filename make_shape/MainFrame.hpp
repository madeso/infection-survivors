#ifndef LLMS_MAIN_FRAME_HPP
#define LLMS_MAIN_FRAME_HPP

#include <string>

#include "wx.hpp"

class WorldView;

class MainFrame : public wxFrame {
public:
	MainFrame();
	~MainFrame();

	void OnUpdateColor(wxScrollEvent& event);

	void OnExportFile(wxCommandEvent& event);
	void OnExportTexture(wxCommandEvent& event);
	void OnNewWorld(wxCommandEvent& event);

	void OnRenderModeWire(wxCommandEvent& event);
	void OnRenderModeHalo(wxCommandEvent& event);

	void OnAspect_1_1(wxCommandEvent& event);
	void OnAspect_14_9(wxCommandEvent& event);
	void OnAspect_16_9(wxCommandEvent& event);
	void OnAspect_1_33(wxCommandEvent& event);
	void OnAspect_1_85(wxCommandEvent& event);
	void OnAspect_2_35(wxCommandEvent& event);

	void OnAspect_4_3(wxCommandEvent& event);
	void OnAspect_5_4(wxCommandEvent& event);

	void setInfoText(const std::string& iInfo);
protected:
	wxMenu* buildFileMenu();
	wxMenu* buildAspectMenu();

	void sendColor();
private:
	DECLARE_EVENT_TABLE()

	wxPanel* panel;

	WorldView* mWorldView;
	wxTextCtrl* mFileProperty;
	wxSlider* mRedSlider;
	wxSlider* mGreenSlider;
	wxSlider* mBlueSlider;
	wxSlider* mAlphaSlider;

	wxComboBox* mWidth;
	wxComboBox* mHeight;
};

#endif