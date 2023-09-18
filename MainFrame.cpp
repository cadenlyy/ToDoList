#include "MainFrame.h"
#include <wx/wx.h>
#include <vector>
#include <string>
#include "Task.h"
#include <wx/sizer.h>

int hour = 0, min = 0;

MainFrame::MainFrame(const wxString& title) : wxFrame(nullptr, wxID_ANY, title)
{
	CreateControls();
	BindEventHandlers();
	AddSavedTasks();
	ShowTime();
}

void MainFrame::CreateControls()
{
	wxFont headLineFont(wxFontInfo(wxSize(0, 36)).Bold());
	wxFont mainFont(wxFontInfo(wxSize(0, 24)));

	panel = new wxPanel(this);
	panel->SetFont(mainFont);

	headLineText = new wxStaticText(panel, wxID_ANY, "Hw List", wxPoint(0, 22), wxSize(800, -1), wxALIGN_CENTER_HORIZONTAL);
	headLineText->SetFont(headLineFont);

	inputField = new wxTextCtrl(panel, wxID_ANY, "", wxPoint(100, 80), wxSize(495, 35), wxTE_PROCESS_ENTER);
	addButton = new wxButton(panel, wxID_ANY, "Add", wxPoint(600, 80), wxSize(100, 35));
	checkListBox = new wxCheckListBox(panel, wxID_ANY, wxPoint(100, 120), wxSize(600, 400));
	clearButton = new wxButton(panel, wxID_ANY, "Clear", wxPoint(100, 525), wxSize(100, 35));
	totalTime = new wxStaticText(panel, wxID_ANY, wxString(""), wxPoint(200, 525), wxSize(100, 35));

	boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizerh = new wxBoxSizer(wxHORIZONTAL);
	boxSizerf = new wxBoxSizer(wxHORIZONTAL);

	boxSizerh->Add(inputField, 10);
	boxSizerh->Add(addButton, 1);

	boxSizerf->Add(clearButton, 0);
	boxSizerf->Add(totalTime, 0);

	boxSizer->Add(headLineText, 0, wxEXPAND);
	boxSizer->Add(boxSizerh, 0, wxEXPAND);
	boxSizer->Add(checkListBox, 0, wxEXPAND);
	boxSizer->Add(boxSizerf, 0);
	panel->SetSizerAndFit(boxSizer);
}

void MainFrame::BindEventHandlers()
{
	addButton->Bind(wxEVT_BUTTON, &MainFrame::OnAddButtonClicked, this);
	inputField->Bind(wxEVT_TEXT_ENTER, &MainFrame::OnInputEnter, this);
	checkListBox->Bind(wxEVT_KEY_DOWN, &MainFrame::OnListKeyDown, this);
	clearButton->Bind(wxEVT_BUTTON, &MainFrame::OnClearButtonClicked, this);
	this->Bind(wxEVT_CLOSE_WINDOW, &MainFrame::OnWindowClosed, this);
}

void MainFrame::AddSavedTasks()
{
	std::vector<Task> tasks = LoadTasksFromFile("tasks.txt");
	for (const Task& task : tasks) {
		int index = checkListBox->GetCount();
		checkListBox->Insert(task.description, index);
		checkListBox->Check(index, task.done);
	}
	std::pair<int, int> p = savedTime("tasks.txt");
	hour = p.first;
	min = p.second;
}

void MainFrame::OnAddButtonClicked(wxCommandEvent& evt)
{
	AddTaskFromInput();
}

void MainFrame::OnInputEnter(wxCommandEvent& evt)
{
	AddTaskFromInput();
}

void MainFrame::OnListKeyDown(wxKeyEvent& evt)
{
	switch (evt.GetKeyCode()) {
	case WXK_DELETE:
		DeleteSelectedTask();
		break;
	case WXK_UP:
		MoveSelectedTask(-1);
		break;
	case WXK_DOWN:
		MoveSelectedTask(1);
		break;
	}
}

void MainFrame::OnClearButtonClicked(wxCommandEvent& evt)
{
	if (checkListBox->IsEmpty()) {
		return;
	}

	wxMessageDialog dialog(this, "Are you sure you want to clear all task?", "Clear", wxYES_NO | wxCANCEL);
	int result = dialog.ShowModal();

	if (result == wxID_YES) {
		checkListBox->Clear();
		hour = 0; min = 0;
	}
	ShowTime();
}

void MainFrame::OnWindowClosed(wxCloseEvent& evt)
{
	std::vector<Task> tasks;
	for (int i = 0; i < checkListBox->GetCount(); i++) {
		Task task;
		task.description = checkListBox->GetString(i);
		task.done = checkListBox->IsChecked(i);
		tasks.push_back(task);
	}

	SaveTasksToFile(tasks, "tasks.txt", hour, min);
	evt.Skip();

}

void MainFrame::AddTime(int h, int m)
{
	min += m;
	hour += h;
	hour += min/60;
	min %= 60;
	ShowTime();
}

void MainFrame::AddTaskFromInput()
{
	wxString description = inputField->GetValue();
	if (!description.IsEmpty()) {
		checkListBox->Insert(description, checkListBox->GetCount());
		inputField->Clear();
	}
	inputField->SetFocus();
	
	AddTime(find_time(std::string(description.mb_str())).first, find_time(std::string(description.mb_str())).second);
}

void MainFrame::DeleteSelectedTask()
{
	int selectedIndex = checkListBox->GetSelection();
	if (selectedIndex == wxNOT_FOUND) {
		return;
	}

	wxString s = checkListBox->GetStringSelection();
	AddTime(-find_time(std::string(s.mb_str())).first,-find_time(std::string(s.mb_str())).second);

	checkListBox->Delete(selectedIndex);
	
	ShowTime();
}

void MainFrame::MoveSelectedTask(int offset)
{
	int selectedIndex = checkListBox->GetSelection();
	if (selectedIndex == wxNOT_FOUND) {
		return;
	}

	int newIndex = selectedIndex + offset;

	if (newIndex >= 0 && newIndex < checkListBox->GetCount()) {
		SwapTask(selectedIndex, newIndex);
		checkListBox->SetSelection(newIndex, true);
	}
}

void MainFrame::SwapTask(int i, int j)
{
	Task taskI{ checkListBox->GetString(i).ToStdString(), checkListBox->IsChecked(i) };
	Task taskJ{ checkListBox->GetString(j).ToStdString(), checkListBox->IsChecked(j) };

	checkListBox->SetString(i, taskJ.description);
	checkListBox->Check(i, taskJ.done);

	checkListBox->SetString(j, taskI.description);
	checkListBox->Check(j, taskI.done);

}

void MainFrame::ShowTime()
{
	std::string tt = std::to_string(hour);
	tt.append("h ");
	tt.append(std::to_string(min));
	tt.push_back('m');
	totalTime->SetLabel(tt);
}

std::pair<int, int> MainFrame::find_time(std::string s)
{
	std::string hour = "", min = "";
	bool mc = true, hc = true;
	for (int i = s.size(); i > 0; i--) {
		if ((s[i] == 'm' or s[i] == 'M') and mc) {
			for (int j = i-1; j > -1; j--) {
				if (isdigit(s[j])) {
					min.push_back(s[j]);
				}
				else {
					mc = false;
					break;
				}
			}
		}
		else if ((s[i] == 'h' or s[i] == 'H') and hc) {
			for (int j = i - 1; j > -1; j--) {
				if (isdigit(s[j])) {
					hour.push_back(s[j]);
				}
				else {
					hc = false;
					break;
				}
			}
		}
	}
	if (hour.size() == 0) hour.push_back('0');
	if (min.size() == 0) min.push_back('0');
	std::reverse(min.begin(), min.end());
	std::pair<int, int> p = { stoi(hour),stoi(min) };
	return p;
}

std::pair<int, int> MainFrame::find_date(std::string s)
{
	
	return std::pair<int, int>();
}

