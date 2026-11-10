#pragma once
#include "tray.h"
#include <windows.h>

namespace fxchain {
void toggleQuickPanel(HWND owner, const std::vector<TrayMenuItem>& menu, int volume,
                      const std::function<QuickPanelStatus()>& status,
                      const std::function<void(int)>& command);
void pumpQuickPanel();
void closeQuickPanel();
bool quickPanelMessage(MSG& msg);
}
