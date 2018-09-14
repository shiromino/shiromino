#include "GuiScreenManager.hpp"

#include "core.h"
#include "game_qs.h"

GuiScreen *mainMenu_create(coreState *cs, GuiScreenManager *mngr, BitFont& font)
{
    SDL_Rect destRect = {0, 0, 640, 480};
    GuiScreen *mainMenu = new GuiScreen {cs, "Main Menu", mainMenuInteractionCallback, destRect};

    SDL_Rect pentominoRect = {20, 20, 100, 20};
    GuiButton *button1 = new GuiButton {0, pentominoRect, "Pentomino C", font};

    SDL_Rect g1MasterRect = {20, 42, 100, 20};
    GuiButton *button2 = new GuiButton {1, g1MasterRect, "G1 Master", font};

    mainMenu->addControlElement(button1);
    mainMenu->addControlElement(button2);

    return mainMenu;
}

void mainMenuInteractionCallback(GuiInteractable& interactable, GuiEvent& event)
{
    if(event.type == mouse_clicked)
    {
        coreState *cs = interactable.getWindow()->origin;

        switch(interactable.ID)
        {
            default:
                break;

            case 0:
                cs->p1game = qs_game_create(cs, 0, MODE_PENTOMINO, -1);
                cs->p1game->init(cs->p1game);
                break;

            case 1:
                cs->p1game = qs_game_create(cs, 0, MODE_G1_MASTER, -1);
                cs->p1game->init(cs->p1game);
                break;
        }
    }
}
