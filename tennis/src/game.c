#include "game.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "save.h"

GameState g_tennis;

static const char* const s_point_names[5] = {"0", "15", "30", "40", "AD"};
static const char* const s_surface_names[3] = {"GRASS", "CLAY", "HARD"};
static const char* const s_ai_names[3] = {"L. VANCE", "K. SATO", "M. THORNE"};

static s16 get_court_half_width(s16 y) {
    if (y < BASELINE_FAR_Y) y = BASELINE_FAR_Y;
    if (y > BASELINE_NEAR_Y) y = BASELINE_NEAR_Y;
    // Linear interpolation: 48 at far baseline, 84 at near baseline
    return 48 + ((y - BASELINE_FAR_Y) * 36) / (BASELINE_NEAR_Y - BASELINE_FAR_Y);
}

static bool is_point_in_court(s16 x, s16 y) {
    if (y < BASELINE_FAR_Y || y > BASELINE_NEAR_Y) return false;
    s16 hw = get_court_half_width(y);
    return (x >= 120 - hw && x <= 120 + hw);
}

static bool is_serve_in_box(s16 x, s16 y, bool receiver_is_player, bool deuce_court) {
    s16 hw = get_court_half_width(y);
    if (receiver_is_player) {
        // Ball landing in near service box (Y between NET_Y and SERVICE_NEAR_Y)
        if (y < NET_Y || y > SERVICE_NEAR_Y) return false;
        if (deuce_court) {
            // Receiver right box (X between 120 and 120 + hw)
            return (x >= 120 && x <= 120 + hw);
        } else {
            // Receiver left box (X between 120 - hw and 120)
            return (x >= 120 - hw && x <= 120);
        }
    } else {
        // Ball landing in far service box (Y between SERVICE_FAR_Y and NET_Y)
        if (y < SERVICE_FAR_Y || y > NET_Y) return false;
        if (deuce_court) {
            // Far deuce box is from 120 - hw to 120
            return (x >= 120 - hw && x <= 120);
        } else {
            // Far ad box is from 120 to 120 + hw
            return (x >= 120 && x <= 120 + hw);
        }
    }
}

static void trigger_chalk_puff(s16 x, s16 y) {
    g_tennis.chalk.x = x;
    g_tennis.chalk.y = y;
    g_tennis.chalk.timer = 15;
    g_tennis.chalk.active = true;
}

static void set_announcement(const char* str, u8 frames) {
    g_tennis.announcement_str = str;
    g_tennis.announcement_timer = frames;
}

static void setup_service(void) {
    g_tennis.ball.bounce_count = 0;
    g_tennis.ball.first_bounce_evaluated = false;
    g_tennis.ball.in_air = false;
    g_tennis.ball.vx = 0;
    g_tennis.ball.vy = 0;
    g_tennis.ball.vz = 0;
    g_tennis.ball.trail_timer = 0;

    bool deuce_side = ((g_tennis.player_points + g_tennis.opponent_points) % 2 == 0);
    g_tennis.player.side_deuce = deuce_side;
    g_tennis.opponent.side_deuce = deuce_side;

    if (g_tennis.server == 0) {
        // Player serves
        g_tennis.player.is_serving = true;
        g_tennis.opponent.is_serving = false;
        g_tennis.player.state = ACTOR_STATE_READY;
        g_tennis.player.x = INT_TO_FP(deuce_side ? 140 : 100);
        g_tennis.player.y = INT_TO_FP(BASELINE_NEAR_Y + 4);

        g_tennis.opponent.state = ACTOR_STATE_READY;
        g_tennis.opponent.x = INT_TO_FP(deuce_side ? 100 : 140);
        g_tennis.opponent.y = INT_TO_FP(BASELINE_FAR_Y + 2);

        g_tennis.ball.x = g_tennis.player.x + INT_TO_FP(6);
        g_tennis.ball.y = g_tennis.player.y - INT_TO_FP(4);
        g_tennis.ball.z = INT_TO_FP(8);
        g_tennis.ball.last_hitter = 0;
    } else {
        // Opponent serves
        g_tennis.player.is_serving = false;
        g_tennis.opponent.is_serving = true;
        g_tennis.opponent.state = ACTOR_STATE_READY;
        g_tennis.opponent.x = INT_TO_FP(deuce_side ? 100 : 140);
        g_tennis.opponent.y = INT_TO_FP(BASELINE_FAR_Y - 2);

        g_tennis.player.state = ACTOR_STATE_READY;
        g_tennis.player.x = INT_TO_FP(deuce_side ? 140 : 100);
        g_tennis.player.y = INT_TO_FP(BASELINE_NEAR_Y + 4);

        g_tennis.ball.x = g_tennis.opponent.x + INT_TO_FP(6);
        g_tennis.ball.y = g_tennis.opponent.y + INT_TO_FP(4);
        g_tennis.ball.z = INT_TO_FP(8);
        g_tennis.ball.last_hitter = 1;
    }

    g_tennis.state = STATE_MATCH_SERVE_WAIT;
}

void game_start_match(CourtSurface surface, AiStyle ai, TennisMode mode) {
    g_tennis.surface = surface;
    g_tennis.ai_style = ai;
    g_tennis.mode = mode;

    g_tennis.player_points = 0;
    g_tennis.opponent_points = 0;
    g_tennis.player_games = 0;
    g_tennis.opponent_games = 0;
    g_tennis.server = 0;
    g_tennis.fault_count = 0;
    g_tennis.current_rally = 0;
    g_tennis.longest_rally_match = 0;
    g_tennis.last_serve_mph = 0;
    g_tennis.target_score = 0;
    g_tennis.target_timer = 60 * 60; // 60 seconds

    graphics_render_court(surface);
    audio_play_bgm(BGM_MATCH);

    setup_service();
    set_announcement(s_ai_names[ai], 60);
}

void game_award_point(u8 winner) {
    if (winner == 0) {
        // Player wins point
        sfx_play_crowd_cheer();
        if (g_tennis.player_points < 3) {
            g_tennis.player_points++;
        } else if (g_tennis.player_points == 3) {
            if (g_tennis.opponent_points < 3) {
                // Game Player!
                g_tennis.player_games++;
                g_tennis.player_points = 0;
                g_tennis.opponent_points = 0;
                g_tennis.server ^= 1;
                set_announcement("GAME PLAYER!", 90);
                sfx_play_umpire_chime();
            } else if (g_tennis.opponent_points == 3) {
                g_tennis.player_points = 4; // Advantage Player
                set_announcement("ADVANTAGE PLAYER", 60);
                sfx_play_umpire_chime();
            } else if (g_tennis.opponent_points == 4) {
                // Back to Deuce
                g_tennis.opponent_points = 3;
                set_announcement("DEUCE", 60);
                sfx_play_umpire_chime();
            }
        } else if (g_tennis.player_points == 4) {
            // Advantage won -> Game!
            g_tennis.player_games++;
            g_tennis.player_points = 0;
            g_tennis.opponent_points = 0;
            g_tennis.server ^= 1;
            set_announcement("GAME PLAYER!", 90);
            sfx_play_umpire_chime();
        }
    } else {
        // Opponent wins point
        sfx_play_umpire_chime();
        if (g_tennis.opponent_points < 3) {
            g_tennis.opponent_points++;
        } else if (g_tennis.opponent_points == 3) {
            if (g_tennis.player_points < 3) {
                // Game Opponent!
                g_tennis.opponent_games++;
                g_tennis.player_points = 0;
                g_tennis.opponent_points = 0;
                g_tennis.server ^= 1;
                set_announcement("GAME OPPONENT!", 90);
            } else if (g_tennis.player_points == 3) {
                g_tennis.opponent_points = 4; // Advantage Opponent
                set_announcement("ADVANTAGE OPPONENT", 60);
            } else if (g_tennis.player_points == 4) {
                // Back to Deuce
                g_tennis.player_points = 3;
                set_announcement("DEUCE", 60);
            }
        } else if (g_tennis.opponent_points == 4) {
            g_tennis.opponent_games++;
            g_tennis.player_points = 0;
            g_tennis.opponent_points = 0;
            g_tennis.server ^= 1;
            set_announcement("GAME OPPONENT!", 90);
        }
    }

    g_tennis.fault_count = 0;
    if (g_tennis.current_rally > g_tennis.longest_rally_match) {
        g_tennis.longest_rally_match = g_tennis.current_rally;
    }
    g_tennis.current_rally = 0;

    // Check Set & Match win (First to 3 games)
    if (g_tennis.player_games >= 3 || g_tennis.opponent_games >= 3) {
        g_tennis.state = STATE_MATCH_GAME_OVER;
        if (g_tennis.player_games >= 3) {
            audio_play_bgm(BGM_VICTORY);
            set_announcement("GAME, SET, MATCH! VICTORY!", 180);
            g_tennis.save_data.matches_won++;
            if (g_tennis.mode == MODE_TOURNAMENT) {
                if (g_tennis.tournament_round < 2) {
                    g_tennis.tournament_round++;
                } else {
                    // Won Tournament Cup!
                    g_tennis.save_data.trophies[g_tennis.surface] = 1;
                    g_tennis.state = STATE_TROPHY_CEREMONY;
                }
            }
        } else {
            set_announcement("GAME, SET, MATCH! DEFEAT", 180);
        }
        g_tennis.save_data.matches_played++;
        save_write(&g_tennis.save_data);
    } else {
        g_tennis.state = STATE_MATCH_POINT_OVER;
    }
}

void game_init(void) {
    g_tennis.state = STATE_TITLE;
    g_tennis.mode = MODE_TOURNAMENT;
    g_tennis.surface = SURFACE_GRASS;
    g_tennis.ai_style = AI_BASELINER;
    g_tennis.tournament_round = 0;
    g_tennis.announcement_str = NULL;
    g_tennis.announcement_timer = 0;
    g_tennis.chalk.active = false;
    g_tennis.game_frame = 0;

    if (!save_read(&g_tennis.save_data)) {
        for (int i = 0; i < 3; i++) g_tennis.save_data.trophies[i] = 0;
        g_tennis.save_data.matches_played = 0;
        g_tennis.save_data.matches_won = 0;
        g_tennis.save_data.total_aces = 0;
        g_tennis.save_data.max_serve_speed = 0;
        g_tennis.save_data.longest_rally = 0;
        g_tennis.save_data.target_high_score = 0;
        save_write(&g_tennis.save_data);
    }

    audio_play_bgm(BGM_MENU);
}

static void update_ball_physics(void) {
    TennisBall* b = &g_tennis.ball;
    if (!b->in_air) return;

    // Gravity
    b->vz -= (INT_TO_FP(1) / 8); // 0.125 px/frame^2
    b->x += b->vx;
    b->y += b->vy;
    b->z += b->vz;

    if (b->trail_timer > 0) b->trail_timer--;

    // Net collision at Y = NET_Y (76)
    if ((b->vy > 0 && b->y >= INT_TO_FP(NET_Y) && b->y - b->vy < INT_TO_FP(NET_Y)) ||
        (b->vy < 0 && b->y <= INT_TO_FP(NET_Y) && b->y - b->vy > INT_TO_FP(NET_Y))) {
        if (b->z < INT_TO_FP(16)) {
            // Hit Net!
            sfx_play_net_hit();
            if (b->z >= INT_TO_FP(14)) {
                // Let cord dribble!
                b->vy = (b->vy > 0) ? (INT_TO_FP(1) / 2) : -(INT_TO_FP(1) / 2);
                b->vz = INT_TO_FP(1);
            } else {
                // Drop dead at net
                b->vx = 0;
                b->vy = 0;
                b->vz = 0;
                b->in_air = false;
                set_announcement("NET!", 60);
                game_award_point(b->last_hitter ^ 1);
                return;
            }
        }
    }

    // Court ground bounce
    if (b->z <= 0) {
        b->z = 0;
        b->bounce_count++;
        s16 bx = FP_TO_INT(b->x);
        s16 by = FP_TO_INT(b->y);

        sfx_play_ball_bounce(g_tennis.surface);

        // Surface modifiers
        fixed_t bounce_ret = INT_TO_FP(3) / 4; // Hard court default 0.75
        fixed_t friction = INT_TO_FP(4) / 5;   // 0.80
        if (g_tennis.surface == SURFACE_GRASS) {
            bounce_ret = (INT_TO_FP(13) / 20); // 0.65 low bounce
            friction = (INT_TO_FP(17) / 20);   // 0.85 fast skid
        } else if (g_tennis.surface == SURFACE_CLAY) {
            bounce_ret = (INT_TO_FP(17) / 20); // 0.85 high bounce
            friction = (INT_TO_FP(3) / 4);     // 0.75 slow
        }

        b->vz = -FP_MUL(b->vz, bounce_ret);
        b->vx = FP_MUL(b->vx, friction);
        b->vy = FP_MUL(b->vy, friction);

        // First bounce evaluation
        if (!b->first_bounce_evaluated) {
            b->first_bounce_evaluated = true;
            bool is_in = false;

            if (g_tennis.ball.last_shot == SHOT_SERVE) {
                // Service box check
                bool deuce = g_tennis.player.side_deuce;
                if (b->last_hitter == 0) {
                    is_in = is_serve_in_box(bx, by, false, deuce);
                } else {
                    is_in = is_serve_in_box(bx, by, true, deuce);
                }

                if (!is_in) {
                    // Serve Fault!
                    g_tennis.fault_count++;
                    if (g_tennis.fault_count == 1) {
                        set_announcement("FAULT! 2ND SERVE", 60);
                        sfx_play_umpire_chime();
                        b->in_air = false;
                        setup_service();
                        return;
                    } else {
                        set_announcement("DOUBLE FAULT!", 60);
                        game_award_point(b->last_hitter ^ 1);
                        return;
                    }
                } else {
                    trigger_chalk_puff(bx, by);
                }
            } else {
                // Normal rally court check
                is_in = is_point_in_court(bx, by);
                if (is_in) {
                    trigger_chalk_puff(bx, by);
                } else {
                    set_announcement("OUT!", 60);
                    b->in_air = false;
                    game_award_point(b->last_hitter ^ 1);
                    return;
                }
            }
        }

        // Second bounce -> Striker wins point!
        if (b->bounce_count >= 2) {
            b->in_air = false;
            game_award_point(b->last_hitter);
            return;
        }
    }
}

static void update_player(void) {
    TennisPlayer* p = &g_tennis.player;
    s16 py = FP_TO_INT(p->y);
    s16 hw = get_court_half_width(py);

    if (p->timer > 0) p->timer--;

    // Reset swing stance when timer expires
    if (p->timer == 0 && (p->state == ACTOR_STATE_SWING_FH || p->state == ACTOR_STATE_SWING_BH ||
                          p->state == ACTOR_STATE_SERVE_HIT || p->state == ACTOR_STATE_SMASH)) {
        p->state = ACTOR_STATE_READY;
    }

    // Serve phase
    if (g_tennis.state == STATE_MATCH_SERVE_WAIT && p->is_serving) {
        if (key_was_pressed(KEY_A)) {
            // Toss ball!
            p->state = ACTOR_STATE_TOSS;
            p->timer = 40;
            g_tennis.ball.in_air = true;
            g_tennis.ball.x = p->x + INT_TO_FP(6);
            g_tennis.ball.y = p->y - INT_TO_FP(4);
            g_tennis.ball.z = INT_TO_FP(8);
            g_tennis.ball.vx = 0;
            g_tennis.ball.vy = 0;
            g_tennis.ball.vz = INT_TO_FP(3) + (INT_TO_FP(1) / 2); // Toss upward
            g_tennis.ball.last_shot = SHOT_SERVE;
            g_tennis.ball.last_hitter = 0;
            sfx_play_racket_slice();
            g_tennis.state = STATE_MATCH_RALLY;
        }
        return;
    }

    // Serve Hit Timing
    if (p->state == ACTOR_STATE_TOSS && p->is_serving) {
        if (key_was_pressed(KEY_A) || key_was_pressed(KEY_B)) {
            s16 bz = FP_TO_INT(g_tennis.ball.z);
            if (bz >= 20) {
                // Hit serve!
                p->state = ACTOR_STATE_SERVE_HIT;
                p->timer = 20;

                u16 mph = 85 + (bz * 2);
                if (bz >= 28) mph = 120 + ((g_tennis.game_frame & 7) * 2); // Max Power Ace!
                g_tennis.last_serve_mph = mph;
                if (mph > g_tennis.save_data.max_serve_speed) g_tennis.save_data.max_serve_speed = mph;

                fixed_t aim_x = 0;
                if (key_is_down(KEY_LEFT)) aim_x = -INT_TO_FP(1);
                if (key_is_down(KEY_RIGHT)) aim_x = INT_TO_FP(1);

                g_tennis.ball.vx = aim_x;
                g_tennis.ball.vy = -INT_TO_FP(4) - (mph > 115 ? INT_TO_FP(1) : 0);
                g_tennis.ball.vz = INT_TO_FP(1);
                g_tennis.ball.first_bounce_evaluated = false;
                g_tennis.ball.bounce_count = 0;

                if (mph >= 120) {
                    graphics_trigger_shake(3, 12);
                    sfx_play_racket_smash();
                    g_tennis.ball.trail_timer = 20;
                    g_tennis.ball.trail_type = 2; // Yellow
                    g_tennis.save_data.total_aces++;
                } else {
                    sfx_play_racket_topspin();
                }
            }
        }
        return;
    }

    // Normal court movement
    fixed_t spd = INT_TO_FP(2);
    p->vx = 0;
    p->vy = 0;

    if (key_is_down(KEY_LEFT))  p->vx -= spd;
    if (key_is_down(KEY_RIGHT)) p->vx += spd;
    if (key_is_down(KEY_UP))    p->vy -= spd;
    if (key_is_down(KEY_DOWN))  p->vy += spd;

    p->x += p->vx;
    p->y += p->vy;

    // Clamp player to near court area
    s16 cx = FP_TO_INT(p->x);
    s16 cy = FP_TO_INT(p->y);
    if (cx < 120 - hw - 8) p->x = INT_TO_FP(120 - hw - 8);
    if (cx > 120 + hw + 8) p->x = INT_TO_FP(120 + hw + 8);
    if (cy < NET_Y + 12) p->y = INT_TO_FP(NET_Y + 12);
    if (cy > BASELINE_NEAR_Y + 14) p->y = INT_TO_FP(BASELINE_NEAR_Y + 14);

    if (p->vx != 0 || p->vy != 0) {
        if (p->state == ACTOR_STATE_READY) p->state = ACTOR_STATE_RUN;
    } else {
        if (p->state == ACTOR_STATE_RUN) p->state = ACTOR_STATE_READY;
    }

    // Racket Swing Strike Detection
    if (key_was_pressed(KEY_A) || key_was_pressed(KEY_B)) {
        s16 bx = FP_TO_INT(g_tennis.ball.x);
        s16 by = FP_TO_INT(g_tennis.ball.y);
        s16 bz = FP_TO_INT(g_tennis.ball.z);

        // Strike zone: Ball is near player and reachable in Z
        if (bx >= cx - 24 && bx <= cx + 24 && by >= cy - 20 && by <= cy + 16 && bz <= 40) {
            bool is_forehand = (bx >= cx);
            p->state = is_forehand ? ACTOR_STATE_SWING_FH : ACTOR_STATE_SWING_BH;
            p->timer = 18;

            fixed_t aim_x = 0;
            if (key_is_down(KEY_LEFT)) aim_x = -INT_TO_FP(1) - (INT_TO_FP(1) / 2);
            if (key_is_down(KEY_RIGHT)) aim_x = INT_TO_FP(1) + (INT_TO_FP(1) / 2);

            if (bz >= 28 && key_was_pressed(KEY_A)) {
                // Overhead Smash!
                sfx_play_racket_smash();
                graphics_trigger_shake(4, 15);
                g_tennis.ball.vx = aim_x;
                g_tennis.ball.vy = -INT_TO_FP(5);
                g_tennis.ball.vz = -INT_TO_FP(1);
                g_tennis.ball.trail_type = 2; // Yellow
                g_tennis.ball.last_shot = SHOT_SMASH;
            } else if (key_is_down(KEY_A) && key_is_down(KEY_B)) {
                // Defensive Lob
                sfx_play_racket_slice();
                g_tennis.ball.vx = aim_x;
                g_tennis.ball.vy = -INT_TO_FP(2);
                g_tennis.ball.vz = INT_TO_FP(5);
                g_tennis.ball.last_shot = SHOT_LOB;
            } else if (key_was_pressed(KEY_B)) {
                // Slice Shot
                sfx_play_racket_slice();
                g_tennis.ball.vx = aim_x;
                g_tennis.ball.vy = -INT_TO_FP(3);
                g_tennis.ball.vz = INT_TO_FP(2);
                g_tennis.ball.trail_type = 1; // Blue
                g_tennis.ball.last_shot = SHOT_SLICE;
            } else {
                // Topspin Drive
                sfx_play_racket_topspin();
                g_tennis.ball.vx = aim_x;
                g_tennis.ball.vy = -INT_TO_FP(4);
                g_tennis.ball.vz = INT_TO_FP(3);
                g_tennis.ball.trail_type = 0; // Red
                g_tennis.ball.last_shot = SHOT_TOPSPIN;
            }

            g_tennis.ball.trail_timer = 20;
            g_tennis.ball.bounce_count = 0;
            g_tennis.ball.first_bounce_evaluated = false;
            g_tennis.ball.last_hitter = 0;
            g_tennis.current_rally++;
        }
    }
}

static void update_opponent_ai(void) {
    TennisPlayer* opp = &g_tennis.opponent;
    if (opp->timer > 0) opp->timer--;

    if (opp->timer == 0 && (opp->state == ACTOR_STATE_SWING_FH || opp->state == ACTOR_STATE_SWING_BH)) {
        opp->state = ACTOR_STATE_READY;
    }

    // Opponent serving
    if (g_tennis.state == STATE_MATCH_SERVE_WAIT && opp->is_serving) {
        if (g_tennis.game_frame % 60 == 30) {
            // Opponent serves!
            opp->state = ACTOR_STATE_SERVE_HIT;
            opp->timer = 20;
            g_tennis.ball.in_air = true;
            g_tennis.ball.x = opp->x + INT_TO_FP(6);
            g_tennis.ball.y = opp->y + INT_TO_FP(4);
            g_tennis.ball.z = INT_TO_FP(24);
            g_tennis.ball.vx = (opp->side_deuce ? -INT_TO_FP(1) : INT_TO_FP(1));
            g_tennis.ball.vy = INT_TO_FP(4);
            g_tennis.ball.vz = INT_TO_FP(2);
            g_tennis.ball.last_shot = SHOT_SERVE;
            g_tennis.ball.last_hitter = 1;
            g_tennis.ball.bounce_count = 0;
            g_tennis.ball.first_bounce_evaluated = false;
            sfx_play_racket_topspin();
            g_tennis.state = STATE_MATCH_RALLY;
        }
        return;
    }

    // AI Tracking & Position
    fixed_t target_x = g_tennis.ball.x;
    fixed_t target_y = INT_TO_FP(BASELINE_FAR_Y + 4);

    if (g_tennis.ai_style == AI_VOLLEYER && g_tennis.ball.last_hitter == 1) {
        target_y = INT_TO_FP(NET_Y - 14); // Net rush!
    }

    fixed_t dx = target_x - opp->x;
    fixed_t dy = target_y - opp->y;
    fixed_t ai_spd = INT_TO_FP(1) + (INT_TO_FP(1) / 2);

    if (dx > INT_TO_FP(4)) opp->x += ai_spd;
    else if (dx < -INT_TO_FP(4)) opp->x -= ai_spd;

    if (dy > INT_TO_FP(4)) opp->y += ai_spd;
    else if (dy < -INT_TO_FP(4)) opp->y -= ai_spd;

    // AI Return Strike
    s16 ox = FP_TO_INT(opp->x);
    s16 oy = FP_TO_INT(opp->y);
    s16 bx = FP_TO_INT(g_tennis.ball.x);
    s16 by = FP_TO_INT(g_tennis.ball.y);
    s16 bz = FP_TO_INT(g_tennis.ball.z);

    if (g_tennis.ball.vy < 0 && bx >= ox - 20 && bx <= ox + 20 && by >= oy - 14 && by <= oy + 16 && bz <= 35) {
        bool is_fh = (bx >= ox);
        opp->state = is_fh ? ACTOR_STATE_SWING_FH : ACTOR_STATE_SWING_BH;
        opp->timer = 18;

        sfx_play_racket_topspin();
        fixed_t aim_x = (g_tennis.player.x > opp->x) ? -INT_TO_FP(1) : INT_TO_FP(1);

        g_tennis.ball.vx = aim_x;
        g_tennis.ball.vy = INT_TO_FP(4);
        g_tennis.ball.vz = INT_TO_FP(2) + (INT_TO_FP(1) / 2);
        g_tennis.ball.bounce_count = 0;
        g_tennis.ball.first_bounce_evaluated = false;
        g_tennis.ball.last_hitter = 1;
        g_tennis.ball.last_shot = SHOT_TOPSPIN;
        g_tennis.current_rally++;
    }
}

void game_update(void) {
    g_tennis.game_frame++;
    audio_update();
    graphics_update_shake();

    if (g_tennis.announcement_timer > 0) g_tennis.announcement_timer--;
    if (g_tennis.chalk.timer > 0) g_tennis.chalk.timer--;

    if (g_tennis.state == STATE_TITLE) {
        if (key_was_pressed(KEY_START) || key_was_pressed(KEY_A)) {
            g_tennis.state = STATE_SURFACE_SELECT;
            sfx_play_racket_topspin();
        }
        return;
    }

    if (g_tennis.state == STATE_SURFACE_SELECT) {
        if (key_was_pressed(KEY_LEFT)) {
            if (g_tennis.surface > 0) g_tennis.surface--;
            sfx_play_racket_slice();
        }
        if (key_was_pressed(KEY_RIGHT)) {
            if (g_tennis.surface < 2) g_tennis.surface++;
            sfx_play_racket_slice();
        }
        if (key_was_pressed(KEY_A) || key_was_pressed(KEY_START)) {
            game_start_match(g_tennis.surface, AI_BASELINER, g_tennis.mode);
        }
        return;
    }

    if (g_tennis.state == STATE_MATCH_POINT_OVER) {
        if (key_was_pressed(KEY_A) || key_was_pressed(KEY_START)) {
            setup_service();
        }
        return;
    }

    if (g_tennis.state == STATE_MATCH_GAME_OVER || g_tennis.state == STATE_TROPHY_CEREMONY) {
        if (key_was_pressed(KEY_A) || key_was_pressed(KEY_START)) {
            g_tennis.state = STATE_TITLE;
            audio_play_bgm(BGM_MENU);
        }
        return;
    }

    update_player();
    update_opponent_ai();
    update_ball_physics();
}

void game_draw(void) {
    graphics_oam_hide_all();

    if (g_tennis.state == STATE_TITLE) {
        graphics_clear_bg0();
        graphics_draw_box(2, 2, 26, 16, PAL_BG_SCOREBOARD);
        graphics_print_text(4, 4,  "GRAND SLAM: TENNIS", PAL_BG_SCOREBOARD);
        graphics_print_text(4, 6,  "ADVANCE TOUR FOR GBA", PAL_BG_SCOREBOARD);
        graphics_print_text(4, 9,  "PRESS START OR A", PAL_BG_SCOREBOARD);
        graphics_print_text(4, 11, "A: TOPSPIN / SERVE", PAL_BG_SCOREBOARD);
        graphics_print_text(4, 12, "B: SLICE   A+B: LOB", PAL_BG_SCOREBOARD);
        graphics_print_text(4, 14, "MATCHES WON: ", PAL_BG_SCOREBOARD);
        graphics_print_num(17, 14, g_tennis.save_data.matches_won, 3, PAL_BG_SCOREBOARD);
        graphics_oam_copy();
        return;
    }

    if (g_tennis.state == STATE_SURFACE_SELECT) {
        graphics_clear_bg0();
        graphics_draw_box(3, 4, 24, 12, PAL_BG_SCOREBOARD);
        graphics_print_text(5, 6,  "SELECT COURT SURFACE", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 8,  "<  ", PAL_BG_SCOREBOARD);
        graphics_print_text(8, 8,  s_surface_names[g_tennis.surface], PAL_BG_SCOREBOARD);
        graphics_print_text(15, 8, "  >", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 11, "PRESS A TO START", PAL_BG_SCOREBOARD);
        graphics_oam_copy();
        return;
    }

    // Render Scoreboard HUD on BG0
    graphics_clear_bg0();

    // Top scoreboard banner
    graphics_draw_box(0, 0, 30, 3, PAL_BG_SCOREBOARD);
    graphics_print_text(1, 1, "YOU", PAL_BG_SCOREBOARD);
    graphics_print_num(5, 1, g_tennis.player_games, 1, PAL_BG_SCOREBOARD);
    graphics_print_text(7, 1, s_point_names[g_tennis.player_points], PAL_BG_SCOREBOARD);

    graphics_print_text(11, 1, "OPP", PAL_BG_SCOREBOARD);
    graphics_print_num(15, 1, g_tennis.opponent_games, 1, PAL_BG_SCOREBOARD);
    graphics_print_text(17, 1, s_point_names[g_tennis.opponent_points], PAL_BG_SCOREBOARD);

    if (g_tennis.last_serve_mph > 0) {
        graphics_print_num(21, 1, g_tennis.last_serve_mph, 3, PAL_BG_SCOREBOARD);
        graphics_print_text(25, 1, "MPH", PAL_BG_SCOREBOARD);
    }

    // Match announcement modal
    if (g_tennis.announcement_timer > 0 && g_tennis.announcement_str) {
        graphics_draw_box(4, 7, 22, 4, PAL_BG_SCOREBOARD);
        graphics_print_text(6, 8, g_tennis.announcement_str, PAL_BG_SCOREBOARD);
    }

    // Trophy Ceremony Modal
    if (g_tennis.state == STATE_TROPHY_CEREMONY) {
        graphics_draw_box(3, 5, 24, 8, PAL_BG_SCOREBOARD);
        graphics_print_text(5, 6, "GRAND SLAM CHAMPION!", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 8, "TROPHY AWARDED TO YOU!", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 10, "PRESS START TO EXIT", PAL_BG_SCOREBOARD);
    }

    // Render Sprites
    u8 sprite_idx = 0;

    // 1. Far Opponent Player
    s16 ox = FP_TO_INT(g_tennis.opponent.x);
    s16 oy = FP_TO_INT(g_tennis.opponent.y);
    u16 opp_tile = SPRITE_OPP_READY_0;
    if (g_tennis.opponent.state == ACTOR_STATE_SWING_FH) opp_tile = SPRITE_OPP_SWING_FH;
    if (g_tennis.opponent.state == ACTOR_STATE_SWING_BH) opp_tile = SPRITE_OPP_SWING_BH;
    graphics_set_sprite(sprite_idx++, ox - 8, oy - 14, opp_tile, 0, 1, PAL_OBJ_OPPONENT, false, false);

    // 2. Ball Drop Shadow on Court Ground
    if (g_tennis.ball.in_air || g_tennis.state == STATE_MATCH_SERVE_WAIT) {
        s16 bx = FP_TO_INT(g_tennis.ball.x);
        s16 by = FP_TO_INT(g_tennis.ball.y);
        graphics_set_sprite(sprite_idx++, bx - 8, by - 4, SPRITE_BALL_SHADOW, 0, 1, PAL_OBJ_BALL, false, false);
    }

    // 3. Tennis Ball (Elevated by Z altitude)
    if (g_tennis.ball.in_air || g_tennis.state == STATE_MATCH_SERVE_WAIT) {
        s16 bx = FP_TO_INT(g_tennis.ball.x);
        s16 by = FP_TO_INT(g_tennis.ball.y) - FP_TO_INT(g_tennis.ball.z);

        u16 ball_tile = SPRITE_BALL_MID;
        if (g_tennis.ball.z > INT_TO_FP(25) || g_tennis.ball.y < INT_TO_FP(60)) ball_tile = SPRITE_BALL_SMALL;
        if (g_tennis.ball.y > INT_TO_FP(120) && g_tennis.ball.z < INT_TO_FP(15)) ball_tile = SPRITE_BALL_LARGE;

        graphics_set_sprite(sprite_idx++, bx - 8, by - 8, ball_tile, 0, 1, PAL_OBJ_BALL, false, false);

        // Swing Trail VFX
        if (g_tennis.ball.trail_timer > 0) {
            u16 trail_tile = SPRITE_SWING_TRAIL_RED;
            if (g_tennis.ball.trail_type == 1) trail_tile = SPRITE_SWING_TRAIL_BLUE;
            if (g_tennis.ball.trail_type == 2) trail_tile = SPRITE_SWING_TRAIL_YEL;
            graphics_set_sprite(sprite_idx++, bx - 8, by + 4, trail_tile, 0, 1, PAL_OBJ_VFX, false, false);
        }
    }

    // 4. Chalk Puff on Line/Court Bounce
    if (g_tennis.chalk.timer > 0) {
        graphics_set_sprite(sprite_idx++, g_tennis.chalk.x - 8, g_tennis.chalk.y - 8, SPRITE_CHALK_PUFF, 0, 1, PAL_OBJ_BALL, false, false);
    }

    // 5. Near Player
    s16 px = FP_TO_INT(g_tennis.player.x);
    s16 py = FP_TO_INT(g_tennis.player.y);
    u16 player_tile = (g_tennis.player.state == ACTOR_STATE_RUN) ? SPRITE_PLAYER_RUN_R : SPRITE_PLAYER_READY_0;

    if (g_tennis.player.state == ACTOR_STATE_SWING_FH) player_tile = SPRITE_PLAYER_SWING_FH_0;
    if (g_tennis.player.state == ACTOR_STATE_SWING_BH) player_tile = SPRITE_PLAYER_SWING_BH_0;
    if (g_tennis.player.state == ACTOR_STATE_TOSS)     player_tile = SPRITE_PLAYER_TOSS;
    if (g_tennis.player.state == ACTOR_STATE_SERVE_HIT) player_tile = SPRITE_PLAYER_SERVE_HIT;
    if (g_tennis.player.state == ACTOR_STATE_SMASH)    player_tile = SPRITE_PLAYER_SMASH;

    graphics_set_sprite(sprite_idx++, px - 8, py - 14, player_tile, 0, 1, PAL_OBJ_PLAYER, false, false);

    graphics_oam_copy();
}
