#include "Menu/ActionOption.h"
#include "Menu/ElementType.h"
#include "Menu/GameMultiOption.h"
#include "Menu/GameOption.h"
#include "Menu/MetaGameOption.h"
#include "Menu/MultiOption.h"
#include "Menu/TextOption.h"
#include "Menu/ToggleOption.h"
#include "MenuOption.h"
#include "CoreState.h"
#include "gfx.h"
#include "QRS0.h"
namespace Shiro {
    MenuOption create_menu_option(ElementType type, int (*value_update_callback)(CoreState *cs), std::string label) {
        MenuOption m;

        m.type = type;
        m.value_update_callback = value_update_callback;
        m.label = label;
        m.x = 0;
        m.y = 0;
        m.render_update = 1;

        m.label_text_flags = 0;
        m.value_text_flags = 0;
        m.label_text_rgba = RGBA_DEFAULT;
        m.value_text_rgba = RGBA_DEFAULT;

        ActionOptionData *d1 = NULL;
        MultiOptionData *d2 = NULL;
        ToggleOptionData *d3 = NULL;
        GameOptionData *d4 = NULL;
        MetaGameOptionData *d5 = NULL;
        GameMultiOptionData *d6 = NULL;
        TextOptionData *d7 = NULL;

        switch(type) {
            case ElementType::MENU_LABEL:
                m.data = NULL;
                m.deleteData = NULL;
                break;

            case ElementType::MENU_ACTION:
                m.data = (ActionOptionData*)new ActionOptionData();
                m.deleteData = delete_opt_data<ActionOptionData>;
                d1 = (ActionOptionData *)m.data;
                d1->action = NULL;
                d1->val = 0;
                break;

            case ElementType::MENU_MULTIOPT:
                m.data = (MultiOptionData*)new MultiOptionData();
                m.deleteData = delete_opt_data<MultiOptionData>;
                d2 = (MultiOptionData *)m.data;
                d2->selection = 0;
                d2->num = 0;
                d2->vals = NULL;
                d2->labels.clear();
                break;

            case ElementType::MENU_TEXTINPUT:
                m.data = (TextOptionData*)new TextOptionData();
                m.deleteData = delete_opt_data<TextOptionData>;
                d7 = (TextOptionData *)m.data;
                d7->active = 0;
                d7->position = 0;
                d7->selection = 0;
                d7->leftmost_position = 0;
                d7->visible_chars = 15;
                d7->text = "";
                break;

            case ElementType::MENU_TOGGLE:
                m.data = (ToggleOptionData*)new ToggleOptionData();
                m.deleteData = delete_opt_data<ToggleOptionData>;
                d3 = (ToggleOptionData *)m.data;
                d3->param = NULL;
                d3->labels[0] = "";
                d3->labels[1] = "";
                break;

            case ElementType::MENU_GAME:
                m.data = (GameOptionData *)new GameOptionData();
                m.deleteData = delete_opt_data<GameOptionData>;
                d4 = (GameOptionData *)m.data;
                d4->mode = MODE_INVALID;
                d4->args.num = 0;
                d4->args.ptrs = NULL;
                break;

            case ElementType::MENU_GAME_MULTIOPT:
                m.data = (GameMultiOptionData *)new GameMultiOptionData();
                m.deleteData = delete_opt_data<GameMultiOptionData>;
                d6 = (GameMultiOptionData *)m.data;
                d6->mode = MODE_INVALID;
                d6->num = 0;
                d6->selection = 0;
                d6->labels.clear();
                d6->args = NULL;
                break;

            case ElementType::MENU_METAGAME:
                m.data = (MetaGameOptionData*)new MetaGameOptionData();
                m.deleteData = delete_opt_data<MetaGameOptionData>;
                d5 = (MetaGameOptionData *)m.data;
                d5->mode = MODE_INVALID;
                d5->submode = MODE_INVALID;
                d5->num_args = 0;
                d5->num_subargs = 0;
                d5->args = NULL;
                d5->sub_args = NULL;
                break;

            default:
                m.data = NULL;
                m.deleteData = NULL;
                break;
        }

        return m;
    }

    void destroy_menu_option(MenuOption& m) {
        MultiOptionData *d2 = NULL;
        ToggleOptionData *d3 = NULL;
        GameOptionData *d4 = NULL;
        GameMultiOptionData *d6 = NULL;

        int i = 0;
        int j = 0;

        switch(m.type) {
            case ElementType::MENU_LABEL:
                break;

            case ElementType::MENU_ACTION:
                break;

            case ElementType::MENU_MULTIOPT:
                d2 = (MultiOptionData *)m.data;
                free(d2->vals);
                break;

            case ElementType::MENU_TOGGLE:
                d3 = (ToggleOptionData *)m.data;
                d3->labels[0] = "";
                d3->labels[1] = "";
                break;

            case ElementType::MENU_GAME:
                d4 = (GameOptionData *)m.data;
                for(i = 0; i < d4->args.num; i++)
                {
                    if(d4->args.ptrs[i])
                        free(d4->args.ptrs[i]);
                }

                free(d4->args.ptrs);
                break;

            case ElementType::MENU_GAME_MULTIOPT:
                d6 = (GameMultiOptionData *)m.data;
                for(i = 0; i < d6->num; i++)
                {
                    if(d6->args[i].ptrs)
                    {
                        for(j = 0; j < d6->args[i].num; j++)
                            free(d6->args[i].ptrs[j]);

                        free(d6->args[i].ptrs);
                    }

                    d6->labels[i] = "";
                }

                free(d6->args);
                break;

            case ElementType::MENU_METAGAME: // TODO
                // d5 = m.data;
                break;

            default:
                break;
        }

        if (m.deleteData != NULL) {
            m.deleteData(m.data);
        }
    }
}