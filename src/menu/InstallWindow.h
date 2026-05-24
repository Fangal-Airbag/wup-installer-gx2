#ifndef INSTALL_WINDOW_H_
#define INSTALL_WINDOW_H_

#include "fs/CFolderList.hpp"
#include "gui/MessageBox.h"
#include "ProgressWindow.h"

class MainWindow;

class InstallWindow : public GuiFrame, public CThread, public sigslot::has_slots<>
{
public:
	InstallWindow(CFolderList * list, bool deleteAfterInstall = false);
	~InstallWindow();
	
	void startInstalling()
	{
		resumeThread();
	}
	
	sigslot::signal1<GuiElement *> installWindowClosed;
	
private:
	void OnValidInstallClick(GuiElement * element, int val);
	void OnDestinationChoice(GuiElement * element, int choice);
	void OnCloseWindow(GuiElement * element, int val);
	void OnWindowClosed(GuiElement * element);
	void OnInstallProcessCancel(GuiElement *element, int val);
	
	void OnOpenEffectFinish(GuiElement *element);
	void OnCloseEffectFinish(GuiElement *element);
	
	void executeThread();
	void InstallProcess(int pos, int total);
	
	GuiFrame * drcFrame;
	
	CFolderList * folderList;
	
	MessageBox * messageBox;
	
	MainWindow * mainWindow;
	
	int folderCount;
	bool canceled;
	bool deleteAfterInstall;
	int target;
	
	enum
	{
		NAND,
		USB
	};
	
};

#endif
