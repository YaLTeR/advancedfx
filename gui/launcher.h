#ifndef __HLAE_LAUNCHER__
#define __HLAE_LAUNCHER__

#include <wx/string.h>
#include <wx/checkbox.h>
#include <wx/stattext.h>
#include <wx/combobox.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statline.h>
#include <wx/textctrl.h>
#include <wx/choice.h>
#include <wx/statbox.h>
#include <wx/dialog.h>
#include <wx/list.h>

#define HLAE_HLDEFAULTPATH wxT("C:\\Program Files\\Steam\\SteamApps")

struct SChoiceElement : public wxObject
{
	wxString describtion;
	wxString value;
};

class CChoiceList : public wxObject
{

private:
	wxList* m_list;
	wxChoice* m_choice;
	wxTextCtrl* m_textctrl;

public:
	CChoiceList();
	~CChoiceList();

	void Update();
	void SetControls(wxChoice* choice, wxTextCtrl* textctrl);
	void AddChoice(const wxString& describtion, const wxString& value);
	wxArrayString GetChoices();
	wxString GetCurrentChoice(const wxString& format = wxT("%s"));

};

class CLauncherDialog : public wxDialog 
{

private:

	CChoiceList* m_depthchoices;
	CChoiceList* m_modchoices;

	wxString m_fullcmdline;

	// Preset variables
	wxString m_additionalcmdline;
	wxString m_path;
	wxString m_depth;
	wxString m_width; 
	wxString m_height;
	bool m_force;


	enum {
		ID_Changed = wxID_HIGHEST+1,
		ID_Launch,
		ID_Browse,
		ID_ChoiceDepth,
		ID_ChoiceMod,
		ID_SavePreset,
		ID_DeletePreset
	};

	wxComboBox* m_cb_preset;
	wxCheckBox* m_ch_force;
	wxTextCtrl* m_tc_path;
	wxChoice* m_c_mod;
	wxTextCtrl* m_tc_mod;
	wxTextCtrl* m_tc_width;
	wxTextCtrl* m_tc_height;
	wxChoice* m_c_depth;
	wxTextCtrl* m_tc_depth;
	wxTextCtrl* m_tc_additionalcmdline;
	wxTextCtrl* m_tc_fullcmdline;

	void OnLaunch(wxCommandEvent& evt);
	void OnSavePreset(wxCommandEvent& evt);
	void OnDeletePreset(wxCommandEvent& evt);
	void OnChoiceMod(wxCommandEvent& evt);
	void OnChoiceDepth(wxCommandEvent& evt);
	void OnChanged(wxCommandEvent& evt);
	void OnBrowse(wxCommandEvent& evt);

	void UpdateChoice(wxChoice* choice, wxTextCtrl* textctrl);
	void UpdateCmdline();

public:
	CLauncherDialog(wxWindow* parent);
	~CLauncherDialog();

};

#endif