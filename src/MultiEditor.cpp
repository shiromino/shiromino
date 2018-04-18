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

void MultiEditor::handleInput()
{

}

void MultiEditor::handleGuiInteraction(GuiInteractable& interactable, GuiEvent& event)
{
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
    }
}

void MultiEditor::backupUsrField()
{
    /*
    if(usr_field_undo.empty())
    {
        gfx_createbutton(
            origin, "CLEAR UNDO", QRS_FIELD_X + (16 * 16) - 6, QRS_FIELD_Y + 23 * 16 + 8 - 6, 0, push_undo_clear_confirm, ufu_not_exists, NULL, 0xC0C0FFFF);
    }
    */

    usr_field_undo.push_back(gridcpy(usr_field, NULL));

    for(auto r : usr_field_redo)
    {
        grid_destroy(r);
    }

    usr_field_redo.clear();
}

void MultiEditor::usrFieldUndo()
{
    if(usr_field_undo.empty())
    {
        return;
    }

    usr_field_redo.push_back(gridcpy(usr_field, NULL));
    usr_field = usr_field_undo.back();
    usr_field_undo.pop_back();
}

void MultiEditor::usrFieldRedo()
{
    if(usr_field_redo.empty())
    {
        return;
    }

    usr_field_undo.push_back(gridcpy(usr_field, NULL));
    usr_field = usr_field_redo.back();
    usr_field_redo.pop_back();
}
/*
int undo_clear_confirm_yes(coreState *cs, void *data)
{
    qrsdata *q = (qrsdata *)cs->p1game->data;
    usr_field_undo_clear(cs, data);
    if(q->pracdata->field_edit_in_progress)
        q->pracdata->field_edit_in_progress = 0;

    cs->button_emergency_override = 0;
    cs->mouse_left_down = 0;
    return 0;
}

int undo_clear_confirm_no(coreState *cs, void *data)
{
    cs->button_emergency_override = 0;
    cs->mouse_left_down = 0;
    return 0;
}
*/
void MultiEditor::clearUsrFieldUndo()
{
    for(auto u : usr_field_undo)
    {
        grid_destroy(u);
    }

    for(auto r : usr_field_redo)
    {
        grid_destroy(r);
    }

    usr_field_undo.clear();
    usr_field_redo.clear();
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
