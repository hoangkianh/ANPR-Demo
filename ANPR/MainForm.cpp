#include "MainForm.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThread]
void Main (array<String^> ^args) 
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	ANPR::MainForm form;
	Application::Run(%form);
}