#include "game.h"
#include "graphics.h"
#include "input.h"
#include "audio.h"
#include "save.h"

GameState g_tennis;

static const char* const s_point_names[5] = {"0", "15", "30", "40", "AD"};
static const char* const s_surface_names[3] = {"GRASS", "CLAY", "HARD"};
static const char* const s_ai_names[3] = {"L. VANCE", "K. SATO", "M. THORNE"};
static const char* const s_ai_titles[3] = {"THE WALL (BASELINER)", "SERVE & VOLLEYER", "THUNDER (POWER SERVER)"};
static const char* const s_round_names[3] = {"QUARTER-FINAL", "SEMI-FINAL", "GRAND FINAL"};

static s16 get_court_half_width(s16 y) {
    if (y < BASELINE_FAR_Y) y = BASELINE_FAR_Y;
    if (y > BASELINE_NEAR_Y) y = BASELINE_NEAR_Y;
    // Linear interpolation: 48 at far baseline (Y=36), 80 at near baseline (Y=140)
    return 48 + ((y - BASELINE_FAR_Y) * (80 - 48)) / (BASELINE_NEAR_Y - BASELINE_FAR_Y);
}

static bool is_point_in_court(s16 x, s16 y) {
    if (y < BASELINE_FAR_Y || y > BASELINE_NEAR_Y) return false;
    s16 hw = get_court_half_width(y);
    return (x >= 120 - hw && x <= 120 + hw);
}

static bool is_serve_in_box(s16 x, s16 y, bool receiver_is_player, bool deuce_court) {
    s16 hw = get_court_half_width(y);
    if (receiver_is_player) {
        // Near service box (between NET_Y and SERVICE_NEAR_Y)
        if (y < NET_Y || y > SERVICE_NEAR_Y) return false;
        if (deuce_court) {
            // Receiver right box: X from 120 to 120 + hw
            return (x >= 120 && x <= 120 + hw);
        } else {
            // Receiver left box: X from 120 - hw to 120
            return (x >= 120 - hw && x <= 120);
        }
    } else {
        // Far service box (between SERVICE_FAR_Y and NET_Y)
        if (y < SERVICE_FAR_Y || y > NET_Y) return false;
        if (deuce_court) {
            // Far deuce box: X from 120 - hw to 120
            return (x >= 120 - hw && x <= 120);
        } else {
            // Far ad box: X from 120 to 120 + hw
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

// Target-Zone Trajectory Launcher: calculates exact parabolic velocities
static void launch_ball(fixed_t x0, fixed_t y0, fixed_t z0,
                        fixed_t tx, fixed_t ty,
                        u16 flight_time, ShotType shot,
                        u8 hitter, u8 trail_type) {
    if (flight_time < 8) flight_time = 8;
    TennisBall* b = &g_tennis.ball;

    b->x = x0;
    b->y = y0;
    b->z = z0;
    b->target_x = tx;
    b->target_y = ty;
    b->flight_time = flight_time;
    b->flight_elapsed = 0;

    b->vx = (tx - x0) / (s32)flight_time;
    b->vy = (ty - y0) / (s32)flight_time;

    // Gravity: g = 0.125 px/frame^2 = INT_TO_FP(1) / 8 = 32 in 8.8 FP
    fixed_t g = INT_TO_FP(1) / 8;
    fixed_t term1 = ((s32)(flight_time - 1) * g) / 2;
    fixed_t term2 = z0 / (s32)flight_time;
    b->vz = term1 - term2;

    // Net clearance verification at Y = NET_Y (76)
    if (b->vy != 0) {
        s32 t_net = (INT_TO_FP(NET_Y) - y0) / b->vy;
        if (t_net > 0 && t_net < (s32)flight_time) {
            fixed_t z_net = z0 + (fixed_t)t_net * b->vz - (((s32)t_net * (t_net - 1) * g) / 2);
            if (z_net < INT_TO_FP(18)) {
                // Boost vz so ball cleanly clears net tape (18 px)
                fixed_t req_vz = (INT_TO_FP(18) - z0 + (((s32)t_net * (t_net - 1) * g) / 2)) / (fixed_t)t_net;
                if (req_vz > b->vz) {
                    b->vz = req_vz;
                }
            }
        }
    }

    b->last_shot = shot;
    b->last_hitter = hitter;
    b->trail_type = trail_type;
    b->trail_timer = (flight_time < 22) ? 22 : 16;
    b->in_air = true;
    b->first_bounce_evaluated = false;
    b->bounce_count = 0;
}

static void targets_init(void) {
    // 3 Target rings on far court
    g_tennis.targets[0].x = 84;
    g_tennis.targets[0].y = 46;
    g_tennis.targets[0].points = 250;
    g_tennis.targets[0].active = true;
    g_tennis.targets[0].timer = 0;
    g_tennis.targets[0].ring_type = 0;

    g_tennis.targets[1].x = 156;
    g_tennis.targets[1].y = 46;
    g_tennis.targets[1].points = 250;
    g_tennis.targets[1].active = true;
    g_tennis.targets[1].timer = 0;
    g_tennis.targets[1].ring_type = 0;

    g_tennis.targets[2].x = 120;
    g_tennis.targets[2].y = 62;
    g_tennis.targets[2].points = 100;
    g_tennis.targets[2].active = true;
    g_tennis.targets[2].timer = 0;
    g_tennis.targets[2].ring_type = 0;

    // Moving Bullseye bonus
    g_tennis.targets[3].x = 120;
    g_tennis.targets[3].y = 40;
    g_tennis.targets[3].points = 500;
    g_tennis.targets[3].active = true;
    g_tennis.targets[3].timer = 0;
    g_tennis.targets[3].ring_type = 1;
}

static void targets_update(void) {
    // Bullseye moves horizontally
    TargetRing* b = &g_tennis.targets[3];
    if (b->active) {
        s16 offset = ((g_tennis.game_frame >> 1) % 60) - 30;
        b->x = 120 + offset;
    }

    // Respawn inactive rings
    for (int i = 0; i < MAX_TARGETS; i++) {
        if (!g_tennis.targets[i].active) {
            if (++g_tennis.targets[i].timer >= 60) {
                g_tennis.targets[i].active = true;
                g_tennis.targets[i].timer = 0;
                if (i == 0) { g_tennis.targets[i].x = 80 + (g_tennis.game_frame & 15); g_tennis.targets[i].y = 44 + ((g_tennis.game_frame >> 2) & 7); }
                if (i == 1) { g_tennis.targets[i].x = 148 + (g_tennis.game_frame & 15); g_tennis.targets[i].y = 44 + ((g_tennis.game_frame >> 2) & 7); }
                if (i == 2) { g_tennis.targets[i].x = 110 + (g_tennis.game_frame & 15); g_tennis.targets[i].y = 58 + ((g_tennis.game_frame >> 2) & 7); }
            }
        }
    }
}

static void targets_check_hit(s16 bx, s16 by) {
    for (int i = 0; i < MAX_TARGETS; i++) {
        if (g_tennis.targets[i].active) {
            s16 dx = bx - g_tennis.targets[i].x;
            s16 dy = by - g_tennis.targets[i].y;
            if (dx >= -12 && dx <= 12 && dy >= -8 && dy <= 8) {
                // Target Shattered!
                g_tennis.targets[i].active = false;
                g_tennis.targets[i].timer = 0;
                g_tennis.target_combo++;
                u32 pts = g_tennis.targets[i].points * g_tennis.target_combo;
                g_tennis.target_score += pts;
                sfx_play_umpire_chime();
                trigger_chalk_puff(g_tennis.targets[i].x, g_tennis.targets[i].y);
                graphics_trigger_shake(2, 8);
                set_announcement(g_tennis.targets[i].ring_type ? "BULLSEYE! 500 PTS!" : "TARGET HIT!", 45);
                return;
            }
        }
    }
}

static void setup_service(void) {
    g_tennis.ball.bounce_count = 0;
    g_tennis.ball.first_bounce_evaluated = false;
    g_tennis.ball.in_air = false;
    g_tennis.ball.vx = 0;
    g_tennis.ball.vy = 0;
    g_tennis.ball.vz = 0;
    g_tennis.ball.trail_timer = 0;

    g_tennis.player.is_charging = false;
    g_tennis.player.charge_power = 0;
    g_tennis.opponent.is_charging = false;
    g_tennis.opponent.charge_power = 0;
    g_tennis.opponent.ai_net_rushing = false;

    bool deuce_side = ((g_tennis.player_points + g_tennis.opponent_points) % 2 == 0);
    g_tennis.player.side_deuce = deuce_side;
    g_tennis.opponent.side_deuce = deuce_side;

    if (g_tennis.server == 0) {
        // Player serves
        g_tennis.player.is_serving = true;
        g_tennis.opponent.is_serving = false;
        g_tennis.player.state = ACTOR_STATE_READY;
        g_tennis.player.x = INT_TO_FP(deuce_side ? 144 : 96);
        g_tennis.player.y = INT_TO_FP(BASELINE_NEAR_Y + 2);

        g_tennis.opponent.state = ACTOR_STATE_READY;
        g_tennis.opponent.x = INT_TO_FP(deuce_side ? 96 : 144);
        g_tennis.opponent.y = INT_TO_FP(BASELINE_FAR_Y + 4);

        g_tennis.ball.x = g_tennis.player.x + INT_TO_FP(6);
        g_tennis.ball.y = g_tennis.player.y - INT_TO_FP(4);
        g_tennis.ball.z = INT_TO_FP(8);
        g_tennis.ball.last_hitter = 0;
    } else {
        // Opponent serves
        g_tennis.player.is_serving = false;
        g_tennis.opponent.is_serving = true;
        g_tennis.opponent.state = ACTOR_STATE_READY;
        g_tennis.opponent.x = INT_TO_FP(deuce_side ? 96 : 144);
        g_tennis.opponent.y = INT_TO_FP(BASELINE_FAR_Y + 2);

        g_tennis.player.state = ACTOR_STATE_READY;
        g_tennis.player.x = INT_TO_FP(deuce_side ? 144 : 96);
        g_tennis.player.y = INT_TO_FP(BASELINE_NEAR_Y + 2);

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

    graphics_render_court(surface);
    audio_play_bgm(BGM_MATCH);

    setup_service();
    if (mode == MODE_TOURNAMENT) {
        set_announcement(s_round_names[g_tennis.tournament_round], 75);
    } else {
        set_announcement(s_ai_names[ai], 60);
    }
}

void game_start_target_practice(void) {
    g_tennis.mode = MODE_TARGET_PRACTICE;
    g_tennis.surface = SURFACE_HARD;
    g_tennis.state = STATE_TARGET_PRACTICE;
    g_tennis.target_score = 0;
    g_tennis.target_timer = 60 * 60; // 60 seconds
    g_tennis.target_combo = 1;
    g_tennis.ball_machine_timer = 30;

    g_tennis.player.x = INT_TO_FP(120);
    g_tennis.player.y = INT_TO_FP(BASELINE_NEAR_Y);
    g_tennis.player.state = ACTOR_STATE_READY;
    g_tennis.player.is_charging = false;
    g_tennis.player.charge_power = 0;

    g_tennis.opponent.x = INT_TO_FP(120);
    g_tennis.opponent.y = INT_TO_FP(BASELINE_FAR_Y);
    g_tennis.opponent.state = ACTOR_STATE_READY;

    g_tennis.ball.in_air = false;

    targets_init();
    graphics_render_court(SURFACE_HARD);
    audio_play_bgm(BGM_MATCH);
    set_announcement("TARGET DRILL! 60 SEC", 75);
}

void game_award_point(u8 winner) {
    if (g_tennis.state == STATE_TARGET_PRACTICE) {
        g_tennis.target_combo = 1;
        g_tennis.ball.in_air = false;
        g_tennis.ball_machine_timer = 30;
        return;
    }

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
    if (g_tennis.current_rally > g_tennis.save_data.longest_rally) {
        g_tennis.save_data.longest_rally = g_tennis.current_rally;
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
                    // Won Grand Slam Cup!
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
    g_tennis.menu_mode = 0;
    g_tennis.menu_opp = 0;
    g_tennis.menu_surface = 0;
    g_tennis.menu_setup_step = 0;
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

    b->flight_elapsed++;
    b->vz -= (INT_TO_FP(1) / 8); // Gravity: 0.125 px/frame^2
    b->x += b->vx;
    b->y += b->vy;
    b->z += b->vz;

    if (b->trail_timer > 0) b->trail_timer--;

    // Net collision at Y = NET_Y (76)
    if ((b->vy > 0 && b->y >= INT_TO_FP(NET_Y) && b->y - b->vy < INT_TO_FP(NET_Y)) ||
        (b->vy < 0 && b->y <= INT_TO_FP(NET_Y) && b->y - b->vy > INT_TO_FP(NET_Y))) {
        if (b->z < INT_TO_FP(16)) {
            // Net cord contact
            sfx_play_net_hit();
            if (b->z >= INT_TO_FP(13)) {
                // Let cord dribble!
                b->vy = (b->vy > 0) ? (INT_TO_FP(1) / 2) : -(INT_TO_FP(1) / 2);
                b->vz = INT_TO_FP(1);
            } else {
                // Stopped dead by net
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

    // Ground bounce
    if (b->z <= 0) {
        b->z = 0;
        b->bounce_count++;
        s16 bx = FP_TO_INT(b->x);
        s16 by = FP_TO_INT(b->y);

        sfx_play_ball_bounce(g_tennis.surface);

        // Surface physical modifiers
        fixed_t bounce_ret = INT_TO_FP(3) / 4; // Hard court default 0.75
        fixed_t friction = INT_TO_FP(4) / 5;   // 0.80
        if (g_tennis.surface == SURFACE_GRASS) {
            bounce_ret = (INT_TO_FP(11) / 20); // 0.55 low skid bounce
            friction = (INT_TO_FP(18) / 20);   // 0.90 fast slick pace
        } else if (g_tennis.surface == SURFACE_CLAY) {
            bounce_ret = (INT_TO_FP(17) / 20); // 0.85 heavy topspin kick
            friction = (INT_TO_FP(14) / 20);   // 0.70 high traction slowdown
        }

        // Slice & Drop shot dampening
        if (b->last_shot == SHOT_SLICE) {
            bounce_ret = FP_MUL(bounce_ret, INT_TO_FP(3) / 5); // 0.60x lower bounce
        } else if (b->last_shot == SHOT_DROP) {
            bounce_ret = FP_MUL(bounce_ret, INT_TO_FP(1) / 3); // 0.33x dying touch
            friction = FP_MUL(friction, INT_TO_FP(1) / 2);
        }

        b->vz = -FP_MUL(b->vz, bounce_ret);
        b->vx = FP_MUL(b->vx, friction);
        b->vy = FP_MUL(b->vy, friction);

        // Target Practice evaluation on far court
        if (g_tennis.state == STATE_TARGET_PRACTICE && b->last_hitter == 0) {
            targets_check_hit(bx, by);
        }

        // First bounce line judgment
        if (!b->first_bounce_evaluated) {
            b->first_bounce_evaluated = true;
            bool is_in = false;

            if (b->last_shot == SHOT_SERVE) {
                // Service box check
                bool deuce = (b->last_hitter == 0) ? g_tennis.player.side_deuce : g_tennis.opponent.side_deuce;
                if (b->last_hitter == 0) {
                    is_in = is_serve_in_box(bx, by, false, deuce);
                } else {
                    is_in = is_serve_in_box(bx, by, true, deuce);
                }

                if (!is_in) {
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
                // Rally court boundary check
                is_in = is_point_in_court(bx, by);
                if (is_in) {
                    // Check if ball landed on / near line for chalk puff
                    s16 hw = get_court_half_width(by);
                    if (bx <= 120 - hw + 2 || bx >= 120 + hw - 2 || by <= BASELINE_FAR_Y + 2 || by >= BASELINE_NEAR_Y - 2) {
                        trigger_chalk_puff(bx, by);
                    }
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

    // Serve Toss Phase
    if (g_tennis.state == STATE_MATCH_SERVE_WAIT && p->is_serving) {
        if (key_was_pressed(KEY_A) || key_was_pressed(KEY_B)) {
            p->state = ACTOR_STATE_TOSS;
            p->timer = 45;
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

    // Serve Strike Hit Timing
    if (p->state == ACTOR_STATE_TOSS && p->is_serving) {
        if (key_was_pressed(KEY_A) || key_was_pressed(KEY_B)) {
            s16 bz = FP_TO_INT(g_tennis.ball.z);
            if (bz >= 18) {
                p->state = ACTOR_STATE_SERVE_HIT;
                p->timer = 20;

                bool deuce = p->side_deuce;
                s16 target_box_x = deuce ? 98 : 142; // Far service box center
                if (key_is_down(KEY_LEFT))  target_box_x -= 16;
                if (key_is_down(KEY_RIGHT)) target_box_x += 16;
                s16 target_box_y = 66;

                u16 flight = 24;
                u16 mph = 95 + (bz * 2);

                if (bz >= 26) {
                    // MAX POWER ACE!
                    flight = 18;
                    mph = 126 + ((g_tennis.game_frame & 7) * 2);
                    graphics_trigger_shake(3, 14);
                    sfx_play_racket_smash();
                    g_tennis.save_data.total_aces++;
                    set_announcement("POWER ACE!", 60);
                } else {
                    sfx_play_racket_topspin();
                }

                g_tennis.last_serve_mph = mph;
                if (mph > g_tennis.save_data.max_serve_speed) {
                    g_tennis.save_data.max_serve_speed = mph;
                }

                launch_ball(g_tennis.ball.x, g_tennis.ball.y, g_tennis.ball.z,
                            INT_TO_FP(target_box_x), INT_TO_FP(target_box_y),
                            flight, SHOT_SERVE, 0, (mph >= 120 ? 2 : 0));
            }
        }
        return;
    }

    // Movement & Charging Logic
    // Charge detection: if opponent hit the ball, player can hold A (Topspin) or B (Slice)
    if (g_tennis.ball.in_air && g_tennis.ball.last_hitter == 1) {
        if (key_is_down(KEY_A) && key_is_down(KEY_B)) {
            p->is_charging = true;
            p->charge_shot = SHOT_LOB;
            if (p->charge_power < 40) p->charge_power++;
        } else if (key_is_down(KEY_A)) {
            p->is_charging = true;
            p->charge_shot = SHOT_TOPSPIN;
            if (p->charge_power < 40) p->charge_power++;
        } else if (key_is_down(KEY_B)) {
            p->is_charging = true;
            p->charge_shot = SHOT_SLICE;
            if (p->charge_power < 40) p->charge_power++;
        } else {
            p->is_charging = false;
        }
    } else {
        p->is_charging = false;
        p->charge_power = 0;
    }

    // Player court movement speed (charging slows down slightly for precision)
    fixed_t spd = p->is_charging ? (INT_TO_FP(1) + (INT_TO_FP(1) / 3)) : INT_TO_FP(2);
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
    if (cy > BASELINE_NEAR_Y + 12) p->y = INT_TO_FP(BASELINE_NEAR_Y + 12);

    // Animation state
    if (p->vx != 0 || p->vy != 0) {
        p->run_anim++;
        if (p->state == ACTOR_STATE_READY) p->state = ACTOR_STATE_RUN;
    } else {
        if (p->state == ACTOR_STATE_RUN) p->state = ACTOR_STATE_READY;
    }

    // Racket Swing Hit Detection (Generous, responsive strike zone)
    s16 bx = FP_TO_INT(g_tennis.ball.x);
    s16 by = FP_TO_INT(g_tennis.ball.y);
    s16 bz = FP_TO_INT(g_tennis.ball.z);

    bool ball_in_strike_zone = (bx >= cx - 28 && bx <= cx + 28 && by >= cy - 24 && by <= cy + 18 && bz <= 45);

    // Trigger swing if in strike zone and (holding charge OR just pressed A/B)
    if (ball_in_strike_zone && (p->is_charging || key_was_pressed(KEY_A) || key_was_pressed(KEY_B))) {
        bool is_fh = (bx >= cx);
        p->state = is_fh ? ACTOR_STATE_SWING_FH : ACTOR_STATE_SWING_BH;
        p->timer = 18;

        ShotType shot = p->is_charging ? p->charge_shot : (key_is_down(KEY_B) ? SHOT_SLICE : SHOT_TOPSPIN);
        if (key_is_down(KEY_A) && key_is_down(KEY_B)) shot = SHOT_LOB;

        // Overhead smash check: ball is high and player is not deep
        if (bz >= 24 && cy <= BASELINE_NEAR_Y && (shot == SHOT_TOPSPIN || key_was_pressed(KEY_A))) {
            shot = SHOT_SMASH;
            p->state = ACTOR_STATE_SMASH;
        }

        // Drop shot check: Down pressed while hitting slice
        if (shot == SHOT_SLICE && key_is_down(KEY_DOWN)) {
            shot = SHOT_DROP;
        }

        // Steer landing coordinates based on D-Pad Left/Right
        s16 target_y = 42; // Deep far court baseline
        s16 target_x = 120;
        u16 flight = 28;
        u8 trail_type = 0; // Red topspin

        if (shot == SHOT_SMASH) {
            flight = 16;
            target_y = 44;
            target_x = key_is_down(KEY_LEFT) ? 80 : (key_is_down(KEY_RIGHT) ? 160 : 120);
            trail_type = 2; // Yellow
            sfx_play_racket_smash();
            graphics_trigger_shake(4, 15);
        } else if (shot == SHOT_LOB) {
            flight = 46;
            target_y = 38;
            target_x = key_is_down(KEY_LEFT) ? 84 : (key_is_down(KEY_RIGHT) ? 156 : 120);
            trail_type = 1; // Blue
            sfx_play_racket_slice();
        } else if (shot == SHOT_DROP) {
            flight = 36;
            target_y = 68; // Just past net cord
            target_x = key_is_down(KEY_LEFT) ? 90 : (key_is_down(KEY_RIGHT) ? 150 : 120);
            trail_type = 1;
            sfx_play_racket_slice();
        } else if (shot == SHOT_SLICE) {
            flight = 32;
            target_y = 54;
            target_x = key_is_down(KEY_LEFT) ? 82 : (key_is_down(KEY_RIGHT) ? 158 : 120);
            trail_type = 1; // Blue
            sfx_play_racket_slice();
        } else {
            // Topspin Drive
            flight = 28 - (p->charge_power / 4); // Up to 18 frames when fully charged!
            target_y = 42 - (p->charge_power / 8);
            target_x = key_is_down(KEY_LEFT) ? (78 - (p->charge_power > 25 ? 4 : 0)) :
                       (key_is_down(KEY_RIGHT) ? (162 + (p->charge_power > 25 ? 4 : 0)) : 120);
            trail_type = (p->charge_power >= 25) ? 2 : 0;
            sfx_play_racket_topspin();
            if (p->charge_power >= 25) {
                graphics_trigger_shake(2, 8);
            }
        }

        // Off-balance penalty: running at max speed sideways adds slight dispersion
        if (p->vx != 0 && p->charge_power == 0) {
            s8 jitter = (s8)((g_tennis.game_frame & 7) - 3);
            target_x += jitter;
        }

        launch_ball(g_tennis.ball.x, g_tennis.ball.y, g_tennis.ball.z,
                    INT_TO_FP(target_x), INT_TO_FP(target_y),
                    flight, shot, 0, trail_type);

        p->is_charging = false;
        p->charge_power = 0;
        g_tennis.current_rally++;

        if (g_tennis.current_rally == 6) {
            sfx_play_crowd_cheer();
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
            opp->state = ACTOR_STATE_SERVE_HIT;
            opp->timer = 20;

            bool deuce = opp->side_deuce;
            s16 target_x = deuce ? 142 : 98; // Player's near service box
            s16 target_y = 90;

            u16 flight = 24;
            u8 trail = 0;

            if (g_tennis.ai_style == AI_POWER_SERVER) {
                flight = 19;
                trail = 2;
                g_tennis.last_serve_mph = 128;
                sfx_play_racket_smash();
                graphics_trigger_shake(3, 12);
            } else {
                g_tennis.last_serve_mph = 104;
                sfx_play_racket_topspin();
            }

            launch_ball(opp->x + INT_TO_FP(6), opp->y + INT_TO_FP(4), INT_TO_FP(24),
                        INT_TO_FP(target_x), INT_TO_FP(target_y),
                        flight, SHOT_SERVE, 1, trail);

            if (g_tennis.ai_style == AI_VOLLEYER) {
                opp->ai_net_rushing = true;
            }
            g_tennis.state = STATE_MATCH_RALLY;
        }
        return;
    }

    // AI Predictive Interception & Positioning
    fixed_t target_x = opp->x;
    fixed_t target_y = INT_TO_FP(BASELINE_FAR_Y + 4);

    if (g_tennis.ai_style == AI_VOLLEYER && opp->ai_net_rushing) {
        // Sato rushes net!
        target_y = INT_TO_FP(NET_Y - 16);
        // But if player lobbed over him, retreat in panic!
        if (g_tennis.ball.last_shot == SHOT_LOB && g_tennis.ball.z > INT_TO_FP(35)) {
            opp->ai_net_rushing = false;
            target_y = INT_TO_FP(BASELINE_FAR_Y + 4);
        }
    }

    // Calculate landing intercept when ball is inbound
    if (g_tennis.ball.last_hitter == 0 && g_tennis.ball.in_air) {
        target_x = g_tennis.ball.target_x;
        if (!opp->ai_net_rushing) {
            target_y = g_tennis.ball.target_y + INT_TO_FP(2);
        }
    }

    fixed_t dx = target_x - opp->x;
    fixed_t dy = target_y - opp->y;
    fixed_t ai_spd = (g_tennis.ai_style == AI_BASELINER) ? (INT_TO_FP(1) + (INT_TO_FP(3) / 4)) :
                     (INT_TO_FP(1) + (INT_TO_FP(1) / 2));

    if (dx > INT_TO_FP(3))  opp->x += ai_spd;
    else if (dx < -INT_TO_FP(3)) opp->x -= ai_spd;

    if (dy > INT_TO_FP(3))  opp->y += ai_spd;
    else if (dy < -INT_TO_FP(3)) opp->y -= ai_spd;

    // AI Strike Contact
    s16 ox = FP_TO_INT(opp->x);
    s16 oy = FP_TO_INT(opp->y);
    s16 bx = FP_TO_INT(g_tennis.ball.x);
    s16 by = FP_TO_INT(g_tennis.ball.y);
    s16 bz = FP_TO_INT(g_tennis.ball.z);

    if (g_tennis.ball.vy < 0 && bx >= ox - 26 && bx <= ox + 26 && by >= oy - 18 && by <= oy + 18 && bz <= 40) {
        bool is_fh = (bx >= ox);
        opp->state = is_fh ? ACTOR_STATE_SWING_FH : ACTOR_STATE_SWING_BH;
        opp->timer = 18;

        s16 target_x_ret = (g_tennis.player.x > opp->x) ? 75 : 165; // Aim across court
        s16 target_y_ret = 128;
        u16 flight = 28;
        u8 trail = 0;
        ShotType shot = SHOT_TOPSPIN;

        if (g_tennis.ai_style == AI_POWER_SERVER) {
            // Marcus Thorne blasts heavy drives
            flight = 21;
            target_y_ret = 132;
            trail = 2;
            sfx_play_racket_smash();
            graphics_trigger_shake(2, 8);
        } else if (g_tennis.ai_style == AI_VOLLEYER && opp->ai_net_rushing) {
            // Kenji Sato volleys sharply
            flight = 24;
            target_y_ret = 110;
            trail = 1;
            shot = SHOT_SLICE;
            sfx_play_racket_slice();
        } else {
            // Leo Vance ultra-reliable topspins
            flight = 30;
            target_y_ret = 134;
            sfx_play_racket_topspin();
        }

        // Unforced error check for Thorne when stretched wide
        if (g_tennis.ai_style == AI_POWER_SERVER && (bx < 80 || bx > 160)) {
            if ((g_tennis.game_frame & 3) == 0) {
                target_x_ret = (target_x_ret < 120) ? 35 : 205; // Hits wide!
            }
        }

        launch_ball(g_tennis.ball.x, g_tennis.ball.y, g_tennis.ball.z,
                    INT_TO_FP(target_x_ret), INT_TO_FP(target_y_ret),
                    flight, shot, 1, trail);

        g_tennis.current_rally++;
    }
}

static void update_target_practice(void) {
    if (g_tennis.target_timer > 0) {
        g_tennis.target_timer--;
    } else {
        // Target Practice Over!
        set_announcement("DRILL COMPLETE!", 120);
        if (g_tennis.target_score > g_tennis.save_data.target_high_score) {
            g_tennis.save_data.target_high_score = g_tennis.target_score;
            save_write(&g_tennis.save_data);
            set_announcement("NEW RECORD SCORE!", 120);
        }
        if (key_was_pressed(KEY_A) || key_was_pressed(KEY_START)) {
            g_tennis.state = STATE_TITLE;
            audio_play_bgm(BGM_MENU);
        }
        return;
    }

    targets_update();

    // Ball machine feed timer
    if (!g_tennis.ball.in_air) {
        if (g_tennis.ball_machine_timer > 0) {
            g_tennis.ball_machine_timer--;
        } else {
            // Fire ball from machine at far court
            g_tennis.ball_machine_timer = 110;
            s16 target_px = FP_TO_INT(g_tennis.player.x) + ((s16)(g_tennis.game_frame & 15) - 8);
            if (target_px < 60) target_px = 60;
            if (target_px > 180) target_px = 180;

            launch_ball(INT_TO_FP(120), INT_TO_FP(BASELINE_FAR_Y), INT_TO_FP(20),
                        INT_TO_FP(target_px), INT_TO_FP(BASELINE_NEAR_Y - 4),
                        32, SHOT_TOPSPIN, 1, 0);
            sfx_play_racket_slice();
        }
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
            g_tennis.state = STATE_MODE_SELECT;
            sfx_play_racket_topspin();
        }
        return;
    }

    if (g_tennis.state == STATE_MODE_SELECT) {
        if (key_was_pressed(KEY_UP)) {
            if (g_tennis.menu_mode > 0) g_tennis.menu_mode--;
            sfx_play_racket_slice();
        }
        if (key_was_pressed(KEY_DOWN)) {
            if (g_tennis.menu_mode < 2) g_tennis.menu_mode++;
            sfx_play_racket_slice();
        }
        if (key_was_pressed(KEY_A) || key_was_pressed(KEY_START)) {
            sfx_play_racket_topspin();
            if (g_tennis.menu_mode == 0) {
                // Grand Slam Tournament -> select surface
                g_tennis.state = STATE_SURFACE_SELECT;
            } else if (g_tennis.menu_mode == 1) {
                // Exhibition Match -> setup
                g_tennis.state = STATE_EXHIBITION_SETUP;
                g_tennis.menu_setup_step = 0;
            } else {
                // Target Practice Drill
                game_start_target_practice();
            }
        }
        if (key_was_pressed(KEY_B)) {
            g_tennis.state = STATE_TITLE;
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
            g_tennis.tournament_round = 0;
            game_start_match(g_tennis.surface, AI_BASELINER, MODE_TOURNAMENT);
        }
        if (key_was_pressed(KEY_B)) {
            g_tennis.state = STATE_MODE_SELECT;
        }
        return;
    }

    if (g_tennis.state == STATE_EXHIBITION_SETUP) {
        if (g_tennis.menu_setup_step == 0) {
            // Select Surface
            if (key_was_pressed(KEY_LEFT) && g_tennis.menu_surface > 0) { g_tennis.menu_surface--; sfx_play_racket_slice(); }
            if (key_was_pressed(KEY_RIGHT) && g_tennis.menu_surface < 2) { g_tennis.menu_surface++; sfx_play_racket_slice(); }
            if (key_was_pressed(KEY_A) || key_was_pressed(KEY_START)) {
                g_tennis.menu_setup_step = 1; // Move to opponent select
                sfx_play_racket_topspin();
            }
            if (key_was_pressed(KEY_B)) g_tennis.state = STATE_MODE_SELECT;
        } else {
            // Select Opponent
            if (key_was_pressed(KEY_LEFT) && g_tennis.menu_opp > 0) { g_tennis.menu_opp--; sfx_play_racket_slice(); }
            if (key_was_pressed(KEY_RIGHT) && g_tennis.menu_opp < 2) { g_tennis.menu_opp++; sfx_play_racket_slice(); }
            if (key_was_pressed(KEY_A) || key_was_pressed(KEY_START)) {
                game_start_match((CourtSurface)g_tennis.menu_surface, (AiStyle)g_tennis.menu_opp, MODE_EXHIBITION);
            }
            if (key_was_pressed(KEY_B)) g_tennis.menu_setup_step = 0;
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
            if (g_tennis.mode == MODE_TOURNAMENT && g_tennis.player_games >= 3 && g_tennis.tournament_round <= 2 && g_tennis.state != STATE_TROPHY_CEREMONY) {
                // Next tournament round!
                AiStyle next_ai = (g_tennis.tournament_round == 1) ? AI_VOLLEYER : AI_POWER_SERVER;
                game_start_match(g_tennis.surface, next_ai, MODE_TOURNAMENT);
            } else {
                g_tennis.state = STATE_TITLE;
                audio_play_bgm(BGM_MENU);
            }
        }
        return;
    }

    if (g_tennis.state == STATE_TARGET_PRACTICE) {
        update_player();
        update_ball_physics();
        update_target_practice();
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
        graphics_draw_box(1, 1, 28, 18, PAL_BG_SCOREBOARD);
        graphics_print_text(3, 3,  "GRAND SLAM: TENNIS ADVANCE", PAL_BG_SCOREBOARD);
        graphics_print_text(3, 5,  "PRESS START OR A TO PLAY", PAL_BG_SCOREBOARD);

        graphics_print_text(3, 7,  "CAREER RECORDS:", PAL_BG_SCOREBOARD);
        graphics_print_text(3, 9,  "MATCHES WON:  ", PAL_BG_SCOREBOARD);
        graphics_print_num(17, 9,  g_tennis.save_data.matches_won, 3, PAL_BG_SCOREBOARD);
        graphics_print_text(3, 10, "TOTAL ACES:   ", PAL_BG_SCOREBOARD);
        graphics_print_num(17, 10, g_tennis.save_data.total_aces, 3, PAL_BG_SCOREBOARD);
        graphics_print_text(3, 11, "MAX SERVE:    ", PAL_BG_SCOREBOARD);
        graphics_print_num(17, 11, g_tennis.save_data.max_serve_speed, 3, PAL_BG_SCOREBOARD);
        graphics_print_text(21, 11, "MPH", PAL_BG_SCOREBOARD);
        graphics_print_text(3, 12, "MAX RALLY:    ", PAL_BG_SCOREBOARD);
        graphics_print_num(17, 12, g_tennis.save_data.longest_rally, 3, PAL_BG_SCOREBOARD);
        graphics_print_text(3, 13, "TARGET HIGH:  ", PAL_BG_SCOREBOARD);
        graphics_print_num(17, 13, g_tennis.save_data.target_high_score, 5, PAL_BG_SCOREBOARD);

        graphics_print_text(3, 15, "CUPS:", PAL_BG_SCOREBOARD);
        graphics_print_text(9, 15, g_tennis.save_data.trophies[0] ? "[G]" : "[ ]", PAL_BG_SCOREBOARD);
        graphics_print_text(13, 15, g_tennis.save_data.trophies[1] ? "[C]" : "[ ]", PAL_BG_SCOREBOARD);
        graphics_print_text(17, 15, g_tennis.save_data.trophies[2] ? "[H]" : "[ ]", PAL_BG_SCOREBOARD);

        graphics_oam_copy();
        return;
    }

    if (g_tennis.state == STATE_MODE_SELECT) {
        graphics_clear_bg0();
        graphics_draw_box(2, 2, 26, 16, PAL_BG_SCOREBOARD);
        graphics_print_text(5, 4, "SELECT GAME MODE", PAL_BG_SCOREBOARD);

        graphics_print_text(5, 7,  (g_tennis.menu_mode == 0) ? "> 1. GRAND SLAM CUP" : "  1. GRAND SLAM CUP", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 9,  (g_tennis.menu_mode == 1) ? "> 2. EXHIBITION MATCH" : "  2. EXHIBITION MATCH", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 11, (g_tennis.menu_mode == 2) ? "> 3. TARGET DRILL" : "  3. TARGET DRILL", PAL_BG_SCOREBOARD);

        graphics_print_text(5, 14, "PRESS A TO SELECT", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 15, "PRESS B TO GO BACK", PAL_BG_SCOREBOARD);
        graphics_oam_copy();
        return;
    }

    if (g_tennis.state == STATE_SURFACE_SELECT) {
        graphics_clear_bg0();
        graphics_draw_box(3, 3, 24, 14, PAL_BG_SCOREBOARD);
        graphics_print_text(5, 5,  "GRAND SLAM TOURNAMENT", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 7,  "SELECT COURT SURFACE:", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 9,  "<  ", PAL_BG_SCOREBOARD);
        graphics_print_text(8, 9,  s_surface_names[g_tennis.surface], PAL_BG_SCOREBOARD);
        graphics_print_text(15, 9, "  >", PAL_BG_SCOREBOARD);

        graphics_print_text(5, 11, s_round_names[0], PAL_BG_SCOREBOARD);
        graphics_print_text(5, 12, "VS LEO VANCE", PAL_BG_SCOREBOARD);
        graphics_print_text(5, 14, "PRESS A TO START", PAL_BG_SCOREBOARD);
        graphics_oam_copy();
        return;
    }

    if (g_tennis.state == STATE_EXHIBITION_SETUP) {
        graphics_clear_bg0();
        graphics_draw_box(2, 2, 26, 16, PAL_BG_SCOREBOARD);
        graphics_print_text(5, 4, "EXHIBITION SETUP", PAL_BG_SCOREBOARD);

        graphics_print_text(4, 7, (g_tennis.menu_setup_step == 0) ? "> SURFACE: < " : "  SURFACE:   ", PAL_BG_SCOREBOARD);
        graphics_print_text(17, 7, s_surface_names[g_tennis.menu_surface], PAL_BG_SCOREBOARD);
        if (g_tennis.menu_setup_step == 0) graphics_print_text(22, 7, " >", PAL_BG_SCOREBOARD);

        graphics_print_text(4, 9, (g_tennis.menu_setup_step == 1) ? "> RIVAL:   < " : "  RIVAL:     ", PAL_BG_SCOREBOARD);
        graphics_print_text(17, 9, s_ai_names[g_tennis.menu_opp], PAL_BG_SCOREBOARD);
        if (g_tennis.menu_setup_step == 1) graphics_print_text(25, 9, " >", PAL_BG_SCOREBOARD);

        graphics_print_text(4, 12, s_ai_titles[g_tennis.menu_opp], PAL_BG_SCOREBOARD);
        graphics_print_text(4, 15, "PRESS A: NEXT / PLAY", PAL_BG_SCOREBOARD);
        graphics_oam_copy();
        return;
    }

    // Render Scoreboard HUD on BG0
    graphics_clear_bg0();

    if (g_tennis.state == STATE_TARGET_PRACTICE) {
        graphics_draw_box(0, 0, 30, 3, PAL_BG_SCOREBOARD);
        graphics_print_text(1, 1, "SCORE", PAL_BG_SCOREBOARD);
        graphics_print_num(7, 1, g_tennis.target_score, 6, PAL_BG_SCOREBOARD);

        graphics_print_text(15, 1, "COMBO", PAL_BG_SCOREBOARD);
        graphics_print_num(21, 1, g_tennis.target_combo, 2, PAL_BG_SCOREBOARD);

        graphics_print_text(24, 1, "T:", PAL_BG_SCOREBOARD);
        graphics_print_num(26, 1, g_tennis.target_timer / 60, 2, PAL_BG_SCOREBOARD);
    } else {
        // Standard Match Scoreboard
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
    }

    // Announcements
    if (g_tennis.announcement_timer > 0 && g_tennis.announcement_str) {
        graphics_draw_box(3, 7, 24, 4, PAL_BG_SCOREBOARD);
        graphics_print_text(5, 8, g_tennis.announcement_str, PAL_BG_SCOREBOARD);
    }

    // Trophy Ceremony Modal
    if (g_tennis.state == STATE_TROPHY_CEREMONY) {
        graphics_draw_box(2, 5, 26, 8, PAL_BG_SCOREBOARD);
        graphics_print_text(4, 6, "GRAND SLAM CHAMPION!", PAL_BG_SCOREBOARD);
        graphics_print_text(4, 8, "TROPHY ADDED TO CABINET!", PAL_BG_SCOREBOARD);
        graphics_print_text(4, 10, "PRESS START TO EXIT", PAL_BG_SCOREBOARD);
    }

    // Render Sprites (OAM)
    u8 sprite_idx = 0;

    // 1. Far Opponent
    if (g_tennis.state != STATE_TARGET_PRACTICE) {
        s16 ox = FP_TO_INT(g_tennis.opponent.x);
        s16 oy = FP_TO_INT(g_tennis.opponent.y);
        u16 opp_tile = SPRITE_OPP_READY_0;
        if (g_tennis.opponent.state == ACTOR_STATE_SWING_FH) opp_tile = SPRITE_OPP_SWING_FH;
        if (g_tennis.opponent.state == ACTOR_STATE_SWING_BH) opp_tile = SPRITE_OPP_SWING_BH;
        if (g_tennis.opponent.state == ACTOR_STATE_RUN) {
            opp_tile = (g_tennis.game_frame & 8) ? SPRITE_OPP_RUN_R : SPRITE_OPP_RUN_L;
        }
        graphics_set_sprite(sprite_idx++, ox - 8, oy - 14, opp_tile, 0, 1, PAL_OBJ_OPPONENT, false, false);
    }

    // 2. Target Rings in Target Practice
    if (g_tennis.state == STATE_TARGET_PRACTICE) {
        for (int i = 0; i < MAX_TARGETS; i++) {
            if (g_tennis.targets[i].active) {
                graphics_set_sprite(sprite_idx++, g_tennis.targets[i].x - 8, g_tennis.targets[i].y - 8,
                                    SPRITE_TARGET_RING, 0, 1, PAL_OBJ_VFX, false, false);
            }
        }
    }

    // 3. Ball Drop Shadow on Court Ground (Scaled subtly with altitude)
    if (g_tennis.ball.in_air || g_tennis.state == STATE_MATCH_SERVE_WAIT) {
        s16 bx = FP_TO_INT(g_tennis.ball.x);
        s16 by = FP_TO_INT(g_tennis.ball.y);
        graphics_set_sprite(sprite_idx++, bx - 8, by - 4, SPRITE_BALL_SHADOW, 0, 1, PAL_OBJ_BALL, false, false);
    }

    // 4. Tennis Ball (Elevated by Z altitude)
    if (g_tennis.ball.in_air || g_tennis.state == STATE_MATCH_SERVE_WAIT) {
        s16 bx = FP_TO_INT(g_tennis.ball.x);
        s16 by = FP_TO_INT(g_tennis.ball.y) - FP_TO_INT(g_tennis.ball.z);

        u16 ball_tile = SPRITE_BALL_MID;
        if (g_tennis.ball.z > INT_TO_FP(26) || g_tennis.ball.y < INT_TO_FP(60)) ball_tile = SPRITE_BALL_SMALL;
        if (g_tennis.ball.y > INT_TO_FP(115) && g_tennis.ball.z < INT_TO_FP(16)) ball_tile = SPRITE_BALL_LARGE;

        graphics_set_sprite(sprite_idx++, bx - 8, by - 8, ball_tile, 0, 1, PAL_OBJ_BALL, false, false);

        // Ball Trail VFX
        if (g_tennis.ball.trail_timer > 0) {
            u16 trail_tile = SPRITE_SWING_TRAIL_RED;
            if (g_tennis.ball.trail_type == 1) trail_tile = SPRITE_SWING_TRAIL_BLUE;
            if (g_tennis.ball.trail_type == 2) trail_tile = SPRITE_SWING_TRAIL_YEL;
            graphics_set_sprite(sprite_idx++, bx - 8, by + 4, trail_tile, 0, 1, PAL_OBJ_VFX, false, false);
        }
    }

    // 5. Chalk Puff VFX on Line Bounce
    if (g_tennis.chalk.timer > 0) {
        graphics_set_sprite(sprite_idx++, g_tennis.chalk.x - 8, g_tennis.chalk.y - 8,
                            SPRITE_CHALK_PUFF, 0, 1, PAL_OBJ_BALL, false, false);
    }

    // 6. Near Player & Charge Aura
    s16 px = FP_TO_INT(g_tennis.player.x);
    s16 py = FP_TO_INT(g_tennis.player.y);

    // Charge aura pulsing sprite
    if (g_tennis.player.is_charging && (g_tennis.game_frame & 2)) {
        u16 aura_tile = (g_tennis.player.charge_shot == SHOT_SLICE) ? SPRITE_SWING_TRAIL_BLUE : SPRITE_SWING_TRAIL_RED;
        if (g_tennis.player.charge_power >= 30) aura_tile = SPRITE_SWING_TRAIL_YEL;
        graphics_set_sprite(sprite_idx++, px - 8, py - 6, aura_tile, 0, 1, PAL_OBJ_VFX, false, false);
    }

    // Player body
    u16 player_tile = SPRITE_PLAYER_READY_0;
    if (g_tennis.player.state == ACTOR_STATE_RUN) {
        player_tile = ((g_tennis.player.run_anim >> 3) & 1) ? SPRITE_PLAYER_RUN_R : SPRITE_PLAYER_RUN_L;
    }
    if (g_tennis.player.state == ACTOR_STATE_SWING_FH)  player_tile = SPRITE_PLAYER_SWING_FH_0;
    if (g_tennis.player.state == ACTOR_STATE_SWING_BH)  player_tile = SPRITE_PLAYER_SWING_BH_0;
    if (g_tennis.player.state == ACTOR_STATE_TOSS)      player_tile = SPRITE_PLAYER_TOSS;
    if (g_tennis.player.state == ACTOR_STATE_SERVE_HIT) player_tile = SPRITE_PLAYER_SERVE_HIT;
    if (g_tennis.player.state == ACTOR_STATE_SMASH)     player_tile = SPRITE_PLAYER_SMASH;

    graphics_set_sprite(sprite_idx++, px - 8, py - 14, player_tile, 0, 1, PAL_OBJ_PLAYER, false, false);

    // 7. Gold Trophy in Ceremony
    if (g_tennis.state == STATE_TROPHY_CEREMONY) {
        graphics_set_sprite(sprite_idx++, 120 - 8, 48, SPRITE_TROPHY_GOLD, 0, 1, PAL_OBJ_TROPHY, false, false);
    }

    graphics_oam_copy();
}
