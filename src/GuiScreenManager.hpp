#ifndef _guiscreenswitchbutton_hpp
#define _guiscreenswitchbutton_hpp

#include <string>
#include <map>
#include <utility>
#include <functional>
#include <SDL2/SDL.h>
#include "SGUIL/SGUIL.hpp"

class GuiScreenManager
{
public:
    GuiScreenManager()
    {
        currentScreen = NULL;
    }

    void loadScreen(std::string name)
    {
        auto it = screens.find(name);
        if(it == screens.end())
        {
            auto itM = screenMakers.find(name);
            if(itM != screenMakers.end())
            {
                GuiScreen *s = (std::get<1>(*itM))(this);
                screens[std::get<0>(*itM)] = s;
                currentScreen = s;
            }
        }
        else
        {
            currentScreen = std::get<1>(*it);
        }
    }

    void drawScreen()
    {
        if(currentScreen)
        {
            currentScreen->draw();
        }
    }

    void handleSDLEvent(SDL_Event& event, GuiPoint logicalMousePos)
    {
        if(currentScreen)
        {
            currentScreen->handleSDLEvent(event, logicalMousePos);
        }
    }

protected:
    std::map<std::string, GuiScreen *> screens;
    std::map<std::string, std::function<GuiScreen *(GuiScreenManager *)>> screenMakers;
    GuiScreen *currentScreen;
};

class GuiScreenSwitchButton : public GuiButton
{
public:
    GuiScreenSwitchButton(int ID, std::string screenName, GuiScreenManager *mngr, std::string displayString, BitFont& font, SDL_Rect relativeDestRect)
        : GuiButton(ID, relativeDestRect, displayString, font)
    {
        this->screenName = screenName;
        this->mngr = mngr;
    }

    virtual void mouseClicked(int x, int y, Uint8 button) override
    {
        if(mngr)
        {
            mngr->loadScreen(screenName);
        }
    }

protected:
    std::string screenName;
    GuiScreenManager *mngr;
};

#endif
