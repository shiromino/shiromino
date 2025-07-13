#include "ShiroPhysoMino.h"
#include "SDL_rect.h"
#include "SDL_render.h"
#include <cstddef>
#include <vector>
#include "CoreState.h"
#include "SPM_Randomizer.h"
#include "SPM_Spec.h"
#include "gui/GUI.h"
#include "input/KeyFlags.h"
#include "replay.h"
#include "video/Render.h"
#include "video/Screen.h"

int TestSPM::init()
{
    if(player.randomizer)
    {
        player.randomizer->init(0);

        for(unsigned int i = 0u; i < spec->numPreviews; i++)
        {
            SPM_minoID t = player.randomizer->pull();
            player.previews.push_back(activateMino(t));
        }
    }

    initNextMino(player);

    player.counters.spawnDelayExpirePoint = player.timings.are;
    player.counters.lockDelayExpirePoint = player.timings.lockDelay;
    player.counters.lineClearExpirePoint = player.timings.lineClear;
    player.counters.dasIntervalExpirePoint = player.timings.dasInterval;

    return 0;
}

TestSPM::~TestSPM() {
    delete field;
    delete timer;
    delete rep;
}

int TestSPM::input()
{
    if(player.mino == NULL)
    {
        return 0;
    }

    if(player.playPhase == spm_player_control)
    {
        if(cs.pressed.a || cs.pressed.c)
        {
            spec->checkedRotate(field, *player.mino, spm_counter_clockwise);
        }

        if(cs.pressed.b)
        {
            spec->checkedRotate(field, *player.mino, spm_clockwise);
        }

        if(cs.pressed.left || cs.is_left_input_repeat(1 + player.timings.das))
        {
            spec->checkedShift(field, *player.mino, {-1, 0});
            /* moved_left = 1; */
        }

        if(cs.pressed.right || cs.is_right_input_repeat(1 + player.timings.das))
        {
            spec->checkedShift(field, *player.mino, {1, 0});
            /* moved_right = 1; */
        }

        /* if(moved_left && moved_right) { */
        /*     if(k->left < k->right) */
        /*         qrs_move(g, p, MOVE_LEFT); */
        /*     else if(k->right < k->left) */
        /*         qrs_move(g, p, MOVE_RIGHT); */
        /* } */

        if(!spec->isGrounded(field, *player.mino))
        {
            player.mino->physicState = spm_physic_falling;
        }

        if(cs.keys.down)
        {
            if(player.mino->physicState == spm_physic_falling)
            {
                spec->checkedFall(field, *player.mino, SPM_SUBUNIT_SCALE);

                if(spec->isGrounded(field, *player.mino))
                {
                    lockDelayExpired(player);
                }
            }
            else if(player.mino->physicState == spm_physic_grounded)
            {
                lockDelayExpired(player);
            }
        }

        if(cs.pressed.up)
        {
            spec->checkedFall(field, *player.mino, 20 * SPM_SUBUNIT_SCALE);
        }

        /*
        if(k->d == 1 && q->hold_enabled)
        {
            qrs_hold(g, p);
        }
        */
    }

    return 0;
}

int TestSPM::frame()
{
    if(player.playPhase == spm_player_control && player.mino->physicState == spm_physic_spawned)
    {
        player.mino->physicState = spm_physic_falling;
        spec->checkedFall(field, *player.mino, player.timings.gravity);
    }
    else
    {
        if(player.playPhase == spm_spawn_delay && !player.counters.usingLineSpawnDelay)
        {
            if(player.counters.spawnDelay == player.counters.spawnDelayExpirePoint)
            {
                player.counters.spawnDelay = 0;
                bool continueGameplay = spawnDelayExpired(player);
                if(!continueGameplay)
                {
                    gamePhase = spm_game_over;
                }

                player.playPhase = spm_player_control;
            }
            else
            {
                player.counters.spawnDelay++;
            }
        }

        if(player.playPhase == spm_player_control && player.mino->physicState == spm_physic_spawned)
        {
            player.mino->physicState = spm_physic_falling;
            spec->checkedFall(field, *player.mino, player.timings.gravity);
        }
        else
        {
            if(player.playPhase == spm_player_control && player.mino->physicState == spm_physic_falling)
            {
                int row = player.mino->position.y;
                spec->checkedFall(field, *player.mino, player.timings.gravity);
                if(player.mino->position.y > row)
                {
                    player.counters.lockDelay = 0;
                }
            }

            if(player.playPhase == spm_player_control && player.mino->physicState == spm_physic_grounded)
            {
                if(spec->isGrounded(field, *player.mino))
                {
                    if(player.counters.lockDelayExpirePoint == 0)
                    {
                        lockDelayExpired(player);
                    }
                    else
                    {
                        player.counters.lockDelay++;
                        if(player.counters.lockDelay == player.counters.lockDelayExpirePoint)
                        {
                            lockDelayExpired(player);
                        }
                    }
                }
                else
                {
                    player.mino->physicState = spm_physic_falling;
                }
            }

            if(player.playPhase == spm_line_clear)
            {
                if(player.counters.lineClear == player.counters.lineClearExpirePoint)
                {
                    player.counters.lineClear = 0;
                    lineClearExpired(player);

                    player.playPhase = spm_spawn_delay;
                }
                else
                {
                    player.counters.lineClear++;
                }
            }

            if(player.playPhase == spm_spawn_delay && player.counters.usingLineSpawnDelay)
            {
                if(player.counters.spawnDelay == player.counters.spawnDelayExpirePoint)
                {
                    player.counters.spawnDelay = 0;
                    bool continueGameplay = spawnDelayExpired(player);
                    if(!continueGameplay)
                    {
                        gamePhase = spm_game_over;
                    }

                    player.playPhase = spm_player_control;
                }
                else
                {
                    player.counters.spawnDelay++;
                }
            }

            if(player.playPhase == spm_player_control && player.mino->physicState == spm_physic_spawned)
            {
                player.mino->physicState = spm_physic_falling;
                spec->checkedFall(field, *player.mino, player.timings.gravity);
            }
        }
    }

    return 0;
}

int TestSPM::draw()
{
    int blockW = 16;
    int blockH = 16;

    SDL_Rect dest = {fieldPos.x, fieldPos.y, blockW, blockH};
    SDL_SetRenderDrawColor(cs.screen.renderer, 255, 255, 255, 180);

    SDL_Rect fieldRect = {fieldPos.x, fieldPos.y, blockW * static_cast<int>(field->getWidth()), blockH * spec->visualFieldH};
    GUIDrawBorder(fieldRect, 1, Shiro::GUI::RGBA_DEFAULT);

    for(std::size_t i = 0; i < field->getWidth(); i++)
    {
        for(std::size_t j = 0; j < std::size_t(spec->visualFieldH); j++)
        {
            std::size_t gridY = j + field->getHeight() - spec->visualFieldH;
            dest.x = fieldPos.x + (i * blockW);
            dest.y = fieldPos.y + (j * blockH);

            if (field->getCell(i, gridY) > 0) {
                Shiro::RenderFillRect(cs.screen, &dest);
            }
        }
    }

    SDL_SetRenderDrawColor(cs.screen.renderer, 220, 30, 30, 180);

    if(player.mino && player.playPhase == spm_player_control)
    {
        Shiro::Grid m = player.mino->currentRotationTable();
        for(int i = 0; i < int(m.getWidth()); i++)
        {
            for(int j = 0; j < int(m.getHeight()); j++)
            {
                int gridX = i + player.mino->position.x;
                int gridY = j + player.mino->position.y;

                if (gridY >= (int(field->getHeight()) - spec->visualFieldH) && m.getCell(i, j)) {
                    dest.x = fieldPos.x + (gridX * blockW);
                    dest.y = fieldPos.y - ((static_cast<int>(field->getHeight()) - spec->visualFieldH) * blockH) + (gridY * blockH);
                    dest.y += (blockH * player.mino->position.subY) / SPM_SUBUNIT_SCALE;
                    Shiro::RenderFillRect(cs.screen, &dest);
                }
            }
        }
    }

    SDL_SetRenderDrawColor(cs.screen.renderer, 30, 220, 30, 180);

    dest.w = 16;
    dest.h = 16;

    int n = 0;
    for(auto p : player.previews)
    {
        if(p)
        {
            Shiro::Grid m = p->currentRotationTable();
            for(std::size_t i = 0; i < m.getWidth(); i++)
            {
                for(std::size_t j = 0; j < m.getHeight(); j++)
                {
                    if (m.getCell(i, j)) {
                        dest.x = fieldPos.x + (3 * 16) + (n * 5 * 16) + (i * 16);
                        dest.y = fieldPos.y - 54 + (j * 16);
                        Shiro::RenderFillRect(cs.screen, &dest);
                    }
                }
            }
        }

        n++;
    }

    SDL_SetRenderDrawColor(cs.screen.renderer, 0, 0, 0, 255);

    return 0;
}

bool TestSPM::spawnDelayExpired(SPM_Player& p)
{
    p.counters.lockDelay = 0;

    bool continueGameplay = initNextMino(p);
    if(!continueGameplay)
    {
        return false;
    }

    SPM_orientation direction = spm_flat;

    if(cs.keys.a || cs.keys.c)
    {
        direction = spm_counter_clockwise;
    }

    if(cs.keys.b)
    {
        direction = spm_clockwise;
    }

    // if(cs.keys.d) { direction = spm_flip; }

    p.mino->orientation = direction;
    if(spec->checkCollision(field, *p.mino))
    {
        p.mino->orientation = spm_flat;
    }

    if(cs.keys.up)
    {
        spec->checkedFall(field, *p.mino, 20 * SPM_SUBUNIT_SCALE);
    }

    if(spec->checkCollision(field, *p.mino))
    {
        spec->imprintMino(field, *p.mino);
        gamePhase = spm_game_over;

        return false;
    }

    return true;
}

bool TestSPM::lockDelayExpired(SPM_Player& p)
{
    spec->imprintMino(field, *p.mino);
    p.mino->physicState = spm_physic_locked;

    int n = spec->checkAndClearLines(field, static_cast<int>(field->getHeight()));

    if(n)
    {
        p.playPhase = spm_line_clear;
    }
    else
    {
        p.playPhase = spm_spawn_delay;
        p.counters.spawnDelayExpirePoint = p.timings.are;
        p.counters.usingLineSpawnDelay = false;
    }

    return true;
}

bool TestSPM::lineClearExpired(SPM_Player& p)
{
    p.playPhase = spm_spawn_delay;
    p.counters.spawnDelayExpirePoint = p.timings.lineAre;
    p.counters.usingLineSpawnDelay = true;

    p.counters.lineClear = 0;
    spec->dropField(field);

    // sfx_play(&cs->assets->dropfield);

    return true;
}

bool TestSPM::initNextMino(SPM_Player& p)
{
    SPM_minoID t = MINO_ID_INVALID;

    if(!p.minoSequence.empty())
    {
        p.minoSeqIndex++;

        if(p.minoSeqIndex < p.minoSequence.size())
        {
            t = p.minoSequence[p.minoSeqIndex];
        }
    }
    else
    {
        t = p.randomizer->pull();
    }

    if(p.mino)
    {
        delete p.mino;
    }

    if(!p.previews.empty())
    {
        p.mino = p.previews[0];

        for(size_t i = 0; i < p.previews.size() - 1; i++)
        {
            p.previews[i] = p.previews[i + 1];
        }

        p.previews.back() = activateMino(t);
    }
    else
    {
        p.mino = activateMino(t);
    }

    if(!p.previews.empty() && p.previews[0])
    {/*
        t = q->previews[0]->qrs_id;

        if(t != PIECE_ID_INVALID)
        {
            int ts = t;
            if(ts >= 18)
                ts -= 18;
            struct sfx *sfx = &cs->assets->piece0 + (ts % 7);
            sfx_play(sfx);
        }*/
    }

    if(p.mino == NULL)
    {
        // stop the game if we can't deal a new piece
        return false;
    }

    p.mino->physicState = spm_physic_spawned;

    return true;
}

ActivatedPolyomino *TestSPM::activateMino(SPM_minoID ID)
{
    if(ID >= spec->polyominoes.size())
    {
        return NULL;
    }

    ActivatedPolyomino *ap = new ActivatedPolyomino{spec->polyominoes[ID], ID, spec->spawnPositions[ID]};
    return ap;
}
