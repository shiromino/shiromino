#include "ScreenManager.h"
#include <functional>
#include "CoreState.h"
#include "game_qs.h"

GUIScreen *mainMenu_create(CoreState *cs, BitFont& font)
{
    SDL_Rect destRect = {0, 0, 640, 480};
    GUIScreen *mainMenu = new GUIScreen {cs, "Main Menu", mainMenuInteractionCallback, destRect};

    SDL_Rect pentominoRect = {20, 20, 100, 20};
    Button *button1 = new Button {0, pentominoRect, "Pentomino C", font};

    SDL_Rect g1MasterRect = {20, 42, 100, 20};
    Button *button2 = new Button {1, g1MasterRect, "G1 Master", font};

    mainMenu->addControlElement(button1);
    mainMenu->addControlElement(button2);

    return mainMenu;
}

void mainMenuInteractionCallback(GUIInteractable& interactable, GUIEvent& event)
{
    if(event.type == mouse_clicked)
    {
        CoreState *cs = interactable.getWindow()->origin;

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
