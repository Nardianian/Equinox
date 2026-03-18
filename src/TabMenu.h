/*
  ==============================================================================
     ______ ____   __  __ ____ _   __ ____  _  __
    / ____// __ \ / / / //  _// | / // __ \| |/ /
   / __/  / / / // / / / / / /  |/ // / / /|   /
  / /___ / /_/ // /_/ /_/ / / /|  // /_/ //   |
 /_____/ \___\_\\____//___//_/ |_/ \____//_/|_|
 
    TabMenu.h
    Author:  Oliver Rasmussen

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
class TabMenu : public TabbedComponent
{
public:
    
    TabMenu(TabbedButtonBar::Orientation orientation);

    void currentTabChanged(int currentTabIndex, const String &currentTabName) override;
    
    std::function<void()> onTabChanged;
    
};
