#pragma once
#include <string>
#include <vector>
#include "Grid.h"
#include "gui/GUI.h"
#include "QRS0.h"
#define USRSEQ_MAX 2000
#define USRSEQ_EXPAND_MAX 4000
enum multiEditorButtonID
{
    return_button_id,
    play_button_id,

    gravity_button_id,
    lock_button_id,
    are_button_id,
    lineare_button_id,
    lineclear_button_id,
    das_button_id,
    width_button_id,
    game_type_button_id,
    invisible_button_id,
    brackets_button_id,
    infinite_floorkicks_button_id,
    lock_protect_button_id,

    piece_sequence_field_id
};

class MultiEditor
{
public:
    MultiEditor();
    ~MultiEditor();

    void handleInput();
    void handleGUIInteraction(GUIInteractable&, GUIEvent&);

    void updateGravity(BindableVariable *);
    void updateLock(BindableVariable *);
    void updateAre(BindableVariable *);
    void updateLineAre(BindableVariable *);
    void updateLineClear(BindableVariable *);
    void updateDas(BindableVariable *);
    void updateWidth(BindableVariable *);
    void updateGameType(BindableVariable *);
    void updateInvisible(BindableVariable *);
    void updateBrackets(BindableVariable *);
    void updateInfiniteFloorkicks(BindableVariable *);
    void updateLockProtect(BindableVariable *);
    void updateUsrseq(BindableVariable *);

    CoreState *origin;

    int game_type;
    int field_w;

    int field_x;
    int field_y;

    std::string usr_seq_str;
    int usr_sequence[USRSEQ_MAX];
    int usr_seq_expand[USRSEQ_EXPAND_MAX];
    int usr_seq_len;
    int usr_seq_expand_len;

    std::vector<Shiro::Grid *> usr_field_undo;
    std::vector<Shiro::Grid *> usr_field_redo;
    int usr_field_undo_len;
    int usr_field_redo_len;
    //bool field_edit_in_progress;

    Shiro::Grid *usr_field;
    //int palette_selection;
    //int field_selection;
    //int field_selection_vertex1_x;
    //int field_selection_vertex1_y;
    //int field_selection_vertex2_x;
    //int field_selection_vertex2_y;

    QRS_Timings *usr_timings;

    bool paused;
    //bool grid_lines_shown;
    bool brackets;
    bool invisible;

    int hist_index;
    int lock_protect;
    bool infinite_floorkicks;
    int piece_subset;

    long randomizer_seed;
};
//GUIScreen *loadEditorScreen(ScreenManager *mngr);