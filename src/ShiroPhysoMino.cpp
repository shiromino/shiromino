#include "ShiroPhysoMino.hpp"

#include <memory>
#include <vector>
#include "SGUIL/SGUIL.hpp"

#include "SPM_Spec.hpp"

using namespace std;

ShiroPhysoMino::ShiroPhysoMino(coreState& cs, SPM_Spec *spec)
    : Game(cs)
{
    this->spec = spec;
    field = grid_create(spec->fieldW, spec->fieldH);
    fieldPos = {48, 60};
    timer = nz_timer_create(60);
    rep = NULL;
    playback = false;
    playbackIndex = 0;
    gamePhase = spm_gameplay;
}

ShiroPhysoMino::~ShiroPhysoMino()
{
    grid_destroy(field);
    nz_timer_destroy(timer);
}

int ShiroPhysoMino::init()
{
    if(player.randomizer)
    {
        player.randomizer->init(0);

        for(int i = 0; i < spec->numPreviews; i++)
        {
            SPM_minoID t = player.randomizer->pull();
            player.previews.insert(player.previews.begin(), spec->activatePolyomino(t));
        }
    }

    initNextMino(player);

    player.counters.spawnDelayExpirePoint = player.timings.are;
    player.counters.lockDelayExpirePoint = player.timings.lockDelay;
    player.counters.lineClearExpirePoint = player.timings.lineClear;
    player.counters.dasIntervalExpirePoint = player.timings.dasInterval;

    return 0;
}

int ShiroPhysoMino::quit()
{
    return 0;
}

int ShiroPhysoMino::input()
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

        if(cs.pressed.left || is_left_input_repeat(&cs, 1 + player.timings.das))
        {
            spec->checkedShift(field, *player.mino, {-1, 0});
            /* moved_left = 1; */
        }

        if(cs.pressed.right || is_right_input_repeat(&cs, 1 + player.timings.das))
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

int ShiroPhysoMino::frame()
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

int ShiroPhysoMino::draw()
{
    SDL_Rect dest = {fieldPos.x, fieldPos.y, 16, 16};
    SDL_SetRenderDrawColor(cs.screen.renderer, 255, 255, 255, 180);

    SDL_Rect fieldRect = {fieldPos.x, fieldPos.y, 16 * field->w, 16 * spec->visualFieldH};
    Gui_DrawBorder(fieldRect, 1, GUI_RGBA_DEFAULT);

    for(int i = 0; i < field->w; i++)
    {
        for(int j = 0; j < spec->visualFieldH; j++)
        {
            int gridY = j + field->h - spec->visualFieldH;
            dest.x = fieldPos.x + (i * 16);
            dest.y = fieldPos.y + (j * 16);

            if(gridgetcell(field, i, gridY) > 0)
            {
                SDL_RenderFillRect(cs.screen.renderer, &dest);
            }
        }
    }

    SDL_SetRenderDrawColor(cs.screen.renderer, 220, 30, 30, 180);

    if(player.mino && player.playPhase == spm_player_control)
    {
        grid_t *m = player.mino->currentRotationTable();
        for(int i = 0; i < m->w; i++)
        {
            for(int j = 0; j < m->h; j++)
            {
                int gridX = i + player.mino->position.x;
                int gridY = j + player.mino->position.y;

                if(gridY >= (field->h - spec->visualFieldH) && gridgetcell(m, i, j))
                {
                    dest.x = fieldPos.x + (gridX * 16);
                    dest.y = fieldPos.y - ((field->h - spec->visualFieldH) * 16) + (gridY * 16);
                    SDL_RenderFillRect(cs.screen.renderer, &dest);
                }
            }
        }
    }

    SDL_SetRenderDrawColor(cs.screen.renderer, 30, 220, 30, 180);

    int n = 0;
    for(auto p : player.previews)
    {
        if(p)
        {
            grid_t *m = p->currentRotationTable();
            for(int i = 0; i < m->w; i++)
            {
                for(int j = 0; j < m->h; j++)
                {
                    if(gridgetcell(m, i, j))
                    {
                        dest.x = fieldPos.x + (3 * 16) + (n * 5 * 16) + (i * 16);
                        dest.y = fieldPos.y - 54 + (j * 16);
                        SDL_RenderFillRect(cs.screen.renderer, &dest);
                    }
                }
            }
        }

        n++;
    }

    SDL_SetRenderDrawColor(cs.screen.renderer, 0, 0, 0, 255);

    return 0;
}

bool ShiroPhysoMino::spawnDelayExpired(SPM_Player& p)
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

bool ShiroPhysoMino::lockDelayExpired(SPM_Player& p)
{
    spec->imprintMino(field, *p.mino);
    p.mino->physicState = spm_physic_locked;

    int n = spec->checkAndClearLines(field);

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

bool ShiroPhysoMino::lineClearExpired(SPM_Player& p)
{
    p.playPhase = spm_spawn_delay;
    p.counters.spawnDelayExpirePoint = p.timings.lineAre;
    p.counters.usingLineSpawnDelay = true;

    p.counters.lineClear = 0;
    spec->dropField(field);

    // sfx_play(&cs->assets->dropfield);

    return true;
}

bool ShiroPhysoMino::initNextMino(SPM_Player& p)
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

        for(int i = 0; i < p.previews.size() - 1; i++)
        {
            p.previews[i] = p.previews[i + 1];
        }

        p.previews.back() = spec->activatePolyomino(t);
    }
    else
    {
        p.mino = spec->activatePolyomino(t);
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
