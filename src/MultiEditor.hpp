#ifndef _multieditor_hpp
#define _multieditor_hpp

#include "SGUIL/SGUIL.hpp"
#include "grid.h"
#include "qrs.h"

class MultiEditor
{
public:
    MultiEditor();
    ~MultiEditor();

    void handleInput();

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

    int game_type;
    int field_w;

    std::string usr_seq_str;
    int usr_sequence[2000];
    int usr_seq_expand[4000];
    int usr_seq_len;
    int usr_seq_expand_len;

    grid_t **usr_field_undo;
    grid_t **usr_field_redo;
    int usr_field_undo_len;
    int usr_field_redo_len;
    bool field_edit_in_progress;

    grid_t *usr_field;
    int palette_selection;
    int field_selection;
    int field_selection_vertex1_x;
    int field_selection_vertex1_y;
    int field_selection_vertex2_x;
    int field_selection_vertex2_y;

    qrs_timings *usr_timings;

    int paused;
    bool grid_lines_shown;
    bool brackets;
    bool invisible;

    int hist_index;
    int lock_protect;
    bool infinite_floorkicks;
    int piece_subset;

    long randomizer_seed;
};

#endif // _multieditor_hpp
