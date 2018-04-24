#include "MultiEditor.hpp"

#include <string>
#include "SGUIL/SGUIL.hpp"

#include "qrs.h"

#define SUBSET_ALL 0
#define SUBSET_TETS 1
#define SUBSET_PENTS 2

#define USRSEQ_ELEM_OOB -1
#define USRSEQ_RPTCOUNT_MAX 1023
#define SEQUENCE_REPEAT_START 2048
#define SEQUENCE_REPEAT_END 4096
#define SEQUENCE_REPEAT_INF 8192

using namespace std;

MultiEditor::MultiEditor()
{

}

MultiEditor::~MultiEditor()
{

}
/*
GuiScreen *makeEditorScreen(GuiScreenManager *mngr)
{
    GuiScreen *screen = new GuiScreen{
        "Multi-Editor",
        ,
    };

    GuiEnumerationButton (BindableEnumeration, displaystring, displaystring font, )
}
*/
void MultiEditor::handleInput()
{/*
    if(origin->logical_mouse_x < 0 || origin->logical_mouse_y < 0)
    {
        return;
    }

    int i = 0;
    int j = 0;
    int c = 0;

    int lesser_x = 0;
    int greater_x = 0;
    int lesser_y = 0;
    int greater_y = 0;

    int edit_action_occurred = 0;

    int cell_x = ((origin->logical_mouse_x - field_x) / 16) - 1;
    int cell_y = ((origin->logical_mouse_y - field_y) / 16) - 2;
    int palette_cell_x = (origin->logical_mouse_x - FIELD_EDITOR_PALETTE_X) / 16;
    int palette_cell_y = (origin->logical_mouse_y - FIELD_EDITOR_PALETTE_Y) / 16;

    if(origin->select_all && !origin->text_editing)
    {
        field_selection = 1;
        field_selection_vertex1_x = 0;
        field_selection_vertex1_y = 0;
        field_selection_vertex2_x = 11;
        field_selection_vertex2_y = 19;
    }

    if(origin->undo && !field_edit_in_progress)
        usr_field_undo(origin, d);

    if(origin->redo && !field_edit_in_progress)
        usr_field_redo(origin, d);

    if(SDL_GetModState() & KMOD_SHIFT && origin->mouse_left_down)
    {
        if(origin->mouse_left_down == BUTTON_PRESSED_THIS_FRAME)
        {
            field_selection = 1;
            field_selection_vertex1_x = cell_x;
            field_selection_vertex1_y = cell_y;
        }

        field_selection_vertex2_x = cell_x;
        field_selection_vertex2_y = cell_y;
    }
    else
    {
        if(origin->mouse_left_down)
        {
            if(palette_cell_x == 0)
            {
                switch(palette_cell_y)
                {
                    case 0:
                        palette_selection = QRS_X + 1;
                        break;
                    case 1:
                        palette_selection = QRS_N + 1;
                        break;
                    case 2:
                        palette_selection = QRS_G + 1;
                        break;
                    case 3:
                        palette_selection = QRS_U + 1;
                        break;
                    case 4:
                        palette_selection = QRS_T + 1;
                        break;
                    case 5:
                        palette_selection = QRS_Fa + 1;
                        break;
                    case 6:
                        break;
                    case 7:
                        palette_selection = QRS_I4 + 1;
                        break;
                    case 8:
                        palette_selection = QRS_T4 + 1;
                        break;
                    case 9:
                        palette_selection = QRS_J4 + 1;
                        break;
                    case 10:
                        palette_selection = QRS_L4 + 1;
                        break;
                    case 11:
                        palette_selection = QRS_O + 1;
                        break;
                    case 12:
                        palette_selection = QRS_S4 + 1;
                        break;
                    case 13:
                        palette_selection = QRS_Z4 + 1;
                        break;
                    case 14:
                        palette_selection = QRS_PIECE_GARBAGE;
                        break;
                    case 15:
                        palette_selection = QRS_PIECE_BRACKETS;
                        break;
                    case 16:
                        palette_selection = QRS_PIECE_GEM;
                        break;
                    default:
                        break;
                }
            }
            else if(field_selection)
            {
                if(origin->mouse_left_down == BUTTON_PRESSED_THIS_FRAME)
                {
                    field_selection = 0;
                    origin->mouse_left_down = 0;
                }
            }
            else if(origin->mouse_left_down && cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20)
            {
                if(gridgetcell(usr_field, cell_x, cell_y + 2) != QRS_FIELD_W_LIMITER)
                {
                    if(palette_selection != QRS_PIECE_GEM)
                    {
                        if(!field_edit_in_progress)
                            usr_field_bkp(origin, d);
                        field_edit_in_progress = 1;
                        edit_action_occurred = 1;
                        gridsetcell(usr_field, cell_x, cell_y + 2, palette_selection);
                    }
                    else if(gridgetcell(usr_field, cell_x, cell_y + 2) > 0)
                    {
                        if(!field_edit_in_progress)
                            usr_field_bkp(origin, d);
                        field_edit_in_progress = 1;
                        edit_action_occurred = 1;
                        gridsetcell(usr_field, cell_x, cell_y + 2, gridgetcell(usr_field, cell_x, cell_y + 2) | QRS_PIECE_GEM);
                    }
                }
            }
        }
        else if(origin->mouse_right_down)
        {
            if(field_selection)
            {
                if(origin->mouse_right_down == BUTTON_PRESSED_THIS_FRAME)
                {
                    field_selection = 0;
                    origin->mouse_right_down = 0;
                }
            }
            else if(origin->mouse_right_down && cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20)
            {
                if(gridgetcell(usr_field, cell_x, cell_y + 2) != QRS_FIELD_W_LIMITER)
                {
                    if(!field_edit_in_progress)
                        usr_field_bkp(origin, d);
                    field_edit_in_progress = 1;
                    edit_action_occurred = 1;
                    gridsetcell(usr_field, cell_x, cell_y + 2, 0);
                }
            }
        }

        if(origin->delete_das == 2 || origin->backspace_das == 2)
        {
            if(field_selection && !origin->text_editing)
            {
                if(field_selection_vertex1_x <= field_selection_vertex2_x)
                {
                    lesser_x = field_selection_vertex1_x;
                    greater_x = field_selection_vertex2_x;
                }
                else
                {
                    lesser_x = field_selection_vertex2_x;
                    greater_x = field_selection_vertex1_x;
                }

                if(field_selection_vertex1_y <= field_selection_vertex2_y)
                {
                    lesser_y = field_selection_vertex1_y;
                    greater_y = field_selection_vertex2_y;
                }
                else
                {
                    lesser_y = field_selection_vertex2_y;
                    greater_y = field_selection_vertex1_y;
                }

                for(i = lesser_x; i <= greater_x; i++)
                {
                    for(j = lesser_y; j <= greater_y; j++)
                    {
                        if(i >= 0 && i < 12 && j >= 0 && j < 20)
                        {
                            if(gridgetcell(usr_field, i, j + 2) != QRS_FIELD_W_LIMITER)
                            {
                                if(!field_edit_in_progress)
                                    usr_field_bkp(origin, d);
                                field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                gridsetcell(usr_field, i, j + 2, 0);
                            }
                        }
                    }
                }

                field_selection = 0;
            }
        }
    }

    c = 0;
    if(origin->zero_pressed)
    {
        c = palette_selection;
        if(field_selection)
            origin->zero_pressed = 0;
    }
    if(origin->one_pressed)
    {
        c = 19;
        if(field_selection)
            origin->one_pressed = 0;
    }
    if(origin->two_pressed)
    {
        c = 20;
        if(field_selection)
            origin->two_pressed = 0;
    }
    if(origin->three_pressed)
    {
        c = 21;
        if(field_selection)
            origin->three_pressed = 0;
    }
    if(origin->four_pressed)
    {
        c = 22;
        if(field_selection)
            origin->four_pressed = 0;
    }
    if(origin->five_pressed)
    {
        c = 23;
        if(field_selection)
            origin->five_pressed = 0;
    }
    if(origin->six_pressed)
    {
        c = 24;
        if(field_selection)
            origin->six_pressed = 0;
    }
    if(origin->seven_pressed)
    {
        c = 25;
        if(field_selection)
            origin->seven_pressed = 0;
    }
    if(origin->nine_pressed)
    {
        c = QRS_PIECE_BRACKETS;
        if(field_selection)
            origin->nine_pressed = 0;
    }

    if(c && field_selection)
    {
        if(field_selection_vertex1_x <= field_selection_vertex2_x)
        {
            lesser_x = field_selection_vertex1_x;
            greater_x = field_selection_vertex2_x;
        }
        else
        {
            lesser_x = field_selection_vertex2_x;
            greater_x = field_selection_vertex1_x;
        }

        if(field_selection_vertex1_y <= field_selection_vertex2_y)
        {
            lesser_y = field_selection_vertex1_y;
            greater_y = field_selection_vertex2_y;
        }
        else
        {
            lesser_y = field_selection_vertex2_y;
            greater_y = field_selection_vertex1_y;
        }

        for(i = lesser_x; i <= greater_x; i++)
        {
            for(j = lesser_y; j <= greater_y; j++)
            {
                if(i >= 0 && i < 12 && j >= 0 && j < 20)
                {
                    if(gridgetcell(usr_field, i, j + 2) != QRS_FIELD_W_LIMITER && c != QRS_PIECE_GEM)
                    {
                        if(SDL_GetModState() & KMOD_SHIFT)
                        {
                            if(IS_STACK(gridgetcell(usr_field, i, j + 2)))
                            {
                                if(!field_edit_in_progress)
                                    usr_field_bkp(origin, d);
                                field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                gridsetcell(usr_field, i, j + 2, c);
                            }
                        }
                        else
                        {
                            if(!field_edit_in_progress)
                                usr_field_bkp(origin, d);
                            field_edit_in_progress = 1;
                            edit_action_occurred = 1;
                            gridsetcell(usr_field, i, j + 2, c);
                        }
                    }
                    else if(gridgetcell(usr_field, i, j + 2) > 0 && c == QRS_PIECE_GEM)
                    {
                        if(SDL_GetModState() & KMOD_SHIFT)
                        {
                            if(IS_STACK(gridgetcell(usr_field, i, j + 2)))
                            {
                                if(!field_edit_in_progress)
                                    usr_field_bkp(origin, d);
                                field_edit_in_progress = 1;
                                edit_action_occurred = 1;
                                gridsetcell(usr_field, i, j + 2, gridgetcell(usr_field, i, j + 2) | c);
                            }
                        }
                        else
                        {
                            if(!field_edit_in_progress)
                                usr_field_bkp(origin, d);
                            field_edit_in_progress = 1;
                            edit_action_occurred = 1;
                            gridsetcell(usr_field, i, j + 2, gridgetcell(usr_field, i, j + 2) | c);
                        }
                    }
                }
            }
        }

        field_selection = 0;
    }
    else if(c)
    {
        if(cell_x >= 0 && cell_x < 12 && cell_y >= 0 && cell_y < 20)
        {
            if(gridgetcell(usr_field, cell_x, cell_y + 2) != QRS_FIELD_W_LIMITER && c != QRS_PIECE_GEM)
            {
                if(SDL_GetModState() & KMOD_SHIFT)
                {
                    if(IS_STACK(gridgetcell(usr_field, cell_x, cell_y + 2)))
                    {
                        if(!field_edit_in_progress)
                            usr_field_bkp(origin, d);
                        field_edit_in_progress = 1;
                        edit_action_occurred = 1;
                        gridsetcell(usr_field, cell_x, cell_y + 2, c);
                    }
                }
                else
                {
                    if(!field_edit_in_progress)
                        usr_field_bkp(origin, d);
                    field_edit_in_progress = 1;
                    edit_action_occurred = 1;
                    gridsetcell(usr_field, cell_x, cell_y + 2, c);
                }
            }
            else if(gridgetcell(usr_field, cell_x, cell_y + 2) > 0 && c == QRS_PIECE_GEM)
            {
                if(SDL_GetModState() & KMOD_SHIFT)
                {
                    if(IS_STACK(gridgetcell(usr_field, cell_x, cell_y + 2)))
                    {
                        if(!field_edit_in_progress)
                            usr_field_bkp(origin, d);
                        field_edit_in_progress = 1;
                        edit_action_occurred = 1;
                        gridsetcell(usr_field, cell_x, cell_y + 2, gridgetcell(usr_field, cell_x, cell_y + 2) | c);
                    }
                }
                else
                {
                    if(!field_edit_in_progress)
                        usr_field_bkp(origin, d);
                    field_edit_in_progress = 1;
                    edit_action_occurred = 1;
                    gridsetcell(usr_field, cell_x, cell_y + 2, gridgetcell(usr_field, cell_x, cell_y + 2) | c);
                }
            }
        }
    }

    if(!edit_action_occurred)
    {
        field_edit_in_progress = 0;
    }*/
}

void MultiEditor::handleGuiInteraction(GuiInteractable& interactable, GuiEvent& event)
{/*
    if(event.type != mouse_clicked)
    {
        return;
    }

    switch(interactable.ID)
    {
        case return_button_id:
            origin->loadParentGui();
            break;

        default:
            break;
    }*/
}

void MultiEditor::updateUsrseq(BindableVariable *seqvar)
{
    usr_seq_str = seqvar->get();

    int num = 0;

    int i = 0;
    int j = 0;
    int k = 0;
    int t = 0;
    unsigned char c;

    int rpt_start = 0;
    int rpt_end = 0;
    int rpt = 0;
    int rpt_count = 0;
    int pre_rpt_count = 0;

    char rpt_count_strbuf[5];
    char name_strbuf[3] = {0, 0, 0};
    string name_str;

    for(i = 0; i < usr_seq_str.length(); i++)
    {
        c = usr_seq_str[i];
        if((c < 'A' || c > 'Z') && !(c == '*' || c == '(' || c == ')'))
        {
            if(rpt_count)
            {
                k = 0;
                while(k < 4 && i < usr_seq_str.length() && usr_seq_str[i] >= '0' && usr_seq_str[i] <= '9')
                {
                    rpt_count_strbuf[k] = usr_seq_str[i];
                    rpt_count_strbuf[k + 1] = '\0';
                    i++;
                    k++;
                }

                i--;

                num++;

                if(k)
                {
                    usr_sequence[num - 1] = (strtol(rpt_count_strbuf, NULL, 10) & 1023);
                }
                else
                {
                    usr_sequence[num - 1] = 1;
                }

                rpt_count = 0;
                continue;
            }
            else
                continue;
        }

        if(rpt_count)
        {
            num++;

            if(i < usr_seq_str.length() - 1)
            {
                if(c == 'I' && usr_seq_str[i + 1] == 'N' && usr_seq_str[i + 2] == 'F')
                {
                    usr_sequence[num - 1] = SEQUENCE_REPEAT_INF;
                    break;
                }
                else
                {
                    usr_sequence[num - 1] = 1;
                }
            }
            else
            {
                usr_sequence[num - 1] = 1;
            }

            rpt_count = 0;
            continue;
        }

        if(c == '*')
        {
            if(rpt)
            {
                if(!rpt_start)
                {
                    rpt_count = 1;
                    pre_rpt_count = 0;
                    rpt = 0;
                    if(!(usr_sequence[num - 1] & SEQUENCE_REPEAT_END))
                        usr_sequence[num - 1] |= SEQUENCE_REPEAT_END;
                }

                continue;
            }
            else
            {
                if(num > 1)
                {
                    if(!(usr_sequence[num - 2] & SEQUENCE_REPEAT_END))
                    {
                        rpt_count = 1;
                        pre_rpt_count = 0;
                        if(!(usr_sequence[num - 1] & SEQUENCE_REPEAT_END))
                        {
                            usr_sequence[num - 1] |= SEQUENCE_REPEAT_END;
                            usr_sequence[num - 1] |= SEQUENCE_REPEAT_START;
                        }
                        continue;
                    }
                    else
                        continue;
                }
                else if(num)
                {
                    usr_sequence[0] |= (SEQUENCE_REPEAT_START | SEQUENCE_REPEAT_END);
                    rpt_count = 1;
                    pre_rpt_count = 0;
                    continue;
                }
                else
                    continue;
            }
        }

        if(c == '(')
        {
            if(rpt)
                continue;

            rpt_start = 1;
            rpt = 1;
            continue;
        }

        if(c == ')')
        {
            if(!rpt)
                continue;

            if(num > 0)
            {
                usr_sequence[num - 1] |= SEQUENCE_REPEAT_END;
                pre_rpt_count = 1;
            }
            continue;
        }

        if(pre_rpt_count)
        {
            num++;
            usr_sequence[num - 1] = 1;
            pre_rpt_count = 0;
            i--;
            continue;
        }

        name_strbuf[0] = usr_seq_str[i];

        if(usr_seq_str[i + 1] == '4')
        {
            name_strbuf[1] = '4';
            name_strbuf[2] = '\0';

            name_str = {name_strbuf};

            for(j = 0; j < 25; j++)
            {
                if(name_str == get_qrspiece_name(j))
                {
                    t = j;
                    if(piece_subset != SUBSET_PENTS)
                    {
                        goto found;
                    }
                }
            }
        }

        name_strbuf[1] = '\0';
        name_str = {name_strbuf};

        for(j = 0; j < 25; j++)
        {
            if(name_str == get_qrspiece_name(j))
            {
                t = j;
                if(piece_subset == SUBSET_TETS)
                {
                    switch(t)
                    {
                        case QRS_I:
                            t += 18;
                            break;
                        case QRS_T:
                            t += 10;
                            break;
                        case QRS_J:
                        case QRS_L:
                        case QRS_S:
                        case QRS_Z:
                            t += 19;
                            break;
                        default:
                            break;
                    }

                    if(t >= 18)
                        goto found;
                }
                else if(piece_subset == SUBSET_PENTS)
                {
                    switch(t)
                    {
                        case QRS_I4:
                            t -= 18;
                            break;
                        case QRS_T4:
                            t -= 10;
                            break;
                        case QRS_J4:
                        case QRS_L4:
                        case QRS_S4:
                        case QRS_Z4:
                            t -= 19;
                            break;
                        default:
                            break;
                    }

                    if(t < 18)
                        goto found;
                }
                else
                {
                    goto found;
                }
            }
        }

        if(usr_seq_str[i + 1] == 'a')
        {
            name_strbuf[1] = 'a';
            name_strbuf[2] = '\0';

            name_str = {name_strbuf};

            for(j = 0; j < 25; j++)
            {
                if(name_str == get_qrspiece_name(j))
                {
                    t = j;
                    if(piece_subset == SUBSET_TETS)
                    {
                        switch(t)
                        {
                            case QRS_I:
                                t += 18;
                                break;
                            case QRS_T:
                                t += 10;
                                break;
                            case QRS_J:
                            case QRS_L:
                            case QRS_S:
                            case QRS_Z:
                                t += 19;
                                break;
                            default:
                                break;
                        }

                        if(t >= 18)
                            goto found;
                    }
                    else if(piece_subset == SUBSET_PENTS)
                    {
                        switch(t)
                        {
                            case QRS_I4:
                                t -= 18;
                                break;
                            case QRS_T4:
                                t -= 10;
                                break;
                            case QRS_J4:
                            case QRS_L4:
                            case QRS_S4:
                            case QRS_Z4:
                                t -= 19;
                                break;
                            default:
                                break;
                        }

                        if(t < 18)
                            goto found;
                    }
                    else
                    {
                        goto found;
                    }
                }
            }
        }
        else if(usr_seq_str[i + 1] == 'b')
        {
            name_strbuf[1] = 'b';
            name_strbuf[2] = '\0';

            name_str = {name_strbuf};

            for(j = 0; j < 25; j++)
            {
                if(name_str == get_qrspiece_name(j))
                {
                    t = j;
                    if(piece_subset == SUBSET_TETS)
                    {
                        switch(t)
                        {
                            case QRS_I:
                                t += 18;
                                break;
                            case QRS_T:
                                t += 10;
                                break;
                            case QRS_J:
                            case QRS_L:
                            case QRS_S:
                            case QRS_Z:
                                t += 19;
                                break;
                            default:
                                break;
                        }

                        if(t >= 18)
                            goto found;
                    }
                    else if(piece_subset == SUBSET_PENTS)
                    {
                        switch(t)
                        {
                            case QRS_I4:
                                t -= 18;
                                break;
                            case QRS_T4:
                                t -= 10;
                                break;
                            case QRS_J4:
                            case QRS_L4:
                            case QRS_S4:
                            case QRS_Z4:
                                t -= 19;
                                break;
                            default:
                                break;
                        }

                        if(t < 18)
                            goto found;
                    }
                    else
                    {
                        goto found;
                    }
                }
            }
        }

        continue;

    found:
        num++;
        usr_sequence[num - 1] = t;

        if(rpt_start)
        {
            usr_sequence[num - 1] |= SEQUENCE_REPEAT_START;
            rpt_start = 0;
        }
        else if(rpt_end)
        {
            usr_sequence[num - 1] |= SEQUENCE_REPEAT_END;
            rpt_end = 0;
        }

        if(num >= USRSEQ_MAX)
        {
            break;
        }
    }

    usr_seq_len = num;

    int expand_count = 0;

    rpt_start = 0;
    rpt_end = 0;
    int rpt_len = 0;
    rpt_count = 0;
    rpt = 0;
    int inf_rpt_len = 0;
    int inf_start = 0;

    int inf = 0;

    i = 0;
    j = 0;
    k = 0;
    int val = 0;

    for(i = 0;; i++)
    {
        if(i >= usr_seq_len || expand_count >= USRSEQ_EXPAND_MAX)
        {
            break;
        }

        val = usr_sequence[i];
        if(rpt)
        {
            /* rpt = 1 implies we encountered the beginning of a grouped subsequence, where groups are assumed
            to have a rep count specified at the end */
            if(val & SEQUENCE_REPEAT_END || i == usr_seq_len - 1)
            {
                rpt = 0;
                rpt_end = i;
                rpt_len = rpt_end - rpt_start + 1; // length of grouped subsequence
                i++;                               // next element of the sequence is either a repetition count or beyond the end of the sequence

                if(i != usr_seq_len && usr_sequence[i] == SEQUENCE_REPEAT_INF)
                {
                    // ^^ bound check must come first to prevent usr_sequence[i] from creating UB
                    inf = 1;
                    inf_rpt_len = rpt_len;
                    inf_start = rpt_start;
                    expand_count += rpt_len;
                    for(k = 0; k < rpt_len; k++)
                    {
                        /* usr_sequence[blah] & 0b11111 zeroes other flags on the piece;
                        first repeated element gets repeat_inf flag;
                        expanded sequence is cut off after the infinitely repeating group and this expansion loop ends */

                        usr_seq_expand[expand_count - rpt_len + k] = (usr_sequence[rpt_start + k] & 0b11111) | (!k ? SEQUENCE_REPEAT_INF : 0);
                    }

                    break;
                }
                else
                {
                    // dealing with finite repetition or open-ended grouped subsequence

                    if(i == usr_seq_len)
                    {
                        rpt_count = 1;
                    }
                    // ^^ force open-ended group to 1 repetition (same as if there was no grouping)
                    else
                    {
                        if(usr_sequence[i] > USRSEQ_RPTCOUNT_MAX) // last-minute bounding of repetition count to something reasonable
                            rpt_count = USRSEQ_RPTCOUNT_MAX;
                        else
                            rpt_count = usr_sequence[i];
                    }

                    for(k = 0; k < rpt_count; k++)
                    { // expand entire repetition to sequence of pieces with no special flags
                        for(j = 0; j < rpt_len; j++)
                        {
                            usr_seq_expand[expand_count + k * rpt_len + j] = usr_sequence[rpt_start + j] & 0b11111;
                        }
                    }

                    expand_count += rpt_count * rpt_len;
                    rpt = 0; // we are done with this grouped subsequence
                }
            }
        }
        else
        {
            if(val & SEQUENCE_REPEAT_START)
            {
                rpt_start = i;
                rpt = 1;
                if(val & SEQUENCE_REPEAT_END || i == usr_seq_len - 1)
                {
                    i--;      // hacky way for the loop to go into the if(rpt) branch and handle these edge cases
                    continue; // the i-- ensures the loop reads the additional flags on this element/doesn't go OOB
                }
            }
            else
            { // no expansion to be done
                expand_count++;
                usr_seq_expand[expand_count - 1] = usr_sequence[i] & 0b11111;
            }
        }
    }

    usr_seq_expand_len = expand_count;
}
