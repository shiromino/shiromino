#pragma once
#include <string>

struct CoreState;

namespace Shiro {
    enum class ElementType;

    // Temporary hack until menu_opt is fully C++'d.
    template<typename opt_data>
    void delete_opt_data(void* data) {
        delete (opt_data*)data;
    }
    struct MenuOption {
        MenuOption();
        ElementType type;
        int (*value_update_callback)(CoreState *cs);
        int render_update;
        std::string label;
        int x;
        int y;
        int value_x;
        int value_y;

        unsigned int label_text_flags;
        unsigned int value_text_flags;
        unsigned int label_text_rgba;
        unsigned int value_text_rgba;

        void *data;
        void (*deleteData)(void* data);
    };
    MenuOption create_menu_option(ElementType type, int (*value_update_callback) (CoreState *cs), std::string label);
    void destroy_menu_option(MenuOption& m);
}