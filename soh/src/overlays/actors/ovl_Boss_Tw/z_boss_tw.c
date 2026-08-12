#include "z_boss_tw.h"
#include "objects/gameplay_keep/gameplay_keep.h"
#include "textures/boss_title_cards/object_tw.h"
#include "objects/object_tw/object_tw.h"
#include "objects/object_fhg/object_fhg.h"
#include "overlays/actors/ovl_Door_Warp1/z_door_warp1.h"
#include "overlays/actors/ovl_En_Bb/z_en_bb.h"
#include "overlays/actors/ovl_En_Firefly/z_en_firefly.h"
#include "overlays/actors/ovl_En_Wf/z_en_wf.h"
#include "soh/ShipUtils.h"
#include "soh/frame_interpolation.h"
#include "soh/Enhancements/game-interactor/GameInteractor.h"
#include "soh/Enhancements/game-interactor/GameInteractor_Hooks.h"
#include "soh/Enhancements/savestate_serialize.h"

#include <string.h>

#define FLAGS                                                                                 \
    (ACTOR_FLAG_ATTENTION_ENABLED | ACTOR_FLAG_HOSTILE | ACTOR_FLAG_UPDATE_CULLING_DISABLED | \
     ACTOR_FLAG_DRAW_CULLING_DISABLED)

#define TWINROVA_PHASE_ONE_VOLLEY_CHANCE 0.4f
#define TWINROVA_PHASE_ONE_VOLLEY_SHOTS 3
#define TWINROVA_PHASE_ONE_VOLLEY_INTERVAL 12
#define TWINROVA_PHASE_ONE_DOUBLE_VOLLEY_DELAY 6
#define TWINROVA_PHASE_ONE_DUAL_VOLLEY_CHANCE 0.35f
#define TWINROVA_PHASE_ONE_DUAL_VOLLEY_SUMMON_LIMIT 2
#define TWINROVA_PHASE_ONE_VOLLEY_RECAST_TELL_TIME 8
#define TWINROVA_PHASE_ONE_CROWDED_VOLLEY_SHOTS 2
#define TWINROVA_PHASE_ONE_VOLLEY_IMPACT_EFFECTS 15
#define TWINROVA_PHASE_ONE_VOLLEY_SPAWN_EFFECTS 12
#define TWINROVA_PHASE_ONE_ESCALATION_HITS 2
#define TWINROVA_PHASE_ONE_TURN_TIME 40
#define TWINROVA_PHASE_ONE_TRAVEL_BASE 50
#define TWINROVA_PHASE_ONE_TRAVEL_RANGE 50
#define TWINROVA_PHASE_ONE_ESCALATED_TRAVEL_BASE 40
#define TWINROVA_PHASE_ONE_ESCALATED_TRAVEL_RANGE 40
#define TWINROVA_PHASE_ONE_PILLAR_TIME 200
#define TWINROVA_PHASE_ONE_ESCALATED_PILLAR_TIME 160
#define TWINROVA_PHASE_ONE_POST_ATTACK_SPEED 1.5f
#define TWINROVA_PHASE_ONE_COMBO_RESET_TIME 12
#define TWINROVA_PHASE_ONE_COMBO_IDLE 0
#define TWINROVA_PHASE_ONE_COMBO_ARMED 1
#define TWINROVA_PHASE_ONE_COMBO_CONSUMED 2
#define TWINROVA_PHASE_ONE_POSITION_ATTEMPTS 8
#define TWINROVA_PHASE_ONE_TARGET_SEPARATION 260.0f
#define TWINROVA_PHASE_ONE_BODY_SEPARATION 150.0f
#define TWINROVA_PHASE_ONE_SEPARATION_HEIGHT 140.0f
#define TWINROVA_PHASE_ONE_SEPARATION_STEP 12.0f
#define TWINROVA_SPIN_TELL_TIME 16
#define TWINROVA_SPIN_ACTIVE_TIME 20
#define TWINROVA_FUSED_SPIN_RECOVERY_TIME 12
#define TWINROVA_SISTER_SPIN_RETRIGGER_COOLDOWN 80
#define TWINROVA_FUSED_SPIN_RETRIGGER_COOLDOWN 90
#define TWINROVA_SPIN_TRIGGER_RANGE 120.0f
#define TWINROVA_SPIN_TRIGGER_HEIGHT 200.0f
#define TWINROVA_SPIN_COLLIDER_HEIGHT 230
#define TWINROVA_SPIN_COLLIDER_Y_SHIFT -140
#define TWINROVA_PILLAR_DIVE_TRACK_TIME 24
#define TWINROVA_PILLAR_DIVE_MIN_HEIGHT 80.0f
#define TWINROVA_PILLAR_DIVE_MAX_HEIGHT 450.0f
#define TWINROVA_PILLAR_DIVE_TIME 10
#define TWINROVA_PILLAR_DIVE_SPIN_TIME 14
#define TWINROVA_PILLAR_DIVE_PASS_DISTANCE 200.0f
#define TWINROVA_PILLAR_DIVE_LOW_HOVER_HEIGHT 70.0f
#define TWINROVA_PILLAR_DIVE_VOLLEY_SHOTS 2
#define TWINROVA_PILLAR_DIVE_VOLLEY_TELL_TIME 3
#define TWINROVA_PILLAR_DIVE_VOLLEY_INTERVAL 8
#define TWINROVA_VOLLEY_MODE_PILLAR_DIVE -4
#define TWINROVA_CURVE_STEER_FRAMES 18
#define TWINROVA_CURVE_YAW_OFFSET 0x600
#define TWINROVA_CURVE_YAW_STEP 0x100
#define TWINROVA_CURVE_PITCH_STEP 0xC0
#define TWINROVA_PHASE_ONE_SPECIAL_CHANCE 0.55f
#define TWINROVA_ESCALATED_PHASE_ONE_SPECIAL_CHANCE 0.70f
#define TWINROVA_SPIRAL_RADIUS 650.0f
#define TWINROVA_SPIRAL_HEIGHT 440.0f
#define TWINROVA_SPIRAL_ANGULAR_SPEED 0x100
#define TWINROVA_SPIRAL_FORMATION_TIMEOUT 120
#define TWINROVA_SPIRAL_FORMATION_DISTANCE 40.0f
#define TWINROVA_SPIRAL_WINDUP 40
#define TWINROVA_SPIRAL_VOLLEYS 4
#define TWINROVA_SPIRAL_VOLLEY_INTERVAL 16
#define TWINROVA_SPIRAL_LANDING_TIME 32
#define TWINROVA_SPIRAL_FOLLOWUP_RECOVERY 12
#define TWINROVA_SPIRAL_SIDE_PLATFORM_OFFSET 600.0f
#define TWINROVA_RAIN_FLIGHT_FRAMES 64.0f
#define TWINROVA_RAIN_GRAVITY -0.65f
#define TWINROVA_RAIN_SPAWN_EFFECTS 6
#define TWINROVA_RAIN_IMPACT_EFFECTS 6
#define TWINROVA_RAIN_SIGIL_SCALE 0.045f
#define TWINROVA_RAIN_IMPACT_RADIUS 90
#define TWINROVA_RAIN_PLAYER_LEAD_FRAMES 6.0f
#define TWINROVA_RAIN_MAX_PLAYER_LEAD 75.0f
#define TWINROVA_RAIN_CHASER_OFFSET 60.0f
#define TWINROVA_RAIN_CUTOFF_OFFSET 150.0f
#define TWINROVA_RAIN_SIDE_CHASER_OFFSET 30.0f
#define TWINROVA_RAIN_SIDE_CUTOFF_OFFSET 160.0f
#define TWINROVA_RAIN_CENTRAL_PLATFORM_LIMIT 280.0f
#define TWINROVA_RAIN_SIDE_PLATFORM_LIMIT 100.0f
#define TWINROVA_RAIN_LOWER_ANCHOR_RADIUS 750.0f
#define TWINROVA_RAIN_LOWER_DIAGONAL_ANCHOR 470.0f
#define TWINROVA_RAIN_TELEGRAPH_POOL_ALPHA_SCALE 0.70f
#define TWINROVA_RAIN_SIGIL_FINAL_PULSE_TIME 14
#define TWINROVA_RAIN_MARKER_PARENT_PADDING 3
#define TWINROVA_RAIN_MARKER_FINAL_PULSE_TIME \
    (TWINROVA_RAIN_SIGIL_FINAL_PULSE_TIME + TWINROVA_RAIN_MARKER_PARENT_PADDING)
#define TWINROVA_PORTAL_VANISH_TIME 16
#define TWINROVA_PORTAL_HIDDEN_TIME 18
#define TWINROVA_PORTAL_APPEAR_TIME 16
#define TWINROVA_PORTAL_AMBUSH_CHANCE 0.65f
#define TWINROVA_ESCALATED_PORTAL_AMBUSH_CHANCE 0.8f
#define TWINROVA_PORTAL_AMBUSH_VANISH_TIME 10
#define TWINROVA_PORTAL_AMBUSH_HIDDEN_TIME 10
#define TWINROVA_PORTAL_AMBUSH_APPEAR_TIME 10
#define TWINROVA_PORTAL_AMBUSH_TELL_TIME 5
#define TWINROVA_PORTAL_AMBUSH_VOLLEY_INTERVAL 8
#define TWINROVA_PORTAL_AMBUSH_RECAST_TELL_TIME 4
#define TWINROVA_PORTAL_AMBUSH_COOLDOWN 140
#define TWINROVA_ESCALATED_PORTAL_AMBUSH_COOLDOWN 110
#define TWINROVA_PORTAL_MODE_REPOSITION 0
#define TWINROVA_PORTAL_MODE_AMBUSH 1
#define TWINROVA_PORTAL_MODE_PRESSURE_TRIANGLE 2
#define TWINROVA_PORTAL_MODE_MASK 0xF
#define TWINROVA_PORTAL_FAKE_OUT_USED 0x10
#define TWINROVA_PORTAL_FAKE_OUT_CHANCE 0.35f
#define TWINROVA_ESCALATED_PORTAL_FAKE_OUT_CHANCE 0.5f
#define TWINROVA_VOLLEY_MODE_AMBUSH -1
#define TWINROVA_VOLLEY_MODE_CROSSFIRE -2
#define TWINROVA_VOLLEY_MODE_PRESSURE_TRIANGLE -3
#define TWINROVA_PRESSURE_TRIANGLE_TELL_TIME 3
#define TWINROVA_PRESSURE_TRIANGLE_VOLLEY_INTERVAL 8
#define TWINROVA_PRESSURE_TRIANGLE_RECAST_TELL_TIME 4
#define TWINROVA_CROSSFIRE_FORMATION_TIME 130
#define TWINROVA_CROSSFIRE_OPENING_TELL_TIME 5
#define TWINROVA_CROSSFIRE_ELEMENT_STAGGER 10
#define TWINROVA_CROSSFIRE_VOLLEY_INTERVAL 20
#define TWINROVA_CROSSFIRE_RECAST_TELL_TIME 4
#define TWINROVA_SPECIAL_MOVE_COOLDOWN 240
#define TWINROVA_ESCALATED_SPECIAL_MOVE_COOLDOWN 200
#define TWINROVA_PHASE_ONE_SPECIAL_PITY_MISSES 2
#define TWINROVA_PHASE_ONE_REGULAR_REPEAT_LIMIT 2
#define TWINROVA_SUMMON_SOFT_LIMIT 4
#define TWINROVA_FUSED_SUMMON_SOFT_LIMIT 6
#define TWINROVA_SUMMON_MAX_GROUP_SIZE 3
#define TWINROVA_BEAM_SUMMON_LIMIT 5
#define TWINROVA_SUMMON_REGISTRY_SIZE \
    (TWINROVA_FUSED_SUMMON_SOFT_LIMIT + TWINROVA_SUMMON_MAX_GROUP_SIZE)
#define TWINROVA_SUMMON_OFFSCREEN_DESPAWN_DELAY 30
#define TWINROVA_SUMMON_EMERGENCE_DELAY 34
#define TWINROVA_SUMMON_WARNING_TIME 30
#define TWINROVA_SUMMON_FORMATION_RADIUS 70.0f
#define TWINROVA_SUMMON_PENDING 1
#define TWINROVA_UPPER_FLOOR_MIN_Y 190.0f
#define TWINROVA_SUMMON_FRUSTUM_MARGIN 1.1f
#define TWINROVA_ATTACK_FRUSTUM_MARGIN 0.9f
#define TWINROVA_BREAKER_SEQUENCE_COOLDOWN 180
#define TWINROVA_FINAL_CYCLE_HEALTH 12
#define TWINROVA_FINAL_BREAKER_SEQUENCE_COOLDOWN 150
#define TWINROVA_BREAKER_SEQUENCE_SHOTS 6
#define TWINROVA_BREAKER_SHOT_INDEX 2
#define TWINROVA_BREAKER_TELL_TIME 30
#define TWINROVA_BREAKER_SHOT_GAP 8
#define TWINROVA_BREAKER_RECOVERY_GAP 16
#define TWINROVA_BREAKER_PROJECTILE_LIFETIME 90
#define TWINROVA_SIGNATURE_PROJECTILE_MAX_LIFETIME 120
#define TWINROVA_DIRECT_BLAST_SPEED 20.0f
#define TWINROVA_BREAKER_TRAIL_INTERVAL 6
#define TWINROVA_CYCLONE_COOLDOWN 210
#define TWINROVA_FINAL_CYCLONE_COOLDOWN 170
#define TWINROVA_CYCLONE_WINDUP_TIME 24
#define TWINROVA_CYCLONE_ACTIVE_TIME 176
#define TWINROVA_CYCLONE_RECOVERY_TIME 12
#define TWINROVA_CYCLONE_ORBIT_RADIUS 580.0f
#define TWINROVA_CYCLONE_FLIGHT_HEIGHT 480.0f
#define TWINROVA_CYCLONE_ANGULAR_SPEED 0x200
#define TWINROVA_CYCLONE_TILT 0x1800
#define TWINROVA_CYCLONE_SIGIL_HEIGHT 900.0f
#define TWINROVA_CYCLONE_SIGIL_SCALE 0.48f
#define TWINROVA_CYCLONE_RAIN_OPENING_DELAY 8
#define TWINROVA_CYCLONE_RAIN_INTERVAL 5
#define TWINROVA_FINAL_CYCLONE_RAIN_INTERVAL 4
#define TWINROVA_CYCLONE_RAIN_STOP_TIME ((s16)TWINROVA_RAIN_FLIGHT_FRAMES + 8)
#define TWINROVA_CYCLONE_RAIN_INITIAL_FALL_SPEED -2.0f
#define TWINROVA_CYCLONE_RAIN_PROJECTILE_SCALE 0.018f
#define TWINROVA_CYCLONE_RAIN_FLIGHT_RADIUS 15
#define TWINROVA_CYCLONE_RAIN_FLIGHT_HEIGHT 24
#define TWINROVA_CYCLONE_RAIN_FLIGHT_Y_SHIFT -12
#define TWINROVA_CYCLONE_RAIN_SIGIL_SCALE 0.03f
#define TWINROVA_CYCLONE_RAIN_IMPACT_RADIUS 60
#define TWINROVA_CYCLONE_RAIN_IMPACT_EFFECTS 3
#define TWINROVA_CYCLONE_RAIN_SPAWN_EFFECTS 2
#define TWINROVA_CYCLONE_RAIN_MAX_PROJECTILES 32
#define TWINROVA_FINAL_CYCLONE_RAIN_MAX_PROJECTILES 40
#define TWINROVA_FALSE_CHARGE_COOLDOWN 180
#define TWINROVA_FINAL_FALSE_CHARGE_COOLDOWN 150
#define TWINROVA_FALSE_CHARGE_REPOSITION_TIME 8
#define TWINROVA_FALSE_CHARGE_MOVE_STEP 140.0f
#define TWINROVA_MINEFIELD_COOLDOWN 360
#define TWINROVA_FINAL_MINEFIELD_COOLDOWN 300
#define TWINROVA_MINEFIELD_WINDUP_TIME 24
#define TWINROVA_MINEFIELD_LAUNCH_GAP 3
#define TWINROVA_MINEFIELD_CAST_READY 0
#define TWINROVA_MINEFIELD_CAST_ACTIVE 1
#define TWINROVA_MINEFIELD_CAST_RECOVER 2
#define TWINROVA_MINEFIELD_CAST_GAP 3
#define TWINROVA_MINEFIELD_FLIGHT_TIME 36
#define TWINROVA_MINEFIELD_SETTLE_TIME 220
#define TWINROVA_MINEFIELD_DETONATE_TIME 10
#define TWINROVA_MINEFIELD_FADE_TIME 18
#define TWINROVA_MINEFIELD_DETONATE_STAGGER 8
#define TWINROVA_MINEFIELD_FOLLOWUP_SHOTS 3
#define TWINROVA_MINEFIELD_FOLLOWUP_GAP 6
#define TWINROVA_MINEFIELD_POST_PACKAGE_FUSE_TIME 70
#define TWINROVA_MINEFIELD_POST_PACKAGE_STAGGER 16
#define TWINROVA_MINEFIELD_FIRE_POOL_RADIUS 105.0f
#define TWINROVA_MINEFIELD_ICE_PATCH_RADIUS 115.0f
#define TWINROVA_MINEFIELD_BURST_HEIGHT 75.0f
#define TWINROVA_MINEFIELD_POOL_TIME 48
#define TWINROVA_MINEFIELD_POOL_FADE_TIME 12
#define TWINROVA_MINEFIELD_MAX_ACTIVE_MINES 4
#define TWINROVA_MINEFIELD_MAX_POOL_ACTORS 2
#define TWINROVA_MINEFIELD_FINAL_LIGHT_PULSE_PEAK_TIME 6
#define TWINROVA_MINEFIELD_FINAL_LIGHT_PULSE_TIME 12
#define TWINROVA_MINEFIELD_ORB_FLOAT_SCALE 5.0f
#define TWINROVA_MINEFIELD_FIRE_ORB_ARM_SCALE 14.0f
#define TWINROVA_MINEFIELD_ICE_ORB_ARM_SCALE 15.3f
#define TWINROVA_MINEFIELD_DETONATION_LIGHTNING_SCALE 4.2f
#define TWINROVA_FUSED_ATTACK_ANIM_MORPH_TIME -2.0f
#define TWINROVA_FUSED_ATTACK_ANIM_SPEED 1.2f
#define TWINROVA_FUSED_CHARGE_ANIM_MORPH_TIME -3.0f
#define TWINROVA_FUSED_CHARGE_ANIM_SPEED 1.15f
#define TWINROVA_FUSED_HOVER_ANIM_MORPH_TIME -3.0f
#define TWINROVA_FALSE_CHARGE_ANIM_SPEED 1.3f
#define TWINROVA_CYCLONE_HOVER_SPEED 1.25f
#define TWINROVA_FUSED_NORMAL_WEIGHT 30
#define TWINROVA_FINAL_FUSED_NORMAL_WEIGHT 25
#define TWINROVA_FUSED_FALSE_CHARGE_WEIGHT 15
#define TWINROVA_FUSED_BREAKER_WEIGHT 20
#define TWINROVA_FUSED_MINEFIELD_WEIGHT 15
#define TWINROVA_FUSED_CYCLONE_WEIGHT 20
#define TWINROVA_FINAL_FUSED_CYCLONE_WEIGHT 25
// Signature projectiles do not use the fused actor's charge-SFX work slot. Reuse it locally to remember that the
// projectile crossed the plane through its committed aim point and is waiting one update for shield collision data.
#define TWINROVA_SIGNATURE_PASSED_AIM PLAYED_CHRG_SFX
// Cyclone uses this projectile-local slot to select one restrained audio/particle cue per second wave. Spiral rain
// keeps its original feedback and does not read this alias.
#define TWINROVA_RAIN_PLAY_CYCLONE_CUE CAN_SHOOT
// Projectile timers tick after their action while boss timers tick before it. Thirteen stored ticks guarantee that
// both paths expose twelve complete controllable updates after the thaw tell before hostile pressure resumes.
#define TWINROVA_FROZEN_ATTACK_GRACE 13
#define TWINROVA_NORMAL_SHOT_RECOVERY 14
#define TWINROVA_FINAL_NORMAL_SHOT_RECOVERY 10
#define TWINROVA_NORMAL_BLAST_SPAWN_EFFECTS 24
#define TWINROVA_SPECIAL_SHOT_RECOVERY 20
#define TWINROVA_FINAL_SPECIAL_SHOT_RECOVERY 15
#define TWINROVA_ARRIVAL_WAIT 6
#define TWINROVA_FINAL_ARRIVAL_WAIT 4
#define TWINROVA_SIEGE_TELEGRAPH_TIME 60
#define TWINROVA_SIEGE_FINAL_WARNING_TIME 20
#define TWINROVA_SIEGE_ACTIVE_TIME 240
#define TWINROVA_SIEGE_FADE_TIME 30
// Link remains frozen for roughly 124 updates. This leaves about 66 updates after thawing, enough to cross the
// authored worst-case escape route before an ice zone may punish him again.
#define TWINROVA_SIEGE_FREEZE_COOLDOWN 190
#define TWINROVA_SIEGE_ZONE_OFFSET 175.0f
#define TWINROVA_SIEGE_ZONE_RADIUS 250.0f
#define TWINROVA_SIEGE_LOWER_ZONE_OFFSET 410.0f
#define TWINROVA_SIEGE_LOWER_ZONE_RADIUS 330.0f
#define TWINROVA_SIEGE_LOWER_FLOOR_Y 0.0f
#define TWINROVA_SIEGE_SIDE_ZONE_OFFSET 600.0f
#define TWINROVA_SIEGE_SIDE_ZONE_RADIUS 160.0f
#define TWINROVA_SIEGE_SIDE_FLOOR_Y 231.0f
#define TWINROVA_SIEGE_ZONE_COUNT 6
#define TWINROVA_SIEGE_CROSSCUT_UPPER_OFFSET 210.0f
#define TWINROVA_SIEGE_CROSSCUT_LOWER_OFFSET 420.0f
// Six slightly overlapping circles form one continuous, truthful annulus. The inner version leaves a generous outer
// escape lane; the outer version compresses that lane without consuming the arena's established perimeter route.
#define TWINROVA_SIEGE_INNER_RING_ORBIT 430.0f
#define TWINROVA_SIEGE_INNER_RING_RADIUS 225.0f
#define TWINROVA_SIEGE_OUTER_RING_ORBIT 560.0f
#define TWINROVA_SIEGE_OUTER_RING_RADIUS 290.0f
#define TWINROVA_SIEGE_RING_ANGLE_STEP 0x2AAB
#define TWINROVA_SIEGE_RING_BOSS_OFFSET 420.0f
// Three linked nodes per deck cover the arena opposite a shared safe sector. Both copies rotate together, so changing
// floors never rerolls or escapes the rule; the existing timer-25 cue announces the exact point where they lock.
// The lower copy follows a square lane just beyond the raised center, keeping every moving sigil visible. Its slightly
// larger zones remain linked even at the lane's diagonal corners.
#define TWINROVA_SIEGE_WEDGE_NODES_PER_DECK 3
#define TWINROVA_SIEGE_WEDGE_NODE_ANGLE_STEP 0x4000
#define TWINROVA_SIEGE_WEDGE_ROTATION_ARC 0x4000
#define TWINROVA_SIEGE_WEDGE_PLAYER_LEAD 0x1000
#define TWINROVA_SIEGE_WEDGE_ATTACK_ANGLE_OFFSET 0x1800
#define TWINROVA_SIEGE_WEDGE_CENTER_THRESHOLD 50.0f
#define TWINROVA_SIEGE_WEDGE_LOCK_TIME 25
#define TWINROVA_SIEGE_WEDGE_ROTATION_TIME \
    (TWINROVA_SIEGE_TELEGRAPH_TIME - TWINROVA_SIEGE_WEDGE_LOCK_TIME)
#define TWINROVA_SIEGE_WEDGE_UPPER_ORBIT 260.0f
#define TWINROVA_SIEGE_WEDGE_UPPER_RADIUS 200.0f
#define TWINROVA_SIEGE_WEDGE_LOWER_LANE 360.0f
#define TWINROVA_SIEGE_WEDGE_LOWER_RADIUS 365.0f
#define TWINROVA_SIEGE_WEDGE_UPPER_ATTACK_RADIUS 800.0f
#define TWINROVA_SIEGE_CLEANSE_RADIUS 370.0f
#define TWINROVA_SIEGE_CLEANSE_HEIGHT 100.0f
#define TWINROVA_SIEGE_SAFE_SIDE_DISTANCE 300.0f
#define TWINROVA_SIEGE_LOWER_ATTACK_RADIUS 850.0f
#define TWINROVA_SIEGE_LOWER_ATTACK_HEIGHT 120.0f
#define TWINROVA_SIEGE_LOWER_PORTAL_HEIGHT 160.0f
#define TWINROVA_SIEGE_UPPER_PORTAL_HEIGHT 300.0f
#define TWINROVA_SIEGE_LOWER_DESCEND_RADIUS 750.0f
#define TWINROVA_SIEGE_UPPER_MOVE_STEP 30.0f
#define TWINROVA_SIEGE_LOWER_MOVE_STEP 24.0f
#define TWINROVA_SIEGE_LOWER_ATTACK_ANGLE_OFFSET 0x1555
#define TWINROVA_SIEGE_MIN_SHOT_DISTANCE 400.0f
#define TWINROVA_SIEGE_SIGIL_SCALE 0.125f
#define TWINROVA_SIEGE_TELEGRAPH_POOL_ALPHA_SCALE 0.70f
#define TWINROVA_SIEGE_IMPACT_EFFECTS 15
#define TWINROVA_SIEGE_USE_ZONE_ELEMENT -1
#define TWINROVA_SIGIL_MESH_RADIUS 2000.0f
// Normalize the polygon's nearest edge, not its corner vertices, so the visible pool always contains the circular
// gameplay boundary. The small corner overshoot is safer than an invisible damaging crescent between mesh edges.
#define TWINROVA_FIRE_POOL_MESH_INRADIUS 7622.247f
#define TWINROVA_ICE_POOL_MESH_INRADIUS 4755.0f
#define TWINROVA_FIRE_POOL_DAMAGE_RADIUS 4550.0f
#define TWINROVA_ICE_POOL_DAMAGE_RADIUS 4600.0f
#define TWINROVA_STUN_TIME 150
#define TWINROVA_CROSS_DECK_STUN_BONUS 50
#define TWINROVA_STUN_RECOVERY_WARNING_TIME 25
#define TWINROVA_STUN_LAYDOWN_LEAD_HEIGHT 80.0f
#define TWINROVA_STUN_DAMAGE_BUDGET 12
#define TWINROVA_STUN_DAMAGE_WARNING 8
#define TWINROVA_GET_UP_RECOVERY 30
#define TWINROVA_FINAL_GET_UP_RECOVERY 25
#define TWINROVA_FUSED_MAX_HEALTH 24

typedef enum {
    TWINROVA_MAGIC_ICE,
    TWINROVA_MAGIC_FIRE,
} TwinrovaMagicElement;

typedef enum {
    TWINROVA_PHASE_ONE_REGULAR_NONE,
    TWINROVA_PHASE_ONE_REGULAR_VOLLEY,
    TWINROVA_PHASE_ONE_REGULAR_BEAM,
} TwinrovaPhaseOneRegularAttack;

typedef enum {
    TWINROVA_PHASE_ONE_SPECIAL_SPIRAL = 1 << 0,
    TWINROVA_PHASE_ONE_SPECIAL_CROSSFIRE = 1 << 1,
    TWINROVA_PHASE_ONE_SPECIAL_PRESSURE_TRIANGLE = 1 << 2,
    TWINROVA_PHASE_ONE_SPECIAL_PORTAL = 1 << 3,
} TwinrovaPhaseOneSpecial;

typedef enum {
    TWINROVA_FUSED_ATTACK_NORMAL,
    TWINROVA_FUSED_ATTACK_FALSE_CHARGE,
    TWINROVA_FUSED_ATTACK_BREAKER,
    TWINROVA_FUSED_ATTACK_MINEFIELD,
    TWINROVA_FUSED_ATTACK_CYCLONE,
    TWINROVA_FUSED_ATTACK_MAX,
} TwinrovaFusedAttack;

typedef enum {
    TWINROVA_BLAST_STRAIGHT,
    TWINROVA_BLAST_CURVING,
    TWINROVA_BLAST_RAIN,
    TWINROVA_BLAST_SIEGE,
    TWINROVA_BLAST_BREAKER,
    TWINROVA_BLAST_LOWER_ROUTE,
    TWINROVA_BLAST_MINEFIELD_FOLLOWUP,
    TWINROVA_BLAST_MINEFIELD,
    TWINROVA_BLAST_MINEFIELD_POOL,
} TwinrovaBlastBehavior;

typedef enum {
    TWINROVA_MINE_STATE_LAUNCH,
    TWINROVA_MINE_STATE_FLOAT,
    TWINROVA_MINE_STATE_ARM,
    TWINROVA_MINE_STATE_DETONATE,
    TWINROVA_MINE_STATE_FADE,
} TwinrovaMineState;

// Mine instances reuse fields that are otherwise inactive on a detached magic blast. `blastType` carries the element,
// `targetPos` is the fixed destination, and `timers[0]` is the current state's lifetime. Keep these aliases local to
// Minefield behavior so regular projectiles retain their original meanings.
#define TWINROVA_MINE_DETONATION_ORDER TW_PLLR_IDX
#define TWINROVA_MINE_FINAL_PULSE_PLAYED UNK_S8

typedef enum {
    TWINROVA_MINEFIELD_SEQUENCE_NONE,
    TWINROVA_MINEFIELD_SEQUENCE_PENDING,
    TWINROVA_MINEFIELD_SEQUENCE_ACTIVE,
    TWINROVA_MINEFIELD_SEQUENCE_OVERLAP,
} TwinrovaMinefieldSequenceState;

typedef enum {
    TWINROVA_BREAKER_STATE_SIEGE_TELEGRAPH,
    TWINROVA_BREAKER_STATE_TELL,
    TWINROVA_BREAKER_STATE_SHOOT,
    TWINROVA_BREAKER_STATE_GAP,
} TwinrovaBreakerState;

typedef enum {
    TWINROVA_SIEGE_STATE_TELEGRAPH,
    TWINROVA_SIEGE_STATE_ACTIVE,
    TWINROVA_SIEGE_STATE_FADE,
} TwinrovaSiegeState;

typedef enum {
    TWINROVA_SIEGE_PATTERN_DIAGONAL,
    TWINROVA_SIEGE_PATTERN_CROSSCUT,
    TWINROVA_SIEGE_PATTERN_INNER_RING,
    TWINROVA_SIEGE_PATTERN_OUTER_RING,
    TWINROVA_SIEGE_PATTERN_ROTATING_WEDGE,
    TWINROVA_SIEGE_PATTERN_MAX,
} TwinrovaSiegePattern;

typedef enum {
    TWINROVA_SUMMON_KEESE_GROUP,
    TWINROVA_SUMMON_GROUND_GROUP,
    TWINROVA_SUMMON_MIXED_GROUP,
    TWINROVA_SUMMON_ELITE_GROUP,
    TWINROVA_SUMMON_GROUP_MAX,
} TwinrovaSummonGroup;

typedef struct {
    Actor* actor;
    s16 actorId;
    u8 canFollowBetweenLevels;
    u8 offscreenTimer;
} TwinrovaSummon;

void BossTw_Init(Actor* thisx, PlayState* play);
void BossTw_Destroy(Actor* thisx, PlayState* play);
void BossTw_Update(Actor* thisx, PlayState* play);
void BossTw_Draw(Actor* thisx, PlayState* play);
void BossTw_Reset(void);

void BossTw_TwinrovaDamage(BossTw* this, PlayState* play, u8 arg2);
void BossTw_TwinrovaSetupFly(BossTw* this, PlayState* play);
void BossTw_DrawEffects(PlayState* play);
void BossTw_TwinrovaLaugh(BossTw* this, PlayState* play);
void BossTw_TwinrovaFly(BossTw* this, PlayState* play);
void BossTw_TwinrovaGetUp(BossTw* this, PlayState* play);
void BossTw_TwinrovaSetupGetUp(BossTw* this, PlayState* play);
void BossTw_TwinrovaSetupLaugh(BossTw* this, PlayState* play);
void BossTw_TwinrovaDoneBlastShoot(BossTw* this, PlayState* play);
void BossTw_TwinrovaSetupDoneBlastShoot(BossTw* this, PlayState* play);
void BossTw_TwinrovaSetupShootBlast(BossTw* this, PlayState* play);
void BossTw_TwinrovaSetupChargeBlast(BossTw* this, PlayState* play);
void BossTw_TwinrovaArriveAtTarget(BossTw* this, PlayState* play);
void BossTw_TwinrovaDeathCS(BossTw* this, PlayState* play);
void BossTw_TwinrovaIntroCS(BossTw* this, PlayState* play);
void BossTw_CSWait(BossTw* this, PlayState* play);
void BossTw_DeathCS(BossTw* this, PlayState* play);
void BossTw_TwinrovaMergeCS(BossTw* this, PlayState* play);
void BossTw_TwinrovaSetupMergeCS(BossTw* this, PlayState* play);
void BossTw_MergeCS(BossTw* this, PlayState* play);
void BossTw_Spin(BossTw* this, PlayState* play);
void BossTw_PillarDive(BossTw* this, PlayState* play);
void BossTw_Laugh(BossTw* this, PlayState* play);
void BossTw_SetupLaugh(BossTw* this, PlayState* play);
void BossTw_FinishBeamShoot(BossTw* this, PlayState* play);
void BossTw_SetupFinishBeamShoot(BossTw* this, PlayState* play);
void BossTw_SetupHitByBeam(BossTw* this, PlayState* play);
void BossTw_HitByBeam(BossTw* this, PlayState* play);
void BossTw_Wait(BossTw* this, PlayState* play);
void BossTw_ShootBeam(BossTw* this, PlayState* play);
void BossTw_FlyTo(BossTw* this, PlayState* play);
void BossTw_SetupShootBeam(BossTw* this, PlayState* play);
void BossTw_SetupBlastVolley(BossTw* this, PlayState* play);
void BossTw_BlastVolley(BossTw* this, PlayState* play);
void BossTw_CrossfireVolley(BossTw* this, PlayState* play);
void BossTw_SpiralBarrage(BossTw* this, PlayState* play);
void BossTw_PortalReposition(BossTw* this, PlayState* play);
void BossTw_SetupTurnToPlayer(BossTw* this, PlayState* play);
void BossTw_TurnToPlayer(BossTw* this, PlayState* play);
void BossTw_TwinrovaUpdate(Actor* thisx, PlayState* play);
void BossTw_TwinrovaDraw(Actor* thisx, PlayState* play);
void BossTw_SetupWait(BossTw* this, PlayState* play);
void BossTw_TwinrovaSetupIntroCS(BossTw* this, PlayState* play);
void BossTw_SetupFlyTo(BossTw* this, PlayState* play);
void BossTw_SetupCSWait(BossTw* this, PlayState* play);
void BossTw_BlastUpdate(Actor* thisx, PlayState* play);
void BossTw_BlastDraw(Actor* thisx, PlayState* play);
void BossTw_BlastFire(BossTw* this, PlayState* play);
void BossTw_BlastIce(BossTw* this, PlayState* play);
void BossTw_SiegeZoneUpdate(Actor* thisx, PlayState* play);
void BossTw_SiegeZoneDraw(Actor* thisx, PlayState* play);
void BossTw_RainMarkerUpdate(Actor* thisx, PlayState* play);
void BossTw_DeathBall(BossTw* this, PlayState* play);
void BossTw_DrawDeathBall(Actor* thisx, PlayState* play);
void BossTw_TwinrovaStun(BossTw* this, PlayState* play);
void BossTw_TwinrovaSpin(BossTw* this, PlayState* play);
void BossTw_TwinrovaShootBlast(BossTw* this, PlayState* play);
void BossTw_TwinrovaChargeBlast(BossTw* this, PlayState* play);
void BossTw_TwinrovaBreakerSequence(BossTw* this, PlayState* play);
void BossTw_TwinrovaCyclone(BossTw* this, PlayState* play);
void BossTw_TwinrovaFalseCharge(BossTw* this, PlayState* play);
void BossTw_TwinrovaMinefield(BossTw* this, PlayState* play);
static void BossTw_TwinrovaMinefieldFollowup(BossTw* this, PlayState* play);
void BossTw_TwinrovaSetupSpin(BossTw* this, PlayState* play);
void BossTw_UpdateEffects(PlayState* play);

static void BossTw_ResetShieldCharge(void);
static void BossTw_ClearSummonedEnemies(PlayState* play);
static void BossTw_ClearPhaseOneMagic(PlayState* play, s32 resolveSummonWarnings);
static void BossTw_ClearSiegeZones(BossTw* owner, PlayState* play, s32 spawnEffects, s16 burstElement,
                                   s32 preserveMinefield);
static void BossTw_CancelPhaseTwoMagic(BossTw* owner, PlayState* play, s32 preserveMinefield);
static s32 BossTw_HasActiveGroundPressure(PlayState* play);
static s32 BossTw_CountActiveMinefieldPools(PlayState* play, BossTw* owner);
static void BossTw_TwinrovaSetupBreakerSequence(BossTw* this, PlayState* play);
static void BossTw_SetupAmbushVolley(BossTw* this, PlayState* play);
static void BossTw_SetupPressureTriangleHandoffVolley(BossTw* this, PlayState* play);
static void BossTw_SetupPillarDiveRetreatVolley(BossTw* this, PlayState* play);
static void BossTw_SelectTwinrovaBlastType(void);
static void BossTw_PrepareSisterForMerge(BossTw* sister);
static void BossTw_SpawnMagicLaunchEffects(PlayState* play, Vec3f* spawnPos, s16 blastType, s16 count);
static void BossTw_ShowFailedMagicCast(PlayState* play, Vec3f* castPos, TwinrovaMagicElement element);
static void BossTw_ClearPlayerBurn(PlayState* play);
static void BossTw_UpdateMinefieldBlast(BossTw* this, PlayState* play);
static void BossTw_DrawMinefieldFuse(BossTw* this, PlayState* play);
static void BossTw_BeginMinefieldFade(BossTw* this);

const ActorInit Boss_Tw_InitVars = {
    ACTOR_BOSS_TW,
    ACTORCAT_BOSS,
    FLAGS,
    OBJECT_TW,
    sizeof(BossTw),
    (ActorFunc)BossTw_Init,
    (ActorFunc)BossTw_Destroy,
    (ActorFunc)BossTw_Update,
    (ActorFunc)BossTw_Draw,
    (ActorResetFunc)BossTw_Reset,
};

static Vec3f D_8094A7D0 = { 0.0f, 0.0f, 1000.0f };
static Vec3f sZeroVector = { 0.0f, 0.0f, 0.0f };

static ColliderCylinderInit sCylinderInitBlasts = {
    {
        COLTYPE_NONE,
        AT_ON | AT_TYPE_ALL,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_PLAYER,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0xFFCFFFFF, 0x00, 0x30 },
        { 0x00100000, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_NORMAL,
        BUMP_ON,
        OCELEM_ON,
    },
    { 25, 35, -17, { 0, 0, 0 } },
};

static ColliderCylinderInit sCylinderInitKoumeKotake = {
    {
        COLTYPE_HIT3,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_PLAYER,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0xFFCFFFFF, 0x00, 0x20 },
        { 0xFFCDFFFE, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_NORMAL,
        BUMP_ON,
        OCELEM_ON,
    },
    { 45, 120, -30, { 0, 0, 0 } },
};

static ColliderCylinderInit sCylinderInitTwinrova = {
    {
        COLTYPE_HIT3,
        AT_ON | AT_TYPE_ENEMY,
        AC_ON | AC_TYPE_PLAYER,
        OC1_ON | OC1_TYPE_ALL,
        OC2_TYPE_1,
        COLSHAPE_CYLINDER,
    },
    {
        ELEMTYPE_UNK0,
        { 0xFFCFFFFF, 0x00, 0x20 },
        { 0xFFCDFFFE, 0x00, 0x00 },
        TOUCH_ON | TOUCH_SFX_NORMAL,
        BUMP_ON | BUMP_HOOKABLE,
        OCELEM_ON,
    },
    { 45, 120, -30, { 0, 0, 0 } },
};

static Vec3f sTwinrovaPillarPos[] = {
    { 580.0f, 380.0f, 0.0f },
    { 0.0f, 380.0f, 580.0f },
    { -580.0f, 380.0f, 0.0f },
    { 0.0f, 380.0f, -580.0f },
};

static Vec3f sTwinrovaPhaseOnePillarPos[] = {
    { 600.0f, 400.0f, 0.0f },
    { 0.0f, 400.0f, 600.0f },
    { -600.0f, 400.0f, 0.0f },
    { 0.0f, 400.0f, -600.0f },
};

static s32 BossTw_IsOverRaisedPlatform(Vec3f* pos) {
    return (fabsf(pos->x) < 350.0f && fabsf(pos->z) < 350.0f) ||
           (fabsf(pos->x) < 110.0f &&
            (fabsf(pos->z - TWINROVA_SPIRAL_SIDE_PLATFORM_OFFSET) < 110.0f ||
             fabsf(pos->z + TWINROVA_SPIRAL_SIDE_PLATFORM_OFFSET) < 110.0f)) ||
           (fabsf(pos->z) < 110.0f &&
            (fabsf(pos->x - TWINROVA_SPIRAL_SIDE_PLATFORM_OFFSET) < 110.0f ||
             fabsf(pos->x + TWINROVA_SPIRAL_SIDE_PLATFORM_OFFSET) < 110.0f));
}

static void BossTw_ProjectOntoSquareLane(Vec3f* pos, f32 laneHalfExtent) {
    f32 dominantAxis = MAX(fabsf(pos->x), fabsf(pos->z));

    if (dominantAxis > 0.0f) {
        f32 laneScale = laneHalfExtent / dominantAxis;

        pos->x *= laneScale;
        pos->z *= laneScale;
    }
}

static u8 sTwInitialized = false;

static InitChainEntry sInitChain[] = {
    ICHAIN_U8(targetMode, 5, ICHAIN_CONTINUE),
    ICHAIN_F32_DIV1000(gravity, 0, ICHAIN_CONTINUE),
    ICHAIN_F32(targetArrowOffset, 0, ICHAIN_STOP),
};

static s8 sEnvType;
static u8 sGroundBlastType;
static BossTw* sKotakePtr;
static BossTw* sKoumePtr;
static BossTw* sTwinrovaPtr;
static u8 sShieldFireCharge;
static u8 sShieldIceCharge;
static f32 D_8094C854;
static f32 D_8094C858;
static u8 sTwinrovaBlastType;
static u8 sFixedBlastType;
static u8 sFixedBlatSeq;
static u8 sFreezeState;
static Vec3f sShieldHitPos;
static s16 sShieldHitYaw;
static u8 sBeamDivertTimer;
static u8 D_8094C86F;
static u8 D_8094C870;
static s16 D_8094C872;
static s16 D_8094C874;
static s16 D_8094C876;
static u8 D_8094C878;
static s16 D_8094C87A;
static s16 D_8094C87C;
static u8 D_8094C87E;
static BossTwEffect sEffects[150];
static TwinrovaSummon sTwinrovaSummons[TWINROVA_SUMMON_REGISTRY_SIZE];
static u32 sTwinrovaSummonUpdateFrame;
static u32 sTwinrovaSharedUpdateFrame;
static u32 sTwinrovaCooldownUpdateFrame;
static u8 sMinefieldLightPulseTimer;
static u8 sMinefieldLightPulseElement;
static u8 sPhaseOneEligibleSpecialMisses;
static u8 sPhaseOneLastRegularAttack;
static u8 sPhaseOneRegularRepeatCount;
static u8 sPhaseOneComboState;
static u8 sPhaseOneSeenSpecials;
static u32 sPhaseOneSchedulerDecisionFrame;
static u32 sPhaseOneCooldownUpdateFrame;

#define BOSS_TW_SHIP_SAVESTATE_FIELDS(F) \
    F(sTwInitialized)                    \
    F(sEnvType)                          \
    F(sGroundBlastType)                  \
    F(sKotakePtr)                        \
    F(sKoumePtr)                         \
    F(sTwinrovaPtr)                      \
    F(sShieldFireCharge)                 \
    F(sShieldIceCharge)                  \
    F(D_8094C854)                        \
    F(D_8094C858)                        \
    F(sTwinrovaBlastType)                \
    F(sFixedBlastType)                   \
    F(sFixedBlatSeq)                     \
    F(sFreezeState)                      \
    F(sShieldHitPos)                     \
    F(sShieldHitYaw)                     \
    F(sBeamDivertTimer)                  \
    F(D_8094C86F)                        \
    F(D_8094C870)                        \
    F(D_8094C872)                        \
    F(D_8094C874)                        \
    F(D_8094C876)                        \
    F(D_8094C878)                        \
    F(D_8094C87A)                        \
    F(D_8094C87C)                        \
    F(D_8094C87E)                        \
    F(sEffects)                          \
    F(sTwinrovaSummons)                  \
    F(sTwinrovaSummonUpdateFrame)        \
    F(sTwinrovaSharedUpdateFrame)        \
    F(sTwinrovaCooldownUpdateFrame)      \
    F(sMinefieldLightPulseTimer)         \
    F(sMinefieldLightPulseElement)       \
    F(sPhaseOneEligibleSpecialMisses)    \
    F(sPhaseOneLastRegularAttack)        \
    F(sPhaseOneRegularRepeatCount)       \
    F(sPhaseOneComboState)               \
    F(sPhaseOneSeenSpecials)             \
    F(sPhaseOneSchedulerDecisionFrame)   \
    F(sPhaseOneCooldownUpdateFrame)

SHIP_SAVESTATE_DEFINE(BossTw, BOSS_TW_SHIP_SAVESTATE_FIELDS)

static void BossTw_ResetPhaseOneRegularHistory(void) {
    sPhaseOneLastRegularAttack = TWINROVA_PHASE_ONE_REGULAR_NONE;
    sPhaseOneRegularRepeatCount = 0;
}

static void BossTw_ResetPhaseOneScheduler(void) {
    sPhaseOneEligibleSpecialMisses = 0;
    sPhaseOneComboState = 0;
    sPhaseOneSeenSpecials = 0;
    BossTw_ResetPhaseOneRegularHistory();
}

static void BossTw_BeginEffectLifetime(BossTwEffect* eff, s16 type) {
    // The address is reused by the fixed pool. A new generation prevents frame interpolation from joining the
    // previous particle's last transform to an unrelated effect spawned into the same slot.
    eff->epoch++;
    eff->type = type;
}

void BossTw_AddDotEffect(PlayState* play, Vec3f* initalPos, Vec3f* initalSpeed, Vec3f* accel, f32 scale, s16 args,
                         s16 countLimit) {
    s16 i;
    BossTwEffect* eff;

    for (i = 0, eff = play->specialEffects; i < countLimit; i++, eff++) {
        if (eff->type == TWEFF_NONE) {
            BossTw_BeginEffectLifetime(eff, TWEFF_DOT);
            eff->pos = *initalPos;
            eff->curSpeed = *initalSpeed;
            eff->accel = *accel;
            eff->workf[EFF_SCALE] = scale / 1000.0f;
            eff->alpha = 255;
            eff->frame = (s16)Rand_ZeroFloat(10.0f);
            eff->work[EFF_ARGS] = args;
            break;
        }
    }
}

void BossTw_AddDmgCloud(PlayState* play, s16 type, Vec3f* initialPos, Vec3f* initalSpeed, Vec3f* accel, f32 scale,
                        s16 alpha, s16 args, s16 countLimit) {
    s16 i;
    BossTwEffect* eff;

    for (i = 0, eff = play->specialEffects; i < countLimit; i++, eff++) {
        if (eff->type == TWEFF_NONE) {
            BossTw_BeginEffectLifetime(eff, type);
            eff->pos = *initialPos;
            eff->curSpeed = *initalSpeed;
            eff->accel = *accel;
            eff->workf[EFF_SCALE] = scale / 1000.0f;
            eff->work[EFF_ARGS] = args;
            eff->alpha = alpha;
            eff->frame = (s16)Rand_ZeroFloat(100.0f);
            break;
        }
    }
}

s32 BossTw_AddRingEffect(PlayState* play, Vec3f* initalPos, f32 scale, f32 arg3, s16 alpha, s16 args, s16 arg6,
                         s16 arg7) {
    s16 i;
    BossTwEffect* eff;
    BossTwEffect* dotFallback = NULL;
    BossTwEffect* flameFallback = NULL;

    for (i = 0, eff = play->specialEffects; i < arg7; i++, eff++) {
        if (eff->type == TWEFF_NONE) {
            break;
        }
        if (eff->type == TWEFF_DOT) {
            dotFallback = eff;
        } else if (eff->type == TWEFF_FLAME) {
            flameFallback = eff;
        }
    }

    if (i == arg7) {
        // Rings carry positional warnings. Under cosmetic saturation, replace a disposable particle so tells
        // remain visible without evicting shield blasts, freeze effects, damage clouds, or shield-hit feedback.
        eff = dotFallback != NULL ? dotFallback : flameFallback;
        if (eff == NULL) {
            return false;
        }
    }

    BossTw_BeginEffectLifetime(eff, TWEFF_RING);
    eff->pos = *initalPos;
    eff->curSpeed = sZeroVector;
    eff->accel = sZeroVector;
    eff->workf[EFF_SCALE] = scale * 0.0025f;
    eff->workf[EFF_DIST] = arg3 * 0.0025f;
    eff->work[EFF_ARGS] = args;
    eff->work[EFF_UNKS1] = arg6;
    eff->alpha = alpha;
    eff->workf[EFF_ROLL] = Rand_ZeroFloat(M_PI);
    eff->frame = 0;
    return true;
}

s32 BossTw_AddPlayerFreezeEffect(PlayState* play, Actor* target) {
    BossTwEffect* eff;
    BossTwEffect* dotFallback = NULL;
    BossTwEffect* flameFallback = NULL;
    s16 i;

    for (eff = play->specialEffects, i = 0; i < ARRAY_COUNT(sEffects); i++, eff++) {
        if (eff->type == TWEFF_NONE) {
            break;
        }
        if (eff->type == TWEFF_DOT) {
            dotFallback = eff;
        } else if (eff->type == TWEFF_FLAME) {
            flameFallback = eff;
        }
    }

    if (i == ARRAY_COUNT(sEffects)) {
        // Freeze is gameplay state, so it takes priority over a disposable cosmetic particle when the shared
        // effect pool is saturated. Never evict shield blasts, warnings, damage clouds, or existing freezes.
        eff = dotFallback != NULL ? dotFallback : flameFallback;
        if (eff == NULL) {
            return false;
        }
    }

    BossTw_BeginEffectLifetime(eff, TWEFF_PLYR_FRZ);
    eff->curSpeed = sZeroVector;
    eff->accel = sZeroVector;
    eff->frame = 0;
    eff->target = target;
    eff->workf[EFF_DIST] = 0.0f;
    eff->workf[EFF_SCALE] = 0.0f;
    eff->workf[EFF_ROLL] = 0.0f;
    if (target == NULL) {
        eff->work[EFF_ARGS] = 100;
    } else {
        eff->work[EFF_ARGS] = 20;
    }
    return true;
}

void BossTw_AddFlameEffect(PlayState* play, Vec3f* initalPos, Vec3f* initalSpeed, Vec3f* accel, f32 scale, s16 args) {
    s16 i;
    BossTwEffect* eff;

    for (i = 0, eff = play->specialEffects; i < ARRAY_COUNT(sEffects); i++, eff++) {
        if (eff->type == TWEFF_NONE) {
            BossTw_BeginEffectLifetime(eff, TWEFF_FLAME);
            eff->pos = *initalPos;
            eff->curSpeed = *initalSpeed;
            eff->accel = *accel;
            eff->workf[EFF_SCALE] = scale / 1000.0f;
            eff->work[EFF_ARGS] = args;
            eff->work[EFF_UNKS1] = 0;
            eff->alpha = 0;
            eff->frame = (s16)Rand_ZeroFloat(1000.0f);
            break;
        }
    }
}

void BossTw_AddMergeFlameEffect(PlayState* play, Vec3f* initialPos, f32 scale, f32 dist, s16 args) {
    s16 i;
    BossTwEffect* eff;

    for (i = 0, eff = play->specialEffects; i < ARRAY_COUNT(sEffects); i++, eff++) {
        if (eff->type == TWEFF_NONE) {
            BossTw_BeginEffectLifetime(eff, TWEFF_MERGEFLAME);
            eff->pos = *initialPos;
            eff->curSpeed = sZeroVector;
            eff->accel = sZeroVector;
            eff->workf[EFF_SCALE] = scale / 1000.0f;
            eff->work[EFF_ARGS] = args;
            eff->work[EFF_UNKS1] = 0;
            eff->workf[EFF_DIST] = dist;
            eff->workf[EFF_ROLL] = Rand_ZeroFloat(2.0f * M_PI);
            eff->alpha = 0;
            eff->frame = (s16)Rand_ZeroFloat(1000.0f);
            break;
        }
    }
}

static BossTwEffect* BossTw_AllocCombatFeedbackEffect(PlayState* play) {
    BossTwEffect* eff;
    BossTwEffect* dotFallback = NULL;
    BossTwEffect* flameFallback = NULL;
    s16 i;

    for (i = 0, eff = play->specialEffects; i < ARRAY_COUNT(sEffects); i++, eff++) {
        if (eff->type == TWEFF_NONE) {
            return eff;
        }
        if (eff->type == TWEFF_DOT) {
            dotFallback = eff;
        } else if (eff->type == TWEFF_FLAME) {
            flameFallback = eff;
        }
    }

    // Charged releases and shield contacts communicate gameplay state. Under cosmetic saturation, replace only a
    // short-lived particle; never evict warnings, hazards, freezes, or another piece of shield feedback.
    eff = dotFallback != NULL ? dotFallback : flameFallback;
    return eff;
}

s32 BossTw_AddShieldBlastEffect(PlayState* play, Vec3f* initalPos, Vec3f* initalSpeed, Vec3f* accel, f32 scale,
                                f32 arg5, s16 alpha, s16 args, Actor* target) {
    BossTwEffect* eff = BossTw_AllocCombatFeedbackEffect(play);

    if (eff == NULL) {
        return false;
    }

    BossTw_BeginEffectLifetime(eff, TWEFF_SHLD_BLST);
    eff->pos = *initalPos;
    eff->curSpeed = *initalSpeed;
    eff->accel = *accel;
    eff->workf[EFF_SCALE] = scale / 1000.0f;
    eff->workf[EFF_DIST] = arg5 / 1000.0f;
    eff->work[EFF_ARGS] = args;
    eff->work[EFF_UNKS1] = 0;
    eff->target = target;
    eff->alpha = alpha;
    eff->frame = (s16)Rand_ZeroFloat(1000.0f);
    return true;
}

void BossTw_AddShieldDeflectEffect(PlayState* play, f32 arg1, s16 arg2) {
    s16 i;
    BossTwEffect* eff;
    Player* player = GET_PLAYER(play);

    sShieldHitPos = player->bodyPartsPos[15];
    sShieldHitYaw = player->actor.shape.rot.y;

    for (i = 0; i < 8; i++) {
        eff = BossTw_AllocCombatFeedbackEffect(play);
        if (eff == NULL) {
            break;
        }
        BossTw_BeginEffectLifetime(eff, TWEFF_SHLD_DEFL);
        eff->pos = sShieldHitPos;
        eff->curSpeed = sZeroVector;
        eff->accel = sZeroVector;
        eff->workf[EFF_ROLL] = i * (M_PI / 4.0f);
        eff->workf[EFF_YAW] = M_PI / 2.0f;
        eff->workf[EFF_DIST] = 0.0f;
        eff->workf[EFF_SCALE] = arg1 / 1000.0f;
        eff->work[EFF_ARGS] = arg2;
        eff->work[EFF_UNKS1] = 0;
        eff->alpha = 255;
        eff->frame = (s16)Rand_ZeroFloat(1000.0f);
    }
}

void BossTw_AddShieldHitEffect(PlayState* play, f32 arg1, s16 arg2) {
    s16 i;
    BossTwEffect* eff;
    Player* player = GET_PLAYER(play);

    sShieldHitPos = player->bodyPartsPos[15];
    sShieldHitYaw = player->actor.shape.rot.y;

    for (i = 0; i < 8; i++) {
        eff = BossTw_AllocCombatFeedbackEffect(play);
        if (eff == NULL) {
            break;
        }
        BossTw_BeginEffectLifetime(eff, TWEFF_SHLD_HIT);
        eff->pos = sShieldHitPos;
        eff->curSpeed = sZeroVector;
        eff->accel = sZeroVector;
        eff->workf[EFF_ROLL] = i * (M_PI / 4.0f);
        eff->workf[EFF_YAW] = M_PI / 2.0f;
        eff->workf[EFF_DIST] = 0.0f;
        eff->workf[EFF_SCALE] = arg1 / 1000.0f;
        eff->work[EFF_ARGS] = arg2;
        eff->work[EFF_UNKS1] = 0;
        eff->alpha = 255;
        eff->frame = (s16)Rand_ZeroFloat(1000.0f);
    }
}

static s32 BossTw_TrySpawnSisters(BossTw* twinrova, PlayState* play) {
    sKotakePtr = (BossTw*)Actor_SpawnAsChild(&play->actorCtx, &twinrova->actor, play, ACTOR_BOSS_TW,
                                             twinrova->actor.world.pos.x, twinrova->actor.world.pos.y,
                                             twinrova->actor.world.pos.z, 0, 0, 0, TW_KOTAKE);
    sKoumePtr = (BossTw*)Actor_SpawnAsChild(&play->actorCtx, &twinrova->actor, play, ACTOR_BOSS_TW,
                                            twinrova->actor.world.pos.x, twinrova->actor.world.pos.y,
                                            twinrova->actor.world.pos.z, 0, 0, 0, TW_KOUME);
    if (sKotakePtr == NULL || sKoumePtr == NULL) {
        if (sKotakePtr != NULL) {
            Actor_Kill(&sKotakePtr->actor);
        }
        if (sKoumePtr != NULL) {
            Actor_Kill(&sKoumePtr->actor);
        }
        sKotakePtr = sKoumePtr = NULL;
        return false;
    }

    sKotakePtr->actor.parent = &sKoumePtr->actor;
    sKoumePtr->actor.parent = &sKotakePtr->actor;

    if (Flags_GetEventChkInf(EVENTCHKINF_BEGAN_TWINROVA_BATTLE)) {
        // Actor_SpawnAsChild initializes each sister before assigning its parent. Reserve their opening routes again
        // now that both cross-sister pointers exist, otherwise a resumed fight can begin with matching destinations.
        BossTw_SetupFlyTo(sKotakePtr, play);
        BossTw_SetupFlyTo(sKoumePtr, play);
    }
    return true;
}

static s32 BossTw_TrySpawnDefeatRewards(BossTw* this, PlayState* play) {
    if (GameInteractor_Should(VB_SPAWN_BLUE_WARP, true, this) &&
        Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_DOOR_WARP1, 600.0f, 230.0f, 0.0f, 0, 0, 0,
                           WARP_DUNGEON_ADULT) == NULL) {
        // The exit is mandatory. Keep the invisible controller alive and retry after actor cleanup frees a slot.
        return false;
    }

    // The heart is a bonus reward and remains best-effort. It is attempted only after the mandatory exit exists,
    // which also prevents duplicates while a full actor pool forces repeated warp attempts.
    if (GameInteractor_Should(VB_SPAWN_HEART_CONTAINER, true)) {
        Actor_Spawn(&play->actorCtx, play, ACTOR_ITEM_B_HEART, -600.0f, 230.0f, 0.0f, 0, 0, 0, 0);
    }

    return true;
}

static void BossTw_WaitForDefeatRewardAllocation(BossTw* this, PlayState* play) {
    if (BossTw_TrySpawnDefeatRewards(this, play)) {
        this->defeatRewardsSpawned = true;
        Actor_Kill(&this->actor);
    }
}

static void BossTw_TwinrovaWaitForSisterAllocation(BossTw* this, PlayState* play) {
    if (!BossTw_TrySpawnSisters(this, play)) {
        return;
    }

    if (Flags_GetEventChkInf(EVENTCHKINF_BEGAN_TWINROVA_BATTLE)) {
        BossTw_SetupWait(this, play);
    } else {
        BossTw_TwinrovaSetupIntroCS(this, play);
        this->actor.world.pos.x = 0.0f;
        this->actor.world.pos.y = 1000.0f;
        this->actor.world.pos.z = 0.0f;
    }
}

void BossTw_Init(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    BossTw* this = (BossTw*)thisx;
    s16 i;

    Actor_ProcessInitChain(&this->actor, sInitChain);
    ActorShape_Init(&this->actor.shape, 0.0f, NULL, 0.0f);
    this->blastBehavior = TWINROVA_BLAST_STRAIGHT;
    this->breakerSequenceSeen = false;
    this->lastSiegePattern = TWINROVA_SIEGE_PATTERN_MAX;
    this->attackPortalActive = false;
    this->cycloneSeen = false;
    this->cycloneCooldown = 0;
    this->pendingFusedAttack = TWINROVA_FUSED_ATTACK_NORMAL;
    this->falseChargeCooldown = 0;
    this->minefieldCooldown = 0;
    this->minefieldSequenceState = TWINROVA_MINEFIELD_SEQUENCE_NONE;
    this->minefieldFollowupSuppressed = false;
    this->minefieldSeen = false;
    this->falseChargeSeen = false;

    if (this->actor.params >= TW_FIRE_BLAST) {
        // Blasts
        Actor_SetScale(&this->actor, 0.01f);
        this->actor.update = BossTw_BlastUpdate;
        this->actor.draw = BossTw_BlastDraw;
        this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;

        Collider_InitCylinder(play, &this->collider);
        Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInitBlasts);

        if (this->actor.params == TW_FIRE_BLAST || this->actor.params == TW_FIRE_BLAST_GROUND) {
            this->actionFunc = BossTw_BlastFire;
            this->collider.info.toucher.effect = 1;
        } else if (this->actor.params == TW_ICE_BLAST || this->actor.params == TW_ICE_BLAST_GROUND) {
            this->actionFunc = BossTw_BlastIce;
        } else if (this->actor.params == TW_FIRE_SIEGE_ZONE || this->actor.params == TW_ICE_SIEGE_ZONE) {
            this->actor.update = BossTw_SiegeZoneUpdate;
            this->actor.draw = BossTw_SiegeZoneDraw;
            this->blastType = this->actor.params == TW_FIRE_SIEGE_ZONE ? TWINROVA_MAGIC_FIRE : TWINROVA_MAGIC_ICE;
            this->csState1 = TWINROVA_SIEGE_STATE_TELEGRAPH;
            this->timers[0] = TWINROVA_SIEGE_TELEGRAPH_TIME;
            this->workf[UNK_F17] = 0.0f;
            this->workf[UNK_F18] = 0.0f;
            return;
        } else if (this->actor.params == TW_FIRE_WARNING_SIGIL || this->actor.params == TW_ICE_WARNING_SIGIL) {
            this->actor.update = BossTw_RainMarkerUpdate;
            this->actor.draw = BossTw_SiegeZoneDraw;
            this->blastType = this->actor.params == TW_FIRE_WARNING_SIGIL ? TWINROVA_MAGIC_FIRE : TWINROVA_MAGIC_ICE;
            this->csState1 = TWINROVA_SIEGE_STATE_TELEGRAPH;
            // Parent validation removes the marker on impact; the padding prevents a one-frame early fade.
            this->timers[0] = (s16)TWINROVA_RAIN_FLIGHT_FRAMES + TWINROVA_RAIN_MARKER_PARENT_PADDING;
            this->workf[UNK_F17] = 0.0f;
            this->workf[UNK_F18] = 0.0f;
            return;
        } else if (this->actor.params >= TW_DEATHBALL_KOTAKE) {
            this->actionFunc = BossTw_DeathBall;
            this->actor.draw = BossTw_DrawDeathBall;
            this->workf[TAIL_ALPHA] = 128.0f;

            if (thisx->params == TW_DEATHBALL_KOTAKE) {
                thisx->world.rot.y = sTwinrovaPtr->actor.world.rot.y + 0x4000;
            } else {
                thisx->world.rot.y = sTwinrovaPtr->actor.world.rot.y - 0x4000;
            }
        }

        this->timers[1] = 150;
        return;
    }

    Actor_SetScale(&this->actor, 2.5 * 0.01f);
    this->actor.colChkInfo.mass = 255;
    this->actor.colChkInfo.health = 0;
    Collider_InitCylinder(play, &this->collider);

    if (!sTwInitialized) {
        sTwInitialized = true;
        play->envCtx.unk_BF = 1;
        play->envCtx.unk_BE = 1;
        play->envCtx.unk_BD = 1;
        play->envCtx.unk_D8 = 0.0f;

        D_8094C874 = D_8094C876 = D_8094C878 = D_8094C87A = D_8094C87C = D_8094C87E = D_8094C870 = D_8094C86F =
            D_8094C872 = sBeamDivertTimer = sEnvType = sGroundBlastType = sFreezeState = sTwinrovaBlastType =
                sFixedBlatSeq = sShieldFireCharge = sShieldIceCharge = 0;

        D_8094C858 = D_8094C854 = 0.0f;
        sFixedBlastType = Rand_ZeroFloat(1.99f);
        play->specialEffects = sEffects;
        memset(sTwinrovaSummons, 0, sizeof(sTwinrovaSummons));
        sTwinrovaSummonUpdateFrame = play->gameplayFrames - 1;
        sTwinrovaSharedUpdateFrame = play->gameplayFrames - 1;
        sTwinrovaCooldownUpdateFrame = play->gameplayFrames - 1;
        sMinefieldLightPulseTimer = 0;
        sMinefieldLightPulseElement = TWINROVA_MAGIC_ICE;
        sPhaseOneSchedulerDecisionFrame = play->gameplayFrames - 1;
        sPhaseOneCooldownUpdateFrame = play->gameplayFrames - 1;
        BossTw_ResetPhaseOneScheduler();

        for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
            sEffects[i].type = TWEFF_NONE;
            sEffects[i].epoch++;
        }
    }

    if (this->actor.params == TW_KOTAKE) {
        Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInitKoumeKotake);
        this->actor.naviEnemyId = 0x33;
        SkelAnime_InitFlex(play, &this->skelAnime, &gTwinrovaKotakeSkel, &gTwinrovaKotakeKoumeFlyAnim, NULL, NULL, 0);

        if (Flags_GetEventChkInf(EVENTCHKINF_BEGAN_TWINROVA_BATTLE)) {
            // began twinrova battle
            BossTw_SetupFlyTo(this, play);
            this->actor.world.pos.x = -600.0f;
            this->actor.world.pos.y = 400.0f;
            this->actor.world.pos.z = 0.0f;
            Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_BOSS);
        } else {
            BossTw_SetupCSWait(this, play);
        }

        Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, -3.0f);
        this->visible = true;
    } else if (this->actor.params == TW_KOUME) {
        Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInitKoumeKotake);
        this->actor.naviEnemyId = 0x32;
        SkelAnime_InitFlex(play, &this->skelAnime, &gTwinrovaKoumeSkel, &gTwinrovaKotakeKoumeFlyAnim, NULL, NULL, 0);

        if (Flags_GetEventChkInf(EVENTCHKINF_BEGAN_TWINROVA_BATTLE)) {
            // began twinrova battle
            BossTw_SetupFlyTo(this, play);
            this->actor.world.pos.x = 600.0f;
            this->actor.world.pos.y = 400.0f;
            this->actor.world.pos.z = 0.0f;
        } else {
            BossTw_SetupCSWait(this, play);
        }

        Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, -3.0f);
        this->visible = true;
    } else {
        // Twinrova
        Collider_SetCylinder(play, &this->collider, &this->actor, &sCylinderInitTwinrova);
        this->actor.naviEnemyId = 0x5B;
        this->actor.colChkInfo.health = TWINROVA_FUSED_MAX_HEALTH;
        this->actor.update = BossTw_TwinrovaUpdate;
        this->actor.draw = BossTw_TwinrovaDraw;
        SkelAnime_InitFlex(play, &this->skelAnime, &gTwinrovaSkel, &gTwinrovaTPoseAnim, NULL, NULL, 0);
        Animation_MorphToLoop(&this->skelAnime, &gTwinrovaTPoseAnim, -3.0f);

        if (Flags_GetEventChkInf(EVENTCHKINF_BEGAN_TWINROVA_BATTLE)) {
            // began twinrova battle
            BossTw_SetupWait(this, play);
        } else {
            BossTw_TwinrovaSetupIntroCS(this, play);
            this->actor.world.pos.x = 0.0f;
            this->actor.world.pos.y = 1000.0f;
            this->actor.world.pos.z = 0.0f;
        }

        this->actor.params = TW_TWINROVA;
        sTwinrovaPtr = this;

        if (Flags_GetClear(play, play->roomCtx.curRoom.num)) {
            // twinrova has been defeated.
            this->visible = false;
            this->actor.world.pos.y = -2000.0f;
            if (BossTw_TrySpawnDefeatRewards(this, play)) {
                this->defeatRewardsSpawned = true;
                Actor_Kill(&this->actor);
            } else {
                // Never strand a previously cleared boss room because the actor pool was transiently full.
                this->actionFunc = BossTw_WaitForDefeatRewardAllocation;
            }
        } else {
            if (!BossTw_TrySpawnSisters(this, play)) {
                // A transiently full actor pool must not delete the boss and strand the room. Keep the invisible
                // controller alive and retry once normal actor cleanup has reclaimed slots.
                this->actionFunc = BossTw_TwinrovaWaitForSisterAllocation;
            }
        }
    }

    this->fogR = play->lightCtx.fogColor[0];
    this->fogG = play->lightCtx.fogColor[1];
    this->fogB = play->lightCtx.fogColor[2];
    this->fogNear = play->lightCtx.fogNear;
    this->fogFar = 1000.0f;
}

static void BossTw_KillRemainingSistersAndDeathballs(PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    Actor* next;

    while (actor != NULL) {
        next = actor->next;
        if (actor->id == ACTOR_BOSS_TW && actor->update != NULL &&
            (actor->params == TW_KOTAKE || actor->params == TW_KOUME || actor->params == TW_DEATHBALL_KOTAKE ||
             actor->params == TW_DEATHBALL_KOUME)) {
            Actor_Kill(actor);
        }
        actor = next;
    }
}

static s32 BossTw_TryAcquireCutsceneCamera(BossTw* this, PlayState* play, u8 playerCsAction) {
    s16 subCamId;
    s16 i;

    if (play->csCtx.state != CS_STATE_IDLE || Play_GetActiveCamId(play) != CAM_ID_MAIN) {
        return false;
    }

    // Twinrova's normal release helper clears every subcamera. Wait for queued/inactive one-point cameras as well as
    // the active one so ending this cinematic can never delete another owner's camera.
    for (i = CAM_ID_SUB_FIRST; i < NUM_CAMS; i++) {
        if (play->cameraPtrs[i] != NULL) {
            return false;
        }
    }

    subCamId = Play_CreateSubCamera(play);
    if (subCamId == SUBCAM_NONE) {
        return false;
    }

    this->subCamId = subCamId;
    func_80064520(play, &play->csCtx);
    Player_SetCsActionWithHaltedActors(play, &this->actor, playerCsAction);
    Play_ChangeCameraStatus(play, CAM_ID_MAIN, CAM_STAT_WAIT);
    Play_ChangeCameraStatus(play, this->subCamId, CAM_STAT_ACTIVE);
    return true;
}

static void BossTw_ReleaseEncounterControl(BossTw* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 subCamId = this->subCamId;
    s32 validOwnedCamera;
    s32 releasePlayer = false;

    // A live controller removal can occur through debug tools or hooks. Scene teardown has already removed Link and
    // does not need camera work; limiting this to Twinrova's recorded subcamera avoids touching unrelated cutscenes.
    if (player == NULL) {
        return;
    }

    validOwnedCamera = subCamId >= CAM_ID_SUB_FIRST && subCamId < NUM_CAMS && play->cameraPtrs[subCamId] != NULL;
    if (subCamId != CAM_ID_MAIN) {
        if (validOwnedCamera && Play_GetActiveCamId(play) == subCamId) {
            Camera* mainCam = Play_GetCamera(play, CAM_ID_MAIN);
            Camera* ownedCam = play->cameraPtrs[subCamId];

            // Copy what was actually rendered. Some merge shots use the secondary authored vectors rather than the
            // primary pair stored on the actor.
            mainCam->eye = ownedCam->eye;
            mainCam->eyeNext = ownedCam->eye;
            mainCam->at = ownedCam->at;
            func_800C08AC(play, subCamId, 0);
            releasePlayer = true;
        } else if (validOwnedCamera) {
            s16 activeCamId = Play_GetActiveCamId(play);

            // Remove only Twinrova's inactive allocation. If the main camera is already active, the encounter still
            // owns the halted-player state and must release it; an active foreign subcamera keeps that ownership.
            Play_ClearCamera(play, subCamId);
            releasePlayer = activeCamId == CAM_ID_MAIN;
        }
        this->subCamId = CAM_ID_MAIN;
        if (releasePlayer && play->csCtx.state != CS_STATE_IDLE) {
            func_80064534(play, &play->csCtx);
        }
        if (releasePlayer) {
            Player_SetCsActionWithHaltedActors(play, &this->actor, 7);
        }
    }

    // Every Twinrova cutscene can author a strong tint. Restore a neutral environment if the owner disappears before
    // its normal release frame instead of leaving the room permanently flashed or darkened.
    play->envCtx.unk_BE = 1;
    play->envCtx.unk_BD = 1;
    play->envCtx.unk_D8 = 0.0f;
}

void BossTw_Destroy(Actor* thisx, PlayState* play) {
    BossTw* this = (BossTw*)thisx;
    s32 removedCurrentSister = false;

    if (thisx->params == TW_KOTAKE && sKotakePtr == this) {
        sKotakePtr = NULL;
        removedCurrentSister = true;
    } else if (thisx->params == TW_KOUME && sKoumePtr == this) {
        sKoumePtr = NULL;
        removedCurrentSister = true;
    } else if (thisx->params == TW_TWINROVA) {
        // During normal actor-context teardown the younger sisters are freed first. During a live debug/hook removal
        // they may still be active and contain unguarded controller references, so retire both ownership cases here.
        BossTw_ReleaseEncounterControl(this, play);
        BossTw_KillRemainingSistersAndDeathballs(play);
        sKotakePtr = NULL;
        sKoumePtr = NULL;
        BossTw_ClearSiegeZones(this, play, false, TWINROVA_SIEGE_USE_ZONE_ELEMENT, false);
        BossTw_ClearSummonedEnemies(play);
        BossTw_ClearPhaseOneMagic(play, false);
        sTwInitialized = false;
        sTwinrovaPtr = NULL;
    }

    if (removedCurrentSister && GET_PLAYER(play) != NULL && play->transitionTrigger == TRANS_TRIGGER_OFF &&
        sTwinrovaPtr != NULL && sTwinrovaPtr->actor.update != NULL) {
        // The encounter requires both sisters even after fusion: death presentation and elemental ownership still
        // reference them. Treat an unexpected live sister removal as removal of the whole encounter, not a stranded
        // half-boss or a later null dereference. Disable the sibling before this actor is freed because both sisters
        // hold raw parent pointers to one another. Normal allocation rollback clears both globals before destruction.
        BossTw_KillRemainingSistersAndDeathballs(play);
        Actor_Kill(&sTwinrovaPtr->actor);
    }

    Collider_DestroyCylinder(play, &this->collider);
    if (thisx->params < TW_FIRE_BLAST) {
        SkelAnime_Free(&this->skelAnime, play);
    }
}

static Actor* BossTw_FindActorByAddress(PlayState* play, Actor* target) {
    Actor* actor;
    s32 category;

    if (target == NULL) {
        return NULL;
    }

    for (category = 0; category < ARRAY_COUNT(play->actorCtx.actorLists); category++) {
        actor = play->actorCtx.actorLists[category].head;
        while (actor != NULL) {
            if (actor == target) {
                return actor;
            }
            actor = actor->next;
        }
    }

    return NULL;
}

static Actor* BossTw_GetActiveSummon(PlayState* play, TwinrovaSummon* summon) {
    Actor* actor = BossTw_FindActorByAddress(play, summon->actor);

    if (actor == NULL || actor->id != summon->actorId || actor->update == NULL ||
        actor->category != ACTORCAT_ENEMY) {
        memset(summon, 0, sizeof(*summon));
        return NULL;
    }

    return actor;
}

static s32 BossTw_IsPlayerHardDisabled(PlayState* play) {
    return sFreezeState != 0 || (GET_PLAYER(play)->stateFlags2 & PLAYER_STATE2_FROZEN);
}

static s32 BossTw_CountActiveSummons(PlayState* play) {
    s32 count = 0;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sTwinrovaSummons); i++) {
        if (BossTw_GetActiveSummon(play, &sTwinrovaSummons[i]) != NULL) {
            count++;
        }
    }

    return count;
}

static s32 BossTw_CanGroundHazardSummon(PlayState* play) {
    if (sTwinrovaPtr == NULL || sTwinrovaPtr->actor.update == NULL || play->csCtx.state != CS_STATE_IDLE) {
        return false;
    }

    if (sTwinrovaPtr->actionFunc == BossTw_Wait) {
        return true;
    }

    // Normal fused blasts may also turn a missed shot into arena pressure. Signature sequences stay add-free so
    // their shield and floor-pattern rules remain readable.
    return sTwinrovaPtr->visible && sTwinrovaPtr->actor.colChkInfo.health > 0 &&
           (sTwinrovaPtr->actionFunc == BossTw_TwinrovaArriveAtTarget ||
            sTwinrovaPtr->actionFunc == BossTw_TwinrovaChargeBlast ||
            sTwinrovaPtr->actionFunc == BossTw_TwinrovaShootBlast ||
            sTwinrovaPtr->actionFunc == BossTw_TwinrovaDoneBlastShoot ||
            sTwinrovaPtr->actionFunc == BossTw_TwinrovaFly ||
            sTwinrovaPtr->actionFunc == BossTw_TwinrovaSpin ||
            sTwinrovaPtr->actionFunc == BossTw_TwinrovaLaugh);
}

static s32 BossTw_GetProjectedSummonPressure(PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s32 projectedCount = BossTw_CountActiveSummons(play);

    while (actor != NULL) {
        BossTw* groundBlast = (BossTw*)actor;

        if (actor->id == ACTOR_BOSS_TW && actor->update != NULL &&
            (actor->params == TW_FIRE_BLAST_GROUND || actor->params == TW_ICE_BLAST_GROUND) &&
            groundBlast->work[CAN_SHOOT]) {
            // Reserve against the largest possible committed group. A soft cap deliberately permits one whole
            // final group, but beam selection must account for the pressure that warning promises.
            projectedCount += TWINROVA_SUMMON_MAX_GROUP_SIZE;
        }
        actor = actor->next;
    }

    return projectedCount;
}

static s32 BossTw_GetSummonSoftLimit(void) {
    return sTwinrovaPtr != NULL && sTwinrovaPtr->actionFunc != BossTw_Wait ? TWINROVA_FUSED_SUMMON_SOFT_LIMIT
                                                                         : TWINROVA_SUMMON_SOFT_LIMIT;
}

static s32 BossTw_TryReserveSummonGroup(BossTw* groundBlast, PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s32 reservedGroupCount = 0;

    if (!BossTw_CanGroundHazardSummon(play)) {
        return false;
    }

    while (actor != NULL) {
        BossTw* otherBlast = (BossTw*)actor;

        if (actor != &groundBlast->actor && actor->id == ACTOR_BOSS_TW && actor->update != NULL &&
            (actor->params == TW_FIRE_BLAST_GROUND || actor->params == TW_ICE_BLAST_GROUND) &&
            otherBlast->work[CAN_SHOOT] && otherBlast->beamShootState != 0) {
            reservedGroupCount++;
        }
        actor = actor->next;
    }

    // Reservations keep simultaneous hazards honest. The fused phase gets two more slots of sustained pressure,
    // while the first-phase crowd budget and its beam-selection rules remain unchanged.
    return BossTw_CountActiveSummons(play) + reservedGroupCount * TWINROVA_SUMMON_MAX_GROUP_SIZE <=
           BossTw_GetSummonSoftLimit();
}

static s32 BossTw_IsActorInCameraFrustumWithMargin(PlayState* play, Actor* actor, f32 margin) {
    Vec3f clipPos;
    f32 clipW;
    f32 horizontalLimit;

    SkinMatrix_Vec3fMtxFMultXYZW(&play->viewProjectionMtxF, &actor->focus.pos, &clipPos, &clipW);
    if (clipW <= 0.0f) {
        return false;
    }

    horizontalLimit = clipW * margin * Ship_GetExtendedAspectRatioMultiplier();
    return fabsf(clipPos.x) <= horizontalLimit && fabsf(clipPos.y) <= clipW * margin;
}

static s32 BossTw_IsActorInCameraFrustum(PlayState* play, Actor* actor) {
    return BossTw_IsActorInCameraFrustumWithMargin(play, actor, TWINROVA_SUMMON_FRUSTUM_MARGIN);
}

static void BossTw_KillSummonedEnemy(TwinrovaSummon* summon, Actor* actor, PlayState* play) {
    if (actor->id == ACTOR_EN_BB) {
        EnBb_KillFlameTrailImmediate((EnBb*)actor, play);
    }

    Actor_Kill(actor);
    memset(summon, 0, sizeof(*summon));
}

static void BossTw_ClearSummonedEnemies(PlayState* play) {
    Actor* actor;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sTwinrovaSummons); i++) {
        actor = BossTw_GetActiveSummon(play, &sTwinrovaSummons[i]);
        if (actor != NULL) {
            BossTw_KillSummonedEnemy(&sTwinrovaSummons[i], actor, play);
        }
    }
}

static void BossTw_ActivateSummonedEnemy(Actor* actor, PlayState* play) {
    if (actor == NULL || actor->init != NULL || actor->update == NULL) {
        return;
    }

    if (actor->id == ACTOR_EN_WF) {
        EnWf* wolfos = (EnWf*)actor;

        if (wolfos->action == WOLFOS_ACTION_WAIT_TO_APPEAR) {
            EnWf_ActivateImmediately(wolfos);
        }
    } else if (actor->id == ACTOR_EN_BB && actor->params == ENBB_RED) {
        EnBb_ActivateRedImmediately((EnBb*)actor, play);
    }
}

static void BossTw_UpdateSummonedEnemies(PlayState* play) {
    Player* player = GET_PLAYER(play);
    TwinrovaSummon* summon;
    Actor* actor;
    s32 playerIsOnUpperFloor;
    s32 summonIsOnUpperFloor;
    s32 i;

    if (player->stateFlags2 & PLAYER_STATE2_FROZEN) {
        // Enemy ice uses the engine's generic freeze flag instead of Twinrova's private effect state. Preserve the
        // same elemental arbitration rule: an ice lockout immediately extinguishes an existing burn.
        BossTw_ClearPlayerBurn(play);
    }

    if (sTwinrovaSummonUpdateFrame == play->gameplayFrames) {
        return;
    }
    sTwinrovaSummonUpdateFrame = play->gameplayFrames;

    for (i = 0; i < ARRAY_COUNT(sTwinrovaSummons); i++) {
        summon = &sTwinrovaSummons[i];
        actor = BossTw_GetActiveSummon(play, summon);
        if (actor == NULL) {
            continue;
        }
        BossTw_ActivateSummonedEnemy(actor, play);

        if (summon->canFollowBetweenLevels || actor->init != NULL || actor->isTargeted ||
            BossTw_IsActorInCameraFrustum(play, actor)) {
            summon->offscreenTimer = 0;
            continue;
        }

        playerIsOnUpperFloor = player->actor.floorHeight >= TWINROVA_UPPER_FLOOR_MIN_Y;
        summonIsOnUpperFloor = actor->floorHeight >= TWINROVA_UPPER_FLOOR_MIN_Y;
        if (playerIsOnUpperFloor == summonIsOnUpperFloor) {
            summon->offscreenTimer = 0;
            continue;
        }

        // Ground-bound pressure on the other deck cannot participate in the fight. Retire it symmetrically after a
        // short off-camera grace period so dropping below cannot preserve an irrelevant pack to suppress Breaker,
        // spin, or future summons. Aerial followers, targeted enemies, and anything still visible are exempt above.
        summon->offscreenTimer++;
        if (summon->offscreenTimer >= TWINROVA_SUMMON_OFFSCREEN_DESPAWN_DELAY) {
            BossTw_KillSummonedEnemy(summon, actor, play);
        }
    }
}

static s32 BossTw_RegisterSummon(Actor* actor, s32 canFollowBetweenLevels) {
    TwinrovaSummon* summon;
    s32 i;

    for (i = 0; i < ARRAY_COUNT(sTwinrovaSummons); i++) {
        summon = &sTwinrovaSummons[i];
        if (summon->actor == NULL) {
            summon->actor = actor;
            summon->actorId = actor->id;
            summon->canFollowBetweenLevels = canFollowBetweenLevels;
            summon->offscreenTimer = 0;
            return true;
        }
    }

    return false;
}

static s32 BossTw_SpawnSummonedEnemy(PlayState* play, Vec3f* origin, TwinrovaMagicElement element, s16 actorId,
                                      s16 params, f32 heightOffset, f32 spawnRadius, s32 canFollowBetweenLevels) {
    Player* player = GET_PLAYER(play);
    Actor* actor;
    Vec3f spawnPos = *origin;
    f32 spawnFloorY;
    f32 radius = spawnRadius * (0.5f + Rand_ZeroFloat(0.5f));
    s16 spawnAngle = (s16)Rand_ZeroFloat(65536.0f);
    s16 spawnYaw;

    spawnPos.x += Math_SinS(spawnAngle) * radius;
    spawnPos.z += Math_CosS(spawnAngle) * radius;

    if (actorId == ACTOR_EN_BW || actorId == ACTOR_EN_FZ || actorId == ACTOR_EN_WF) {
        CollisionPoly* floorPoly;
        Vec3f floorProbe = spawnPos;
        f32 floorY;

        floorProbe.y += 50.0f;
        floorY = BgCheck_EntityRaycastFloor2(play, &play->colCtx, &floorPoly, &floorProbe);
        if (floorY <= BGCHECK_Y_MIN || fabsf(floorY - origin->y) > 1.0f) {
            spawnPos.x = origin->x;
            spawnPos.z = origin->z;
            floorProbe = *origin;
            floorProbe.y += 50.0f;
            floorY = BgCheck_EntityRaycastFloor2(play, &play->colCtx, &floorPoly, &floorProbe);
        }

        if (floorY > BGCHECK_Y_MIN) {
            spawnPos.y = floorY;
        }
    }

    spawnFloorY = spawnPos.y;
    spawnPos.y += heightOffset;
    spawnYaw = Math_Vec3f_Yaw(&spawnPos, &player->actor.world.pos);

    // These actors must remain parentless; En_Wf interprets any parent as an En_Encount1 spawner.
    actor = Actor_Spawn(&play->actorCtx, play, actorId, spawnPos.x, spawnPos.y, spawnPos.z, 0, spawnYaw, 0, params);
    if (actor == NULL) {
        return false;
    }

    // Some aerial enemies begin hidden and do not run a floor check until they emerge.
    actor->floorHeight = spawnFloorY;
    actor->dropFlag |= ACTOR_DROP_FLAG_NO_DROP;
    BossTw_ActivateSummonedEnemy(actor, play);

    if (!BossTw_RegisterSummon(actor, canFollowBetweenLevels)) {
        if (actor->id == ACTOR_EN_BB) {
            EnBb_KillFlameTrailImmediate((EnBb*)actor, play);
        }
        Actor_Kill(actor);
        return false;
    }

    // Resolve the warning at each actual formation point, especially for immediately activated elite enemies.
    BossTw_SpawnMagicLaunchEffects(play, &spawnPos, element,
                                   actorId == ACTOR_EN_BB || actorId == ACTOR_EN_WF ? 8 : 4);
    return true;
}

static Vec3f BossTw_GetSummonFormationPos(Vec3f* origin, s16 baseAngle, s32 index, s32 count, f32 radius) {
    Vec3f spawnPos = *origin;
    s16 angle = baseAngle + (s16)((0x10000 / count) * index);

    spawnPos.x += Math_SinS(angle) * radius;
    spawnPos.z += Math_CosS(angle) * radius;
    return spawnPos;
}

static Vec3f BossTw_GetGroundSummonFormationPos(PlayState* play, Vec3f* origin, s16 baseAngle, s32 index, s32 count,
                                                 f32 radius) {
    CollisionPoly* floorPoly;
    Vec3f spawnPos = BossTw_GetSummonFormationPos(origin, baseAngle, index, count, radius);
    Vec3f floorProbe = spawnPos;
    f32 floorY;

    floorProbe.y += 80.0f;
    floorY = BgCheck_EntityRaycastFloor2(play, &play->colCtx, &floorPoly, &floorProbe);
    if (floorY <= BGCHECK_Y_MIN || fabsf(floorY - origin->y) > 5.0f) {
        return *origin;
    }

    spawnPos.y = floorY;
    return spawnPos;
}

static s32 BossTw_SpawnElementalKeese(PlayState* play, Vec3f* origin, TwinrovaMagicElement element) {
    s16 params = element == TWINROVA_MAGIC_FIRE ? KEESE_FIRE_FLY : KEESE_ICE_FLY;

    return BossTw_SpawnSummonedEnemy(play, origin, element, ACTOR_EN_FIREFLY, params, 75.0f, 18.0f, true);
}

static s32 BossTw_SpawnElementalGroundEnemy(PlayState* play, Vec3f* origin, TwinrovaMagicElement element) {
    if (element == TWINROVA_MAGIC_FIRE) {
        return BossTw_SpawnSummonedEnemy(play, origin, element, ACTOR_EN_BW, 0, 0.0f, 12.0f, false);
    }
    return BossTw_SpawnSummonedEnemy(play, origin, element, ACTOR_EN_FZ, 0, 0.0f, 0.0f, false);
}

static s32 BossTw_SpawnSummonGroup(PlayState* play, Vec3f* origin, TwinrovaMagicElement element,
                                    TwinrovaSummonGroup group) {
    Vec3f spawnPos;
    s16 baseAngle;
    s32 count;
    s32 i;
    s32 expectedCount = 0;
    s32 spawnedCount = 0;

    // The soft-limit decision was reserved when the visible warning began, so simultaneous warned groups remain
    // valid here. Breaker and Siege never authorize this path.
    if (!BossTw_CanGroundHazardSummon(play)) {
        BossTw_ShowFailedMagicCast(play, origin, element);
        return 0;
    }

    baseAngle = (s16)Rand_ZeroFloat(65536.0f);
    switch (group) {
        case TWINROVA_SUMMON_KEESE_GROUP:
            expectedCount = 3;
            for (i = 0; i < 3; i++) {
                spawnPos = BossTw_GetSummonFormationPos(origin, baseAngle, i, 3, TWINROVA_SUMMON_FORMATION_RADIUS);
                spawnedCount += BossTw_SpawnElementalKeese(play, &spawnPos, element);
            }
            break;

        case TWINROVA_SUMMON_GROUND_GROUP:
            count = element == TWINROVA_MAGIC_FIRE ? 2 : 1;
            expectedCount = count;
            for (i = 0; i < count; i++) {
                spawnPos = BossTw_GetGroundSummonFormationPos(play, origin, baseAngle, i, count,
                                                              TWINROVA_SUMMON_FORMATION_RADIUS * 0.65f);
                spawnedCount += BossTw_SpawnElementalGroundEnemy(play, &spawnPos, element);
            }
            break;

        case TWINROVA_SUMMON_MIXED_GROUP:
            expectedCount = 3;
            for (i = 0; i < 2; i++) {
                spawnPos = BossTw_GetSummonFormationPos(origin, baseAngle, i, 3, TWINROVA_SUMMON_FORMATION_RADIUS);
                spawnedCount += BossTw_SpawnElementalKeese(play, &spawnPos, element);
            }
            spawnPos = BossTw_GetGroundSummonFormationPos(play, origin, baseAngle, 2, 3,
                                                          TWINROVA_SUMMON_FORMATION_RADIUS);
            spawnedCount += BossTw_SpawnElementalGroundEnemy(play, &spawnPos, element);
            break;

        case TWINROVA_SUMMON_ELITE_GROUP:
            expectedCount = 1;
            spawnPos = BossTw_GetGroundSummonFormationPos(play, origin, baseAngle, 0, 1,
                                                          TWINROVA_SUMMON_FORMATION_RADIUS * 0.55f);
            if (element == TWINROVA_MAGIC_FIRE) {
                // A Red Bubble floats, but its AI is tethered to its home floor and cannot return from below.
                spawnedCount +=
                    BossTw_SpawnSummonedEnemy(play, &spawnPos, element, ACTOR_EN_BB, ENBB_RED, 80.0f, 0.0f, false);
            } else {
                spawnedCount +=
                    BossTw_SpawnSummonedEnemy(play, &spawnPos, element, ACTOR_EN_WF, (s16)0xFF01, 0.0f, 0.0f, false);
            }
            break;

        default:
            return 0;
    }

    if (spawnedCount < expectedCount) {
        // Resolve a committed group warning honestly even when only part of its formation fits in the actor pool.
        BossTw_ShowFailedMagicCast(play, origin, element);
    }

    return spawnedCount;
}

static s32 BossTw_ShowCommittedSummonWarning(BossTw* groundBlast, PlayState* play,
                                              TwinrovaMagicElement element) {
    s32 eliteWarning = groundBlast->csState2 == TWINROVA_SUMMON_ELITE_GROUP;

    // Every committed emergence uses the long-lived ring branch so the marker remains visible through all 30
    // warning updates. Elite groups are still distinguished by their larger footprint, particle count, and sound.
    if (!BossTw_AddRingEffect(play, &groundBlast->actor.world.pos, eliteWarning ? 0.8f : 0.55f,
                              eliteWarning ? 4.0f : 3.5f, 255, element, 0, ARRAY_COUNT(sEffects))) {
        return false;
    }
    BossTw_SpawnMagicLaunchEffects(play, &groundBlast->actor.world.pos, element, eliteWarning ? 24 : 12);
    Audio_PlayActorSound2(&groundBlast->actor,
                          eliteWarning ? NA_SE_EN_TWINROBA_TRANSFORM : NA_SE_EN_TWINROBA_MASIC_SET);
    return true;
}

static s32 BossTw_IsSisterActor(Actor* actor) {
    return actor != NULL && actor->id == ACTOR_BOSS_TW &&
           (actor->params == TW_KOTAKE || actor->params == TW_KOUME);
}

static s32 BossTw_IsPhaseOneVolleyBlast(BossTw* this) {
    return (this->actor.params == TW_FIRE_BLAST || this->actor.params == TW_ICE_BLAST) &&
           BossTw_IsSisterActor(this->actor.parent);
}

static s32 BossTw_IsSignatureProjectile(BossTw* this) {
    return this->blastBehavior == TWINROVA_BLAST_SIEGE || this->blastBehavior == TWINROVA_BLAST_BREAKER ||
           this->blastBehavior == TWINROVA_BLAST_MINEFIELD_FOLLOWUP;
}

static s16 BossTw_GetPhaseOneHitCount(void) {
    if (sKotakePtr == NULL || sKoumePtr == NULL) {
        return 0;
    }

    return sKotakePtr->actor.colChkInfo.health + sKoumePtr->actor.colChkInfo.health;
}

static s32 BossTw_IsPhaseOneEscalated(void) {
    return BossTw_GetPhaseOneHitCount() >= TWINROVA_PHASE_ONE_ESCALATION_HITS;
}

static s16 BossTw_GetPhaseOnePacingTier(void) {
    return CLAMP_MAX(BossTw_GetPhaseOneHitCount(), TWINROVA_PHASE_ONE_ESCALATION_HITS + 1);
}

static s16 BossTw_GetPhaseOneTurnTime(void) {
    return TWINROVA_PHASE_ONE_TURN_TIME - (BossTw_GetPhaseOnePacingTier() * 5);
}

static s16 BossTw_GetPortalHiddenTime(BossTw* this) {
    s16 portalMode = this->csState2 & TWINROVA_PORTAL_MODE_MASK;
    s16 baseTime = (portalMode == TWINROVA_PORTAL_MODE_AMBUSH ||
                    portalMode == TWINROVA_PORTAL_MODE_PRESSURE_TRIANGLE)
                       ? TWINROVA_PORTAL_AMBUSH_HIDDEN_TIME
                       : TWINROVA_PORTAL_HIDDEN_TIME;

    return MAX(baseTime - (BossTw_GetPhaseOnePacingTier() * 2), 6);
}

static f32 BossTw_GetPhaseOneComboChance(void) {
    return 0.4f + (BossTw_GetPhaseOnePacingTier() * 0.05f);
}

static s16 BossTw_GetPhaseOneSpecialCooldown(void) {
    return BossTw_IsPhaseOneEscalated() ? TWINROVA_ESCALATED_SPECIAL_MOVE_COOLDOWN
                                        : TWINROVA_SPECIAL_MOVE_COOLDOWN;
}

static s32 BossTw_IsPhaseOneHitGoalReached(void) {
    return BossTw_GetPhaseOneHitCount() >= 4;
}

static Actor* BossTw_GetShieldBlastTarget(BossTw* blast) {
    BossTw* owner;

    if (BossTw_IsPhaseOneVolleyBlast(blast)) {
        owner = (BossTw*)blast->actor.parent;
        if (owner->actor.params == TW_KOUME) {
            return sKotakePtr != NULL ? &sKotakePtr->actor : NULL;
        }

        return sKoumePtr != NULL ? &sKoumePtr->actor : NULL;
    }

    return sTwinrovaPtr != NULL ? &sTwinrovaPtr->actor : NULL;
}

static s32 BossTw_HasActivePhaseOneMagic(PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s16 i;

    while (actor != NULL) {
        if (actor->id == ACTOR_BOSS_TW && actor->update != NULL &&
            (actor->params == TW_FIRE_BLAST || actor->params == TW_ICE_BLAST) &&
            BossTw_IsSisterActor(actor->parent) && ((BossTw*)actor)->csState1 != 2) {
            return true;
        }
        actor = actor->next;
    }

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (sEffects[i].type == TWEFF_SHLD_BLST && BossTw_IsSisterActor(sEffects[i].target)) {
            return true;
        }
    }

    return false;
}

static s32 BossTw_HasCommittedPhaseOneShieldRelease(PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s16 i;

    while (actor != NULL) {
        if (actor->id == ACTOR_BOSS_TW && actor->update != NULL && BossTw_IsPhaseOneVolleyBlast((BossTw*)actor) &&
            ((BossTw*)actor)->csState1 == 10) {
            return true;
        }
        actor = actor->next;
    }

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (sEffects[i].type == TWEFF_SHLD_BLST && BossTw_IsSisterActor(sEffects[i].target)) {
            return true;
        }
    }

    return false;
}

static void BossTw_CancelGroundSummonWarning(BossTw* groundBlast, PlayState* play, s32 showFeedback) {
    if (groundBlast->beamShootState != 0 && showFeedback) {
        TwinrovaMagicElement element = groundBlast->actor.params == TW_FIRE_BLAST_GROUND ? TWINROVA_MAGIC_FIRE
                                                                                         : TWINROVA_MAGIC_ICE;

        BossTw_ShowFailedMagicCast(play, &groundBlast->actor.world.pos, element);
    }

    groundBlast->work[CAN_SHOOT] = false;
    groundBlast->beamShootState = 0;
    groundBlast->timers[3] = 0;
}

static void BossTw_ClearGroundHazards(PlayState* play, s32 resolveSummonWarnings) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    Actor* next;

    while (actor != NULL) {
        next = actor->next;
        if (actor->id == ACTOR_BOSS_TW && actor->update != NULL &&
            (actor->params == TW_FIRE_BLAST_GROUND || actor->params == TW_ICE_BLAST_GROUND)) {
            BossTw_CancelGroundSummonWarning((BossTw*)actor, play, resolveSummonWarnings);
            Actor_Kill(actor);
        }
        actor = next;
    }

    if (sKoumePtr != NULL) {
        sKoumePtr->workf[UNK_F9] = 0.0f;
        sKoumePtr->workf[UNK_F10] = 0.0f;
        sKoumePtr->workf[UNK_F11] = 0.0f;
        sKoumePtr->workf[UNK_F12] = 0.0f;
        sKoumePtr->workf[UNK_F13] = 0.0f;
    }
    if (sKotakePtr != NULL) {
        sKotakePtr->workf[UNK_F9] = 0.0f;
        sKotakePtr->workf[UNK_F11] = 0.0f;
        sKotakePtr->workf[UNK_F12] = 0.0f;
        sKotakePtr->workf[UNK_F14] = 0.0f;
        sKotakePtr->workf[UNK_F15] = 0.0f;
        sKotakePtr->workf[UNK_F16] = 0.0f;
    }

    sGroundBlastType = 0;
}

static void BossTw_ClearPlayerFreeze(PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 i;

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (sEffects[i].type == TWEFF_PLYR_FRZ && sEffects[i].target == NULL) {
            sEffects[i].type = TWEFF_NONE;
            sEffects[i].alpha = 0;
        }
    }

    // The player category is destroyed before bosses during scene teardown. Global/effect state still needs reset,
    // but the controller destructor must not dereference a player that has already left the actor list.
    if (player != NULL) {
        player->stateFlags2 &= ~PLAYER_STATE2_PAUSE_MOST_UPDATING;
    }
    sFreezeState = 0;
}

static void BossTw_ClearPlayerBurn(PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 i;

    if (player == NULL) {
        return;
    }

    player->bodyIsBurning = false;
    for (i = 0; i < ARRAY_COUNT(player->bodyFlameTimers); i++) {
        player->bodyFlameTimers[i] = 0;
    }
}

static void BossTw_BeginPlayerFreeze(PlayState* play) {
    // Ice replaces fire as the active elemental punishment. Keeping burn alive through the long forced freeze would
    // stack two damage-over-time states while Link cannot roll to extinguish it.
    BossTw_ClearPlayerBurn(play);
    if (sFreezeState == 0) {
        sFreezeState = 1;
    }
}

static void BossTw_ClearPhaseOneMagic(PlayState* play, s32 resolveSummonWarnings) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    Actor* next;
    s16 i;

    while (actor != NULL) {
        next = actor->next;
        if (actor->id == ACTOR_BOSS_TW && actor->update != NULL &&
            ((actor->params == TW_FIRE_BLAST || actor->params == TW_ICE_BLAST) ||
             actor->params == TW_FIRE_WARNING_SIGIL || actor->params == TW_ICE_WARNING_SIGIL)) {
            Actor_Kill(actor);
        }
        actor = next;
    }

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (sEffects[i].type == TWEFF_SHLD_BLST || sEffects[i].type == TWEFF_SHLD_DEFL) {
            sEffects[i].alpha = 0;
            sEffects[i].type = TWEFF_NONE;
            sEffects[i].target = NULL;
        }
    }

    BossTw_ClearGroundHazards(play, resolveSummonWarnings);
    BossTw_ClearPlayerFreeze(play);
    BossTw_ClearPlayerBurn(play);
    D_8094C870 = 0;
    sEnvType = 0;
    BossTw_ResetShieldCharge();
    BossTw_ResetPhaseOneScheduler();
}

static BossTw* BossTw_SpawnMagicBlast(BossTw* owner, PlayState* play, Vec3f* spawnPos, s16 magicParams,
                                      TwinrovaBlastBehavior behavior) {
    BossTw* magic = (BossTw*)Actor_SpawnAsChild(&play->actorCtx, &owner->actor, play, ACTOR_BOSS_TW, spawnPos->x,
                                                spawnPos->y, spawnPos->z, 0, 0, 0, magicParams);

    if (magic == NULL) {
        return NULL;
    }

    magic->blastType = magicParams == TW_ICE_BLAST ? TWINROVA_MAGIC_ICE : TWINROVA_MAGIC_FIRE;
    magic->blastBehavior = behavior;
    if (behavior != TWINROVA_BLAST_RAIN) {
        if (BossTw_IsPlayerHardDisabled(play)) {
            // Seed collision grace before the projectile's first update so actor-list insertion order cannot make a
            // new shot hostile during the freeze effect's final update. Movement and lifetime remain uninterrupted.
            magic->timers[3] = TWINROVA_FROZEN_ATTACK_GRACE;
        } else if (BossTw_IsSisterActor(&owner->actor) && owner->actionFunc == BossTw_BlastVolley &&
                   owner->timers[3] != 0) {
            // A volley may keep releasing during the controllable thaw grace. Transfer the caster's remaining
            // collision-only grace so that uninterrupted release cannot produce an immediately hostile new orb.
            magic->timers[3] = owner->timers[3];
        }
    }
    if (behavior == TWINROVA_BLAST_SIEGE || behavior == TWINROVA_BLAST_BREAKER ||
        behavior == TWINROVA_BLAST_LOWER_ROUTE || behavior == TWINROVA_BLAST_MINEFIELD_FOLLOWUP) {
        // Remote routes need an explicit miss timeout just like the signature sequence; they cannot rely on crossing
        // a floor after Link changes elevation.
        magic->timers[1] = TWINROVA_BREAKER_PROJECTILE_LIFETIME;
    }
    if (!BossTw_IsSisterActor(&owner->actor)) {
        sEnvType = magic->blastType + 1;
    }
    return magic;
}

static void BossTw_SpawnMagicLaunchEffects(PlayState* play, Vec3f* spawnPos, s16 blastType, s16 count) {
    Vec3f velocity;
    Vec3f accel = { 0.0f, 0.0f, 0.0f };
    s16 i;

    for (i = 0; i < count; i++) {
        velocity.x = Rand_CenteredFloat(30.0f);
        velocity.y = Rand_CenteredFloat(30.0f);
        velocity.z = Rand_CenteredFloat(30.0f);
        BossTw_AddDotEffect(play, spawnPos, &velocity, &accel, (s16)Rand_ZeroFloat(2.0f) + 11, blastType, 75);
    }
}

static s32 BossTw_IsSiegeZoneActor(Actor* actor) {
    return actor != NULL && actor->id == ACTOR_BOSS_TW &&
           (actor->params == TW_FIRE_SIEGE_ZONE || actor->params == TW_ICE_SIEGE_ZONE);
}

static s32 BossTw_HasSiegeZones(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        if (BossTw_IsSiegeZoneActor(actor) && actor->update != NULL && actor->parent == &owner->actor &&
            ((BossTw*)actor)->blastBehavior != TWINROVA_BLAST_MINEFIELD_POOL &&
            ((BossTw*)actor)->csState1 != TWINROVA_SIEGE_STATE_FADE) {
            return true;
        }
        actor = actor->next;
    }

    return false;
}

static s32 BossTw_IsLowerSiegeZone(BossTw* zone) {
    return zone->actor.world.pos.y < TWINROVA_UPPER_FLOOR_MIN_Y;
}

static s32 BossTw_IsSideSiegeZone(BossTw* zone) {
    return !BossTw_IsLowerSiegeZone(zone) &&
           (fabsf(zone->actor.world.pos.x) > TWINROVA_SIEGE_SIDE_ZONE_OFFSET * 0.5f ||
            fabsf(zone->actor.world.pos.z) > TWINROVA_SIEGE_SIDE_ZONE_OFFSET * 0.5f);
}

static f32 BossTw_GetSiegeZoneRadius(BossTw* zone) {
    if (zone->workf[SIEGE_RADIUS] > 0.0f) {
        return zone->workf[SIEGE_RADIUS];
    }

    if (BossTw_IsLowerSiegeZone(zone)) {
        return TWINROVA_SIEGE_LOWER_ZONE_RADIUS;
    }

    return BossTw_IsSideSiegeZone(zone) ? TWINROVA_SIEGE_SIDE_ZONE_RADIUS : TWINROVA_SIEGE_ZONE_RADIUS;
}

static f32 BossTw_GetSiegeSigilScale(BossTw* zone) {
    return TWINROVA_SIEGE_SIGIL_SCALE * (BossTw_GetSiegeZoneRadius(zone) / TWINROVA_SIEGE_ZONE_RADIUS);
}

static f32 BossTw_GetSiegeCleanseRadius(BossTw* zone) {
    return TWINROVA_SIEGE_CLEANSE_RADIUS +
           (BossTw_GetSiegeZoneRadius(zone) - TWINROVA_SIEGE_ZONE_RADIUS);
}

static void BossTw_UpdateSiegeWedgeTelegraph(BossTw* zone) {
    s32 elapsedTime = TWINROVA_SIEGE_TELEGRAPH_TIME - zone->timers[0];
    s32 rotationAngle;
    s16 angle;

    if (elapsedTime < 0) {
        elapsedTime = 0;
    } else if (elapsedTime > TWINROVA_SIEGE_WEDGE_ROTATION_TIME) {
        elapsedTime = TWINROVA_SIEGE_WEDGE_ROTATION_TIME;
    }

    // Derive the orbit from authored state instead of accumulating position changes. Save/load and update order can
    // never shear the two fans apart, and timer 25 through activation always advertises one frozen layout.
    rotationAngle = (TWINROVA_SIEGE_WEDGE_ROTATION_ARC * elapsedTime) / TWINROVA_SIEGE_WEDGE_ROTATION_TIME;
    angle = (s16)(zone->work[YAW_TGT] + ((s32)zone->beamShootState * rotationAngle));
    zone->actor.world.pos.x = Math_SinS(angle) * zone->workf[UNK_F16];
    zone->actor.world.pos.z = Math_CosS(angle) * zone->workf[UNK_F16];
    if (BossTw_IsLowerSiegeZone(zone)) {
        // Use one continuous square lane instead of conditionally pushing blocked points outward. This keeps the
        // rotating tell smooth, visible, and linked through the lane's wider diagonal corners.
        BossTw_ProjectOntoSquareLane(&zone->actor.world.pos, TWINROVA_SIEGE_WEDGE_LOWER_LANE);
    }
}

static s32 BossTw_IsSiegePatternEligible(TwinrovaSiegePattern pattern, TwinrovaMagicElement element,
                                        s32 playerOnLowerDeck, s32 wedgePatternEligible) {
    if (pattern == TWINROVA_SIEGE_PATTERN_ROTATING_WEDGE) {
        return wedgePatternEligible;
    }
    if (pattern == TWINROVA_SIEGE_PATTERN_INNER_RING || pattern == TWINROVA_SIEGE_PATTERN_OUTER_RING) {
        // Ring control is Ice's lower-deck identity. Keeping it off the upper deck prevents dropping through the floor
        // from becoming a universal, consequence-free answer to a three-second arena tell.
        return element == TWINROVA_MAGIC_ICE && playerOnLowerDeck;
    }
    if (pattern == TWINROVA_SIEGE_PATTERN_DIAGONAL && playerOnLowerDeck) {
        // Fire owns the opposing-quadrant pattern below. Both elements may still use it above, where Ice needs a
        // second legal layout whenever a side-platform position makes the rotating wedge unsafe to author.
        return element == TWINROVA_MAGIC_FIRE;
    }
    return true;
}

static TwinrovaSiegePattern BossTw_SelectSiegePattern(BossTw* owner, Player* player,
                                                       TwinrovaMagicElement element) {
    TwinrovaSiegePattern candidates[TWINROVA_SIEGE_PATTERN_MAX];
    s32 playerOnLowerDeck = player->actor.floorHeight < TWINROVA_UPPER_FLOOR_MIN_Y;
    s32 wedgePatternEligible = playerOnLowerDeck || (fabsf(player->actor.world.pos.x) < 350.0f &&
                                                     fabsf(player->actor.world.pos.z) < 350.0f);
    s16 candidateCount = 0;
    s16 i;

    for (i = 0; i < TWINROVA_SIEGE_PATTERN_MAX; i++) {
        if (i == owner->lastSiegePattern ||
            !BossTw_IsSiegePatternEligible((TwinrovaSiegePattern)i, element, playerOnLowerDeck,
                                           wedgePatternEligible)) {
            continue;
        }
        candidates[candidateCount++] = (TwinrovaSiegePattern)i;
    }

    // Prefer variation. Defensively re-allow the previous pattern if future geometry or element restrictions ever
    // leave no alternative; repeating a truthful layout is safer than indexing an empty candidate list.
    if (candidateCount == 0) {
        for (i = 0; i < TWINROVA_SIEGE_PATTERN_MAX; i++) {
            if (BossTw_IsSiegePatternEligible((TwinrovaSiegePattern)i, element, playerOnLowerDeck,
                                              wedgePatternEligible)) {
                candidates[candidateCount++] = (TwinrovaSiegePattern)i;
            }
        }
    }

    return candidates[(s16)Rand_ZeroFloat(candidateCount - 0.01f)];
}

static BossTw* BossTw_SpawnSiegeZone(BossTw* owner, PlayState* play, s16 params, Vec3f* pos, f32 radius) {
    BossTw* zone = (BossTw*)Actor_SpawnAsChild(&play->actorCtx, &owner->actor, play, ACTOR_BOSS_TW, pos->x, pos->y,
                                               pos->z, 0, 0, 0, params);

    if (zone != NULL) {
        zone->workf[SIEGE_RADIUS] = radius;
    }
    return zone;
}

static BossTw* BossTw_SpawnMinefieldPool(BossTw* mine, PlayState* play) {
    s16 params = mine->blastType == TWINROVA_MAGIC_FIRE ? TW_FIRE_SIEGE_ZONE : TW_ICE_SIEGE_ZONE;
    f32 radius = mine->blastType == TWINROVA_MAGIC_FIRE ? TWINROVA_MINEFIELD_FIRE_POOL_RADIUS
                                                         : TWINROVA_MINEFIELD_ICE_PATCH_RADIUS;
    Actor* ownerActor = BossTw_FindActorByAddress(play, mine->actor.parent);
    BossTw* pool;

    if (ownerActor == NULL || ownerActor->update == NULL || ownerActor->id != ACTOR_BOSS_TW ||
        ownerActor->params != TW_TWINROVA) {
        return NULL;
    }
    if (BossTw_CountActiveMinefieldPools(play, (BossTw*)ownerActor) >= TWINROVA_MINEFIELD_MAX_POOL_ACTORS) {
        return NULL;
    }

    pool = BossTw_SpawnSiegeZone((BossTw*)ownerActor, play, params, &mine->targetPos, radius);

    if (pool != NULL) {
        pool->blastBehavior = TWINROVA_BLAST_MINEFIELD_POOL;
        pool->csState1 = TWINROVA_SIEGE_STATE_ACTIVE;
        pool->timers[0] = TWINROVA_MINEFIELD_POOL_TIME;
        pool->workf[UNK_F17] = BossTw_GetSiegeSigilScale(pool);
        pool->workf[UNK_F18] = 255.0f;
        // Mine pools are compact residual hazards only; they never reserve or release a ground-hazard summon.
        pool->work[CAN_SHOOT] = false;
        pool->beamShootState = 0;
    }

    return pool;
}

static s32 BossTw_IsFusedDirectBlast(BossTw* blast, BossTw* owner) {
    return blast != NULL && blast->actor.update != NULL &&
           (blast->actor.params == TW_FIRE_BLAST || blast->actor.params == TW_ICE_BLAST) &&
           blast->actor.parent == &owner->actor;
}

static s32 BossTw_IsCycloneRain(BossTw* blast) {
    return blast != NULL && blast->blastBehavior == TWINROVA_BLAST_RAIN && sTwinrovaPtr != NULL &&
           BossTw_IsFusedDirectBlast(blast, sTwinrovaPtr);
}

static s16 BossTw_CountCycloneRainProjectiles(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s16 count = 0;

    while (actor != NULL) {
        if (actor->id == ACTOR_BOSS_TW && BossTw_IsFusedDirectBlast((BossTw*)actor, owner) &&
            ((BossTw*)actor)->blastBehavior == TWINROVA_BLAST_RAIN) {
            // Count the short impact fade too. It still occupies an actor slot even though its damage is over.
            count++;
        }
        actor = actor->next;
    }

    return count;
}

static s32 BossTw_HasActiveFusedDirectBlast(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        BossTw* blast = (BossTw*)actor;

        if (actor->id == ACTOR_BOSS_TW && BossTw_IsFusedDirectBlast(blast, owner)) {
            if (blast->blastBehavior == TWINROVA_BLAST_MINEFIELD) {
                // Mine states are not projectile states: numeric state 2 is ARMED, not impact/fade. Keep every
                // authoritative mine state active and release scheduling only when its dedicated fade begins.
                if (blast->csState1 < TWINROVA_MINE_STATE_FADE) {
                    return true;
                }
            } else if (blast->csState1 != 2) {
                return true;
            }
        }
        actor = actor->next;
    }

    return false;
}

static s32 BossTw_HasActiveFusedNonMineBlast(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        BossTw* blast = (BossTw*)actor;

        if (actor->id == ACTOR_BOSS_TW && BossTw_IsFusedDirectBlast(blast, owner) &&
            blast->blastBehavior != TWINROVA_BLAST_MINEFIELD && blast->csState1 != 2) {
            return true;
        }
        actor = actor->next;
    }

    return false;
}

static s32 BossTw_CountActiveMinefieldMines(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s32 count = 0;

    while (actor != NULL) {
        if (actor->id == ACTOR_BOSS_TW && BossTw_IsFusedDirectBlast((BossTw*)actor, owner) &&
            ((BossTw*)actor)->blastBehavior == TWINROVA_BLAST_MINEFIELD &&
            ((BossTw*)actor)->csState1 < TWINROVA_MINE_STATE_FADE) {
            count++;
        }
        actor = actor->next;
    }

    return count;
}

static s32 BossTw_HasActiveMinefieldMines(PlayState* play, BossTw* owner) {
    return BossTw_CountActiveMinefieldMines(play, owner) != 0;
}

static s32 BossTw_AreAllMinefieldMinesArmed(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s32 foundMine = false;

    while (actor != NULL) {
        BossTw* mine = (BossTw*)actor;

        if (actor->id == ACTOR_BOSS_TW && BossTw_IsFusedDirectBlast(mine, owner) &&
            mine->blastBehavior == TWINROVA_BLAST_MINEFIELD && mine->csState1 < TWINROVA_MINE_STATE_FADE) {
            foundMine = true;
            if (mine->csState1 < TWINROVA_MINE_STATE_ARM) {
                return false;
            }
        }
        actor = actor->next;
    }

    return foundMine;
}

static void BossTw_AccelerateMinefieldFuses(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        BossTw* mine = (BossTw*)actor;

        if (actor->id == ACTOR_BOSS_TW && BossTw_IsFusedDirectBlast(mine, owner) &&
            mine->blastBehavior == TWINROVA_BLAST_MINEFIELD && mine->csState1 == TWINROVA_MINE_STATE_ARM) {
            s16 acceleratedTime = TWINROVA_MINEFIELD_POST_PACKAGE_FUSE_TIME +
                                  mine->work[TWINROVA_MINE_DETONATION_ORDER] *
                                      TWINROVA_MINEFIELD_POST_PACKAGE_STAGGER;

            if (mine->timers[0] > acceleratedTime) {
                mine->timers[0] = acceleratedTime;
            }
        }
        actor = actor->next;
    }
}

static s32 BossTw_CountActiveMinefieldPools(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s32 count = 0;

    while (actor != NULL) {
        BossTw* pool = (BossTw*)actor;

        if (BossTw_IsSiegeZoneActor(actor) && actor->update != NULL && actor->parent == &owner->actor &&
            pool->blastBehavior == TWINROVA_BLAST_MINEFIELD_POOL &&
            pool->csState1 == TWINROVA_SIEGE_STATE_ACTIVE && pool->timers[0] != 0) {
            count++;
        }
        actor = actor->next;
    }

    return count;
}

static s32 BossTw_IsFinalMinefieldDetonation(BossTw* mine, PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        BossTw* other = (BossTw*)actor;

        if (actor != &mine->actor && actor->id == ACTOR_BOSS_TW &&
            BossTw_IsFusedDirectBlast(other, (BossTw*)mine->actor.parent) &&
            other->blastBehavior == TWINROVA_BLAST_MINEFIELD && other->csState1 < TWINROVA_MINE_STATE_FADE &&
            other->work[TWINROVA_MINE_DETONATION_ORDER] > mine->work[TWINROVA_MINE_DETONATION_ORDER]) {
            return false;
        }
        actor = actor->next;
    }

    return true;
}

static void BossTw_ResetShieldCharge(void) {
    sShieldFireCharge = 0;
    sShieldIceCharge = 0;
    D_8094C854 = 0.0f;
    D_8094C858 = 0.0f;
    D_8094C86F = 0;
    D_8094C872 = 0;
}

static void BossTw_UpdateShieldChargeState(void) {
    s16 chargeLevel = sShieldFireCharge | sShieldIceCharge;

    if (chargeLevel == 1) {
        Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_CHARGE_LV1 & ~SFX_FLAG);
    } else if (chargeLevel == 2) {
        Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_CHARGE_LV2 & ~SFX_FLAG);
    } else if (chargeLevel == 3) {
        Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_CHARGE_LV3 & ~SFX_FLAG);
    }

    if (chargeLevel != 0 && chargeLevel < 4) {
        Math_ApproachF(&D_8094C854, 255.0f, 1.0f, 20.0f);
    } else if (chargeLevel == 0) {
        D_8094C854 = 0.0f;
    } else {
        Math_ApproachF(&D_8094C854, 0.0f, 1.0f, 10.0f);
        if (D_8094C854 == 0.0f) {
            sShieldIceCharge = 0;
            sShieldFireCharge = 0;
        }
    }

    if (D_8094C86F != 0) {
        f32 step = D_8094C872 > 0 ? 100.0f : 60.0f;

        D_8094C86F--;
        Math_ApproachF(&D_8094C858, 255.0f, 1.0f, step);
    } else {
        f32 step = D_8094C872 > 0 ? 40.0f : 20.0f;

        Math_ApproachF(&D_8094C858, 0.0f, 1.0f, step);
    }
}

static void BossTw_DisruptShieldCharge(PlayState* play, TwinrovaMagicElement incomingElement) {
    s32 hadCharge = sShieldFireCharge != 0 || sShieldIceCharge != 0;
    s32 hadCommittedCharge = sShieldFireCharge >= 3 || sShieldIceCharge >= 3;

    BossTw_ResetShieldCharge();
    if (!hadCharge) {
        return;
    }

    if (!hadCommittedCharge && sTwinrovaPtr != NULL && sTwinrovaPtr->actor.update != NULL &&
        sTwinrovaPtr->visible && sTwinrovaPtr->actionFunc != BossTw_TwinrovaBreakerSequence) {
        // After an opposing hit, restart a full three-shot package so the player cannot be left in a
        // deterministic reset-at-one / finish-at-two loop. The Breaker already authors its own recovery run.
        sFixedBlastType = sTwinrovaBlastType;
        sFixedBlatSeq = 0;
    }

    // A charge shatter is feedback only. The vanilla deflection effect ricochets into a real ground hazard.
    BossTw_AddShieldHitEffect(play, 16.0f, incomingElement);
    play->envCtx.unk_D8 = 1.0f;
    Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_REFLECT_MG2);
}

static s32 BossTw_HasCommittedFusedShieldRelease(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    s16 i;

    while (actor != NULL) {
        if (actor->id == ACTOR_BOSS_TW && BossTw_IsFusedDirectBlast((BossTw*)actor, owner) &&
            ((BossTw*)actor)->csState1 == 10) {
            return true;
        }
        actor = actor->next;
    }

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (sEffects[i].type == TWEFF_SHLD_BLST &&
            (sEffects[i].target == &owner->actor || (sEffects[i].target == NULL && owner == sTwinrovaPtr))) {
            return true;
        }
    }

    return false;
}

static s32 BossTw_HasActiveBreakerProjectile(PlayState* play, BossTw* owner) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        if (actor->id == ACTOR_BOSS_TW && BossTw_IsFusedDirectBlast((BossTw*)actor, owner) &&
            (((BossTw*)actor)->blastBehavior == TWINROVA_BLAST_SIEGE ||
             ((BossTw*)actor)->blastBehavior == TWINROVA_BLAST_BREAKER) &&
            ((BossTw*)actor)->csState1 != 2) {
            return true;
        }
        actor = actor->next;
    }

    return false;
}

static void BossTw_SpawnSiegeBurst(BossTw* zone, PlayState* play, s16 count, s16 burstElement) {
    Vec3f pos;
    Vec3f velocity;
    Vec3f accel = { 0.0f, 0.1f, 0.0f };
    f32 radius;
    f32 zoneRadius = BossTw_GetSiegeZoneRadius(zone);
    s16 angle;
    s16 i;

    if (burstElement == TWINROVA_SIEGE_USE_ZONE_ELEMENT) {
        burstElement = zone->blastType;
    }

    // Lower-floor copies, side platforms, and compact Ring nodes keep full authoritative sigils. Spend fewer shared
    // effect slots on their decorative particles so six simultaneous zones cannot crowd out the projectile tell.
    if (BossTw_IsLowerSiegeZone(zone) || BossTw_IsSideSiegeZone(zone) ||
        zoneRadius < TWINROVA_SIEGE_ZONE_RADIUS) {
        count = (count + 1) / 2;
    }

    BossTw_AddRingEffect(play, &zone->actor.world.pos,
                         0.4f * (zoneRadius / TWINROVA_SIEGE_ZONE_RADIUS), 3.0f, 255, burstElement, 1,
                         ARRAY_COUNT(sEffects));
    for (i = 0; i < count; i++) {
        pos = zone->actor.world.pos;
        radius = sqrtf(Rand_ZeroOne()) * zoneRadius;
        angle = (s16)Rand_ZeroFloat(65536.0f);
        pos.x += Math_SinS(angle) * radius;
        pos.y += Rand_ZeroFloat(20.0f);
        pos.z += Math_CosS(angle) * radius;
        velocity.x = Rand_CenteredFloat(8.0f);
        velocity.y = Rand_ZeroFloat(8.0f) + 2.0f;
        velocity.z = Rand_CenteredFloat(8.0f);
        BossTw_AddFlameEffect(play, &pos, &velocity, &accel, Rand_ZeroFloat(8.0f) + 16.0f, burstElement);
    }
}

static void BossTw_ClearSiegeZone(BossTw* zone, PlayState* play, s32 spawnEffects, s16 burstElement) {
    if (spawnEffects) {
        BossTw_SpawnSiegeBurst(zone, play, 10, burstElement);
    }
    Actor_Kill(&zone->actor);
}

static void BossTw_ClearSiegeZones(BossTw* owner, PlayState* play, s32 spawnEffects, s16 burstElement,
                                   s32 preserveMinefield) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    Actor* next;

    while (actor != NULL) {
        next = actor->next;
        if (BossTw_IsSiegeZoneActor(actor) && actor->update != NULL && actor->parent == &owner->actor &&
            (!preserveMinefield || ((BossTw*)actor)->blastBehavior != TWINROVA_BLAST_MINEFIELD_POOL)) {
            BossTw_ClearSiegeZone((BossTw*)actor, play, spawnEffects, burstElement);
        }
        actor = next;
    }
}

static void BossTw_ArmIceSiegeFreezeCooldown(BossTw* owner, PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        if (BossTw_IsSiegeZoneActor(actor) && actor->update != NULL && actor->parent == &owner->actor &&
            ((BossTw*)actor)->blastType == TWINROVA_MAGIC_ICE) {
            ((BossTw*)actor)->work[BURN_TMR] = TWINROVA_SIEGE_FREEZE_COOLDOWN;
        }
        actor = actor->next;
    }
}

static void BossTw_PlaySiegeCue(BossTw* source, PlayState* play, u16 sfxId) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    Actor* nearestOnDeck = NULL;
    Actor* nearestFallback = NULL;
    Player* player = GET_PLAYER(play);
    f32 nearestOnDeckDistSq = 0.0f;
    f32 nearestFallbackDistSq = 0.0f;
    s32 playerOnLowerFloor = player->actor.floorHeight < TWINROVA_UPPER_FLOOR_MIN_Y;

    while (actor != NULL) {
        if (BossTw_IsSiegeZoneActor(actor) && actor->update != NULL && actor->parent == source->actor.parent &&
            ((BossTw*)actor)->csState1 != TWINROVA_SIEGE_STATE_FADE) {
            f32 xDiff = actor->world.pos.x - player->actor.world.pos.x;
            f32 zDiff = actor->world.pos.z - player->actor.world.pos.z;
            f32 distSq = SQ(xDiff) + SQ(zDiff);

            if (nearestFallback == NULL || distSq < nearestFallbackDistSq) {
                nearestFallback = actor;
                nearestFallbackDistSq = distSq;
            }
            if (BossTw_IsLowerSiegeZone((BossTw*)actor) == playerOnLowerFloor &&
                (nearestOnDeck == NULL || distSq < nearestOnDeckDistSq)) {
                nearestOnDeck = actor;
                nearestOnDeckDistSq = distSq;
            }
        }
        actor = actor->next;
    }

    if (nearestOnDeck == &source->actor) {
        Audio_PlayActorSound2(&source->actor, sfxId);
    } else if (nearestOnDeck == NULL && nearestFallback == &source->actor) {
        // Ring patterns mark only their starting deck, and a charged wake can cleanse every local Wedge node. Keep any
        // remote remainder audible without falsely pointing Link toward the other floor.
        Sfx_PlaySfxCentered(sfxId);
    }
}

static void BossTw_ClearSiegeZonesNearShieldBlast(BossTw* owner, PlayState* play, Vec3f* blastPos,
                                                    TwinrovaMagicElement blastElement) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    Actor* next;
    s32 clearedAny = false;

    while (actor != NULL) {
        BossTw* zone = (BossTw*)actor;

        next = actor->next;
        // The charged stream carries a broad Siege-only cleansing wake, while its actual damage core stays narrow.
        // Minefield is independent arena pressure and deliberately survives both the wake and the resulting stun.
        if (BossTw_IsSiegeZoneActor(actor) && actor->update != NULL && actor->parent == &owner->actor &&
            zone->blastBehavior != TWINROVA_BLAST_MINEFIELD_POOL && zone->blastType != blastElement &&
            fabsf(actor->world.pos.y - blastPos->y) < TWINROVA_SIEGE_CLEANSE_HEIGHT &&
            (SQ(actor->world.pos.x - blastPos->x) + SQ(actor->world.pos.z - blastPos->z)) <
                SQ(BossTw_GetSiegeCleanseRadius(zone))) {
            BossTw_ClearSiegeZone(zone, play, true, blastElement);
            clearedAny = true;
        }
        actor = next;
    }

    if (clearedAny) {
        SoundSource_PlaySfxAtFixedWorldPos(
            play, blastPos, 20,
            blastElement == TWINROVA_MAGIC_FIRE ? NA_SE_EV_ICE_MELT : NA_SE_EN_EXTINCT);
    }
}

static s32 BossTw_SpawnSiegeZones(BossTw* owner, PlayState* play, TwinrovaMagicElement element) {
    Player* player = GET_PLAYER(play);
    TwinrovaSiegePattern pattern = BossTw_SelectSiegePattern(owner, player, element);
    BossTw* zone;
    Vec3f zonePos[TWINROVA_SIEGE_ZONE_COUNT];
    f32 zoneRadius[TWINROVA_SIEGE_ZONE_COUNT];
    f32 diagonal;
    f32 safeSide;
    s32 sideAxis;
    s32 blockedAxis;
    f32 side;
    f32 playerSafeAxisPos;
    f32 ringOrbit;
    f32 ringRadius;
    Vec3f wedgeTargetA;
    Vec3f wedgeTargetB;
    f32 wedgeTargetADistSq;
    f32 wedgeTargetBDistSq;
    Vec3f lowerTargetA;
    Vec3f lowerTargetB;
    f32 lowerTargetADistSq;
    f32 lowerTargetBDistSq;
    s32 useLowerTargetA;
    s16 ringAngle;
    s16 wedgeStartSafeAngle = 0;
    s16 wedgeDirection = 0;
    s16 wedgeFinalSafeAngle;
    s16 wedgePlayerAngle;
    s16 wedgeTargetAngleA;
    s16 wedgeTargetAngleB;
    s16 lowerBaseAngle;
    s32 lowerWedge;
    s16 params = element == TWINROVA_MAGIC_FIRE ? TW_FIRE_SIEGE_ZONE : TW_ICE_SIEGE_ZONE;
    s16 spawnedZoneCount = 0;
    s16 i;

    BossTw_ClearSiegeZones(owner, play, false, TWINROVA_SIEGE_USE_ZONE_ELEMENT, false);

    switch (pattern) {
        case TWINROVA_SIEGE_PATTERN_CROSSCUT:
            blockedAxis = Rand_ZeroOne() < 0.5f ? 0 : 1;
            playerSafeAxisPos = blockedAxis == 0 ? player->actor.world.pos.z : player->actor.world.pos.x;
            safeSide = playerSafeAxisPos > 0.0f ? -1.0f : 1.0f;
            for (i = 0; i < TWINROVA_SIEGE_ZONE_COUNT; i++) {
                s32 lowerZone = i >= 2 && i < 4;
                f32 zoneOffset = lowerZone ? TWINROVA_SIEGE_CROSSCUT_LOWER_OFFSET
                                           : (i >= 4 ? TWINROVA_SIEGE_SIDE_ZONE_OFFSET
                                                     : TWINROVA_SIEGE_CROSSCUT_UPPER_OFFSET);

                side = (i & 1) == 0 ? -1.0f : 1.0f;
                zonePos[i].x = blockedAxis == 0 ? side * zoneOffset : 0.0f;
                zonePos[i].y = lowerZone ? TWINROVA_SIEGE_LOWER_FLOOR_Y
                                         : (i >= 4 ? TWINROVA_SIEGE_SIDE_FLOOR_Y : 241.0f);
                zonePos[i].z = blockedAxis == 1 ? side * zoneOffset : 0.0f;
                zoneRadius[i] = lowerZone ? TWINROVA_SIEGE_LOWER_ZONE_RADIUS
                                          : (i >= 4 ? TWINROVA_SIEGE_SIDE_ZONE_RADIUS
                                                    : TWINROVA_SIEGE_ZONE_RADIUS);
            }
            owner->targetPos.x = blockedAxis == 1 ? safeSide * TWINROVA_SIEGE_SAFE_SIDE_DISTANCE : 0.0f;
            owner->targetPos.z = blockedAxis == 0 ? safeSide * TWINROVA_SIEGE_SAFE_SIDE_DISTANCE : 0.0f;
            break;

        case TWINROVA_SIEGE_PATTERN_INNER_RING:
        case TWINROVA_SIEGE_PATTERN_OUTER_RING:
            ringOrbit = pattern == TWINROVA_SIEGE_PATTERN_INNER_RING ? TWINROVA_SIEGE_INNER_RING_ORBIT
                                                                     : TWINROVA_SIEGE_OUTER_RING_ORBIT;
            ringRadius = pattern == TWINROVA_SIEGE_PATTERN_INNER_RING ? TWINROVA_SIEGE_INNER_RING_RADIUS
                                                                      : TWINROVA_SIEGE_OUTER_RING_RADIUS;
            // Choose only rotations whose zone centers remain visible between the raised center and side platforms.
            // Random unrestricted angles can put a perfectly accurate lower-floor sigil underneath solid geometry.
            if (pattern == TWINROVA_SIEGE_PATTERN_INNER_RING) {
                ringAngle = Rand_ZeroOne() < 0.5f ? 0 : 0x1555;
            } else {
                ringAngle = Rand_ZeroOne() < 0.5f ? 0x0AAB : 0x2000;
            }
            for (i = 0; i < TWINROVA_SIEGE_ZONE_COUNT; i++) {
                s16 angle = (s16)(ringAngle + (i * TWINROVA_SIEGE_RING_ANGLE_STEP));

                zonePos[i].x = Math_SinS(angle) * ringOrbit;
                zonePos[i].y = TWINROVA_SIEGE_LOWER_FLOOR_Y;
                zonePos[i].z = Math_CosS(angle) * ringOrbit;
                zoneRadius[i] = ringRadius;
            }
            // The linked circles form a truthful hazard ring. Attack across its center, rather than vertically from
            // above it, so Mirror Shield interception stays reliable and the return path reads across the layout.
            if (fabsf(player->actor.world.pos.x) >= fabsf(player->actor.world.pos.z)) {
                owner->targetPos.x = player->actor.world.pos.x > 0.0f ? -TWINROVA_SIEGE_RING_BOSS_OFFSET
                                                                     : TWINROVA_SIEGE_RING_BOSS_OFFSET;
                owner->targetPos.z = 0.0f;
            } else {
                owner->targetPos.x = 0.0f;
                owner->targetPos.z = player->actor.world.pos.z > 0.0f ? -TWINROVA_SIEGE_RING_BOSS_OFFSET
                                                                     : TWINROVA_SIEGE_RING_BOSS_OFFSET;
            }
            break;

        case TWINROVA_SIEGE_PATTERN_ROTATING_WEDGE: {
            lowerWedge = player->actor.floorHeight < TWINROVA_UPPER_FLOOR_MIN_Y;
            if ((SQ(player->actor.world.pos.x) + SQ(player->actor.world.pos.z)) <
                SQ(TWINROVA_SIEGE_WEDGE_CENTER_THRESHOLD)) {
                wedgePlayerAngle = player->actor.shape.rot.y;
            } else {
                wedgePlayerAngle =
                    Math_FAtan2F(player->actor.world.pos.x, player->actor.world.pos.z) * (32768.0f / M_PI);
            }
            wedgeDirection = Rand_ZeroOne() < 0.5f ? -1 : 1;
            wedgeFinalSafeAngle = (s16)(wedgePlayerAngle + (wedgeDirection * TWINROVA_SIEGE_WEDGE_PLAYER_LEAD));
            wedgeStartSafeAngle =
                (s16)(wedgeFinalSafeAngle - (wedgeDirection * TWINROVA_SIEGE_WEDGE_ROTATION_ARC));

            for (i = 0; i < TWINROVA_SIEGE_ZONE_COUNT; i++) {
                s32 lowerZone = i >= TWINROVA_SIEGE_WEDGE_NODES_PER_DECK;
                s16 nodeIndex = (i % TWINROVA_SIEGE_WEDGE_NODES_PER_DECK) + 1;
                s16 angle =
                    (s16)(wedgeStartSafeAngle + (nodeIndex * TWINROVA_SIEGE_WEDGE_NODE_ANGLE_STEP));
                f32 orbit = lowerZone ? TWINROVA_SIEGE_WEDGE_LOWER_LANE : TWINROVA_SIEGE_WEDGE_UPPER_ORBIT;

                zonePos[i].x = Math_SinS(angle) * orbit;
                zonePos[i].y = lowerZone ? TWINROVA_SIEGE_LOWER_FLOOR_Y : 241.0f;
                zonePos[i].z = Math_CosS(angle) * orbit;
                if (lowerZone) {
                    BossTw_ProjectOntoSquareLane(&zonePos[i], TWINROVA_SIEGE_WEDGE_LOWER_LANE);
                }
                zoneRadius[i] = lowerZone ? TWINROVA_SIEGE_WEDGE_LOWER_RADIUS
                                          : TWINROVA_SIEGE_WEDGE_UPPER_RADIUS;
            }

            if (lowerWedge) {
                // Shared lower-floor routing negates this provisional vector before choosing its two outer launch
                // candidates. Point it away from the gap so both final candidates remain within the safe sector.
                owner->targetPos.x = -Math_SinS(wedgeFinalSafeAngle) * TWINROVA_SIEGE_RING_BOSS_OFFSET;
                owner->targetPos.z = -Math_CosS(wedgeFinalSafeAngle) * TWINROVA_SIEGE_RING_BOSS_OFFSET;
            } else {
                wedgeTargetAngleA = (s16)(wedgeFinalSafeAngle + TWINROVA_SIEGE_WEDGE_ATTACK_ANGLE_OFFSET);
                wedgeTargetAngleB = (s16)(wedgeFinalSafeAngle - TWINROVA_SIEGE_WEDGE_ATTACK_ANGLE_OFFSET);
                wedgeTargetA.x = Math_SinS(wedgeTargetAngleA) * TWINROVA_SIEGE_WEDGE_UPPER_ATTACK_RADIUS;
                wedgeTargetA.z = Math_CosS(wedgeTargetAngleA) * TWINROVA_SIEGE_WEDGE_UPPER_ATTACK_RADIUS;
                wedgeTargetB.x = Math_SinS(wedgeTargetAngleB) * TWINROVA_SIEGE_WEDGE_UPPER_ATTACK_RADIUS;
                wedgeTargetB.z = Math_CosS(wedgeTargetAngleB) * TWINROVA_SIEGE_WEDGE_UPPER_ATTACK_RADIUS;
                wedgeTargetADistSq = SQ(wedgeTargetA.x - player->actor.world.pos.x) +
                                     SQ(wedgeTargetA.z - player->actor.world.pos.z);
                wedgeTargetBDistSq = SQ(wedgeTargetB.x - player->actor.world.pos.x) +
                                     SQ(wedgeTargetB.z - player->actor.world.pos.z);
                if (wedgeTargetADistSq >= wedgeTargetBDistSq) {
                    owner->targetPos.x = wedgeTargetA.x;
                    owner->targetPos.z = wedgeTargetA.z;
                } else {
                    owner->targetPos.x = wedgeTargetB.x;
                    owner->targetPos.z = wedgeTargetB.z;
                }
            }
            break;
        }

        case TWINROVA_SIEGE_PATTERN_DIAGONAL:
        default:
            diagonal = Rand_ZeroOne() < 0.5f ? 1.0f : -1.0f;
            safeSide = (player->actor.world.pos.x - (diagonal * player->actor.world.pos.z)) > 0.0f ? -1.0f : 1.0f;
            if (fabsf(player->actor.world.pos.x) > TWINROVA_SIEGE_SIDE_ZONE_OFFSET * 0.5f ||
                fabsf(player->actor.world.pos.z) > TWINROVA_SIEGE_SIDE_ZONE_OFFSET * 0.5f) {
                sideAxis = fabsf(player->actor.world.pos.x) >= fabsf(player->actor.world.pos.z) ? 0 : 1;
            } else {
                sideAxis = Rand_ZeroOne() < 0.5f ? 0 : 1;
            }

            for (i = 0; i < TWINROVA_SIEGE_ZONE_COUNT; i++) {
                s32 lowerZone = i >= 2 && i < 4;
                s32 sideZone = i >= 4;
                f32 zoneOffset = lowerZone ? TWINROVA_SIEGE_LOWER_ZONE_OFFSET : TWINROVA_SIEGE_ZONE_OFFSET;

                side = (i & 1) == 0 ? -1.0f : 1.0f;
                if (sideZone) {
                    zonePos[i].x = sideAxis == 0 ? side * TWINROVA_SIEGE_SIDE_ZONE_OFFSET : 0.0f;
                    zonePos[i].z = sideAxis == 1 ? side * TWINROVA_SIEGE_SIDE_ZONE_OFFSET : 0.0f;
                    zonePos[i].y = TWINROVA_SIEGE_SIDE_FLOOR_Y;
                    zoneRadius[i] = TWINROVA_SIEGE_SIDE_ZONE_RADIUS;
                } else {
                    zonePos[i].x = side * zoneOffset;
                    zonePos[i].z = side * diagonal * zoneOffset;
                    zonePos[i].y = lowerZone ? TWINROVA_SIEGE_LOWER_FLOOR_Y : 241.0f;
                    zoneRadius[i] = lowerZone ? TWINROVA_SIEGE_LOWER_ZONE_RADIUS : TWINROVA_SIEGE_ZONE_RADIUS;
                }
            }
            owner->targetPos.x = safeSide * TWINROVA_SIEGE_SAFE_SIDE_DISTANCE;
            owner->targetPos.z = -safeSide * diagonal * TWINROVA_SIEGE_SAFE_SIDE_DISTANCE;
            break;
    }

    for (i = 0; i < TWINROVA_SIEGE_ZONE_COUNT; i++) {
        zone = BossTw_SpawnSiegeZone(owner, play, params, &zonePos[i], zoneRadius[i]);
        if (zone != NULL) {
            zone->csState2 = pattern;
            if (pattern == TWINROVA_SIEGE_PATTERN_ROTATING_WEDGE) {
                // These fields are private to the Siege zone update: immutable start angle, shared rotation direction,
                // and orbit radius. No overlay-global phase is needed, so save states remain deterministic.
                s32 lowerZone = i >= TWINROVA_SIEGE_WEDGE_NODES_PER_DECK;
                s16 nodeIndex = (i % TWINROVA_SIEGE_WEDGE_NODES_PER_DECK) + 1;

                zone->work[YAW_TGT] =
                    (s16)(wedgeStartSafeAngle + (nodeIndex * TWINROVA_SIEGE_WEDGE_NODE_ANGLE_STEP));
                zone->beamShootState = wedgeDirection;
                zone->workf[UNK_F16] =
                    lowerZone ? TWINROVA_SIEGE_WEDGE_LOWER_LANE : TWINROVA_SIEGE_WEDGE_UPPER_ORBIT;
            }
            spawnedZoneCount++;
        }
    }

    if (spawnedZoneCount != TWINROVA_SIEGE_ZONE_COUNT) {
        // Never run an incomplete or invisible Siege when the actor pool is exhausted.
        BossTw_ClearSiegeZones(owner, play, false, TWINROVA_SIEGE_USE_ZONE_ELEMENT, false);
        return false;
    }

    owner->lastSiegePattern = pattern;
    // On the lower deck, commit from the player's safe outer sector. The angular offset keeps Twinrova far enough
    // away for a readable shield intercept, while the outer radius avoids every raised platform footprint.
    if (player->actor.floorHeight < TWINROVA_UPPER_FLOOR_MIN_Y) {
        lowerBaseAngle =
            Math_FAtan2F(-owner->targetPos.x, -owner->targetPos.z) * (32768.0f / M_PI);
        lowerTargetA.x =
            Math_SinS(lowerBaseAngle + TWINROVA_SIEGE_LOWER_ATTACK_ANGLE_OFFSET) * TWINROVA_SIEGE_LOWER_ATTACK_RADIUS;
        lowerTargetA.z =
            Math_CosS(lowerBaseAngle + TWINROVA_SIEGE_LOWER_ATTACK_ANGLE_OFFSET) * TWINROVA_SIEGE_LOWER_ATTACK_RADIUS;
        lowerTargetB.x =
            Math_SinS(lowerBaseAngle - TWINROVA_SIEGE_LOWER_ATTACK_ANGLE_OFFSET) * TWINROVA_SIEGE_LOWER_ATTACK_RADIUS;
        lowerTargetB.z =
            Math_CosS(lowerBaseAngle - TWINROVA_SIEGE_LOWER_ATTACK_ANGLE_OFFSET) * TWINROVA_SIEGE_LOWER_ATTACK_RADIUS;
        lowerTargetADistSq = SQ(lowerTargetA.x - player->actor.world.pos.x) +
                             SQ(lowerTargetA.z - player->actor.world.pos.z);
        lowerTargetBDistSq = SQ(lowerTargetB.x - player->actor.world.pos.x) +
                             SQ(lowerTargetB.z - player->actor.world.pos.z);
        if (lowerTargetADistSq < SQ(TWINROVA_SIEGE_MIN_SHOT_DISTANCE) &&
            lowerTargetBDistSq < SQ(TWINROVA_SIEGE_MIN_SHOT_DISTANCE)) {
            useLowerTargetA = lowerTargetADistSq >= lowerTargetBDistSq;
        } else if (lowerTargetADistSq < SQ(TWINROVA_SIEGE_MIN_SHOT_DISTANCE)) {
            useLowerTargetA = false;
        } else if (lowerTargetBDistSq < SQ(TWINROVA_SIEGE_MIN_SHOT_DISTANCE)) {
            useLowerTargetA = true;
        } else {
            // Prefer the shorter intercept so even an edge-to-edge shot resolves within its authored lifetime.
            useLowerTargetA = lowerTargetADistSq <= lowerTargetBDistSq;
        }
        if (useLowerTargetA) {
            owner->targetPos.x = lowerTargetA.x;
            owner->targetPos.z = lowerTargetA.z;
        } else {
            owner->targetPos.x = lowerTargetB.x;
            owner->targetPos.z = lowerTargetB.z;
        }
        owner->targetPos.y = TWINROVA_SIEGE_LOWER_ATTACK_HEIGHT;
    } else {
        // The long tell doubles as reposition time: Twinrova commits from the layout's advertised safe space.
        owner->targetPos.y = 420.0f;
    }
    return true;
}

static void BossTw_GetLowerSiegePortalPos(BossTw* owner, Player* player, Vec3f* portalPos) {
    Vec3f candidateA;
    Vec3f candidateB;
    f32 candidateADistSq;
    f32 candidateBDistSq;
    s16 playerAngle = Math_FAtan2F(player->actor.world.pos.x, player->actor.world.pos.z) * (32768.0f / M_PI);

    // A portal thirty degrees around Link's radial sector is always outside the raised geometry and at least
    // 425 units away. Pick the side closer to Twinrova so the scepter-to-portal handoff remains easy to follow.
    candidateA.x =
        Math_SinS(playerAngle + TWINROVA_SIEGE_LOWER_ATTACK_ANGLE_OFFSET) * TWINROVA_SIEGE_LOWER_ATTACK_RADIUS;
    candidateA.z =
        Math_CosS(playerAngle + TWINROVA_SIEGE_LOWER_ATTACK_ANGLE_OFFSET) * TWINROVA_SIEGE_LOWER_ATTACK_RADIUS;
    candidateB.x =
        Math_SinS(playerAngle - TWINROVA_SIEGE_LOWER_ATTACK_ANGLE_OFFSET) * TWINROVA_SIEGE_LOWER_ATTACK_RADIUS;
    candidateB.z =
        Math_CosS(playerAngle - TWINROVA_SIEGE_LOWER_ATTACK_ANGLE_OFFSET) * TWINROVA_SIEGE_LOWER_ATTACK_RADIUS;
    candidateADistSq = SQ(candidateA.x - owner->actor.world.pos.x) + SQ(candidateA.z - owner->actor.world.pos.z);
    candidateBDistSq = SQ(candidateB.x - owner->actor.world.pos.x) + SQ(candidateB.z - owner->actor.world.pos.z);
    if (candidateADistSq <= candidateBDistSq) {
        portalPos->x = candidateA.x;
        portalPos->z = candidateA.z;
    } else {
        portalPos->x = candidateB.x;
        portalPos->z = candidateB.z;
    }
}

static void BossTw_GetLowerMagicPortalPos(BossTw* owner, Player* player, Vec3f* portalPos) {
    *portalPos = owner->actor.world.pos;
    BossTw_GetLowerSiegePortalPos(owner, player, portalPos);
    portalPos->y += 30.0f;
    if (portalPos->y > TWINROVA_SIEGE_LOWER_PORTAL_HEIGHT) {
        portalPos->y = TWINROVA_SIEGE_LOWER_PORTAL_HEIGHT;
    }
}

static void BossTw_CancelPhaseTwoMagic(BossTw* owner, PlayState* play, s32 preserveMinefield) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    Actor* next;
    s16 i;

    // A successful charged hit cancels Twinrova's direct cast package, but a committed Minefield remains independent
    // arena pressure. Death and encounter teardown pass false and still retire every child actor.
    while (actor != NULL) {
        next = actor->next;
        if (actor->id == ACTOR_BOSS_TW && BossTw_IsFusedDirectBlast((BossTw*)actor, owner) &&
            (!preserveMinefield || ((BossTw*)actor)->blastBehavior != TWINROVA_BLAST_MINEFIELD)) {
            Actor_Kill(actor);
        }
        actor = next;
    }

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (sEffects[i].type == TWEFF_SHLD_BLST &&
            (sEffects[i].target == &owner->actor || (sEffects[i].target == NULL && owner == sTwinrovaPtr))) {
            sEffects[i].alpha = 0;
            sEffects[i].type = TWEFF_NONE;
            sEffects[i].target = NULL;
        }
    }

    D_8094C870 = 0;
    if (!preserveMinefield) {
        sMinefieldLightPulseTimer = 0;
    }
    // Canceling an in-progress Minefield follow-up must not leave its scheduler state ACTIVE after its projectiles are
    // gone. OVERLAP lets surviving mines finish naturally and releases the cooldown gate as soon as authority ends.
    owner->minefieldSequenceState =
        preserveMinefield &&
                (BossTw_HasActiveMinefieldMines(play, owner) || BossTw_CountActiveMinefieldPools(play, owner) != 0)
            ? TWINROVA_MINEFIELD_SEQUENCE_OVERLAP
            : TWINROVA_MINEFIELD_SEQUENCE_NONE;
    owner->minefieldFollowupSuppressed = false;
    owner->pendingFusedAttack = TWINROVA_FUSED_ATTACK_NORMAL;
    BossTw_ResetShieldCharge();
}

static void BossTw_ShowPhaseOneEscalation(PlayState* play) {
    s16 specialCooldown = BossTw_GetPhaseOneSpecialCooldown();
    s16 kotakeCooldownLimit;
    s16 koumeCooldownLimit;

    if (sKotakePtr == NULL || sKoumePtr == NULL) {
        return;
    }

    BossTw_AddRingEffect(play, &sKotakePtr->actor.world.pos, 0.75f, 3.8f, 255, TWINROVA_MAGIC_ICE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_AddRingEffect(play, &sKoumePtr->actor.world.pos, 0.75f, 3.8f, 255, TWINROVA_MAGIC_FIRE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_SpawnMagicLaunchEffects(play, &sKotakePtr->actor.world.pos, TWINROVA_MAGIC_ICE, 12);
    BossTw_SpawnMagicLaunchEffects(play, &sKoumePtr->actor.world.pos, TWINROVA_MAGIC_FIRE, 12);
    Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_POWERUP);
    play->envCtx.unk_D8 = 1.0f;
    Rumble_Request(0.0f, 120, 8, 4);

    // Convert cooldowns already in flight without adding a persistent cooldown-kind field. The remaining ranges can
    // overlap after a shared cooldown ages below 140, so values in that range intentionally receive the more
    // aggressive Ambush cap as part of the escalation; at worst this trims thirty extra updates from one old special.
    kotakeCooldownLimit = sKotakePtr->timers[4] <= TWINROVA_PORTAL_AMBUSH_COOLDOWN
                               ? TWINROVA_ESCALATED_PORTAL_AMBUSH_COOLDOWN
                               : specialCooldown;
    koumeCooldownLimit = sKoumePtr->timers[4] <= TWINROVA_PORTAL_AMBUSH_COOLDOWN
                              ? TWINROVA_ESCALATED_PORTAL_AMBUSH_COOLDOWN
                              : specialCooldown;
    if (sKotakePtr->timers[4] > kotakeCooldownLimit) {
        sKotakePtr->timers[4] = kotakeCooldownLimit;
    }
    if (sKoumePtr->timers[4] > koumeCooldownLimit) {
        sKoumePtr->timers[4] = koumeCooldownLimit;
    }
}

static s32 BossTw_TryHitSisterWithMagic(BossTw* target, PlayState* play, TwinrovaMagicElement magicElement) {
    BossTw* otherSister;
    Vec3f pos;
    Vec3f velocity;
    Vec3f accel = { 0.0f, 0.0f, 0.0f };
    s16 i;

    if (target == NULL || target->actor.update == NULL || sTwinrovaPtr == NULL || BossTw_IsPhaseOneHitGoalReached() ||
        sTwinrovaPtr->actionFunc != BossTw_Wait ||
        (target->actor.params != TW_KOTAKE && target->actor.params != TW_KOUME) ||
        (target->actor.params == TW_KOTAKE && magicElement != TWINROVA_MAGIC_FIRE) ||
        (target->actor.params == TW_KOUME && magicElement != TWINROVA_MAGIC_ICE) ||
        target->actionFunc == BossTw_HitByBeam || target->actionFunc == BossTw_PortalReposition) {
        return false;
    }

    otherSister = target == sKotakePtr ? sKoumePtr : sKotakePtr;
    if (target->actionFunc == BossTw_SpiralBarrage) {
        // A reflected rain shot is a valid mastery reward, but interrupting the paired state must still spend the
        // authored special budget. Otherwise both sisters can roll another spiral as soon as the residual rain dies.
        target->timers[4] = BossTw_GetPhaseOneSpecialCooldown();
        if (otherSister != NULL) {
            otherSister->timers[4] = target->timers[4];
        }
    }

    for (i = 0; i < 50; i++) {
        pos.x = target->actor.world.pos.x + Rand_CenteredFloat(50.0f);
        pos.y = target->actor.world.pos.y + Rand_CenteredFloat(50.0f);
        pos.z = target->actor.world.pos.z + Rand_CenteredFloat(50.0f);
        velocity.x = Rand_CenteredFloat(20.0f);
        velocity.y = Rand_CenteredFloat(20.0f);
        velocity.z = Rand_CenteredFloat(20.0f);
        BossTw_AddFlameEffect(play, &pos, &velocity, &accel, Rand_ZeroFloat(10.0f) + 25.0f, magicElement);
    }

    BossTw_SetupHitByBeam(target, play);
    Audio_PlayActorSound2(&target->actor, NA_SE_EN_TWINROBA_DAMAGE_VOICE);
    play->envCtx.unk_D8 = 1.0f;
    target->actor.colChkInfo.health++;
    if (BossTw_GetPhaseOneHitCount() == TWINROVA_PHASE_ONE_ESCALATION_HITS) {
        BossTw_ShowPhaseOneEscalation(play);
    }
    if (BossTw_IsPhaseOneHitGoalReached()) {
        BossTw_AddRingEffect(play, &target->actor.world.pos, 0.9f, 4.0f, 255, magicElement, 1,
                             ARRAY_COUNT(sEffects));
        Audio_PlayActorSound2(&target->actor, NA_SE_EN_TWINROBA_TRANSFORM);
        Rumble_Request(0.0f, 180, 12, 6);
        BossTw_ClearSummonedEnemies(play);
        BossTw_ClearPhaseOneMagic(play, true);
        if (otherSister != NULL) {
            BossTw_PrepareSisterForMerge(otherSister);
            BossTw_SetupFlyTo(otherSister, play);
        }
    }
    return true;
}

static s32 BossTw_IsNormalPhaseOneMovement(BossTw* sister) {
    return sister != NULL &&
           (sister->actionFunc == BossTw_FlyTo || sister->actionFunc == BossTw_TurnToPlayer);
}

static s32 BossTw_HasPhaseOneReservedDestination(BossTw* sister) {
    return sister->actionFunc == BossTw_FlyTo || sister->actionFunc == BossTw_PortalReposition ||
           sister->actionFunc == BossTw_CrossfireVolley ||
           (sister->actionFunc == BossTw_BlastVolley &&
            sister->beamShootState == TWINROVA_VOLLEY_MODE_PILLAR_DIVE) ||
           (sister->actionFunc == BossTw_SpiralBarrage && sister->csState1 == 0);
}

static f32 BossTw_GetPhaseOneSiblingClearanceSq(BossTw* sister, Vec3f* candidate) {
    BossTw* otherSister = (BossTw*)sister->actor.parent;
    f32 currentClearanceSq;

    if (!BossTw_IsSisterActor(otherSister != NULL ? &otherSister->actor : NULL) ||
        otherSister->actor.update == NULL) {
        return SQ(2000.0f);
    }

    currentClearanceSq = SQ(candidate->x - otherSister->actor.world.pos.x) +
                         SQ(candidate->z - otherSister->actor.world.pos.z);
    if (BossTw_HasPhaseOneReservedDestination(otherSister)) {
        f32 targetClearanceSq = SQ(candidate->x - otherSister->targetPos.x) +
                                SQ(candidate->z - otherSister->targetPos.z);

        return MIN(currentClearanceSq, targetClearanceSq);
    }

    return currentClearanceSq;
}

static s32 BossTw_IsPhaseOneDestinationClear(BossTw* sister, Vec3f* candidate, f32 minTravelDistance) {
    f32 travelDistanceSq = SQ(candidate->x - sister->actor.world.pos.x) +
                           SQ(candidate->z - sister->actor.world.pos.z);

    return travelDistanceSq >= SQ(minTravelDistance) &&
           BossTw_GetPhaseOneSiblingClearanceSq(sister, candidate) >= SQ(TWINROVA_PHASE_ONE_TARGET_SEPARATION);
}

static s32 BossTw_HasPhaseOneSpacing(BossTw* sister, f32 minimumDistance) {
    BossTw* otherSister = (BossTw*)sister->actor.parent;

    return BossTw_IsSisterActor(otherSister != NULL ? &otherSister->actor : NULL) &&
           otherSister->actor.update != NULL &&
           (SQ(sister->actor.world.pos.x - otherSister->actor.world.pos.x) +
            SQ(sister->actor.world.pos.z - otherSister->actor.world.pos.z)) >=
               SQ(minimumDistance);
}

static void BossTw_SelectSeparatedPhaseOnePillar(BossTw* sister, f32 minTravelDistance) {
    s16 firstIndex = Rand_ZeroFloat(ARRAY_COUNT(sTwinrovaPhaseOnePillarPos) - 0.01f);
    s16 bestIndex = firstIndex;
    f32 bestClearanceSq = -1.0f;
    s16 i;

    for (i = 0; i < ARRAY_COUNT(sTwinrovaPhaseOnePillarPos); i++) {
        s16 index = (firstIndex + i) % ARRAY_COUNT(sTwinrovaPhaseOnePillarPos);
        Vec3f* candidate = &sTwinrovaPhaseOnePillarPos[index];
        f32 clearanceSq = BossTw_GetPhaseOneSiblingClearanceSq(sister, candidate);

        if (BossTw_IsPhaseOneDestinationClear(sister, candidate, minTravelDistance)) {
            sister->targetPos = *candidate;
            return;
        }
        if (clearanceSq > bestClearanceSq) {
            bestClearanceSq = clearanceSq;
            bestIndex = index;
        }
    }

    // Four fixed pillars always provide a useful fallback. Choose the one furthest from both the sibling's body and
    // its reserved destination so actor-pool/update-order edge cases still cannot stack both witches on one point.
    sister->targetPos = sTwinrovaPhaseOnePillarPos[bestIndex];
}

static s32 BossTw_IsPhaseOneSeparationState(BossTw* sister) {
    return sister->actionFunc == BossTw_FlyTo || sister->actionFunc == BossTw_TurnToPlayer ||
           sister->actionFunc == BossTw_PortalReposition || sister->actionFunc == BossTw_CrossfireVolley ||
           sister->actionFunc == BossTw_Spin || sister->actionFunc == BossTw_PillarDive ||
           (sister->actionFunc == BossTw_SpiralBarrage && sister->csState1 == 0);
}

static void BossTw_SeparatePhaseOneSisters(BossTw* sister) {
    BossTw* otherSister = (BossTw*)sister->actor.parent;
    f32 xDiff;
    f32 zDiff;
    f32 distanceSq;
    f32 distance;
    f32 correction;

    if (!BossTw_IsSisterActor(otherSister != NULL ? &otherSister->actor : NULL) ||
        otherSister->actor.update == NULL || !sister->visible || !otherSister->visible ||
        sister->actor.scale.x < 0.005f || otherSister->actor.scale.x < 0.005f ||
        !BossTw_IsPhaseOneSeparationState(sister) || !BossTw_IsPhaseOneSeparationState(otherSister) ||
        fabsf(sister->actor.world.pos.y - otherSister->actor.world.pos.y) >=
            TWINROVA_PHASE_ONE_SEPARATION_HEIGHT) {
        return;
    }

    if (sister->actionFunc == BossTw_Spin) {
        // Keep the committed counter centered on its tell; the still-mobile sibling performs the separation.
        return;
    }

    xDiff = sister->actor.world.pos.x - otherSister->actor.world.pos.x;
    zDiff = sister->actor.world.pos.z - otherSister->actor.world.pos.z;
    distanceSq = SQ(xDiff) + SQ(zDiff);
    if (distanceSq >= SQ(TWINROVA_PHASE_ONE_BODY_SEPARATION)) {
        return;
    }

    if (distanceSq < 1.0f) {
        // A deterministic elemental side breaks an exact overlap without relying on an undefined zero-length normal.
        xDiff = sister->actor.params == TW_KOUME ? 1.0f : -1.0f;
        zDiff = 0.0f;
        distance = 1.0f;
    } else {
        distance = sqrtf(distanceSq);
    }

    correction = MIN(TWINROVA_PHASE_ONE_BODY_SEPARATION - distance, TWINROVA_PHASE_ONE_SEPARATION_STEP);
    xDiff /= distance;
    zDiff /= distance;
    sister->actor.world.pos.x += xDiff * correction;
    sister->actor.world.pos.z += zDiff * correction;
}

static void BossTw_RecordPhaseOneSpecialAttack(void) {
    // Keep the seen-special mask across successful moves so the scheduler presents the full repertoire before it
    // starts another cycle. Only the short-term pity, combo, and regular-repeat histories reset here.
    sPhaseOneEligibleSpecialMisses = 0;
    sPhaseOneComboState = TWINROVA_PHASE_ONE_COMBO_IDLE;
    BossTw_ResetPhaseOneRegularHistory();
}

static void BossTw_RecordPhaseOneSpecialMiss(void) {
    if (sPhaseOneEligibleSpecialMisses < TWINROVA_PHASE_ONE_SPECIAL_PITY_MISSES) {
        sPhaseOneEligibleSpecialMisses++;
    }
}

static void BossTw_RecordPhaseOneRegularHistory(TwinrovaPhaseOneRegularAttack attack) {
    if (sPhaseOneLastRegularAttack == attack) {
        if (sPhaseOneRegularRepeatCount < TWINROVA_PHASE_ONE_REGULAR_REPEAT_LIMIT) {
            sPhaseOneRegularRepeatCount++;
        }
    } else {
        sPhaseOneLastRegularAttack = attack;
        sPhaseOneRegularRepeatCount = 1;
    }
}

static void BossTw_RecordPhaseOneRegularAttack(TwinrovaPhaseOneRegularAttack attack) {
    if (sPhaseOneComboState == TWINROVA_PHASE_ONE_COMBO_IDLE) {
        sPhaseOneComboState = TWINROVA_PHASE_ONE_COMBO_ARMED;
    }

    BossTw_RecordPhaseOneRegularHistory(attack);
}

static void BossTw_ShowSisterAttackTell(BossTw* sister, PlayState* play, f32 ringScale, s16 particleCount) {
    TwinrovaMagicElement element =
        sister->actor.params == TW_KOUME ? TWINROVA_MAGIC_FIRE : TWINROVA_MAGIC_ICE;

    sister->scepterAlpha = 255.0f;
    BossTw_AddRingEffect(play, &sister->beamOrigin, ringScale, 3.5f, 255, element, 1, ARRAY_COUNT(sEffects));
    BossTw_SpawnMagicLaunchEffects(play, &sister->beamOrigin, element, particleCount);
    Audio_PlayActorSound2(&sister->actor, NA_SE_EN_TWINROBA_MASIC_SET);
}

static s32 BossTw_ShouldSuppressSisterAttackCollision(BossTw* sister, PlayState* play) {
    if (BossTw_IsPlayerHardDisabled(play)) {
        // Simulation never pauses with Link. Refresh only the collision grace while animation, movement, effects,
        // beam extension, attack timers, and projectile release continue normally.
        if (sister->timers[3] < TWINROVA_FROZEN_ATTACK_GRACE) {
            sister->timers[3] = TWINROVA_FROZEN_ATTACK_GRACE;
        }
    }

    if (!BossTw_IsPlayerHardDisabled(play) && sister->timers[3] == TWINROVA_FROZEN_ATTACK_GRACE - 1) {
        BossTw_ShowSisterAttackTell(sister, play, 0.55f, 8);
    }

    return sister->timers[3] != 0;
}

static void BossTw_SpawnPortalBurst(BossTw* this, PlayState* play, Vec3f* origin) {
    Vec3f pos;
    Vec3f velocity;
    Vec3f accel = { 0.0f, 0.1f, 0.0f };
    s16 i;

    for (i = 0; i < 24; i++) {
        pos.x = origin->x + Rand_CenteredFloat(70.0f);
        pos.y = origin->y + Rand_CenteredFloat(70.0f);
        pos.z = origin->z + Rand_CenteredFloat(70.0f);
        velocity.x = Rand_CenteredFloat(12.0f);
        velocity.y = Rand_CenteredFloat(12.0f);
        velocity.z = Rand_CenteredFloat(12.0f);
        BossTw_AddFlameEffect(play, &pos, &velocity, &accel, Rand_ZeroFloat(10.0f) + 20.0f, this->actor.params);
    }

    BossTw_AddRingEffect(play, origin, 0.4f, 3.0f, 255, this->actor.params, 1, 150);
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_TRANSFORM);
}

static void BossTw_SelectPortalDestination(BossTw* this) {
    // Reserve against both the sibling's current body and its committed destination. Checking only the current
    // position allowed a moving sister to arrive on the same pillar during the teleport's hidden interval.
    BossTw_SelectSeparatedPhaseOnePillar(this, 450.0f);
}

static void BossTw_SelectPortalAmbushDestination(BossTw* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s16 firstIndex = Rand_ZeroFloat(ARRAY_COUNT(sTwinrovaPhaseOnePillarPos) - 0.01f);
    s16 bestIndex = -1;
    f32 bestFlankScore = -1.0f;
    s16 i;

    for (i = 0; i < ARRAY_COUNT(sTwinrovaPhaseOnePillarPos); i++) {
        s16 index = (firstIndex + i) % ARRAY_COUNT(sTwinrovaPhaseOnePillarPos);
        Vec3f* candidate = &sTwinrovaPhaseOnePillarPos[index];

        if (BossTw_IsPhaseOneDestinationClear(this, candidate, 450.0f)) {
            s16 candidateYaw = Math_FAtan2F(candidate->x - player->actor.world.pos.x,
                                             candidate->z - player->actor.world.pos.z) *
                               (32768.0f / M_PI);
            // Favor side and rear pillars. The existing destination-clear test remains authoritative for separation.
            f32 flankScore = 1.0f - Math_CosS((s16)(candidateYaw - player->actor.shape.rot.y));

            if (flankScore > bestFlankScore) {
                bestFlankScore = flankScore;
                bestIndex = index;
            }
        }
    }

    if (bestIndex >= 0) {
        this->targetPos = sTwinrovaPhaseOnePillarPos[bestIndex];
    } else {
        BossTw_SelectPortalDestination(this);
    }
}

static s32 BossTw_IsPortalAmbush(BossTw* this) {
    return (this->csState2 & TWINROVA_PORTAL_MODE_MASK) == TWINROVA_PORTAL_MODE_AMBUSH;
}

static s32 BossTw_IsPortalPressureTriangle(BossTw* this) {
    return (this->csState2 & TWINROVA_PORTAL_MODE_MASK) == TWINROVA_PORTAL_MODE_PRESSURE_TRIANGLE;
}

static s32 BossTw_UsesFastPortalTiming(BossTw* this) {
    return BossTw_IsPortalAmbush(this) || BossTw_IsPortalPressureTriangle(this);
}

static s32 BossTw_ShouldUsePortalFakeOut(BossTw* this) {
    f32 chance = BossTw_IsPhaseOneEscalated() ? TWINROVA_ESCALATED_PORTAL_FAKE_OUT_CHANCE
                                               : TWINROVA_PORTAL_FAKE_OUT_CHANCE;

    return !(this->csState2 & TWINROVA_PORTAL_FAKE_OUT_USED) && Rand_ZeroOne() < chance;
}

static void BossTw_BeginPortalFakeOut(BossTw* this, PlayState* play) {
    this->csState2 |= TWINROVA_PORTAL_FAKE_OUT_USED;
    this->csState1 = 0;
    this->timers[0] = BossTw_UsesFastPortalTiming(this) ? TWINROVA_PORTAL_AMBUSH_VANISH_TIME
                                                         : TWINROVA_PORTAL_VANISH_TIME;
    this->work[CAN_SHOOT] = false;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    this->scepterAlpha = 0.0f;
    this->flameAlpha = 0.0f;
    this->spawnPortalAlpha = 0.0f;
    this->workf[UNK_F17] = 0.0f;
    this->workf[UNK_F18] = 0.0f;
    this->portalRotation = 0.0f;
    if (BossTw_IsPortalAmbush(this)) {
        BossTw_SelectPortalAmbushDestination(this, play);
    } else {
        BossTw_SelectPortalDestination(this);
    }
}

static void BossTw_SetupPortalReposition(BossTw* this) {
    this->actionFunc = BossTw_PortalReposition;
    this->csState1 = 0;
    this->csState2 = TWINROVA_PORTAL_MODE_REPOSITION;
    this->timers[0] = TWINROVA_PORTAL_VANISH_TIME;
    this->timers[4] = BossTw_GetPhaseOneSpecialCooldown();
    this->work[CAN_SHOOT] = false;
    this->actor.speedXZ = 0.0f;
    this->actor.velocity.x = this->actor.velocity.y = this->actor.velocity.z = 0.0f;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    this->scepterAlpha = 0.0f;
    this->flameAlpha = 0.0f;
    this->spawnPortalAlpha = 0.0f;
    this->workf[UNK_F17] = 0.0f;
    this->workf[UNK_F18] = 0.0f;
    this->portalRotation = 0.0f;
    BossTw_SelectPortalDestination(this);
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, -5.0f);
}

static void BossTw_SetupPortalAmbush(BossTw* this, PlayState* play) {
    BossTw_SetupPortalReposition(this);
    this->csState2 = TWINROVA_PORTAL_MODE_AMBUSH;
    this->timers[0] = TWINROVA_PORTAL_AMBUSH_VANISH_TIME;
    this->timers[4] = BossTw_IsPhaseOneEscalated() ? TWINROVA_ESCALATED_PORTAL_AMBUSH_COOLDOWN
                                                    : TWINROVA_PORTAL_AMBUSH_COOLDOWN;
    BossTw_SelectPortalAmbushDestination(this, play);
}

static void BossTw_SetupPressureTrianglePortal(BossTw* this, PlayState* play) {
    BossTw_SetupPortalReposition(this);
    this->csState2 = TWINROVA_PORTAL_MODE_PRESSURE_TRIANGLE;
    this->timers[0] = TWINROVA_PORTAL_AMBUSH_VANISH_TIME;
    BossTw_SelectPortalAmbushDestination(this, play);
}

static void BossTw_StartPortalSpecial(BossTw* this, PlayState* play) {
    f32 ambushChance =
        BossTw_IsPhaseOneEscalated() ? TWINROVA_ESCALATED_PORTAL_AMBUSH_CHANCE : TWINROVA_PORTAL_AMBUSH_CHANCE;

    if (Rand_ZeroOne() < ambushChance) {
        BossTw_SetupPortalAmbush(this, play);
    } else {
        BossTw_SetupPortalReposition(this);
    }
}

static void BossTw_StartPortalFollowup(BossTw* this, BossTw* otherTw, PlayState* play) {
    s32 canBeam = otherTw != NULL && BossTw_IsNormalPhaseOneMovement(otherTw) &&
                  BossTw_GetProjectedSummonPressure(play) <= TWINROVA_BEAM_SUMMON_LIMIT &&
                  !BossTw_HasActivePhaseOneMagic(play) &&
                  BossTw_IsActorInCameraFrustumWithMargin(play, &this->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN) &&
                  BossTw_IsActorInCameraFrustumWithMargin(play, &otherTw->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN);

    if (canBeam && Rand_ZeroOne() < 0.5f) {
        this->work[CAN_SHOOT] = false;
        BossTw_SetupShootBeam(this, play);
        BossTw_RecordPhaseOneRegularAttack(TWINROVA_PHASE_ONE_REGULAR_BEAM);
    } else if (BossTw_IsActorInCameraFrustumWithMargin(play, &this->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN)) {
        this->work[CAN_SHOOT] = false;
        BossTw_SetupBlastVolley(this, play);
        BossTw_RecordPhaseOneRegularAttack(TWINROVA_PHASE_ONE_REGULAR_VOLLEY);
    } else {
        BossTw_SetupTurnToPlayer(this, play);
        this->timers[0] = TWINROVA_PHASE_ONE_COMBO_RESET_TIME;
    }
}

static s32 BossTw_CanStartPortalReposition(BossTw* this, BossTw* otherTw, PlayState* play) {
    return this->timers[4] == 0 && otherTw != NULL && otherTw->timers[4] == 0 &&
           BossTw_IsNormalPhaseOneMovement(otherTw) && !BossTw_HasActivePhaseOneMagic(play);
}

static void BossTw_SetupCrossfireBlastVolley(BossTw* sister, PlayState* play) {
    sister->actionFunc = BossTw_BlastVolley;
    Animation_MorphToLoop(&sister->skelAnime, &gTwinrovaKotakeKoumeAttackLoopAnim, -3.0f);
    sister->workf[ANIM_SW_TGT] = 10000.0f;
    sister->csState1 = 2;
    sister->csState2 = -1;
    sister->timers[0] = 0;
    sister->timers[3] = 0;
    sister->work[YAW_TGT] = (s16)(play->gameplayFrames + 1);
    sister->beamShootState = TWINROVA_VOLLEY_MODE_CROSSFIRE;
    sister->actor.speedXZ = 0.0f;
    BossTw_ShowSisterAttackTell(sister, play, 0.6f, 12);
}

static void BossTw_StartCrossfireVolley(PlayState* play) {
    // Fire always opens, then each sister repeats every twenty updates. This produces a strict fire -> ice -> fire ->
    // ice sequence at a readable ten-update cadence instead of relying on actor update order.
    BossTw_SetupCrossfireBlastVolley(sKoumePtr, play);
    BossTw_SetupCrossfireBlastVolley(sKotakePtr, play);
}

static void BossTw_ConfigureCrossfireSister(BossTw* sister) {
    sister->actionFunc = BossTw_CrossfireVolley;
    sister->csState1 = 0;
    sister->timers[0] = TWINROVA_CROSSFIRE_FORMATION_TIME;
    sister->work[CAN_SHOOT] = false;
    sister->actor.speedXZ = 0.0f;
    sister->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    sister->scepterAlpha = 255.0f;
    Animation_MorphToLoop(&sister->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, -5.0f);
}

static void BossTw_StartCrossfireFormation(void) {
    BossTw_ConfigureCrossfireSister(sKoumePtr);
    BossTw_SelectSeparatedPhaseOnePillar(sKoumePtr, 120.0f);
    BossTw_ConfigureCrossfireSister(sKotakePtr);
    BossTw_SelectSeparatedPhaseOnePillar(sKotakePtr, 120.0f);
    sKotakePtr->timers[4] = sKoumePtr->timers[4] = BossTw_GetPhaseOneSpecialCooldown();
}

static s32 BossTw_CanStartCrossfireVolley(BossTw* this, BossTw* otherTw, PlayState* play) {
    return sKotakePtr != NULL && sKoumePtr != NULL && BossTw_CanStartPortalReposition(this, otherTw, play);
}

static void BossTw_StartPressureTriangle(BossTw* this, BossTw* otherTw, PlayState* play) {
    BossTw_SetupBlastVolley(this, play);
    this->beamShootState = TWINROVA_VOLLEY_MODE_PRESSURE_TRIANGLE;
    BossTw_SetupPressureTrianglePortal(otherTw, play);
    this->timers[4] = otherTw->timers[4] = BossTw_GetPhaseOneSpecialCooldown();
}

static s32 BossTw_CanStartPressureTriangle(BossTw* this, BossTw* otherTw, PlayState* play) {
    // Pressure Triangle opens with an immediate volley. Unlike the other specials, it has no formation phase that
    // naturally brings an off-screen caster into view, so its lead sister must already be readable.
    return BossTw_CanStartPortalReposition(this, otherTw, play) &&
           BossTw_IsActorInCameraFrustumWithMargin(play, &this->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN);
}

static void BossTw_ConfigureSpiralSister(BossTw* sister, s16 orbitAngle) {
    sister->actionFunc = BossTw_SpiralBarrage;
    sister->csState1 = 0;
    sister->csState2 = TWINROVA_SPIRAL_VOLLEYS;
    sister->timers[0] = TWINROVA_SPIRAL_FORMATION_TIMEOUT;
    sister->work[YAW_TGT] = orbitAngle;
    sister->targetPos.x = Math_SinS(orbitAngle) * TWINROVA_SPIRAL_RADIUS;
    sister->targetPos.y = TWINROVA_SPIRAL_HEIGHT + (Math_SinS(orbitAngle * 2) * 25.0f);
    sister->targetPos.z = Math_CosS(orbitAngle) * TWINROVA_SPIRAL_RADIUS;
    sister->work[CAN_SHOOT] = false;
    sister->actor.speedXZ = 0.0f;
    sister->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    sister->scepterAlpha = 255.0f;
    Animation_MorphToLoop(&sister->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, -5.0f);
}

static s32 BossTw_CanStartSpiralBarrage(BossTw* this, BossTw* otherTw, PlayState* play) {
    return sKotakePtr != NULL && sKoumePtr != NULL && BossTw_CanStartPortalReposition(this, otherTw, play) &&
           BossTw_CountActiveSummons(play) <= TWINROVA_PHASE_ONE_DUAL_VOLLEY_SUMMON_LIMIT &&
           !BossTw_HasActiveGroundPressure(play);
}

static void BossTw_StartSpiralBarrage(void) {
    s16 orbitAngle;
    s16 rainPatternRotation;

    orbitAngle = Math_FAtan2F(sKotakePtr->actor.world.pos.x, sKotakePtr->actor.world.pos.z) * (32768.0f / M_PI);
    rainPatternRotation = (s16)Rand_CenteredFloat(65535.0f);
    BossTw_ConfigureSpiralSister(sKotakePtr, orbitAngle);
    BossTw_ConfigureSpiralSister(sKoumePtr, orbitAngle + 0x8000);
    sKotakePtr->timers[4] = sKoumePtr->timers[4] = BossTw_GetPhaseOneSpecialCooldown();
    // Rotate the authored layout once per barrage. Every landing point remains fixed after launch and fully marked.
    sKotakePtr->beamShootState = sKoumePtr->beamShootState = rainPatternRotation;
}

static void BossTw_SetupSpiralVolleyLanding(void) {
    s16 bestIndex = 0;
    f32 bestTravelDistanceSq = FLT_MAX;
    s16 i;

    // Keep the sisters on opposite fixed pillars, choosing the pair nearest their final orbit positions so the rain
    // recovery reads as a controlled landing instead of a long cross-arena reset.
    for (i = 0; i < ARRAY_COUNT(sTwinrovaPhaseOnePillarPos); i++) {
        Vec3f* koumeTarget = &sTwinrovaPhaseOnePillarPos[i];
        Vec3f* kotakeTarget = &sTwinrovaPhaseOnePillarPos[(i + 2) % ARRAY_COUNT(sTwinrovaPhaseOnePillarPos)];
        f32 travelDistanceSq = SQ(sKoumePtr->actor.world.pos.x - koumeTarget->x) +
                                SQ(sKoumePtr->actor.world.pos.z - koumeTarget->z) +
                                SQ(sKotakePtr->actor.world.pos.x - kotakeTarget->x) +
                                SQ(sKotakePtr->actor.world.pos.z - kotakeTarget->z);

        if (travelDistanceSq < bestTravelDistanceSq) {
            bestTravelDistanceSq = travelDistanceSq;
            bestIndex = i;
        }
    }

    sKoumePtr->targetPos = sTwinrovaPhaseOnePillarPos[bestIndex];
    sKotakePtr->targetPos = sTwinrovaPhaseOnePillarPos[(bestIndex + 2) % ARRAY_COUNT(sTwinrovaPhaseOnePillarPos)];
    sKoumePtr->csState1 = sKotakePtr->csState1 = 3;
    sKoumePtr->timers[0] = sKotakePtr->timers[0] = TWINROVA_SPIRAL_LANDING_TIME;
}

static void BossTw_StartSpiralVolleyFollowup(PlayState* play) {
    BossTw* attacker = Rand_ZeroOne() < 0.5f ? sKoumePtr : sKotakePtr;
    BossTw* passive = attacker == sKoumePtr ? sKotakePtr : sKoumePtr;
    BossTw* swap;

    if (!BossTw_IsActorInCameraFrustumWithMargin(play, &attacker->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN) &&
        BossTw_IsActorInCameraFrustumWithMargin(play, &passive->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN)) {
        swap = attacker;
        attacker = passive;
        passive = swap;
    }

    BossTw_SetupTurnToPlayer(passive, play);
    passive->actor.speedXZ = 0.0f;
    passive->work[CAN_SHOOT] = false;

    // Both attacks require a visible source. A beam additionally requires its redirect target on camera; residual
    // rain may remain active because this authored sequence intentionally hands off before every impact visual fades.
    if (BossTw_GetProjectedSummonPressure(play) <= TWINROVA_BEAM_SUMMON_LIMIT &&
        BossTw_IsActorInCameraFrustumWithMargin(play, &attacker->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN) &&
        BossTw_IsActorInCameraFrustumWithMargin(play, &passive->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN) &&
        Rand_ZeroOne() < 0.5f) {
        attacker->work[CAN_SHOOT] = false;
        BossTw_SetupShootBeam(attacker, play);
        BossTw_RecordPhaseOneRegularHistory(TWINROVA_PHASE_ONE_REGULAR_BEAM);
    } else if (BossTw_IsActorInCameraFrustumWithMargin(play, &attacker->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN)) {
        attacker->work[CAN_SHOOT] = false;
        BossTw_SetupBlastVolley(attacker, play);
        BossTw_RecordPhaseOneRegularHistory(TWINROVA_PHASE_ONE_REGULAR_VOLLEY);
    } else {
        BossTw_SetupTurnToPlayer(attacker, play);
        attacker->timers[0] = TWINROVA_PHASE_ONE_COMBO_RESET_TIME;
        passive->timers[0] = TWINROVA_PHASE_ONE_COMBO_RESET_TIME;
    }
}

static s32 BossTw_TryStartPhaseOneSpecial(BossTw* this, s32 canStartSpiral, s32 canStartCrossfire,
                                           s32 canStartPressureTriangle, s32 canStartPortal, s32 forceSpecial,
                                           PlayState* play) {
    u8 eligible = (canStartSpiral ? TWINROVA_PHASE_ONE_SPECIAL_SPIRAL : 0) |
                  (canStartCrossfire ? TWINROVA_PHASE_ONE_SPECIAL_CROSSFIRE : 0) |
                  (canStartPressureTriangle ? TWINROVA_PHASE_ONE_SPECIAL_PRESSURE_TRIANGLE : 0) |
                  (canStartPortal ? TWINROVA_PHASE_ONE_SPECIAL_PORTAL : 0);
    u8 candidates;
    s16 candidateCount = 0;
    s16 selectedIndex;
    s16 bit;
    u8 selected = 0;

    if (eligible == 0 ||
        (!forceSpecial &&
         Rand_ZeroOne() >= (BossTw_IsPhaseOneEscalated() ? TWINROVA_ESCALATED_PHASE_ONE_SPECIAL_CHANCE
                                                         : TWINROVA_PHASE_ONE_SPECIAL_CHANCE))) {
        return false;
    }

    // Pick uniformly from eligible specials that have not appeared in the current repertoire cycle. If every
    // currently legal move has appeared, clear only those bits; a temporarily gated move remains prioritized when
    // its arena conditions become legal again.
    candidates = eligible & ~sPhaseOneSeenSpecials;
    if (candidates == 0) {
        sPhaseOneSeenSpecials &= ~eligible;
        candidates = eligible;
    }
    for (bit = 1; bit <= TWINROVA_PHASE_ONE_SPECIAL_PORTAL; bit <<= 1) {
        if (candidates & bit) {
            candidateCount++;
        }
    }
    selectedIndex = (s16)Rand_ZeroFloat(candidateCount);
    for (bit = 1; bit <= TWINROVA_PHASE_ONE_SPECIAL_PORTAL; bit <<= 1) {
        if ((candidates & bit) && selectedIndex-- == 0) {
            selected = bit;
            break;
        }
    }
    sPhaseOneSeenSpecials |= selected;

    switch (selected) {
        case TWINROVA_PHASE_ONE_SPECIAL_SPIRAL:
            BossTw_StartSpiralBarrage();
            break;
        case TWINROVA_PHASE_ONE_SPECIAL_CROSSFIRE:
            BossTw_StartCrossfireFormation();
            break;
        case TWINROVA_PHASE_ONE_SPECIAL_PRESSURE_TRIANGLE:
            BossTw_StartPressureTriangle(this, (BossTw*)this->actor.parent, play);
            break;
        case TWINROVA_PHASE_ONE_SPECIAL_PORTAL:
            BossTw_StartPortalSpecial(this, play);
            break;
        default:
            return false;
    }
    return true;
}

static s16 BossTw_GetSpiralFrameAngle(PlayState* play) {
    return (s16)((u16)play->gameplayFrames * TWINROVA_SPIRAL_ANGULAR_SPEED);
}

static void BossTw_UpdateSpiralFlight(BossTw* this, PlayState* play) {
    s16 orbitAngle = this->work[YAW_TGT];
    s16 tangentYaw;
    f32 maxMoveStep = this->csState1 == 0 ? 28.0f : 22.0f;

    if (this->csState1 >= 3) {
        // The barrage ends on fixed opposing pillars. Keep those targets intact through the short recovery instead of
        // resuming the orbit and forcing a neutral reposition before the follow-up attack.
        tangentYaw = Math_Vec3f_Yaw(&this->actor.world.pos, &this->targetPos);
        Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.3f, 28.0f);
        Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.2f, 8.0f);
        Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.3f, 28.0f);
        Math_ApproachS(&this->actor.world.rot.y, tangentYaw, 5, 0x1000);
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1000);
        Math_ApproachS(&this->actor.world.rot.x, 0, 5, 0x800);
        Math_ApproachS(&this->actor.shape.rot.x, 0, 5, 0x800);
        SkelAnime_Update(&this->skelAnime);
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FLY - SFX_FLAG);
        return;
    }

    if (this->csState1 != 0) {
        orbitAngle += BossTw_GetSpiralFrameAngle(play);
    }
    this->targetPos.x = Math_SinS(orbitAngle) * TWINROVA_SPIRAL_RADIUS;
    this->targetPos.y = TWINROVA_SPIRAL_HEIGHT + (Math_SinS(orbitAngle * 2) * 25.0f);
    this->targetPos.z = Math_CosS(orbitAngle) * TWINROVA_SPIRAL_RADIUS;
    tangentYaw = this->csState1 == 0 ? Math_Vec3f_Yaw(&this->actor.world.pos, &this->targetPos)
                                     : orbitAngle + 0x4000;

    Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.3f, maxMoveStep);
    Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.2f, 8.0f);
    Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.3f, maxMoveStep);
    Math_ApproachS(&this->actor.world.rot.y, tangentYaw, 5, 0x1000);
    Math_ApproachS(&this->actor.shape.rot.y, tangentYaw, 5, 0x1000);
    Math_ApproachS(&this->actor.world.rot.x, 0, 5, 0x800);
    Math_ApproachS(&this->actor.shape.rot.x, 0, 5, 0x800);
    SkelAnime_Update(&this->skelAnime);
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FLY - SFX_FLAG);
}

static f32 BossTw_FitRainAnchorAxis(f32 desiredAnchor, f32 minBound, f32 maxBound, f32 firstOffset,
                                     f32 secondOffset) {
    f32 minOffset = MIN(firstOffset, secondOffset);
    f32 maxOffset = MAX(firstOffset, secondOffset);

    return CLAMP(desiredAnchor, minBound - minOffset, maxBound - maxOffset);
}

static void BossTw_GetRainTargets(PlayState* play, s16 volleyIndex, s16 patternRotation, Vec3f* firstTarget,
                                  Vec3f* secondTarget) {
    Player* player = GET_PLAYER(play);
    Vec3f anchor = player->actor.world.pos;
    Vec3f platformCenter = { 0.0f, 0.0f, 0.0f };
    Vec3f chaserOffset;
    Vec3f cutoffOffset;
    Vec3f chaserTarget;
    Vec3f cutoffTarget;
    s32 playerIsOnUpperFloor = player->actor.floorHeight >= TWINROVA_UPPER_FLOOR_MIN_Y;
    s32 playerIsOnCentralPlatform = playerIsOnUpperFloor && fabsf(player->actor.world.pos.x) < 350.0f &&
                                    fabsf(player->actor.world.pos.z) < 350.0f;
    f32 chaserRadius = TWINROVA_RAIN_CHASER_OFFSET;
    f32 cutoffRadius = TWINROVA_RAIN_CUTOFF_OFFSET;
    f32 anchorDistance;
    f32 anchorScale;
    f32 directionX;
    f32 directionZ;
    s16 landingAngle = 0x1000 + (volleyIndex * 0x2000) + patternRotation;

    // Sample Link only once for the synchronized pair. The chaser sits inside the impact radius of his led position;
    // the cutoff closes the opposite lane. Both points remain committed and non-homing after this calculation.
    anchor.x += CLAMP(player->actor.velocity.x * TWINROVA_RAIN_PLAYER_LEAD_FRAMES,
                      -TWINROVA_RAIN_MAX_PLAYER_LEAD, TWINROVA_RAIN_MAX_PLAYER_LEAD);
    anchor.z += CLAMP(player->actor.velocity.z * TWINROVA_RAIN_PLAYER_LEAD_FRAMES,
                      -TWINROVA_RAIN_MAX_PLAYER_LEAD, TWINROVA_RAIN_MAX_PLAYER_LEAD);
    directionX = Math_SinS(landingAngle);
    directionZ = Math_CosS(landingAngle);

    if (playerIsOnCentralPlatform) {
        anchor.y = 240.0f;
    } else if (playerIsOnUpperFloor) {
        chaserRadius = TWINROVA_RAIN_SIDE_CHASER_OFFSET;
        cutoffRadius = TWINROVA_RAIN_SIDE_CUTOFF_OFFSET;
        anchor.y = 230.0f;
        if (fabsf(player->actor.world.pos.x) > fabsf(player->actor.world.pos.z)) {
            platformCenter.x = player->actor.world.pos.x < 0.0f ? -TWINROVA_SPIRAL_SIDE_PLATFORM_OFFSET
                                                                : TWINROVA_SPIRAL_SIDE_PLATFORM_OFFSET;
        } else {
            platformCenter.z = player->actor.world.pos.z < 0.0f ? -TWINROVA_SPIRAL_SIDE_PLATFORM_OFFSET
                                                                : TWINROVA_SPIRAL_SIDE_PLATFORM_OFFSET;
        }
    } else {
        anchor.y = 0.0f;
    }

    chaserOffset.x = directionX * chaserRadius;
    chaserOffset.y = 0.0f;
    chaserOffset.z = directionZ * chaserRadius;
    cutoffOffset.x = -directionX * cutoffRadius;
    cutoffOffset.y = 0.0f;
    cutoffOffset.z = -directionZ * cutoffRadius;

    if (playerIsOnCentralPlatform) {
        anchor.x = BossTw_FitRainAnchorAxis(anchor.x, -TWINROVA_RAIN_CENTRAL_PLATFORM_LIMIT,
                                            TWINROVA_RAIN_CENTRAL_PLATFORM_LIMIT, chaserOffset.x,
                                            cutoffOffset.x);
        anchor.z = BossTw_FitRainAnchorAxis(anchor.z, -TWINROVA_RAIN_CENTRAL_PLATFORM_LIMIT,
                                            TWINROVA_RAIN_CENTRAL_PLATFORM_LIMIT, chaserOffset.z,
                                            cutoffOffset.z);
    } else if (playerIsOnUpperFloor) {
        anchor.x = BossTw_FitRainAnchorAxis(anchor.x,
                                            platformCenter.x - TWINROVA_RAIN_SIDE_PLATFORM_LIMIT,
                                            platformCenter.x + TWINROVA_RAIN_SIDE_PLATFORM_LIMIT,
                                            chaserOffset.x, cutoffOffset.x);
        anchor.z = BossTw_FitRainAnchorAxis(anchor.z,
                                            platformCenter.z - TWINROVA_RAIN_SIDE_PLATFORM_LIMIT,
                                            platformCenter.z + TWINROVA_RAIN_SIDE_PLATFORM_LIMIT,
                                            chaserOffset.z, cutoffOffset.z);
    } else {
        // A circular anchor bound plus the 150-unit cutoff keeps both centers inside the arena's true rotated
        // octagonal limit. Do not independently clamp the points: that could collapse opposing elements together.
        anchorDistance = sqrtf(SQ(anchor.x) + SQ(anchor.z));
        if (anchorDistance > TWINROVA_RAIN_LOWER_ANCHOR_RADIUS) {
            anchorScale = TWINROVA_RAIN_LOWER_ANCHOR_RADIUS / anchorDistance;
            anchor.x *= anchorScale;
            anchor.z *= anchorScale;
        }
    }

    chaserTarget.x = anchor.x + chaserOffset.x;
    chaserTarget.y = anchor.y;
    chaserTarget.z = anchor.z + chaserOffset.z;
    cutoffTarget.x = anchor.x + cutoffOffset.x;
    cutoffTarget.y = anchor.y;
    cutoffTarget.z = anchor.z + cutoffOffset.z;

    if (!playerIsOnUpperFloor &&
        (BossTw_IsOverRaisedPlatform(&chaserTarget) || BossTw_IsOverRaisedPlatform(&cutoffTarget))) {
        // Lower-deck warnings must not disappear beneath a raised platform. A same-quadrant diagonal is the nearest
        // universally clear fallback and still preserves the full 210-unit chaser/cutoff separation.
        anchor.x = (anchor.x < 0.0f ? -1.0f : 1.0f) * TWINROVA_RAIN_LOWER_DIAGONAL_ANCHOR;
        anchor.z = (anchor.z < 0.0f ? -1.0f : 1.0f) * TWINROVA_RAIN_LOWER_DIAGONAL_ANCHOR;
        chaserTarget.x = anchor.x + chaserOffset.x;
        chaserTarget.z = anchor.z + chaserOffset.z;
        cutoffTarget.x = anchor.x + cutoffOffset.x;
        cutoffTarget.z = anchor.z + cutoffOffset.z;
    }

    // Swap the direct chaser between the two outputs. Phase one alternates which sister pressures Link directly;
    // Cyclone reuses the same lane variation for its paired same-element drops.
    if ((volleyIndex & 1) == 0) {
        *firstTarget = chaserTarget;
        *secondTarget = cutoffTarget;
    } else {
        *firstTarget = cutoffTarget;
        *secondTarget = chaserTarget;
    }
}

static s32 BossTw_TryStageRainBlast(BossTw* owner, PlayState* play, Vec3f* spawnPos, Vec3f* landingPos,
                                    TwinrovaMagicElement element, BossTw** magicOut, Actor** warningOut) {
    BossTw* magic;
    Actor* warning;
    s16 warningParams = element == TWINROVA_MAGIC_FIRE ? TW_FIRE_WARNING_SIGIL : TW_ICE_WARNING_SIGIL;
    s16 magicParams = element == TWINROVA_MAGIC_FIRE ? TW_FIRE_BLAST : TW_ICE_BLAST;

    *magicOut = NULL;
    *warningOut = NULL;

    magic = BossTw_SpawnMagicBlast(owner, play, spawnPos, magicParams, TWINROVA_BLAST_RAIN);
    if (magic == NULL) {
        return false;
    }

    magic->targetPos = *landingPos;
    warning = Actor_SpawnAsChild(&play->actorCtx, &magic->actor, play, ACTOR_BOSS_TW, landingPos->x, landingPos->y,
                                 landingPos->z, 0, 0, 0, warningParams);
    if (warning == NULL) {
        // A rain strike is never allowed to exist without its authoritative floor warning.
        Actor_Kill(&magic->actor);
        return false;
    }

    *magicOut = magic;
    *warningOut = warning;
    return true;
}

static void BossTw_RollbackStagedRain(BossTw* magic, Actor* warning) {
    if (warning != NULL && warning->update != NULL) {
        Actor_Kill(warning);
    }
    if (magic != NULL && magic->actor.update != NULL) {
        Actor_Kill(&magic->actor);
    }
}

static void BossTw_SpawnSpiralRainPair(PlayState* play, s16 volleyIndex) {
    BossTw* kotakeMagic = NULL;
    BossTw* koumeMagic = NULL;
    Actor* kotakeWarning = NULL;
    Actor* koumeWarning = NULL;
    Vec3f kotakeTarget;
    Vec3f koumeTarget;

    BossTw_GetRainTargets(play, volleyIndex, sKotakePtr->beamShootState, &kotakeTarget, &koumeTarget);
    if (!BossTw_TryStageRainBlast(sKotakePtr, play, &sKotakePtr->beamOrigin, &kotakeTarget, TWINROVA_MAGIC_ICE,
                                  &kotakeMagic, &kotakeWarning) ||
        !BossTw_TryStageRainBlast(sKoumePtr, play, &sKoumePtr->beamOrigin, &koumeTarget, TWINROVA_MAGIC_FIRE,
                                  &koumeMagic, &koumeWarning)) {
        // The move promises a synchronized elemental pair. Under actor pressure, roll back every staged component
        // rather than allowing one fully marked strike to survive as an unintended asymmetric pattern.
        BossTw_RollbackStagedRain(kotakeMagic, kotakeWarning);
        BossTw_RollbackStagedRain(koumeMagic, koumeWarning);
        BossTw_ShowFailedMagicCast(play, &sKotakePtr->beamOrigin, TWINROVA_MAGIC_ICE);
        BossTw_ShowFailedMagicCast(play, &sKoumePtr->beamOrigin, TWINROVA_MAGIC_FIRE);
        return;
    }

    // Commit cosmetic feedback only after both projectiles and both authoritative floor markers exist.
    BossTw_SpawnMagicLaunchEffects(play, &sKotakePtr->beamOrigin, kotakeMagic->blastType,
                                   TWINROVA_RAIN_SPAWN_EFFECTS);
    BossTw_SpawnMagicLaunchEffects(play, &sKoumePtr->beamOrigin, koumeMagic->blastType,
                                   TWINROVA_RAIN_SPAWN_EFFECTS);
    Audio_PlayActorSound2(&sKotakePtr->actor, NA_SE_EN_TWINROBA_THROW_MASIC);
    Audio_PlayActorSound2(&sKoumePtr->actor, NA_SE_EN_TWINROBA_THROW_MASIC);
}

static s32 BossTw_SpawnCycloneRainPair(BossTw* owner, PlayState* play, s16 volleyIndex) {
    BossTw* firstMagic = NULL;
    BossTw* secondMagic = NULL;
    Actor* firstWarning = NULL;
    Actor* secondWarning = NULL;
    TwinrovaMagicElement element = (TwinrovaMagicElement)owner->blastType;
    Vec3f firstTarget;
    Vec3f secondTarget;
    Vec3f firstSpawn;
    Vec3f secondSpawn;

    BossTw_GetRainTargets(play, volleyIndex, owner->beamShootState, &firstTarget, &secondTarget);
    // The fused storm falls out of the overhead rune itself. Each projectile begins directly above its truthful floor
    // marker, producing a readable vertical rain rather than looking like another scepter shot from the moving boss.
    firstSpawn = firstTarget;
    secondSpawn = secondTarget;
    firstSpawn.y = secondSpawn.y = TWINROVA_CYCLONE_SIGIL_HEIGHT;

    if (!BossTw_TryStageRainBlast(owner, play, &firstSpawn, &firstTarget, element, &firstMagic, &firstWarning) ||
        !BossTw_TryStageRainBlast(owner, play, &secondSpawn, &secondTarget, element, &secondMagic, &secondWarning)) {
        BossTw_RollbackStagedRain(firstMagic, firstWarning);
        BossTw_RollbackStagedRain(secondMagic, secondWarning);
        BossTw_ShowFailedMagicCast(play, &firstSpawn, element);
        return false;
    }

    // Dense rain needs one rhythm cue, not four simultaneous cues per pair. Every strike remains independently marked;
    // only the redundant particles, warning sound, impact sound, and light flash are reduced to every second wave.
    firstMagic->work[TWINROVA_RAIN_PLAY_CYCLONE_CUE] = (volleyIndex & 1) == 0;
    secondMagic->work[TWINROVA_RAIN_PLAY_CYCLONE_CUE] = false;
    if (firstMagic->work[TWINROVA_RAIN_PLAY_CYCLONE_CUE]) {
        BossTw_SpawnMagicLaunchEffects(play, &firstSpawn, element, TWINROVA_CYCLONE_RAIN_SPAWN_EFFECTS);
        BossTw_SpawnMagicLaunchEffects(play, &secondSpawn, element, TWINROVA_CYCLONE_RAIN_SPAWN_EFFECTS);
        Audio_PlayActorSound2(&owner->actor, NA_SE_EN_TWINROBA_THROW_MASIC);
        play->envCtx.unk_D8 = 0.45f;
    }
    return true;
}

void BossTw_SpiralBarrage(BossTw* this, PlayState* play) {
    BossTw* otherTw = (BossTw*)this->actor.parent;

    if (otherTw == NULL || otherTw->actionFunc != BossTw_SpiralBarrage) {
        BossTw_SetupFlyTo(this, play);
        return;
    }

    BossTw_UpdateSpiralFlight(this, play);
    if (this != sKoumePtr) {
        return;
    }

    switch (this->csState1) {
        case 0:
            if (this->timers[0] == 0 ||
                (Math_Vec3f_DistXYZ(&this->actor.world.pos, &this->targetPos) <
                     TWINROVA_SPIRAL_FORMATION_DISTANCE &&
                 Math_Vec3f_DistXYZ(&otherTw->actor.world.pos, &otherTw->targetPos) <
                     TWINROVA_SPIRAL_FORMATION_DISTANCE)) {
                this->csState1 = otherTw->csState1 = 1;
                this->work[YAW_TGT] -= BossTw_GetSpiralFrameAngle(play);
                otherTw->work[YAW_TGT] -= BossTw_GetSpiralFrameAngle(play);
                this->timers[0] = otherTw->timers[0] = TWINROVA_SPIRAL_WINDUP;
                BossTw_ShowSisterAttackTell(this, play, 0.65f, 20);
                BossTw_ShowSisterAttackTell(otherTw, play, 0.65f, 20);
            }
            break;

        case 1:
            if (this->timers[0] == 30 || this->timers[0] == 10) {
                BossTw_ShowSisterAttackTell(this, play, this->timers[0] == 10 ? 0.7f : 0.5f,
                                            this->timers[0] == 10 ? 16 : 8);
                BossTw_ShowSisterAttackTell(otherTw, play, this->timers[0] == 10 ? 0.7f : 0.5f,
                                            this->timers[0] == 10 ? 16 : 8);
            }
            if (this->timers[0] == 0) {
                this->csState1 = otherTw->csState1 = 2;
                this->timers[0] = otherTw->timers[0] = 0;
                Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeAttackLoopAnim, -5.0f);
                Animation_MorphToLoop(&otherTw->skelAnime, &gTwinrovaKotakeKoumeAttackLoopAnim, -5.0f);
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_VOICE);
            }
            break;

        case 2:
            if (this->timers[0] == 0) {
                s16 volleyIndex = TWINROVA_SPIRAL_VOLLEYS - this->csState2;

                BossTw_SpawnSpiralRainPair(play, volleyIndex);
                this->csState2--;
                otherTw->csState2 = this->csState2;

                if (this->csState2 == 0) {
                    BossTw_SetupSpiralVolleyLanding();
                } else {
                    this->timers[0] = otherTw->timers[0] = TWINROVA_SPIRAL_VOLLEY_INTERVAL;
                }
            }
            break;

        case 3:
            if (this->timers[0] == 0) {
                this->csState1 = otherTw->csState1 = 4;
                this->timers[0] = otherTw->timers[0] = TWINROVA_SPIRAL_FOLLOWUP_RECOVERY;
                BossTw_ShowSisterAttackTell(this, play, 0.5f, 10);
                BossTw_ShowSisterAttackTell(otherTw, play, 0.5f, 10);
            }
            break;

        case 4:
            if (this->timers[0] == 0) {
                // The start-to-start cooldown was committed with the formation. Immediately hand initiative back to
                // one sister while both remain visibly staged on their landing pillars.
                BossTw_StartSpiralVolleyFollowup(play);
            }
            break;
    }
}

void BossTw_PortalReposition(BossTw* this, PlayState* play) {
    f32 scale = this->actor.scale.x;

    this->work[INVINC_TIMER] = 2;
    this->portalRotation += 0.25f;
    SkelAnime_Update(&this->skelAnime);

    switch (this->csState1) {
        case 0:
            Math_ApproachF(&this->workf[UNK_F17], 0.06f, 0.5f, 0.008f);
            Math_ApproachF(&this->workf[UNK_F18], 255.0f, 1.0f, 30.0f);
            Math_ApproachF(&scale, 0.0f, 0.5f, 0.003f);
            Actor_SetScale(&this->actor, scale);
            this->actor.shape.rot.y += 0x1800;
            this->actor.world.rot.y = this->actor.shape.rot.y;

            if (this->timers[0] == 0 || scale == 0.0f) {
                Vec3f sourcePos = this->actor.world.pos;

                BossTw_SpawnPortalBurst(this, play, &sourcePos);
                this->visible = false;
                Actor_SetScale(&this->actor, 0.0f);
                this->actor.world.pos = this->targetPos;
                this->csState1 = 1;
                this->timers[0] = BossTw_GetPortalHiddenTime(this);
            }
            break;

        case 1:
            Math_ApproachF(&this->workf[UNK_F18], 255.0f, 1.0f, 30.0f);
            if (this->timers[0] == 0) {
                this->visible = true;
                Actor_SetScale(&this->actor, 0.001f);
                BossTw_SpawnPortalBurst(this, play, &this->actor.world.pos);
                this->csState1 = 2;
                this->timers[0] = BossTw_UsesFastPortalTiming(this)
                                      ? TWINROVA_PORTAL_AMBUSH_APPEAR_TIME
                                      : TWINROVA_PORTAL_APPEAR_TIME;
            }
            break;

        case 2:
            scale = this->actor.scale.x;
            Math_ApproachF(&scale, 0.025f, 0.5f, 0.003f);
            Actor_SetScale(&this->actor, scale);
            Math_ApproachF(&this->workf[UNK_F18], 0.0f, 1.0f, 30.0f);
            Math_ApproachF(&this->workf[UNK_F17], 0.0f, 0.5f, 0.008f);

            if (this->timers[0] == 0) {
                BossTw* otherTw = (BossTw*)this->actor.parent;

                Actor_SetScale(&this->actor, 0.025f);
                this->visible = true;
                this->workf[UNK_F17] = this->workf[UNK_F18] = 0.0f;
                this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
                this->actor.world.rot.x = this->actor.shape.rot.x = 0;
                this->work[CAN_SHOOT] = true;
                if (BossTw_IsPortalPressureTriangle(this)) {
                    // The flanker is visible and tracking, but remains harmless until the lead volley is completely
                    // spent. That produces a tight handoff without concurrent sister-owned projectile releases.
                    if (otherTw != NULL && otherTw->actionFunc == BossTw_BlastVolley &&
                        otherTw->beamShootState == TWINROVA_VOLLEY_MODE_PRESSURE_TRIANGLE) {
                        return;
                    }
                    BossTw_SetupPressureTriangleHandoffVolley(this, play);
                } else if ((otherTw == NULL || otherTw->actionFunc != BossTw_ShootBeam) &&
                    BossTw_ShouldUsePortalFakeOut(this)) {
                    // The first arrival is intentionally harmless: flash the element, reserve a new pillar, then
                    // replay the portal once. The flag in csState2 makes a second feint impossible this sequence.
                    BossTw_ShowSisterAttackTell(this, play, 0.8f, 20);
                    BossTw_BeginPortalFakeOut(this, play);
                } else if (otherTw != NULL && otherTw->actionFunc == BossTw_ShootBeam) {
                    // Finish the portal before honoring beam-target staging; never interrupt the readable vanish.
                    BossTw_SetupFlyTo(this, play);
                } else if (BossTw_IsPortalAmbush(this)) {
                    // The portal burst is the positional warning. Convert it directly into a short, elemental volley
                    // before the sibling can roll a beam, preserving one readable ranged source at a time.
                    BossTw_SetupAmbushVolley(this, play);
                } else {
                    // The portal exit is the positional tell. Spend it on a normal attack whenever the arena can
                    // support one instead of forcing a long neutral turn before pressure resumes.
                    BossTw_StartPortalFollowup(this, otherTw, play);
                }
            }
            break;
    }
}

void BossTw_CrossfireVolley(BossTw* this, PlayState* play) {
    BossTw* otherTw = (BossTw*)this->actor.parent;
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    f32 xzDist;
    s16 yawTarget;
    s16 pitchTarget;

    SkelAnime_Update(&this->skelAnime);
    xDiff = this->targetPos.x - this->actor.world.pos.x;
    yDiff = this->targetPos.y - this->actor.world.pos.y;
    zDiff = this->targetPos.z - this->actor.world.pos.z;
    xzDist = sqrtf(SQ(xDiff) + SQ(zDiff));
    yawTarget = Math_FAtan2F(xDiff, zDiff) * (32768.0f / M_PI);
    pitchTarget = Math_FAtan2F(yDiff, xzDist) * (32768.0f / M_PI);

    if (this->csState1 == 0) {
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FLY - SFX_FLAG);
        Math_ApproachS(&this->actor.world.rot.x, pitchTarget, 0xA, this->rotateSpeed);
        Math_ApproachS(&this->actor.world.rot.y, yawTarget, 0xA, this->rotateSpeed);
        Math_ApproachS(&this->actor.shape.rot.y, yawTarget, 0xA, this->rotateSpeed);
        Math_ApproachS(&this->actor.shape.rot.x, pitchTarget, 0xA, this->rotateSpeed);
        Math_ApproachF(&this->rotateSpeed, 4096.0f, 1.0f, 100.0f);

        if (this->timers[0] != 0) {
            Math_ApproachF(&this->actor.speedXZ, 10.0f, 1.0f, 1.0f);
            Actor_UpdateVelocityXYZ(&this->actor);
            Actor_UpdatePos(&this->actor);
        } else {
            // A worst-case opposite-pillar turn can consume the full travel budget before reaching formation. Finish
            // the last gap directly so the authored crossfire positions remain truthful without a visible snap.
            this->actor.speedXZ = 0.0f;
            Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.5f, 40.0f);
            Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.5f, 30.0f);
            Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.5f, 40.0f);
        }

        if (xzDist < 70.0f && fabsf(yDiff) < 70.0f) {
            this->csState1 = 1;
            this->actor.speedXZ = 0.0f;
            this->actor.world.rot.x = this->actor.shape.rot.x = 0;
        }
    } else {
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1000);
    }

    if (this->csState1 == 1 && otherTw != NULL && otherTw->actionFunc == BossTw_CrossfireVolley &&
        otherTw->csState1 == 1) {
        BossTw_StartCrossfireVolley(play);
    }
}

static void BossTw_SetupCoordinatedBlastVolley(BossTw* this, BossTw* otherTw, PlayState* play) {
    BossTw_SetupBlastVolley(this, play);

    if (otherTw != NULL && BossTw_IsNormalPhaseOneMovement(otherTw) &&
        BossTw_CountActiveSummons(play) <= TWINROVA_PHASE_ONE_DUAL_VOLLEY_SUMMON_LIMIT &&
        !BossTw_HasActiveGroundPressure(play) &&
        BossTw_IsActorInCameraFrustumWithMargin(play, &this->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN) &&
        BossTw_IsActorInCameraFrustumWithMargin(play, &otherTw->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN) &&
        Rand_ZeroOne() < TWINROVA_PHASE_ONE_DUAL_VOLLEY_CHANCE) {
        // Author the pair as one move. Both actors share the next gameplay frame as their animation epoch, so actor
        // list order cannot create a late join or coincident opposite-element releases.
        BossTw_SetupBlastVolley(otherTw, play);
        otherTw->beamShootState = true;
    }
}

void BossTw_SetupTurnToPlayer(BossTw* this, PlayState* play) {
    BossTw* otherTw = (BossTw*)this->actor.parent;

    this->actionFunc = BossTw_TurnToPlayer;

    if ((otherTw != NULL) && (otherTw->actionFunc == BossTw_ShootBeam)) {
        this->timers[0] = 40;
    } else {
        this->timers[0] = BossTw_GetPhaseOneTurnTime();
    }

    this->rotateSpeed = 0.0f;
    // FlyTo used to carry its full ten-unit velocity through most of the turn state. Brake at commitment so a sister
    // cannot glide through the sibling after already reaching her reserved destination.
    this->actor.speedXZ = MIN(this->actor.speedXZ, 4.0f);
}

void BossTw_TurnToPlayer(BossTw* this, PlayState* play) {
    BossTw* otherTw = (BossTw*)this->actor.parent;
    s32 projectedSummonPressure;
    s32 canStartSpiral;
    s32 canStartCrossfire;
    s32 canStartPressureTriangle;
    s32 canStartPortal;
    s32 hasActivePhaseOneMagic;
    s32 bothRegularAttacksEligible;
    s32 forceVolley;
    s32 suppressVolley;

    SkelAnime_Update(&this->skelAnime);
    Math_ApproachF(&this->actor.speedXZ, 0.0f, 1.0f, 1.0f);
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, this->rotateSpeed);
    Math_ApproachS(&this->actor.shape.rot.x, 0, 5, this->rotateSpeed);
    Math_ApproachF(&this->rotateSpeed, 4096.0f, 1.0f, 200.0f);
    Actor_UpdateVelocityXYZ(&this->actor);
    Actor_UpdatePos(&this->actor);
    if (this->timers[0] == 0) {
        if (sPhaseOneSchedulerDecisionFrame == play->gameplayFrames) {
            // Hyper Bosses may substep neutral movement. Never allow those extra updates, or the sibling's later
            // actor update, to turn one shared choice into multiple attack rolls in the same gameplay frame.
            this->timers[0] = 2;
            return;
        }

        if (otherTw != NULL && otherTw->actionFunc == BossTw_TurnToPlayer && otherTw->timers[0] <= 1) {
            if (Rand_ZeroOne() < 0.5f) {
                // When both sisters become ready together, randomly let the other one choose first. Without this
                // brief handoff, actor-list update order deterministically favors the same element every time.
                // Two ticks also mark this sister as deferred before the other actor updates later in the same frame.
                this->timers[0] = 2;
                return;
            }

            // This sister won the shared decision. Reserve the sibling now so a movement fallback cannot let it roll
            // the same scheduler again later in this gameplay frame and double-count one special-pity opportunity.
            otherTw->timers[0] = 2;
        }
        sPhaseOneSchedulerDecisionFrame = play->gameplayFrames;

        projectedSummonPressure = BossTw_GetProjectedSummonPressure(play);
        canStartSpiral = BossTw_CanStartSpiralBarrage(this, otherTw, play);
        canStartCrossfire = BossTw_CanStartCrossfireVolley(this, otherTw, play);
        canStartPressureTriangle = BossTw_CanStartPressureTriangle(this, otherTw, play);
        canStartPortal = BossTw_CanStartPortalReposition(this, otherTw, play);
        hasActivePhaseOneMagic = BossTw_HasActivePhaseOneMagic(play);
        bothRegularAttacksEligible =
            projectedSummonPressure <= TWINROVA_BEAM_SUMMON_LIMIT && BossTw_IsNormalPhaseOneMovement(otherTw) &&
            !hasActivePhaseOneMagic &&
            BossTw_IsActorInCameraFrustumWithMargin(play, &this->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN) &&
            BossTw_IsActorInCameraFrustumWithMargin(play, &otherTw->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN);
        forceVolley = bothRegularAttacksEligible &&
                      sPhaseOneLastRegularAttack == TWINROVA_PHASE_ONE_REGULAR_BEAM &&
                      sPhaseOneRegularRepeatCount >= TWINROVA_PHASE_ONE_REGULAR_REPEAT_LIMIT;
        suppressVolley = bothRegularAttacksEligible &&
                         sPhaseOneLastRegularAttack == TWINROVA_PHASE_ONE_REGULAR_VOLLEY &&
                         sPhaseOneRegularRepeatCount >= TWINROVA_PHASE_ONE_REGULAR_REPEAT_LIMIT;

        // Beam and volley commitments need visible sources. A beam additionally needs its passive sister already
        // visible so the redirect puzzle never depends on a late portal or a long cross-arena staging flight.
        if (BossTw_IsPhaseOneHitGoalReached() || otherTw == NULL || otherTw->actionFunc == BossTw_ShootBeam ||
            otherTw->actionFunc == BossTw_Spin) {
            BossTw_SetupFlyTo(this, play);
            return;
        }

        if (BossTw_TryStartPhaseOneSpecial(
                this, canStartSpiral, canStartCrossfire, canStartPressureTriangle, canStartPortal,
                (canStartSpiral || canStartCrossfire || canStartPressureTriangle || canStartPortal) &&
                    sPhaseOneEligibleSpecialMisses >= TWINROVA_PHASE_ONE_SPECIAL_PITY_MISSES,
                play)) {
            BossTw_RecordPhaseOneSpecialAttack();
            return;
        }
        if (canStartSpiral || canStartCrossfire || canStartPressureTriangle || canStartPortal) {
            // One shared scheduler decision may reject the special category roll, but it contributes one pity miss.
            BossTw_RecordPhaseOneSpecialMiss();
        }

        if (!BossTw_HasPhaseOneSpacing(this, TWINROVA_PHASE_ONE_TARGET_SEPARATION)) {
            // Special moves establish their own authored formation. Routine beams and volleys must first reclaim
            // enough horizontal space that both elemental sources and the redirect target remain visually distinct.
            BossTw_SetupFlyTo(this, play);
            return;
        }

        if (projectedSummonPressure > TWINROVA_BEAM_SUMMON_LIMIT) {
            // Keep one readable ranged threat while adds own the pressure budget.
            if (otherTw->actionFunc == BossTw_BlastVolley || hasActivePhaseOneMagic ||
                otherTw->actionFunc == BossTw_PortalReposition ||
                !BossTw_IsActorInCameraFrustumWithMargin(play, &this->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN)) {
                BossTw_SetupFlyTo(this, play);
                return;
            }
            this->work[CAN_SHOOT] = false;
            BossTw_SetupBlastVolley(this, play);
            this->actor.speedXZ = 0.0f;
            BossTw_RecordPhaseOneRegularAttack(TWINROVA_PHASE_ONE_REGULAR_VOLLEY);
            return;
        }

        if (otherTw->actionFunc == BossTw_BlastVolley) {
            // A dual volley is committed atomically when the first sister chooses it. Never join a running windup;
            // late entry makes opposite elements coincide unpredictably and turns shield routing into update-order RNG.
            BossTw_SetupFlyTo(this, play);
            return;
        }

        if (otherTw->actionFunc != BossTw_PortalReposition && !hasActivePhaseOneMagic &&
            (forceVolley || (!suppressVolley && Rand_ZeroOne() < TWINROVA_PHASE_ONE_VOLLEY_CHANCE))) {
            if (BossTw_IsActorInCameraFrustumWithMargin(play, &this->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN)) {
                this->work[CAN_SHOOT] = false;
                BossTw_SetupCoordinatedBlastVolley(this, otherTw, play);
                BossTw_RecordPhaseOneRegularAttack(TWINROVA_PHASE_ONE_REGULAR_VOLLEY);
            } else {
                // Curving projectiles need an on-screen source tell. Reposition instead of silently replacing the
                // selected volley with a beam or releasing it from outside the player's view.
                BossTw_SetupFlyTo(this, play);
            }
            return;
        }

        if (bothRegularAttacksEligible) {
            this->work[CAN_SHOOT] = false;
            BossTw_SetupShootBeam(this, play);
            this->actor.speedXZ = 0.0f;
            BossTw_RecordPhaseOneRegularAttack(TWINROVA_PHASE_ONE_REGULAR_BEAM);
            return;
        }

        BossTw_SetupFlyTo(this, play);
    }
}

void BossTw_SetupFlyTo(BossTw* this, PlayState* play) {
    BossTw* otherTw = (BossTw*)this->actor.parent;
    s32 otherSisterIsBeaming = otherTw != NULL && otherTw->actionFunc == BossTw_ShootBeam;
    s32 useEscalatedPacing = BossTw_IsPhaseOneEscalated();
    s32 usePillarDestination = true;
    s16 attempt;
    s16 travelBase = useEscalatedPacing ? TWINROVA_PHASE_ONE_ESCALATED_TRAVEL_BASE
                                        : TWINROVA_PHASE_ONE_TRAVEL_BASE;
    s16 travelRange = useEscalatedPacing ? TWINROVA_PHASE_ONE_ESCALATED_TRAVEL_RANGE
                                         : TWINROVA_PHASE_ONE_TRAVEL_RANGE;

    if (otherSisterIsBeaming) {
        // Beam authorization already proved this sister visible. Hold that known-good redirect target instead of
        // risking a cross-arena route that cannot be guaranteed to finish before the beam becomes active.
        BossTw_SetupTurnToPlayer(this, play);
        this->actor.speedXZ = 0.0f;
        this->work[CAN_SHOOT] = false;
        return;
    }

    this->unk_5F8 = 1;
    this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    this->actionFunc = BossTw_FlyTo;
    this->rotateSpeed = 0.0f;
    this->work[CAN_SHOOT] = true;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, -10.0f);
    if (Rand_ZeroOne() < 0.5f) {
        for (attempt = 0; attempt < TWINROVA_PHASE_ONE_POSITION_ATTEMPTS; attempt++) {
            Vec3f candidate;

            candidate.x = Rand_CenteredFloat(800.0f);
            candidate.y = Rand_ZeroFloat(200.0f) + 340.0f;
            candidate.z = Rand_CenteredFloat(800.0f);
            if (BossTw_IsPhaseOneDestinationClear(this, &candidate, 120.0f)) {
                this->targetPos = candidate;
                usePillarDestination = false;
                break;
            }
        }
    }

    if (usePillarDestination) {
        BossTw_SelectSeparatedPhaseOnePillar(this, 120.0f);
        this->timers[0] = useEscalatedPacing ? TWINROVA_PHASE_ONE_ESCALATED_PILLAR_TIME
                                             : TWINROVA_PHASE_ONE_PILLAR_TIME;
    } else {
        this->timers[0] = (s16)Rand_ZeroFloat(travelRange) + travelBase;
    }
}

void BossTw_FlyTo(BossTw* this, PlayState* play) {
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    f32 pitchTarget;
    f32 yawTarget;
    f32 xzDist;

    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FLY - SFX_FLAG);
    Math_ApproachF(&this->scepterAlpha, 0.0f, 1.0f, 10.0f);
    SkelAnime_Update(&this->skelAnime);

    xDiff = this->targetPos.x - this->actor.world.pos.x;
    yDiff = this->targetPos.y - this->actor.world.pos.y;
    zDiff = this->targetPos.z - this->actor.world.pos.z;

    yawTarget = (s16)(Math_FAtan2F(xDiff, zDiff) * (32768.0f / M_PI));
    xzDist = sqrtf(SQ(xDiff) + SQ(zDiff));
    pitchTarget = (s16)(Math_FAtan2F(yDiff, xzDist) * (32768.0f / M_PI));

    Math_ApproachS(&this->actor.world.rot.x, pitchTarget, 0xA, this->rotateSpeed);
    Math_ApproachS(&this->actor.world.rot.y, yawTarget, 0xA, this->rotateSpeed);
    Math_ApproachS(&this->actor.shape.rot.y, yawTarget, 0xA, this->rotateSpeed);
    Math_ApproachS(&this->actor.shape.rot.x, pitchTarget, 0xA, this->rotateSpeed);
    Math_ApproachF(&this->rotateSpeed, 4096.0f, 1.0f, 100.0f);
    Math_ApproachF(&this->actor.speedXZ, 10.0f, 1.0f, 1.0f);
    Actor_UpdateVelocityXYZ(&this->actor);
    Actor_UpdatePos(&this->actor);

    if ((this->timers[0] == 0) || (xzDist < 70.0f)) {
        BossTw_SetupTurnToPlayer(this, play);
    }
}

void BossTw_SetupShootBeam(BossTw* this, PlayState* play) {
    BossTw* otherTw = (BossTw*)this->actor.parent;
    Player* player = GET_PLAYER(play);

    this->actionFunc = BossTw_ShootBeam;
    Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaKotakeKoumeChargeUpAttackStartAnim, -5.0f);
    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeChargeUpAttackStartAnim);
    this->timers[1] = 70;
    this->timers[3] = 0;
    this->targetPos = player->actor.world.pos;
    this->csState1 = 0;
    this->beamDist = 0.0f;
    this->beamReflectionDist = 0.0f;
    this->beamShootState = -1;
    this->beamScale = 0.01f;
    this->beamReflectionOrigin = this->beamOrigin;
    this->flameAlpha = 0.0f;
    this->spawnPortalAlpha = 0.0f;
    this->spawnPortalScale = 2000.0f;
    this->updateRate1 = 0.0f;
    this->portalRotation = 0.0f;
    this->updateRate2 = 0.0f;

    if (BossTw_IsNormalPhaseOneMovement(otherTw)) {
        // Stop the visible passive sister at the same instant as the beam and keep her available as its redirect
        // target. TurnToPlayer refreshes this hold for as long as the caster remains in ShootBeam.
        BossTw_SetupTurnToPlayer(otherTw, play);
        otherTw->actor.speedXZ = 0.0f;
        otherTw->work[CAN_SHOOT] = false;
    }
}

void BossTw_SetupBlastVolley(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_BlastVolley;
    Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaKotakeKoumeAttackStartAnim, -5.0f);
    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeAttackStartAnim);
    this->csState1 = 0;
    this->csState2 = -1;
    this->timers[0] = 0;
    this->timers[3] = 0;
    this->work[YAW_TGT] = (s16)(play->gameplayFrames + 1);
    this->beamShootState = false;
    this->actor.speedXZ = 0.0f;
    BossTw_ShowSisterAttackTell(this, play, 0.45f, 12);
}

static void BossTw_SetupAmbushVolley(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_BlastVolley;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeAttackLoopAnim, -3.0f);
    this->workf[ANIM_SW_TGT] = 10000.0f;
    this->csState1 = 2 + (BossTw_IsPhaseOneEscalated() || Rand_ZeroOne() < 0.5f);
    this->csState2 = 0;
    this->timers[0] = TWINROVA_PORTAL_AMBUSH_TELL_TIME;
    this->timers[3] = 0;
    this->beamShootState = TWINROVA_VOLLEY_MODE_AMBUSH;
    this->actor.speedXZ = 0.0f;
    // The teleport itself establishes the new firing angle. This compact elemental tell is only for the first shot.
    BossTw_ShowSisterAttackTell(this, play, 0.65f, 16);
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_VOICE);
}

static void BossTw_SetupPressureTriangleHandoffVolley(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_BlastVolley;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeAttackLoopAnim, -3.0f);
    this->workf[ANIM_SW_TGT] = 10000.0f;
    this->csState1 = 2;
    this->csState2 = 0;
    this->timers[0] = TWINROVA_PRESSURE_TRIANGLE_TELL_TIME;
    this->timers[3] = 0;
    this->beamShootState = TWINROVA_VOLLEY_MODE_PRESSURE_TRIANGLE;
    this->actor.speedXZ = 0.0f;
    BossTw_ShowSisterAttackTell(this, play, 0.65f, 16);
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_VOICE);
}

static void BossTw_SetupPillarDiveRetreatVolley(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_BlastVolley;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeAttackLoopAnim, -3.0f);
    this->workf[ANIM_SW_TGT] = 10000.0f;
    this->csState1 = TWINROVA_PILLAR_DIVE_VOLLEY_SHOTS;
    this->csState2 = 0;
    this->timers[0] = TWINROVA_PILLAR_DIVE_VOLLEY_TELL_TIME;
    this->beamShootState = TWINROVA_VOLLEY_MODE_PILLAR_DIVE;
    this->actor.speedXZ = 0.0f;
    BossTw_ShowSisterAttackTell(this, play, 0.65f, 16);
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_VOICE);
}

void BossTw_BlastVolley(BossTw* this, PlayState* play) {
    BossTw* magic;
    s16 magicParams;
    s16 recastTellTime = this->beamShootState == TWINROVA_VOLLEY_MODE_AMBUSH
                               ? TWINROVA_PORTAL_AMBUSH_RECAST_TELL_TIME
                               : this->beamShootState == TWINROVA_VOLLEY_MODE_CROSSFIRE
                                     ? TWINROVA_CROSSFIRE_RECAST_TELL_TIME
                                     : this->beamShootState == TWINROVA_VOLLEY_MODE_PRESSURE_TRIANGLE
                                           ? TWINROVA_PRESSURE_TRIANGLE_RECAST_TELL_TIME
                                     : this->beamShootState == TWINROVA_VOLLEY_MODE_PILLAR_DIVE
                                           ? TWINROVA_PILLAR_DIVE_VOLLEY_TELL_TIME
                                     : TWINROVA_PHASE_ONE_VOLLEY_RECAST_TELL_TIME;

    BossTw_ShouldSuppressSisterAttackCollision(this, play);

    if (this->csState2 < 0) {
        // Coordinated sisters may be changed into this action on opposite sides of the actor update order. Begin on
        // the shared next gameplay frame so both windups advance from frame zero together.
        if ((s16)((u16)play->gameplayFrames - (u16)this->work[YAW_TGT]) < 0) {
            return;
        }
        this->csState2 = 0;
        if (this->beamShootState == TWINROVA_VOLLEY_MODE_CROSSFIRE) {
            this->timers[0] = TWINROVA_CROSSFIRE_OPENING_TELL_TIME +
                              (this->actor.params == TW_KOTAKE ? TWINROVA_CROSSFIRE_ELEMENT_STAGGER : 0);
        }
    }

    SkelAnime_Update(&this->skelAnime);
    if (this->beamShootState == TWINROVA_VOLLEY_MODE_PILLAR_DIVE) {
        Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.2f, 24.0f);
        Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.2f, 12.0f);
        Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.2f, 24.0f);
    } else {
        Math_ApproachF(&this->actor.speedXZ, 0.0f, 1.0f, 1.0f);
    }
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1000);

    if (this->csState1 == 0) {
        if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
            Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeAttackLoopAnim, 0.0f);
            this->csState1 = BossTw_GetProjectedSummonPressure(play) > TWINROVA_SUMMON_SOFT_LIMIT
                                 ? TWINROVA_PHASE_ONE_CROWDED_VOLLEY_SHOTS
                                 : TWINROVA_PHASE_ONE_VOLLEY_SHOTS;
            // Give the refreshed commitment flash a real anticipation window. A coordinated sister adds six updates
            // to this same timer, preserving the even 6/6 alternating rhythm without shortening either tell.
            this->timers[0] = TWINROVA_PHASE_ONE_VOLLEY_RECAST_TELL_TIME +
                              (this->beamShootState > 0 ? TWINROVA_PHASE_ONE_DOUBLE_VOLLEY_DELAY : 0);
            // Refresh the setup flash after the long start animation so it remains visible through the first release.
            BossTw_ShowSisterAttackTell(this, play, 0.55f, 8);
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_VOICE);
        }
        return;
    }

    if (this->timers[0] != 0) {
        if (this->csState2 > 0 && this->timers[0] == recastTellTime) {
            // Each follow-up gets its own readable commitment instead of emerging silently from an arbitrary point
            // in the 16-frame loop animation.
            BossTw_ShowSisterAttackTell(this, play, 0.4f, 6);
        }
        return;
    }

    magicParams = this->actor.params == TW_KOUME ? TW_FIRE_BLAST : TW_ICE_BLAST;
    magic = BossTw_SpawnMagicBlast(this, play, &this->beamOrigin, magicParams, TWINROVA_BLAST_CURVING);

    if (magic != NULL) {
        BossTw_SpawnMagicLaunchEffects(play, &this->beamOrigin, magic->blastType,
                                       TWINROVA_PHASE_ONE_VOLLEY_SPAWN_EFFECTS);
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_THROW_MASIC);
    } else {
        // The animation still consumes this pressure slot, but the promised release must never resolve silently.
        // With a full actor pool, a visible fizzle is fairer than trapping both sisters in an unbounded retry loop.
        BossTw_ShowFailedMagicCast(play, &this->beamOrigin,
                                   this->actor.params == TW_KOUME ? TWINROVA_MAGIC_FIRE : TWINROVA_MAGIC_ICE);
    }

    this->csState1--;
    this->csState2++;
    if (this->csState1 == 0) {
        BossTw_SetupFinishBeamShoot(this, play);
    } else {
        Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeAttackLoopAnim, 0.0f);
        this->timers[0] = this->beamShootState == TWINROVA_VOLLEY_MODE_AMBUSH
                              ? TWINROVA_PORTAL_AMBUSH_VOLLEY_INTERVAL
                              : this->beamShootState == TWINROVA_VOLLEY_MODE_CROSSFIRE
                                    ? TWINROVA_CROSSFIRE_VOLLEY_INTERVAL
                                    : this->beamShootState == TWINROVA_VOLLEY_MODE_PRESSURE_TRIANGLE
                                          ? TWINROVA_PRESSURE_TRIANGLE_VOLLEY_INTERVAL
                                    : this->beamShootState == TWINROVA_VOLLEY_MODE_PILLAR_DIVE
                                          ? TWINROVA_PILLAR_DIVE_VOLLEY_INTERVAL
                                    : TWINROVA_PHASE_ONE_VOLLEY_INTERVAL;
    }
}

static s32 BossTw_HasGroundHazard(PlayState* play, s16 params) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        if (actor->id == ACTOR_BOSS_TW && actor->update != NULL && actor->params == params) {
            return true;
        }
        actor = actor->next;
    }

    return false;
}

static s32 BossTw_HasActiveGroundPressure(PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        if (actor->id == ACTOR_BOSS_TW && actor->update != NULL &&
            (actor->params == TW_FIRE_BLAST_GROUND || actor->params == TW_ICE_BLAST_GROUND)) {
            BossTw* groundBlast = (BossTw*)actor;
            // Ground actors deliberately outlive their damage timer to render the pool's fade. Only global elemental
            // ownership or an already-advertised summon is gameplay pressure; actor existence alone is visual state.
            s32 activePool = groundBlast->timers[0] != 0 &&
                             ((actor->params == TW_FIRE_BLAST_GROUND && sGroundBlastType == 1) ||
                              (actor->params == TW_ICE_BLAST_GROUND && sGroundBlastType == 2));
            s32 committedSummon = groundBlast->work[CAN_SHOOT] && groundBlast->beamShootState != 0;

            if (activePool || committedSummon) {
                return true;
            }
        }
        actor = actor->next;
    }

    return false;
}

static s32 BossTw_HasActiveArenaControlPattern(BossTw* owner, PlayState* play) {
    // This intentionally ignores fading actors. Their draw lifetime is presentation, not a second gameplay hazard.
    return BossTw_HasActiveGroundPressure(play) || BossTw_HasSiegeZones(play, owner) ||
           BossTw_CountActiveMinefieldPools(play, owner) != 0;
}

static void BossTw_RestoreGroundHazardEnvironment(void) {
    if (sGroundBlastType == 1) {
        sEnvType = 4;
    } else if (sGroundBlastType == 2) {
        sEnvType = 3;
    } else {
        sEnvType = 0;
    }
}

static void BossTw_ReplaceExistingGroundHazard(PlayState* play, s16 params, Actor* replacement) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;
    Actor* next;

    while (actor != NULL) {
        next = actor->next;
        if (actor != replacement && actor->id == ACTOR_BOSS_TW && actor->update != NULL && actor->params == params) {
            BossTw_CancelGroundSummonWarning((BossTw*)actor, play, true);
            Actor_Kill(actor);
        }
        actor = next;
    }
}

void BossTw_SpawnGroundBlast(BossTw* this, PlayState* play, s16 blastType) {
    BossTw* groundBlast;
    s16 i;
    Vec3f pos;
    Vec3f velocity;
    Vec3f accel;

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        velocity.x = Rand_CenteredFloat(20.0f);
        velocity.y = Rand_ZeroFloat(10.0f);
        velocity.z = Rand_CenteredFloat(20.0f);
        accel.y = 0.2f;
        accel.x = Rand_CenteredFloat(0.25f);
        accel.z = Rand_CenteredFloat(0.25f);
        pos = this->groundBlastPos;
        BossTw_AddDotEffect(play, &pos, &velocity, &accel, (s16)Rand_ZeroFloat(2.0f) + 8, blastType, 75);
    }

    if (blastType == 1) {
        groundBlast =
            (BossTw*)Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_BOSS_TW, this->groundBlastPos.x,
                                        this->groundBlastPos.y, this->groundBlastPos.z, 0, 0, 0, TW_FIRE_BLAST_GROUND);
        if (groundBlast != NULL) {
            BossTw_ReplaceExistingGroundHazard(play, TW_FIRE_BLAST_GROUND, &groundBlast->actor);
            sGroundBlastType = 1;
            groundBlast->timers[0] =
                sTwinrovaPtr != NULL && sTwinrovaPtr->actionFunc == BossTw_Wait ? 100 : 50;
            if (BossTw_CanGroundHazardSummon(play)) {
                groundBlast->timers[3] = TWINROVA_SUMMON_EMERGENCE_DELAY;
                groundBlast->work[CAN_SHOOT] = true;
            }
            sKoumePtr->workf[KM_GD_FLM_A] = sKoumePtr->workf[KM_GD_SMOKE_A] = sKoumePtr->workf[KM_GRND_CRTR_A] = 255.0f;
            sKoumePtr->workf[KM_GD_FLM_SCL] = 1.0f;
            sKoumePtr->workf[KM_GD_CRTR_SCL] = 0.005f;
            sKoumePtr->groundBlastPos2 = groundBlast->actor.world.pos;
            sEnvType = 4;
        } else {
            if (!BossTw_HasGroundHazard(play, TW_FIRE_BLAST_GROUND)) {
                if (sGroundBlastType == 1) {
                    sGroundBlastType = 0;
                }
                sKoumePtr->workf[UNK_F9] = sKoumePtr->workf[UNK_F10] = sKoumePtr->workf[UNK_F11] = 0.0f;
                sKoumePtr->workf[UNK_F12] = sKoumePtr->workf[UNK_F13] = 0.0f;
            }
            BossTw_RestoreGroundHazardEnvironment();
        }
    } else {
        groundBlast =
            (BossTw*)Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_BOSS_TW, this->groundBlastPos.x,
                                        this->groundBlastPos.y, this->groundBlastPos.z, 0, 0, 0, TW_ICE_BLAST_GROUND);
        if (groundBlast != NULL) {
            BossTw_ReplaceExistingGroundHazard(play, TW_ICE_BLAST_GROUND, &groundBlast->actor);
            sGroundBlastType = 2;
            groundBlast->timers[0] =
                sTwinrovaPtr != NULL && sTwinrovaPtr->actionFunc == BossTw_Wait ? 100 : 50;
            if (BossTw_CanGroundHazardSummon(play)) {
                groundBlast->timers[3] = TWINROVA_SUMMON_EMERGENCE_DELAY;
                groundBlast->work[CAN_SHOOT] = true;
            }

            sKotakePtr->workf[UNK_F11] = 50.0f;
            sKotakePtr->workf[UNK_F9] = 250.0f;
            sKotakePtr->workf[UNK_F12] = 0.005f;
            sKotakePtr->workf[UNK_F14] = 1.0f;
            sKotakePtr->workf[UNK_F16] = 70.0f;
            sKotakePtr->groundBlastPos2 = groundBlast->actor.world.pos;
            sEnvType = 3;
        } else {
            if (!BossTw_HasGroundHazard(play, TW_ICE_BLAST_GROUND)) {
                if (sGroundBlastType == 2) {
                    sGroundBlastType = 0;
                }
                sKotakePtr->workf[UNK_F9] = sKotakePtr->workf[UNK_F11] = sKotakePtr->workf[UNK_F12] = 0.0f;
                sKotakePtr->workf[UNK_F14] = sKotakePtr->workf[UNK_F15] = sKotakePtr->workf[UNK_F16] = 0.0f;
            }
            BossTw_RestoreGroundHazardEnvironment();
        }
    }
}

s32 BossTw_BeamHitPlayerCheck(BossTw* this, PlayState* play) {
    Vec3f offset;
    Vec3f beamDistFromPlayer;
    Player* player = GET_PLAYER(play);
    s16 i;

    offset.x = player->actor.world.pos.x - this->beamOrigin.x;
    offset.y = player->actor.world.pos.y - this->beamOrigin.y;
    offset.z = player->actor.world.pos.z - this->beamOrigin.z;

    Matrix_RotateX(-this->beamPitch, MTXMODE_NEW);
    Matrix_RotateY(-this->beamYaw, MTXMODE_APPLY);
    Matrix_MultVec3f(&offset, &beamDistFromPlayer);

    if (fabsf(beamDistFromPlayer.x) < 20.0f && fabsf(beamDistFromPlayer.y) < 50.0f && beamDistFromPlayer.z > 100.0f &&
        beamDistFromPlayer.z <= this->beamDist) {
        if ((this->actor.params == TW_KOUME && sShieldIceCharge != 0) ||
            (this->actor.params == TW_KOTAKE && sShieldFireCharge != 0)) {
            BossTw_DisruptShieldCharge(play, (TwinrovaMagicElement)this->actor.params);
        }

        if (sTwinrovaPtr->timers[2] == 0) {
            sTwinrovaPtr->timers[2] = 150;
            this->beamDist = sqrtf(SQ(offset.x) + SQ(offset.y) + SQ(offset.z));
            Actor_SetPlayerKnockbackLarge(play, &this->actor, 3.0f, this->actor.shape.rot.y, 0.0f, 0x20);

            if (this->actor.params == 0) {
                BossTw_BeginPlayerFreeze(play);
            } else if (!player->bodyIsBurning) {
                for (i = 0; i < ARRAY_COUNT(player->bodyFlameTimers); i++) {
                    player->bodyFlameTimers[i] = Rand_S16Offset(0, 200);
                }

                player->bodyIsBurning = true;
                Player_PlaySfx(&player->actor, player->ageProperties->unk_92 + NA_SE_VO_LI_DEMO_DAMAGE);
            }
        }

        return true;
    }
    return false;
}

/**
 * Checks if the beam shot by `this` will be reflected
 * returns 0 if the beam will not be reflected,
 * returns 1 if the beam will be reflected,
 * and returns 2 if the beam will be diverted backwards
 */
s32 BossTw_CheckBeamReflection(BossTw* this, PlayState* play) {
    Vec3f offset;
    Vec3f vec;
    Player* player = GET_PLAYER(play);

    if (player->stateFlags1 & PLAYER_STATE1_SHIELDING &&
        (s16)(player->actor.shape.rot.y - this->actor.shape.rot.y + 0x8000) < 0x2000 &&
        (s16)(player->actor.shape.rot.y - this->actor.shape.rot.y + 0x8000) > -0x2000) {
        // player is shielding and facing angles are less than 45 degrees in either direction
        offset.x = 0.0f;
        offset.y = 0.0f;
        offset.z = 10.0f;

        // set beam check point to 10 units in front of link.
        Matrix_RotateY(player->actor.shape.rot.y / 32768.0f * M_PI, MTXMODE_NEW);
        Matrix_MultVec3f(&offset, &vec);

        // calculates a vector where the origin is at the beams origin,
        // and the positive z axis is pointing in the direction the beam
        // is shooting
        offset.x = player->actor.world.pos.x + vec.x - this->beamOrigin.x;
        offset.y = player->actor.world.pos.y + vec.y - this->beamOrigin.y;
        offset.z = player->actor.world.pos.z + vec.z - this->beamOrigin.z;

        Matrix_RotateX(-this->beamPitch, MTXMODE_NEW);
        Matrix_RotateY(-this->beamYaw, MTXMODE_APPLY);
        Matrix_MultVec3f(&offset, &vec);

        if (fabsf(vec.x) < 30.0f && fabsf(vec.y) < 70.0f && vec.z > 100.0f && vec.z <= this->beamDist) {
            // if the beam's origin is within 30 x units, 70 y units, is farther than 100 units
            // and the distance from the beams origin to 10 units in front of link is less than the beams
            // current distance (the distance of the beam is equal to or longer than the distance to 10 units
            // in front of link)
            // Stored Mirror Shield energy belongs to the player, not the currently equipped shield model. An
            // opposing beam contact always breaks it, so swapping shields cannot preserve an invalid charge.
            if ((this->actor.params == TW_KOUME && sShieldIceCharge != 0) ||
                (this->actor.params == TW_KOTAKE && sShieldFireCharge != 0)) {
                BossTw_DisruptShieldCharge(play, (TwinrovaMagicElement)this->actor.params);
            }

            if (Player_HasMirrorShieldEquipped(play)) {
                // player has mirror shield equipped
                this->beamDist = sqrtf(SQ(offset.x) + SQ(offset.y) + SQ(offset.z));
                return 1;
            }

            if (sBeamDivertTimer > 10) {
                return 0;
            }

            if (sBeamDivertTimer == 0) {
                // beam hit the shield, normal shield equipped,
                // divert the beam backwards from link's Y rotation
                BossTw_AddShieldDeflectEffect(play, 10.0f, this->actor.params);
                play->envCtx.unk_D8 = 1.0f;
                this->timers[0] = 10;
                Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_REFLECT_MG2);
            }

            sBeamDivertTimer++;
            this->beamDist = sqrtf(SQ(offset.x) + SQ(offset.y) + SQ(offset.z));
            return 2;
        }
    }

    return 0;
}

s32 BossTw_BeamReflHitCheck(BossTw* this, Vec3f* pos) {
    Vec3f offset;
    Vec3f beamDistFromTarget;

    offset.x = pos->x - this->beamReflectionOrigin.x;
    offset.y = pos->y - this->beamReflectionOrigin.y;
    offset.z = pos->z - this->beamReflectionOrigin.z;

    Matrix_RotateX(-this->beamReflectionPitch, MTXMODE_NEW);
    Matrix_RotateY(-this->beamReflectionYaw, MTXMODE_APPLY);
    Matrix_MultVec3f(&offset, &beamDistFromTarget);

    if (fabsf(beamDistFromTarget.x) < 50.0f && fabsf(beamDistFromTarget.y) < 50.0f && beamDistFromTarget.z > 100.0f &&
        beamDistFromTarget.z <= this->beamReflectionDist) {
        this->beamReflectionDist = sqrtf(SQ(offset.x) + SQ(offset.y) + SQ(offset.z)) * 1.1f;
        return true;
    } else {
        return false;
    }
}

f32 BossTw_GetFloorY(Vec3f* pos) {
    Vec3f posRotated;

    if (fabsf(pos->x) < 350.0f && fabsf(pos->z) < 350.0f && pos->y < 240.0f) {
        if (pos->y > 200.0f) {
            return 240.0f;
        }
        return 35.0f;
    }

    if (fabsf(pos->x) < 110.0f && ((fabsf(pos->z - 600.0f) < 110.0f) || (fabsf(pos->z + 600.0f) < 110.0f)) &&
        (pos->y < 230.0f)) {
        if (pos->y > 190.0f) {
            return 230.0f;
        }
        return 35.0f;
    }

    if (fabsf(pos->z) < 110.0f && ((fabsf(pos->x - 600.0f) < 110.0f) || (fabsf(pos->x + 600.0f) < 110.0f)) &&
        (pos->y < 230.0f)) {
        if (pos->y > 190.0f) {
            return 230.0f;
        }
        return 35.0f;
    }

    if (pos->y < -20.0f) {
        return 0.0f;
    }

    if (fabsf(pos->x) > 1140.0f || fabsf(pos->z) > 1140.0f) {
        return 35.0f;
    }

    Matrix_Push();
    Matrix_RotateY((45.0f * (M_PI / 180.0f)), MTXMODE_NEW);
    Matrix_MultVec3f(pos, &posRotated);
    Matrix_Pop();

    if (fabsf(posRotated.x) > 920.0f || fabsf(posRotated.z) > 920.0f) {
        return 35.0f;
    }

    return -100.0f;
}

void BossTw_ShootBeam(BossTw* this, PlayState* play) {
    s16 i;
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    f32 floorY;
    Vec3f sp130;
    Vec3s sp128;
    Player* player = GET_PLAYER(play);
    BossTw* otherTw = (BossTw*)this->actor.parent;
    Input* input = &play->state.input[0];
    s32 suppressPlayerInteraction;

    suppressPlayerInteraction = BossTw_ShouldSuppressSisterAttackCollision(this, play);

    Math_ApproachF(&this->actor.world.pos.y, 400.0f, 0.05f, this->actor.speedXZ);
    Math_ApproachF(&this->actor.speedXZ, 5.0f, 1.0f, 0.25f);
    SkelAnime_Update(&this->skelAnime);
    this->beamRoll += -0.3f;

    if (this->timers[1] != 0) {
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, this->rotateSpeed);
        if ((player->stateFlags1 & PLAYER_STATE1_SHIELDING) &&
            ((s16)((player->actor.shape.rot.y - this->actor.shape.rot.y) + 0x8000) < 0x2000) &&
            ((s16)((player->actor.shape.rot.y - this->actor.shape.rot.y) + 0x8000) > -0x2000)) {
            Math_ApproachF(&this->targetPos.x, player->bodyPartsPos[15].x, 1.0f, 400.0f);
            Math_ApproachF(&this->targetPos.y, player->bodyPartsPos[15].y, 1.0f, 400.0f);
            Math_ApproachF(&this->targetPos.z, player->bodyPartsPos[15].z, 1.0f, 400.0f);
        } else {
            Math_ApproachF(&this->targetPos.x, player->actor.world.pos.x, 1.0f, 400.0f);
            Math_ApproachF(&this->targetPos.y, player->actor.world.pos.y + 30.0f, 1.0f, 400.0f);
            Math_ApproachF(&this->targetPos.z, player->actor.world.pos.z, 1.0f, 400.0f);
        }

        this->timers[0] = 70;
        this->groundBlastPos.x = this->groundBlastPos.y = this->groundBlastPos.z = 0.0f;
        this->portalRotation += this->updateRate2 * 0.0025f;
        Math_ApproachF(&this->spawnPortalAlpha, 255.0f, 1.0f, 10.0f);
        Math_ApproachF(&this->updateRate2, 50.0f, 1.0f, 2.0f);

        if (this->timers[1] < 50) {
            if (this->timers[1] < 10) {
                if (this->timers[1] == 9) {
                    play->envCtx.unk_D8 = 0.5f;
                    play->envCtx.unk_BD = 3 - this->actor.params;
                    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
                }

                if (this->timers[1] == 5) {
                    this->scepterAlpha = 255;
                }

                if (this->timers[1] > 4) {
                    s16 j;
                    for (j = 0; j < 2; j++) {
                        for (i = 0; i < ARRAY_COUNT(this->scepterFlamePos); i++) {
                            Vec3f pos;
                            Vec3f velocity;
                            Vec3f accel;

                            pos.x = this->scepterFlamePos[i].x;
                            pos.y = this->scepterFlamePos[i].y;
                            pos.z = this->scepterFlamePos[i].z;
                            velocity.x = Rand_CenteredFloat(10.0f);
                            velocity.y = Rand_CenteredFloat(10.0f);
                            velocity.z = Rand_CenteredFloat(10.0f);
                            accel.x = 0.0f;
                            accel.y = 0.0f;
                            accel.z = 0.0f;
                            BossTw_AddFlameEffect(play, &pos, &velocity, &accel, Rand_ZeroFloat(10.0f) + 25.0f,
                                                  this->actor.params);
                        }
                    }
                }
            }

            if (this->timers[1] < 20) {
                Math_ApproachF(&this->flameAlpha, 0, 1.0f, 20.0f);
                Math_ApproachF(&this->spawnPortalAlpha, 0, 1.0f, 30.0f);
            } else {
                Math_ApproachF(&this->flameAlpha, 255.0f, 1.0f, 10.0f);
                if (this->actor.params == 1) {
                    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MS_FIRE - SFX_FLAG);
                } else {
                    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MS_FREEZE - SFX_FLAG);
                }
            }

            this->flameRotation += this->updateRate1 * 0.0025f;
            Math_ApproachF(&this->spawnPortalScale, 0.0f, 0.1f, this->updateRate1);
            Math_ApproachF(&this->updateRate1, 50.0f, 1.0f, 2.0f);
        }

        if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
            Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeChargeUpAttackLoopAnim, 0.0f);
            this->workf[ANIM_SW_TGT] = 10000.0f;
        }

        if (this->timers[1] == 1) {
            Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaKotakeKoumeAttackStartAnim, 0.0f);
            this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeAttackStartAnim);
            this->unk_4DC = 0.0f;
            this->spawnPortalAlpha = 0.0f;
            this->flameAlpha = 0.0f;
            sBeamDivertTimer = 0;
        }
    } else {
        if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
            Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeAttackLoopAnim, 0.0f);
            this->workf[ANIM_SW_TGT] = 10000.0f;
        }

        if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT] - 5.0f)) {
            this->beamShootState = 0;
            sEnvType = this->actor.params + 1;
        }

        if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT] - 13.0f)) {
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_THROW_MASIC);
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_VOICE);
        }

        xDiff = this->targetPos.x - this->beamOrigin.x;
        yDiff = this->targetPos.y - this->beamOrigin.y;
        zDiff = this->targetPos.z - this->beamOrigin.z;

        this->beamYaw = Math_FAtan2F(xDiff, zDiff);
        this->beamPitch = -Math_FAtan2F(yDiff, sqrtf(SQ(xDiff) + SQ(zDiff)));

        switch (this->beamShootState) {
            case -1:
                break;
            case 0:
                if (this->timers[0] != 0) {
                    s32 beamReflection =
                        suppressPlayerInteraction ? 0 : BossTw_CheckBeamReflection(this, play);

                    if (beamReflection == 1) {
                        Vec3f pos;
                        Vec3f velocity;
                        Vec3f accel = { 0.0f, 0.0f, 0.0f };

                        for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
                            velocity.x = Rand_CenteredFloat(15.0f);
                            velocity.y = Rand_CenteredFloat(15.0f);
                            velocity.z = Rand_CenteredFloat(15.0f);
                            pos = player->bodyPartsPos[15];
                            BossTw_AddDotEffect(play, &pos, &velocity, &accel, (s16)Rand_ZeroFloat(2.0f) + 5,
                                                this->actor.params, 150);
                        }

                        this->beamShootState = 1;
                        Sfx_PlaySfxAtPos(&player->actor.projectedPos, NA_SE_IT_SHIELD_REFLECT_MG);
                        Matrix_MtxFToYXZRotS(&player->shieldMf, &sp128, 0);
                        sp128.y += 0x8000;
                        sp128.x = -sp128.x;
                        this->magicDir.x = sp128.x;
                        this->magicDir.y = sp128.y;
                        this->groundBlastPos.x = 0.0f;
                        this->groundBlastPos.y = 0.0f;
                        this->groundBlastPos.z = 0.0f;
                        play->envCtx.unk_D8 = 1.0f;
                        Rumble_Request(0.0f, 0x64, 5, 4);
                    } else if (beamReflection == 0) {
                        if (!suppressPlayerInteraction) {
                            BossTw_BeamHitPlayerCheck(this, play);
                        }

                        if (this->csState1 == 0) {
                            Math_ApproachF(&this->beamDist, 2.0f * sqrtf(SQ(xDiff) + SQ(yDiff) + SQ(zDiff)), 1.0f,
                                           40.0f);
                        }
                    }
                }

                SkinMatrix_Vec3fMtxFMultXYZW(&play->viewProjectionMtxF, &this->beamReflectionOrigin, &this->unk_54C,
                                             &this->actor.projectedW);

                if (this->actor.params == 1) {
                    Audio_PlaySoundGeneral(NA_SE_EN_TWINROBA_SHOOT_FIRE - SFX_FLAG, &this->unk_54C, 4,
                                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                           &gSfxDefaultReverb);
                } else {
                    Audio_PlaySoundGeneral(NA_SE_EN_TWINROBA_SHOOT_FREEZE - SFX_FLAG, &this->unk_54C, 4,
                                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                           &gSfxDefaultReverb);
                }
                break;

            case 1:
                if (CHECK_BTN_ALL(input->cur.button, BTN_R)) {
                    Player* player = GET_PLAYER(play);

                    this->beamDist = sqrtf(SQ(xDiff) + SQ(yDiff) + SQ(zDiff));
                    Math_ApproachF(&this->beamReflectionDist, 2000.0f, 1.0f, 40.0f);
                    Math_ApproachF(&this->targetPos.x, player->bodyPartsPos[15].x, 1.0f, 400.0f);
                    Math_ApproachF(&this->targetPos.y, player->bodyPartsPos[15].y, 1.0f, 400.0f);
                    Math_ApproachF(&this->targetPos.z, player->bodyPartsPos[15].z, 1.0f, 400.0f);
                    if ((this->work[CS_TIMER_1] % 4) == 0) {
                        BossTw_AddRingEffect(play, &player->bodyPartsPos[15], 0.5f, 3.0f, 0xFF, this->actor.params, 1,
                                             150);
                    }
                } else {
                    this->beamShootState = 0;
                    this->beamReflectionDist = 0.0f;
                }

                SkinMatrix_Vec3fMtxFMultXYZW(&play->viewProjectionMtxF, &this->unk_530, &this->unk_558,
                                             &this->actor.projectedW);

                if (this->actor.params == 1) {
                    Audio_PlaySoundGeneral(NA_SE_EN_TWINROBA_SHOOT_FIRE - SFX_FLAG, &this->unk_558, 4U,
                                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                           &gSfxDefaultReverb);
                    Audio_PlaySoundGeneral(NA_SE_EN_TWINROBA_REFL_FIRE - SFX_FLAG, &this->unk_558, 4,
                                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                           &gSfxDefaultReverb);
                } else {
                    Audio_PlaySoundGeneral(NA_SE_EN_TWINROBA_SHOOT_FREEZE - SFX_FLAG, &this->unk_558, 4,
                                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                           &gSfxDefaultReverb);
                    Audio_PlaySoundGeneral(NA_SE_EN_TWINROBA_REFL_FREEZE - SFX_FLAG, &this->unk_558, 4,
                                           &gSfxDefaultFreqAndVolScale, &gSfxDefaultFreqAndVolScale,
                                           &gSfxDefaultReverb);
                }
                break;
        }

        if (this->timers[0] == 0 && (sEnvType == 1 || sEnvType == 2)) {
            sEnvType = 0;
        }

        if (this->timers[0] == 0) {
            Math_ApproachF(&this->beamScale, 0.0f, 1.0f, 0.0005f);

            if (this->beamScale == 0.0f) {
                BossTw_SetupFinishBeamShoot(this, play);
                this->beamReflectionDist = 0.0f;
                this->beamDist = 0.0f;
            }
        }
    }

    Matrix_Translate(this->beamOrigin.x, this->beamOrigin.y, this->beamOrigin.z, MTXMODE_NEW);
    Matrix_RotateY(this->beamYaw, MTXMODE_APPLY);
    Matrix_RotateX(this->beamPitch, MTXMODE_APPLY);

    sp130.x = 0.0f;
    sp130.y = 0.0f;
    sp130.z = this->beamDist + -5.0f;

    Matrix_MultVec3f(&sp130, &this->beamReflectionOrigin);

    if ((this->csState1 == 0) && (this->beamShootState == 0) && (this->timers[0] != 0)) {
        this->groundBlastPos.y = BossTw_GetFloorY(&this->beamReflectionOrigin);

        if (this->groundBlastPos.y >= 0.0f) {
            this->csState1 = 1;
            this->groundBlastPos.x = this->beamReflectionOrigin.x;
            this->groundBlastPos.z = this->beamReflectionOrigin.z;
            BossTw_SpawnGroundBlast(this, play, this->actor.params);
            this->timers[0] = 20;
        }
    }

    if (this->beamShootState == 1) {
        if (this->csState1 == 0) {
            Matrix_MtxFToYXZRotS(&player->shieldMf, &sp128, 0);
            sp128.y += 0x8000;
            sp128.x = -sp128.x;
            Math_ApproachS(&this->magicDir.x, sp128.x, 5, 0x2000);
            Math_ApproachS(&this->magicDir.y, sp128.y, 5, 0x2000);
            this->beamReflectionPitch = (this->magicDir.x / 32768.0f) * M_PI;
            this->beamReflectionYaw = (this->magicDir.y / 32768.0f) * M_PI;
        }

        Matrix_Translate(this->beamReflectionOrigin.x, this->beamReflectionOrigin.y, this->beamReflectionOrigin.z,
                         MTXMODE_NEW);
        Matrix_RotateY(this->beamReflectionYaw, MTXMODE_APPLY);
        Matrix_RotateX(this->beamReflectionPitch, MTXMODE_APPLY);

        sp130.x = 0.0f;
        sp130.y = 0.0f;
        sp130.z = this->beamReflectionDist + -170.0f;

        Matrix_MultVec3f(&sp130, &this->unk_530);

        if (this->csState1 == 0) {
            sp130.z = 0.0f;

            for (i = 0; i < 200; i++) {
                Vec3f spBC;

                Matrix_MultVec3f(&sp130, &spBC);
                floorY = BossTw_GetFloorY(&spBC);
                this->groundBlastPos.y = floorY;

                if (floorY >= 0.0f) {
                    if ((this->groundBlastPos.y != 35.0f) && (0.0f < this->beamReflectionPitch) &&
                        (this->timers[0] != 0)) {
                        this->csState1 = 1;
                        this->groundBlastPos.x = spBC.x;
                        this->groundBlastPos.z = spBC.z;
                        BossTw_SpawnGroundBlast(this, play, this->actor.params);
                        this->timers[0] = 20;
                    } else {
                        for (i = 0; i < 5; i++) {
                            Vec3f velocity;
                            Vec3f accel;

                            velocity.x = Rand_CenteredFloat(20.0f);
                            velocity.y = Rand_CenteredFloat(20.0f);
                            velocity.z = Rand_CenteredFloat(20.0f);

                            accel.x = 0.0f;
                            accel.y = 0.0f;
                            accel.z = 0.0f;

                            BossTw_AddFlameEffect(play, &this->unk_530, &velocity, &accel,
                                                  Rand_ZeroFloat(10.0f) + 25.0f, this->actor.params);
                        }

                        this->beamReflectionDist = sp130.z;
                        Math_ApproachF(&play->envCtx.unk_D8, 0.8f, 1.0f, 0.2f);
                    }
                    break;
                }

                sp130.z += 20.0f;

                if (this->beamReflectionDist < sp130.z) {
                    break;
                }
            }
        }

        if (BossTw_BeamReflHitCheck(this, &this->actor.world.pos) && (this->work[CS_TIMER_1] % 4) == 0) {
            BossTw_AddRingEffect(play, &this->unk_530, 0.5f, 3.0f, 255, this->actor.params, 1, 150);
        }

        if (BossTw_BeamReflHitCheck(this, &otherTw->actor.world.pos)) {
            BossTw_TryHitSisterWithMagic(otherTw, play,
                                         this->actor.params == TW_KOUME ? TWINROVA_MAGIC_FIRE : TWINROVA_MAGIC_ICE);
        }
    }
}

void BossTw_SetupFinishBeamShoot(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_FinishBeamShoot;
    Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaKotakeKoumeAttackEndAnim, 0.0f);
    if (BossTw_IsSisterActor(&this->actor)) {
        this->skelAnime.playSpeed = TWINROVA_PHASE_ONE_POST_ATTACK_SPEED;
    }
    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeAttackEndAnim);
}

static s32 BossTw_TryStartPhaseOneCombo(BossTw* this, PlayState* play) {
    BossTw* otherTw = (BossTw*)this->actor.parent;
    s32 canUseThisSister = BossTw_IsSisterActor(&this->actor) && !BossTw_IsPhaseOneHitGoalReached();
    s32 otherIsReady = otherTw != NULL &&
                       (BossTw_IsNormalPhaseOneMovement(otherTw) ||
                        (this == sKoumePtr && otherTw->actionFunc == BossTw_FinishBeamShoot));
    s32 otherCompletingSameMove = otherTw != NULL &&
                                  (otherTw->actionFunc == BossTw_BlastVolley ||
                                   otherTw->actionFunc == BossTw_FinishBeamShoot);
    s32 thisIsVisible;
    s32 otherIsVisible;
    s32 canVolley;
    s32 canBeam;

    if (!canUseThisSister || sPhaseOneComboState == TWINROVA_PHASE_ONE_COMBO_IDLE) {
        return false;
    }

    if (sPhaseOneComboState == TWINROVA_PHASE_ONE_COMBO_ARMED) {
        if (!otherIsReady) {
            if (!otherCompletingSameMove) {
                // A damage reaction or unrelated state broke the intended link. Never carry this budget into a later
                // neutral attack; only the second half of the same coordinated volley may finish the roll.
                sPhaseOneComboState = TWINROVA_PHASE_ONE_COMBO_IDLE;
            }
            return false;
        }
        if (Rand_ZeroOne() >= BossTw_GetPhaseOneComboChance()) {
            sPhaseOneComboState = TWINROVA_PHASE_ONE_COMBO_IDLE;
            return false;
        }

        thisIsVisible = BossTw_IsActorInCameraFrustumWithMargin(play, &this->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN);
        otherIsVisible =
            BossTw_IsActorInCameraFrustumWithMargin(play, &otherTw->actor, TWINROVA_ATTACK_FRUSTUM_MARGIN);
        canVolley = thisIsVisible && BossTw_GetProjectedSummonPressure(play) <= TWINROVA_BEAM_SUMMON_LIMIT;
        canBeam = canVolley && otherIsVisible && !BossTw_HasActivePhaseOneMagic(play);

        if (!canVolley) {
            // Spend an unsafe combo roll on the authored reset instead of releasing from off-camera or through an
            // already-saturated add field.
            sPhaseOneComboState = TWINROVA_PHASE_ONE_COMBO_IDLE;
            BossTw_SetupTurnToPlayer(this, play);
            this->timers[0] = TWINROVA_PHASE_ONE_COMBO_RESET_TIME;
            if (BossTw_IsNormalPhaseOneMovement(otherTw) || otherTw->actionFunc == BossTw_FinishBeamShoot) {
                BossTw_SetupTurnToPlayer(otherTw, play);
                otherTw->timers[0] = TWINROVA_PHASE_ONE_COMBO_RESET_TIME;
            }
            return true;
        }

        // A combo is exactly one additional normal move. Record it for repetition control without rearming the budget.
        sPhaseOneComboState = TWINROVA_PHASE_ONE_COMBO_CONSUMED;
        if (!BossTw_IsNormalPhaseOneMovement(otherTw)) {
            BossTw_SetupTurnToPlayer(otherTw, play);
            otherTw->actor.speedXZ = 0.0f;
            otherTw->work[CAN_SHOOT] = false;
        }
        if (canBeam && Rand_ZeroOne() < 0.5f) {
            this->work[CAN_SHOOT] = false;
            BossTw_SetupShootBeam(this, play);
            BossTw_RecordPhaseOneRegularAttack(TWINROVA_PHASE_ONE_REGULAR_BEAM);
        } else {
            this->work[CAN_SHOOT] = false;
            BossTw_SetupBlastVolley(this, play);
            BossTw_RecordPhaseOneRegularAttack(TWINROVA_PHASE_ONE_REGULAR_VOLLEY);
        }
        return true;
    }

    // The linked move has completed. Both sisters take one short, visible reset before regular scheduling resumes.
    sPhaseOneComboState = TWINROVA_PHASE_ONE_COMBO_IDLE;
    BossTw_SetupTurnToPlayer(this, play);
    this->timers[0] = TWINROVA_PHASE_ONE_COMBO_RESET_TIME;
    if (BossTw_IsNormalPhaseOneMovement(otherTw)) {
        BossTw_SetupTurnToPlayer(otherTw, play);
        otherTw->timers[0] = TWINROVA_PHASE_ONE_COMBO_RESET_TIME;
    }
    return true;
}

void BossTw_FinishBeamShoot(BossTw* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_ApproachF(&this->scepterAlpha, 0.0f, 1.0f, 10.0f);

    if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
        if (BossTw_TryStartPhaseOneCombo(this, play)) {
            return;
        }
        if (sTwinrovaPtr->timers[2] == 0) {
            BossTw_SetupFlyTo(this, play);
        } else {
            BossTw_SetupLaugh(this, play);
        }

        this->scepterAlpha = 0.0f;
    }
}

void BossTw_SetupHitByBeam(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_HitByBeam;
    Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaKotakeKoumeDamageStartAnim, 0.0f);
    this->timers[0] = 53;
    this->actor.speedXZ = 0.0f;

    if (this->actor.params == 0) {
        this->work[FOG_TIMER] = 20;
    }
}

void BossTw_HitByBeam(BossTw* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if ((this->work[CS_TIMER_1] % 4) == 0) {
        Vec3f pos;
        Vec3f velocity;
        Vec3f accel;

        pos.x = this->actor.world.pos.x + Rand_CenteredFloat(80.0f);
        pos.y = this->actor.world.pos.y + Rand_CenteredFloat(80.0f);
        pos.z = this->actor.world.pos.z + Rand_CenteredFloat(80.0f);

        velocity.x = 0.0f;
        velocity.y = 0.0f;
        velocity.z = 0.0f;

        accel.x = 0.0f;
        accel.y = 0.1f;
        accel.z = 0.0f;

        BossTw_AddDmgCloud(play, this->actor.params + 2, &pos, &velocity, &accel, Rand_ZeroFloat(10.0f) + 15.0f, 0, 0,
                           150);
    }

    if (this->actor.params == 1) {
        Math_ApproachF(&this->fogR, 255.0f, 1.0f, 30.0f);
        Math_ApproachF(&this->fogG, 255.0f, 1.0f, 30.0f);
        Math_ApproachF(&this->fogB, 255.0f, 1.0f, 30.0f);
        Math_ApproachF(&this->fogNear, 900.0f, 1.0f, 30.0f);
        Math_ApproachF(&this->fogFar, 1099.0f, 1.0f, 30.0f);
    }

    Math_ApproachF(&this->actor.world.pos.y, ((Math_SinS(this->work[CS_TIMER_1] * 1500) * 20.0f) + 350.0f) + 50.0f,
                   0.1f, this->actor.speedXZ);
    Math_ApproachF(&this->actor.speedXZ, 5.0f, 1.0f, 1.0f);

    this->actor.world.pos.y -= 50.0f;
    Actor_UpdateBgCheckInfo(play, &this->actor, 50.0f, 50.0f, 100.0f, 4);
    this->actor.world.pos.y += 50.0f;

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        this->actor.speedXZ = 0.0f;
    }

    if (this->timers[0] == 1) {
        Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaKotakeKoumeDamageEndAnim, 0.0f);
        this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeDamageEndAnim);
    }

    if ((this->timers[0] == 0) && Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
        BossTw_SetupFlyTo(this, play);
    }
}

void BossTw_SetupLaugh(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_Laugh;
    Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaKotakeKoumeLaughAnim, 0.0f);
    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeLaughAnim);
    this->actor.speedXZ = 0.0f;
}

void BossTw_Laugh(BossTw* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if (Animation_OnFrame(&this->skelAnime, 10.0f)) {
        if (this->actor.params == TW_KOUME) {
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_LAUGH);
        } else {
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_LAUGH2);
        }
    }

    if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
        BossTw_SetupFlyTo(this, play);
    }
}

void BossTw_SetupSpin(BossTw* this, PlayState* play) {
    TwinrovaMagicElement element =
        this->actor.params == TW_KOUME ? TWINROVA_MAGIC_FIRE : TWINROVA_MAGIC_ICE;

    this->actionFunc = BossTw_Spin;
    Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaKotakeKoumeSpinAnim, -3.0f);
    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeSpinAnim);
    this->actor.speedXZ = 0.0f;
    this->timers[0] = TWINROVA_SPIN_TELL_TIME + TWINROVA_SPIN_ACTIVE_TIME;
    // This reactive counter is a distinct authored beat, so regular beam/volley repetition begins fresh afterward.
    BossTw_ResetPhaseOneRegularHistory();
    BossTw_AddRingEffect(play, &this->actor.world.pos, 0.35f, 3.0f, 255, element, 1, ARRAY_COUNT(sEffects));
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_ROLL);
}

void BossTw_Spin(BossTw* this, PlayState* play) {
    BossTw_ShouldSuppressSisterAttackCollision(this, play);

    if (this->timers[0] > TWINROVA_SPIN_ACTIVE_TIME) {
        // The animation and ring warn Link before the enlarged damaging contact becomes active.
        SkelAnime_Update(&this->skelAnime);
    } else if (this->timers[0] != 0) {
        this->collider.base.colType = COLTYPE_METAL;
        this->actor.shape.rot.y -= 0x3000;

        if ((this->timers[0] % 4) == 0) {
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_ROLL);
        }
    } else {
        SkelAnime_Update(&this->skelAnime);
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.world.rot.y, 3, 0x2000);

        if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
            // The counter should punish one greedy melee commitment, not chain forever while Link is recovering.
            // Timer 2 is otherwise unused by the sisters and therefore cannot delay beams or projectile tells.
            this->timers[2] = TWINROVA_SISTER_SPIN_RETRIGGER_COOLDOWN;
            BossTw_SetupFlyTo(this, play);
        }
    }
}

static void BossTw_SetupPillarDive(BossTw* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    TwinrovaMagicElement element =
        this->actor.params == TW_KOUME ? TWINROVA_MAGIC_FIRE : TWINROVA_MAGIC_ICE;

    this->actionFunc = BossTw_PillarDive;
    this->csState1 = 0;
    this->timers[0] = TWINROVA_PILLAR_DIVE_TIME;
    this->timers[2] = TWINROVA_SISTER_SPIN_RETRIGGER_COOLDOWN;
    this->groundBlastPos = this->actor.world.pos;
    this->targetPos = player->actor.world.pos;
    this->targetPos.y += TWINROVA_PILLAR_DIVE_LOW_HOVER_HEIGHT;
    this->actor.speedXZ = 0.0f;
    this->sisterUnderPlayerTimer = 0;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeSpinAnim, -3.0f);
    BossTw_AddRingEffect(play, &this->actor.world.pos, 0.35f, 3.0f, 255, element, 1, ARRAY_COUNT(sEffects));
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_ROLL);
}

void BossTw_PillarDive(BossTw* this, PlayState* play) {
    f32 xDiff = this->targetPos.x - this->actor.world.pos.x;
    f32 yDiff = this->targetPos.y - this->actor.world.pos.y;
    f32 zDiff = this->targetPos.z - this->actor.world.pos.z;
    f32 xzDist = sqrtf(SQ(xDiff) + SQ(zDiff));
    s16 yawTarget = Math_FAtan2F(xDiff, zDiff) * (32768.0f / M_PI);

    BossTw_ShouldSuppressSisterAttackCollision(this, play);
    SkelAnime_Update(&this->skelAnime);
    Math_ApproachS(&this->actor.world.rot.y, yawTarget, 5, 0x1800);
    Math_ApproachS(&this->actor.shape.rot.y, yawTarget, 5, 0x1800);

    if (this->csState1 == 0) {
        Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.5f, 40.0f);
        Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.5f, 35.0f);
        Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.5f, 40.0f);

        if (xzDist < 45.0f && fabsf(yDiff) < 25.0f) {
            f32 passX = this->targetPos.x - this->groundBlastPos.x;
            f32 passZ = this->targetPos.z - this->groundBlastPos.z;
            f32 passDist = sqrtf(SQ(passX) + SQ(passZ));

            if (passDist < 1.0f) {
                passX = this->actor.params == TW_KOUME ? 1.0f : -1.0f;
                passZ = 0.0f;
                passDist = 1.0f;
            }
            this->targetPos.x += (passX / passDist) * TWINROVA_PILLAR_DIVE_PASS_DISTANCE;
            this->targetPos.z += (passZ / passDist) * TWINROVA_PILLAR_DIVE_PASS_DISTANCE;
            this->csState1 = 1;
            this->timers[0] = TWINROVA_PILLAR_DIVE_SPIN_TIME;
        }
    } else {
        this->collider.base.colType = COLTYPE_METAL;
        Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.45f, 32.0f);
        Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.45f, 32.0f);
        this->actor.shape.rot.y -= 0x3000;

        if ((this->timers[0] % 4) == 0) {
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_ROLL);
        }
        if (this->timers[0] == 0) {
            BossTw_SelectSeparatedPhaseOnePillar(this, 120.0f);
            BossTw_SetupPillarDiveRetreatVolley(this, play);
        }
    }
}

void BossTw_SetupMergeCS(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_MergeCS;
    this->rotateSpeed = 0.0f;
    this->actor.speedXZ = 0.0f;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, -10.0f);
}

void BossTw_MergeCS(BossTw* this, PlayState* play) {
    if (sTwinrovaPtr != NULL && sTwinrovaPtr->actionFunc == BossTw_TwinrovaMergeCS &&
        sTwinrovaPtr->csState2 == 0) {
        return;
    }

    Math_ApproachF(&this->scepterAlpha, 0.0f, 1.0f, 10.0f);
    SkelAnime_Update(&this->skelAnime);
}

void BossTw_SetupWait(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_Wait;
    this->visible = false;
    this->actor.world.pos.y = -2000.0f;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
}

static void BossTw_PrepareSisterForMerge(BossTw* sister) {
    if (sister == NULL) {
        return;
    }

    sister->visible = true;
    sister->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
    Actor_SetScale(&sister->actor, 0.025f);
    sister->actor.velocity.x = sister->actor.velocity.y = sister->actor.velocity.z = 0.0f;
    sister->actor.speedXZ = 0.0f;
    sister->actor.world.rot.x = sister->actor.shape.rot.x = 0;
    sister->work[INVINC_TIMER] = 0;
    sister->work[CAN_SHOOT] = false;
    sister->beamShootState = 0;
    sister->beamScale = 0.0f;
    sister->beamDist = 0.0f;
    sister->beamReflectionDist = 0.0f;
    sister->scepterAlpha = 255.0f;
    sister->flameAlpha = 0.0f;
    sister->spawnPortalAlpha = 0.0f;
    sister->workf[UNK_F17] = 0.0f;
    sister->workf[UNK_F18] = 0.0f;
}

void BossTw_Wait(BossTw* this, PlayState* play) {
    if (this->actor.params == TW_TWINROVA && sKoumePtr != NULL && sKotakePtr != NULL &&
        BossTw_IsPhaseOneHitGoalReached() && sKoumePtr->actionFunc != BossTw_HitByBeam &&
        sKotakePtr->actionFunc != BossTw_HitByBeam) {
        BossTw_TwinrovaSetupMergeCS(this, play);
        BossTw_SetupMergeCS(sKotakePtr, play);
        BossTw_SetupMergeCS(sKoumePtr, play);
    }
}

s32 BossTw_SkipToSecondPhase(PlayState* play) {
    if (play == NULL || play->csCtx.state != CS_STATE_IDLE || Play_GetActiveCamId(play) != CAM_ID_MAIN ||
        play->pauseCtx.state != 0 || play->pauseCtx.debugState != 0 ||
        play->gameOverCtx.state != GAMEOVER_INACTIVE || play->transitionTrigger != TRANS_TRIGGER_OFF ||
        sTwinrovaPtr == NULL || sKotakePtr == NULL || sKoumePtr == NULL || sTwinrovaPtr->actor.update == NULL ||
        sKotakePtr->actor.update == NULL || sKoumePtr->actor.update == NULL ||
        sTwinrovaPtr->actor.params != TW_TWINROVA || sTwinrovaPtr->actionFunc != BossTw_Wait) {
        return false;
    }

    BossTw_TwinrovaSetupMergeCS(sTwinrovaPtr, play);
    BossTw_SetupMergeCS(sKotakePtr, play);
    BossTw_SetupMergeCS(sKoumePtr, play);
    return true;
}

s32 BossTw_ShouldUseNormalUpdateRate(Actor* actor) {
    BossTw* this = (BossTw*)actor;

    return actor != NULL && actor->id == ACTOR_BOSS_TW &&
           (this->actionFunc == BossTw_TwinrovaWaitForSisterAllocation ||
            this->actionFunc == BossTw_WaitForDefeatRewardAllocation || this->actionFunc == BossTw_Wait ||
            // Keep the shared scheduler boundary at one update so a deferred sister reaches the other actor's base
            // update before Hyper Bosses can consume its two-tick handoff inside the same gameplay frame.
            (this->actionFunc == BossTw_TurnToPlayer && this->timers[0] <= 2) ||
            this->actionFunc == BossTw_Spin || this->actionFunc == BossTw_PillarDive ||
            this->actionFunc == BossTw_CrossfireVolley || this->actionFunc == BossTw_ShootBeam ||
            this->actionFunc == BossTw_FinishBeamShoot || this->actionFunc == BossTw_BlastVolley ||
            this->actionFunc == BossTw_SpiralBarrage || this->actionFunc == BossTw_PortalReposition ||
             this->actionFunc == BossTw_TwinrovaChargeBlast || this->actionFunc == BossTw_TwinrovaShootBlast ||
             this->actionFunc == BossTw_TwinrovaCyclone || this->actionFunc == BossTw_TwinrovaFalseCharge ||
             this->actionFunc == BossTw_TwinrovaMinefield ||
             this->actionFunc == BossTw_TwinrovaMinefieldFollowup ||
             this->actionFunc == BossTw_TwinrovaSpin ||
            this->actionFunc == BossTw_TwinrovaBreakerSequence ||
            this->actionFunc == BossTw_TwinrovaStun || this->actionFunc == BossTw_TwinrovaGetUp ||
            this->actionFunc == BossTw_TwinrovaDoneBlastShoot);
}

void BossTw_TwinrovaSetupMergeCS(BossTw* this, PlayState* play) {
    BossTw_ClearSummonedEnemies(play);
    BossTw_ClearPhaseOneMagic(play, true);
    BossTw_PrepareSisterForMerge(sKotakePtr);
    BossTw_PrepareSisterForMerge(sKoumePtr);
    this->actionFunc = BossTw_TwinrovaMergeCS;
    this->csState2 = 0;
    this->csState1 = 0;
    BossTw_ResetShieldCharge();
    D_8094C870 = 0;
    sEnvType = 0;
}

void BossTw_TwinrovaMergeCS(BossTw* this, PlayState* play) {
    s16 i;
    Vec3f spB0;
    Vec3f spA4;
    Player* player = GET_PLAYER(play);

    switch (this->csState2) {
        case 0:
            if (!BossTw_TryAcquireCutsceneCamera(this, play, 0x39)) {
                // Hold the harmless merge pose until the camera system can honor the cinematic.
                return;
            }
            this->csState2 = 1;
            BossTw_PrepareSisterForMerge(sKotakePtr);
            BossTw_PrepareSisterForMerge(sKoumePtr);
            BossTw_SetupMergeCS(sKotakePtr, play);
            BossTw_SetupMergeCS(sKoumePtr, play);
            this->subCamDist = 800.0f;
            this->subCamYaw = M_PI;
            sKoumePtr->actor.world.rot.x = 0;
            sKoumePtr->actor.shape.rot.x = 0;
            sKotakePtr->actor.world.rot.x = 0;
            sKotakePtr->actor.shape.rot.x = 0;
            this->workf[UNK_F9] = 0.0f;
            this->workf[UNK_F10] = 0.0f;
            this->workf[UNK_F11] = 600.0f;
            Audio_QueueSeqCmd(0x1 << 28 | SEQ_PLAYER_BGM_MAIN << 24 | 0xC800FF);
            this->work[CS_TIMER_2] = 0;
            // fallthrough
        case 1:
            if (this->work[CS_TIMER_2] == 20) {
                Message_StartTextbox(play, 0x6059, NULL);
            }

            if (this->work[CS_TIMER_2] == 80) {
                Message_StartTextbox(play, 0x605A, NULL);
            }

            this->subCamAt.x = 0.0f;
            this->subCamAt.y = 440.0f;
            this->subCamAt.z = 0.0f;

            spB0.x = 0.0f;
            spB0.y = 0.0f;
            spB0.z = this->subCamDist;

            Matrix_RotateY(this->subCamYaw, MTXMODE_NEW);
            Matrix_MultVec3f(&spB0, &spA4);

            this->subCamEye.x = spA4.x;
            this->subCamEye.y = 300.0f;
            this->subCamEye.z = spA4.z;

            Math_ApproachF(&this->subCamYaw, 0.3f, 0.02f, 0.03f);
            Math_ApproachF(&this->subCamDist, 200.0f, 0.1f, 5.0f);
            break;

        case 2:
            spB0.x = 0.0f;
            spB0.y = 0.0f;
            spB0.z = this->subCamDist;
            Matrix_RotateY(this->subCamYaw, MTXMODE_NEW);
            Matrix_MultVec3f(&spB0, &spA4);
            this->subCamEye.x = spA4.x;
            this->subCamEye.z = spA4.z;
            Math_ApproachF(&this->subCamEye.y, 420.0f, 0.1f, this->subCamUpdateRate * 20.0f);
            Math_ApproachF(&this->subCamAt.y, 470.0f, 0.1f, this->subCamUpdateRate * 6.0f);
            Math_ApproachF(&this->subCamYaw, 0.3f, 0.02f, 0.03f);
            Math_ApproachF(&this->subCamDist, 60.0f, 0.1f, this->subCamUpdateRate * 32.0f);
            Math_ApproachF(&this->subCamUpdateRate, 1, 1, 0.1f);
            break;
    }

    if (this->subCamId != 0) {
        if (this->unk_5F9 == 0) {
            Play_CameraSetAtEye(play, this->subCamId, &this->subCamAt, &this->subCamEye);
        } else {
            Play_CameraSetAtEye(play, this->subCamId, &this->subCamAt2, &this->subCamEye2);
        }
    }

    switch (this->csState1) {
        case 0:
            Audio_PlayActorSound2(&sKotakePtr->actor, NA_SE_EN_TWINROBA_FLY - SFX_FLAG);
            Audio_PlayActorSound2(&sKoumePtr->actor, NA_SE_EN_TWINROBA_FLY - SFX_FLAG);
            spB0.x = this->workf[UNK_F11];
            spB0.y = 400.0f;
            spB0.z = 0.0f;
            Matrix_RotateY(this->workf[UNK_F9], MTXMODE_NEW);
            Matrix_MultVec3f(&spB0, &spA4);
            sKoumePtr->actor.world.pos.x = spA4.x;
            sKoumePtr->actor.world.pos.y = spA4.y;
            sKoumePtr->actor.world.pos.z = spA4.z;
            sKoumePtr->actor.shape.rot.y = (this->workf[UNK_F9] / M_PI) * 32768.0f;
            sKotakePtr->actor.world.pos.x = -spA4.x;
            sKotakePtr->actor.world.pos.y = spA4.y;
            sKotakePtr->actor.world.pos.z = -spA4.z;
            sKotakePtr->actor.shape.rot.y = ((this->workf[UNK_F9] / M_PI) * 32768.0f) + 32768.0f;
            Math_ApproachF(&this->workf[UNK_F11], 0.0f, 0.1f, 7.0f);
            this->workf[UNK_F9] -= this->workf[UNK_F10];
            Math_ApproachF(&this->workf[UNK_F10], 0.5f, 1, 0.0039999997f);
            if (this->workf[UNK_F11] < 10.0f) {
                if (!this->work[PLAYED_CHRG_SFX]) {
                    Audio_PlayActorSound2(&sKoumePtr->actor, NA_SE_EN_TWINROBA_POWERUP);
                    this->work[PLAYED_CHRG_SFX] = true;
                }

                Math_ApproachF(&sKoumePtr->actor.scale.x, 0.005000001f, 1, 0.0003750001f);

                for (i = 0; i < 4; i++) {
                    Vec3f pos;
                    f32 yOffset;
                    f32 xScale;

                    xScale = sKoumePtr->actor.scale.x * 3000.0f;
                    yOffset = Rand_CenteredFloat(xScale * 2.0f);
                    pos.x = 3000.0f;
                    pos.y = 400.0f + yOffset;
                    pos.z = 0.0f;
                    BossTw_AddMergeFlameEffect(play, &pos, Rand_ZeroFloat(5.0f) + 10.0f,
                                               sqrtf(SQ(xScale) - SQ(yOffset)), Rand_ZeroFloat(1.99f));
                }

                if (sKoumePtr->actor.scale.x <= 0.0051f) {
                    Vec3f pos;
                    Vec3f velocity;
                    Vec3f accel;

                    this->actor.world.pos.y = 400.0f;

                    for (i = 0; i < 50; i++) {
                        pos = this->actor.world.pos;
                        velocity.x = Rand_CenteredFloat(20.0f);
                        velocity.y = Rand_CenteredFloat(20.0f);
                        velocity.z = Rand_CenteredFloat(20.0f);
                        pos.x += velocity.x;
                        pos.y += velocity.y;
                        pos.z += velocity.z;
                        accel.z = accel.y = accel.x = 0.0f;
                        BossTw_AddFlameEffect(play, &pos, &velocity, &accel, Rand_ZeroFloat(10.0f) + 25.0f,
                                              velocity.x < 0.0f);
                    }

                    this->csState1 = 1;
                    this->visible = true;
                    this->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
                    this->actor.shape.rot.y = 0;
                    BossTw_SetupWait(sKotakePtr, play);
                    BossTw_SetupWait(sKoumePtr, play);
                    Actor_SetScale(&this->actor, 0.0f);
                    Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaIntroAnim, 0.0f);
                    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaIntroAnim);
                    this->timers[0] = 50;
                    Player_SetCsActionWithHaltedActors(play, &this->actor, 2);
                    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_TRANSFORM);
                    Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_BOSS);
                }
            }

            sKotakePtr->actor.scale.x = sKotakePtr->actor.scale.y = sKotakePtr->actor.scale.z =
                sKoumePtr->actor.scale.y = sKoumePtr->actor.scale.z = sKoumePtr->actor.scale.x;
            break;

        case 1:
            if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
                Animation_MorphToLoop(&this->skelAnime, &gTwinrovaHoverAnim, -15.0f);
            }

            sEnvType = -1;
            play->envCtx.unk_BD = 4;
            Math_ApproachF(&play->envCtx.unk_D8, 1, 1, 0.1f);
            // fallthrough
        case 2:
            SkelAnime_Update(&this->skelAnime);
            Math_ApproachF(&this->actor.scale.x, 0.0069999993f, 1, 0.0006999999f);
            this->actor.scale.y = this->actor.scale.z = this->actor.scale.x;

            if (this->timers[0] == 1) {
                this->csState2 = 2;
                this->subCamUpdateRate = 0.0f;
                this->timers[1] = 65;
                this->timers[2] = 90;
                this->timers[3] = 50;
                player->actor.world.pos.x = 0.0f;
                player->actor.world.pos.y = 240.0f;
                player->actor.world.pos.z = 270.0f;
                player->actor.world.rot.y = player->actor.shape.rot.y = -0x8000;
                this->subCamEye2.x = 0.0f;
                this->subCamEye2.y = 290.0f;
                this->subCamEye2.z = 222.0f;
                this->subCamAt2.x = player->actor.world.pos.x;
                this->subCamAt2.y = player->actor.world.pos.y + 54.0f;
                this->subCamAt2.z = player->actor.world.pos.z;
            }

            if (this->timers[3] == 19) {
                Player_SetCsActionWithHaltedActors(play, &this->actor, 5);
            }

            if (this->timers[3] == 16) {
                Player_PlaySfx(&player->actor, player->ageProperties->unk_92 + NA_SE_VO_LI_SURPRISE);
            }

            if ((this->timers[3] != 0) && (this->timers[3] < 20)) {
                this->unk_5F9 = 1;
                Math_ApproachF(&this->subCamEye2.z, 242.0f, 0.2f, 100.0f);
            } else {
                this->unk_5F9 = 0;
            }

            if (this->timers[1] == 8) {
                this->work[TW_BLINK_IDX] = 8;
                Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_YOUNG_WINK);
            }
            if (this->timers[2] == 4) {
                sEnvType = 0;
                play->envCtx.unk_BE = 5;
            }

            if (this->timers[2] == 1) {
                Camera* cam = Play_GetCamera(play, CAM_ID_MAIN);

                cam->eye = this->subCamEye;
                cam->eyeNext = this->subCamEye;
                cam->at = this->subCamAt;
                func_800C08AC(play, this->subCamId, 0);
                this->subCamId = 0;
                this->csState2 = this->subCamId;
                func_80064534(play, &play->csCtx);
                Player_SetCsActionWithHaltedActors(play, &this->actor, 7);
                this->work[TW_PLLR_IDX] = 0;
                this->targetPos = sTwinrovaPillarPos[0];
                BossTw_TwinrovaSetupFly(this, play);
            }
            break;
    }
}

void BossTw_SetupDeathCS(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_DeathCS;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaKotakeKoumeIdleLoopAnim, -3.0f);
    this->unk_5F8 = 0;
    this->work[CS_TIMER_2] = Rand_ZeroFloat(20.0f);
}

void BossTw_DeathCS(BossTw* this, PlayState* play) {
    if (this->timers[0] == 0) {
        SkelAnime_Update(&this->skelAnime);
    }

    Math_ApproachS(&this->actor.shape.rot.y, this->work[YAW_TGT], 5, this->rotateSpeed);
    Math_ApproachF(&this->rotateSpeed, 20480.0f, 1.0f, 1000.0f);

    if (sTwinrovaPtr->work[CS_TIMER_2] > 140) {
        Math_ApproachF(&this->fogR, 100.0f, 1.0f, 15.0f);
        Math_ApproachF(&this->fogG, 255.0f, 1.0f, 15.0f);
        Math_ApproachF(&this->fogB, 255.0f, 1.0f, 15.0f);
        Math_ApproachF(&this->fogNear, 850.0f, 1.0f, 15.0f);
        Math_ApproachF(&this->fogFar, 1099.0f, 1.0f, 15.0f);
    }
}

void BossTw_SetupCSWait(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_CSWait;
    this->visible = false;
    this->actor.world.pos.y = -2000.0f;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
}

/**
 * Do nothing while waiting for the inital cutscene to start
 */
void BossTw_CSWait(BossTw* this, PlayState* play) {
}

void BossTw_TwinrovaSetupIntroCS(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_TwinrovaIntroCS;
    this->visible = false;
    this->actor.world.pos.y = -2000.0f;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
}

void BossTw_TwinrovaIntroCS(BossTw* this, PlayState* play) {
    u8 updateCam = 0;
    s16 i;
    Vec3f sp90;
    Vec3f sp84;
    Player* player = GET_PLAYER(play);

    if (this->csSfxTimer > 220 && this->csSfxTimer < 630) {
        Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_UNARI - SFX_FLAG);
    }

    if (this->csSfxTimer == 180) {
        Sfx_PlaySfxAtPos(&D_8094A7D0, NA_SE_EN_TWINROBA_LAUGH);
        Sfx_PlaySfxAtPos(&D_8094A7D0, NA_SE_EN_TWINROBA_LAUGH2);
        Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_KOTAKE_KOUME);
    }

    this->csSfxTimer++;

    switch (this->csState2) {
        case 0:
            this->csSfxTimer = 0;

            if (SQ(player->actor.world.pos.x) + SQ(player->actor.world.pos.z) < SQ(150.0f)) {
                if (!BossTw_TryAcquireCutsceneCamera(this, play, 0x39)) {
                    break;
                }
                player->actor.world.pos.x = player->actor.world.pos.z = .0f;
                this->csState2 = 1;
                this->subCamEye.x = 0.0f;
                this->subCamEye.y = 350;
                this->subCamEye.z = 200;

                this->subCamEyeTarget.x = 450;
                this->subCamEyeTarget.y = 900;

                this->subCamAt.x = 0;
                this->subCamAt.y = 270;
                this->subCamAt.z = 0;

                this->subCamAtTarget.x = 0;
                this->subCamAtTarget.y = 240;
                this->subCamAtTarget.z = 140;

                this->subCamEyeTarget.z = 530;
                this->subCamEyeStep.x = fabsf(this->subCamEyeTarget.x - this->subCamEye.x);
                this->subCamEyeStep.y = fabsf(this->subCamEyeTarget.y - this->subCamEye.y);
                this->subCamEyeStep.z = fabsf(this->subCamEyeTarget.z - this->subCamEye.z);
                this->subCamAtStep.x = fabsf(this->subCamAtTarget.x - this->subCamAt.x);
                this->subCamAtStep.y = fabsf(this->subCamAtTarget.y - this->subCamAt.y);
                this->subCamAtStep.z = fabsf(this->subCamAtTarget.z - this->subCamAt.z);

                this->subCamDistStep = 0.05f;
                this->work[CS_TIMER_1] = 0;
            }
            break;

        case 1:
            updateCam = 1;

            if (this->work[CS_TIMER_1] == 30) {
                Message_StartTextbox(play, 0x6048, NULL);
            }

            Math_ApproachF(&this->subCamUpdateRate, 0.01f, 1.0f, 0.0001f);

            if (this->work[CS_TIMER_1] > 100) {
                play->envCtx.unk_BD = 0;
                Math_ApproachF(&play->envCtx.unk_D8, 1.0f, 1.0f, 0.03f);
            }

            if (this->work[CS_TIMER_1] == 180) {
                Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_APPEAR_MS);
            }

            if (this->work[CS_TIMER_1] > 180) {
                this->spawnPortalScale = 0.05f;
                Math_ApproachF(&this->spawnPortalAlpha, 255.0f, 1.0f, 5.f);

                if (this->work[CS_TIMER_1] >= 236) {
                    this->csState2 = 2;
                    sKoumePtr->visible = 1;
                    Animation_MorphToLoop(&sKoumePtr->skelAnime, &gTwinrovaKotakeKoumeIdleLoopAnim, 0.0f);
                    sKoumePtr->actor.world.pos.x = 0.0f;
                    sKoumePtr->actor.world.pos.y = 80.0f;
                    sKoumePtr->actor.world.pos.z = 600.0f;
                    sKoumePtr->actor.shape.rot.y = sKoumePtr->actor.world.rot.y = -0x8000;

                    this->subCamEye.x = -30;
                    this->subCamEye.y = 260;
                    this->subCamEye.z = 470;

                    this->subCamAt.x = 0.0F;
                    this->subCamAt.y = 270;
                    this->subCamAt.z = 600.0F;

                    this->work[CS_TIMER_1] = 0;

                    Actor_SetScale(&sKoumePtr->actor, 0.014999999f);
                }
            }
            break;

        case 2:
            SkelAnime_Update(&sKoumePtr->skelAnime);
            Math_ApproachF(&sKoumePtr->actor.world.pos.y, 240.0f, 0.05f, 5.0f);
            this->subCamEye.x -= 0.2f;
            this->subCamEye.z += 0.2f;

            if (this->work[CS_TIMER_1] > 50) {
                this->csState2 = 3;

                this->subCamEyeTarget.x = -30;
                this->subCamEyeTarget.y = 260;
                this->subCamEyeTarget.z = 530;

                this->subCamAtTarget.x = 0.0f;
                this->subCamAtTarget.y = 265;
                this->subCamAtTarget.z = 580;

                this->subCamEyeStep.x = fabsf(this->subCamEyeTarget.x - this->subCamEye.x);
                this->subCamEyeStep.y = fabsf(this->subCamEyeTarget.y - this->subCamEye.y);
                this->subCamEyeStep.z = fabsf(this->subCamEyeTarget.z - this->subCamEye.z);
                this->subCamAtStep.x = fabsf(this->subCamAtTarget.x - this->subCamAt.x);
                this->subCamAtStep.y = fabsf(this->subCamAtTarget.y - this->subCamAt.y);
                this->subCamAtStep.z = fabsf(this->subCamAtTarget.z - this->subCamAt.z);
                this->subCamUpdateRate = 0;
                this->subCamDistStep = 0.1f;
                this->work[CS_TIMER_1] = 0;
            }
            break;

        case 3:
            SkelAnime_Update(&sKoumePtr->skelAnime);
            updateCam = 1;
            Math_ApproachF(&sKoumePtr->actor.world.pos.y, 240.0f, 0.05f, 5.0f);
            Math_ApproachF(&this->subCamUpdateRate, 1.0f, 1.0f, 0.02f);

            if (this->work[CS_TIMER_1] == 30) {
                Message_StartTextbox(play, 0x6049, NULL);
            }

            if (this->work[CS_TIMER_1] > 80) {
                this->csState2 = 4;
                this->actor.speedXZ = 0;

                this->subCamEyeTarget.x = -80.0f;
                this->subCamEyeTarget.y = 260.0f;
                this->subCamEyeTarget.z = 430.0f;

                this->subCamAtTarget.x = sKoumePtr->actor.world.pos.x;
                this->subCamAtTarget.y = sKoumePtr->actor.world.pos.y + 20.0f;
                this->subCamAtTarget.z = sKoumePtr->actor.world.pos.z;

                this->subCamEyeStep.x = fabsf(this->subCamEyeTarget.x - this->subCamEye.x);
                this->subCamEyeStep.y = fabsf(this->subCamEyeTarget.y - this->subCamEye.y);
                this->subCamEyeStep.z = fabsf(this->subCamEyeTarget.z - this->subCamEye.z);
                this->subCamAtStep.x = fabsf(this->subCamAtTarget.x - this->subCamAt.x);
                this->subCamAtStep.y = fabsf(this->subCamAtTarget.y - this->subCamAt.y);
                this->subCamAtStep.z = fabsf(this->subCamAtTarget.z - this->subCamAt.z);
                this->subCamUpdateRate = 0.0f;
                this->subCamDistStep = 0.05f;
                Animation_MorphToPlayOnce(&sKoumePtr->skelAnime, &gTwinrovaKotakeKoumeIdleEndAnim, 0.0f);
                this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeIdleEndAnim);
                this->work[CS_TIMER_1] = 0;
            }
            break;

        case 4:
            updateCam = 1;
            SkelAnime_Update(&sKoumePtr->skelAnime);
            this->subCamAtTarget.y = 20.0f + sKoumePtr->actor.world.pos.y;
            Math_ApproachF(&sKoumePtr->actor.world.pos.y, 350, 0.1f, this->actor.speedXZ);
            Math_ApproachF(&this->actor.speedXZ, 9.0f, 1.0f, 0.9f);
            Math_ApproachF(&this->subCamUpdateRate, 1.0f, 1.0f, 0.02f);

            if (this->work[CS_TIMER_1] >= 30) {
                if (this->work[CS_TIMER_1] < 45) {
                    play->envCtx.unk_BE = 0;
                    play->envCtx.unk_BD = 2;
                    play->envCtx.unk_D8 = 1.0f;
                } else {
                    Math_ApproachZeroF(&play->envCtx.unk_D8, 1.0f, 0.1f);
                }

                if (this->work[CS_TIMER_1] == 30) {
                    for (i = 0; i < 50; i++) {
                        Vec3f pos;
                        Vec3f velocity;

                        pos.x = sKoumePtr->actor.world.pos.x + Rand_CenteredFloat(50.0f);
                        pos.y = sKoumePtr->actor.world.pos.y + Rand_CenteredFloat(50.0f);
                        pos.z = sKoumePtr->actor.world.pos.z + Rand_CenteredFloat(50.0f);
                        velocity.x = Rand_CenteredFloat(20.0f);
                        velocity.y = Rand_CenteredFloat(20.0f);
                        velocity.z = Rand_CenteredFloat(20.0f);
                        BossTw_AddFlameEffect(play, &pos, &velocity, &sZeroVector, Rand_ZeroFloat(10.0f) + 25.0f, 1);
                    }

                    Audio_PlayActorSound2(&sKoumePtr->actor, NA_SE_EN_TWINROBA_TRANSFORM);
                    play->envCtx.unk_D8 = 0;
                }

                if (this->work[CS_TIMER_1] >= 35) {
                    if (this->work[CS_TIMER_1] < 50) {
                        Math_ApproachF(&sKoumePtr->actor.scale.x,
                                       ((Math_SinS(this->work[CS_TIMER_1] * 0x4200) * 20.0f) / 10000.0f) + 0.024999999f,
                                       1.0f, 0.005f);
                    } else {
                        if (this->work[CS_TIMER_1] == 50) {
                            Animation_MorphToPlayOnce(&sKoumePtr->skelAnime, &gTwinrovaKotakeKoumeLaughAnim, -5);
                            this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeLaughAnim);
                        }

                        if (this->work[CS_TIMER_1] == 60) {
                            Audio_PlayActorSound2(&sKoumePtr->actor, NA_SE_EN_TWINROBA_LAUGH);
                        }

                        if (Animation_OnFrame(&sKoumePtr->skelAnime, this->workf[ANIM_SW_TGT])) {
                            Animation_MorphToLoop(&sKoumePtr->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, 0.f);
                            this->workf[ANIM_SW_TGT] = 1000.0f;
                        }

                        Math_ApproachF(&sKoumePtr->actor.scale.x, 0.024999999f, 0.1f, 0.005f);
                    }

                    Actor_SetScale(&sKoumePtr->actor, sKoumePtr->actor.scale.x);
                    sKoumePtr->actor.shape.rot.y = -0x8000;
                    sKoumePtr->unk_5F8 = 1;

                    if (this->work[CS_TIMER_1] == 0x64) {
                        this->csState2 = 10;
                        this->work[CS_TIMER_1] = 0;
                        this->subCamYawStep = 0.0f;
                        sKotakePtr->visible = 1;
                        Animation_MorphToLoop(&sKotakePtr->skelAnime, &gTwinrovaKotakeKoumeIdleLoopAnim, 0.0f);
                        sKotakePtr->actor.world.pos.x = 0.0f;
                        sKotakePtr->actor.world.pos.y = 80.0f;
                        sKotakePtr->actor.world.pos.z = -600.0f;
                        sKotakePtr->actor.shape.rot.y = sKotakePtr->actor.world.rot.y = 0;
                        this->work[CS_TIMER_1] = 0;

                        this->subCamEye.x = -30.0f;
                        this->subCamEye.y = 260.0f;
                        this->subCamEye.z = -470.0f;

                        this->subCamAt.x = 0;
                        this->subCamAt.y = 270.0f;
                        this->subCamAt.z = -600.0f;
                        Actor_SetScale(&sKotakePtr->actor, 0.014999999f);
                    }
                } else {
                    sKoumePtr->actor.shape.rot.y = sKoumePtr->actor.shape.rot.y + (s16)this->subCamYawStep;
                }
            } else {
                if ((this->work[CS_TIMER_1] % 8) == 0) {
                    Audio_PlayActorSound2(&sKoumePtr->actor, NA_SE_EN_TWINROBA_ROLL);
                }

                sKoumePtr->actor.shape.rot.y = sKoumePtr->actor.shape.rot.y + (s16)this->subCamYawStep;
                Math_ApproachF(&this->subCamYawStep, 12288.0f, 1.0f, 384.0f);

                if (Animation_OnFrame(&sKoumePtr->skelAnime, this->workf[ANIM_SW_TGT])) {
                    Animation_MorphToLoop(&sKoumePtr->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, 0.0f);
                    this->workf[ANIM_SW_TGT] = 1000.0f;
                }
            }
            break;

        case 10:
            SkelAnime_Update(&sKotakePtr->skelAnime);
            Math_ApproachF(&sKotakePtr->actor.world.pos.y, 240.0f, 0.05f, 5.0f);
            this->subCamEye.x -= 0.2f;
            this->subCamEye.z -= 0.2f;

            if (this->work[CS_TIMER_1] >= 0x33) {
                this->csState2 = 11;
                this->subCamEyeTarget.x = -30;
                this->subCamEyeTarget.y = 260;
                this->subCamEyeTarget.z = -530;
                this->subCamAtTarget.x = 0;
                this->subCamAtTarget.y = 265;
                this->subCamAtTarget.z = -580;
                this->subCamEyeStep.x = fabsf(this->subCamEyeTarget.x - this->subCamEye.x);
                this->subCamEyeStep.y = fabsf(this->subCamEyeTarget.y - this->subCamEye.y);
                this->subCamEyeStep.z = fabsf(this->subCamEyeTarget.z - this->subCamEye.z);
                this->subCamAtStep.x = fabsf(this->subCamAtTarget.x - this->subCamAt.x);
                this->subCamAtStep.y = fabsf(this->subCamAtTarget.y - this->subCamAt.y);
                this->subCamAtStep.z = fabsf(this->subCamAtTarget.z - this->subCamAt.z);
                this->subCamUpdateRate = 0;
                this->subCamDistStep = 0.1f;
                this->work[CS_TIMER_1] = 0;
            }
            break;

        case 11:
            SkelAnime_Update(&sKotakePtr->skelAnime);
            updateCam = 1;
            Math_ApproachF(&sKotakePtr->actor.world.pos.y, 240.0f, 0.05f, 5.0f);
            Math_ApproachF(&this->subCamUpdateRate, 1.0f, 1.0f, 0.02f);

            if (this->work[CS_TIMER_1] == 30) {
                Message_StartTextbox(play, 0x604A, NULL);
            }

            if (this->work[CS_TIMER_1] > 80) {
                this->csState2 = 12;
                this->actor.speedXZ = 0;

                this->subCamEyeTarget.y = 260.0f;
                this->subCamEyeTarget.x = -80.0f;
                this->subCamEyeTarget.z = -430.0f;

                this->subCamAtTarget.x = sKotakePtr->actor.world.pos.x;
                this->subCamAtTarget.y = sKotakePtr->actor.world.pos.y + 20.0f;
                this->subCamAtTarget.z = sKotakePtr->actor.world.pos.z;

                this->subCamEyeStep.x = fabsf(this->subCamEyeTarget.x - this->subCamEye.x);
                this->subCamEyeStep.y = fabsf(this->subCamEyeTarget.y - this->subCamEye.y);
                this->subCamEyeStep.z = fabsf(this->subCamEyeTarget.z - this->subCamEye.z);
                this->subCamAtStep.x = fabsf(this->subCamAtTarget.x - this->subCamAt.x);
                this->subCamAtStep.y = fabsf(this->subCamAtTarget.y - this->subCamAt.y);
                this->subCamAtStep.z = fabsf(this->subCamAtTarget.z - this->subCamAt.z);
                this->subCamUpdateRate = 0;
                this->subCamDistStep = 0.05f;
                Animation_MorphToPlayOnce(&sKotakePtr->skelAnime, &gTwinrovaKotakeKoumeIdleEndAnim, 0);
                this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeIdleEndAnim);
                this->work[CS_TIMER_1] = 0;
            }
            break;

        case 12:
            updateCam = 1;
            SkelAnime_Update(&sKotakePtr->skelAnime);
            this->subCamAtTarget.y = sKotakePtr->actor.world.pos.y + 20.0f;
            Math_ApproachF(&sKotakePtr->actor.world.pos.y, 350, 0.1f, this->actor.speedXZ);
            Math_ApproachF(&this->actor.speedXZ, 9.0f, 1.0f, 0.9f);
            Math_ApproachF(&this->subCamUpdateRate, 1.0f, 1.0f, 0.02f);

            if (this->work[CS_TIMER_1] >= 30) {
                if (this->work[CS_TIMER_1] < 45) {
                    play->envCtx.unk_BD = 3;
                    play->envCtx.unk_D8 = 1.0f;
                } else {
                    Math_ApproachZeroF(&play->envCtx.unk_D8, 1.0f, 0.1f);
                }

                if (this->work[CS_TIMER_1] == 30) {
                    for (i = 0; i < 50; i++) {
                        Vec3f pos;
                        Vec3f velocity;
                        pos.x = sKotakePtr->actor.world.pos.x + Rand_CenteredFloat(50.0f);
                        pos.y = sKotakePtr->actor.world.pos.y + Rand_CenteredFloat(50.0f);
                        pos.z = sKotakePtr->actor.world.pos.z + Rand_CenteredFloat(50.0f);
                        velocity.x = Rand_CenteredFloat(20.0f);
                        velocity.y = Rand_CenteredFloat(20.0f);
                        velocity.z = Rand_CenteredFloat(20.0f);
                        BossTw_AddFlameEffect(play, &pos, &velocity, &sZeroVector, Rand_ZeroFloat(10.f) + 25.0f, 0);
                    }

                    Audio_PlayActorSound2(&sKotakePtr->actor, NA_SE_EN_TWINROBA_TRANSFORM);
                    play->envCtx.unk_D8 = 0.0f;
                }

                if (this->work[CS_TIMER_1] >= 35) {
                    if (this->work[CS_TIMER_1] < 50) {
                        Math_ApproachF(&sKotakePtr->actor.scale.x,
                                       ((Math_SinS(this->work[CS_TIMER_1] * 0x4200) * 20.0f) / 10000.0f) + 0.024999999f,
                                       1.0f, 0.005f);
                    } else {
                        if (this->work[CS_TIMER_1] == 50) {
                            Animation_MorphToPlayOnce(&sKotakePtr->skelAnime, &gTwinrovaKotakeKoumeLaughAnim, -5.0f);
                            this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaKotakeKoumeLaughAnim);
                        }

                        if (this->work[CS_TIMER_1] == 60) {
                            Audio_PlayActorSound2(&sKotakePtr->actor, NA_SE_EN_TWINROBA_LAUGH2);
                        }

                        if (Animation_OnFrame(&sKotakePtr->skelAnime, this->workf[ANIM_SW_TGT])) {
                            Animation_MorphToLoop(&sKotakePtr->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, 0.0f);
                            this->workf[ANIM_SW_TGT] = 1000.0f;
                        }

                        Math_ApproachF(&sKotakePtr->actor.scale.x, 0.024999999f, 0.1f, 0.005f);
                    }

                    Actor_SetScale(&sKotakePtr->actor, sKotakePtr->actor.scale.x);
                    sKotakePtr->actor.shape.rot.y = 0;
                    sKotakePtr->unk_5F8 = 1;

                    if (this->work[CS_TIMER_1] == 100) {
                        this->csState2 = 20;
                        this->work[CS_TIMER_1] = 0;

                        this->workf[UNK_F11] = 600.0f;

                        this->subCamEye.x = 800.0f;
                        this->subCamEye.y = 300.0f;
                        this->subCamEye.z = 0;

                        this->subCamAt.x = 0.0f;
                        this->subCamAt.y = 400.0f;
                        this->subCamAt.z = 0;

                        this->workf[UNK_F9] = -M_PI / 2.0f;
                        this->workf[UNK_F10] = 0.0f;

                        this->subCamEyeStep.x = 0.0f;
                        this->spawnPortalAlpha = 0.0f;
                    }
                } else {
                    sKotakePtr->actor.shape.rot.y = sKotakePtr->actor.shape.rot.y + (s16)this->subCamYawStep;
                }
            } else {
                if ((this->work[CS_TIMER_1] % 8) == 0) {
                    Audio_PlayActorSound2(&sKotakePtr->actor, NA_SE_EN_TWINROBA_ROLL);
                }

                sKotakePtr->actor.shape.rot.y = sKotakePtr->actor.shape.rot.y + (s16)this->subCamYawStep;
                Math_ApproachF(&this->subCamYawStep, 12288.0f, 1.0f, 384.0f);

                if (Animation_OnFrame(&sKotakePtr->skelAnime, this->workf[ANIM_SW_TGT])) {
                    Animation_MorphToLoop(&sKotakePtr->skelAnime, &gTwinrovaKotakeKoumeFlyAnim, 0.0f);
                    this->workf[ANIM_SW_TGT] = 1000.0f;
                }
            }
            break;

        case 20:
            if (this->work[CS_TIMER_1] > 20 && this->work[CS_TIMER_1] < 120) {
                play->envCtx.unk_BD = 1;
                Math_ApproachF(&play->envCtx.unk_D8, 1.0f, 1.0f, 0.015f);
            }

            if (this->work[CS_TIMER_1] == 90) {
                Audio_QueueSeqCmd(0x1 << 28 | SEQ_PLAYER_BGM_MAIN << 24 | 0x5A00FF);
            }

            if (this->work[CS_TIMER_1] == 120) {
                sEnvType = 0;
                play->envCtx.unk_BE = 1;
                play->envCtx.unk_BD = 1;
                play->envCtx.unk_D8 = 0.0f;
                TitleCard_InitBossName(play, &play->actorCtx.titleCtx, SEGMENTED_TO_VIRTUAL(gTwinrovaTitleCardENGTex),
                                       160, 180, 128, 40, true);
                Flags_SetEventChkInf(EVENTCHKINF_BEGAN_TWINROVA_BATTLE);
                Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_BOSS);
            }

            if (this->work[CS_TIMER_1] >= 160) {
                if (this->work[CS_TIMER_1] == 160) {
                    this->subCamEyeStep.x = 0.0f;
                }
                Math_ApproachF(&this->subCamEye.x, 0.0f, 0.05f, this->subCamEyeStep.x * 0.5f);
                Math_ApproachF(&this->subCamEye.z, 1000.0f, 0.05f, this->subCamEyeStep.x);
                Math_ApproachF(&this->subCamEyeStep.x, 40.0f, 1.0f, 1);
            } else {
                Math_ApproachF(&this->subCamEye.x, 300.0f, 0.05f, this->subCamEyeStep.x);
                Math_ApproachF(&this->subCamEyeStep.x, 5.0f, 1.0f, 0.5f);
            }

            if (this->work[CS_TIMER_1] < 200) {
                Audio_PlayActorSound2(&sKoumePtr->actor, NA_SE_EN_TWINROBA_FLY - SFX_FLAG);
                Audio_PlayActorSound2(&sKotakePtr->actor, NA_SE_EN_TWINROBA_FLY - SFX_FLAG);
                sp90.x = this->workf[UNK_F11];
                sp90.y = 400.0f;
                sp90.z = 0.0f;
                Matrix_RotateY(this->workf[UNK_F9], MTXMODE_NEW);
                Matrix_MultVec3f(&sp90, &sp84);
                sKoumePtr->actor.world.pos.x = sp84.x;
                sKoumePtr->actor.world.pos.y = sp84.y;
                sKoumePtr->actor.world.pos.z = sp84.z;
                sKoumePtr->actor.world.rot.y = sKoumePtr->actor.shape.rot.y = (this->workf[UNK_F9] / M_PI) * 32768.0f;
                sKotakePtr->actor.world.pos.x = -sp84.x;
                sKotakePtr->actor.world.pos.y = sp84.y;
                sKotakePtr->actor.world.pos.z = -sp84.z;
                sKotakePtr->actor.shape.rot.y = sKotakePtr->actor.world.rot.y =
                    ((this->workf[UNK_F9] / M_PI) * 32768.0f) + 32768.0f;
                Math_ApproachF(&this->workf[UNK_F11], 80.0f, 0.1f, 5.0f);
                this->workf[UNK_F9] -= this->workf[UNK_F10];
                Math_ApproachF(&this->workf[UNK_F10], 0.19999999f, 1.0f, 0.0019999994f);
            }

            if (this->work[CS_TIMER_1] == 200) {
                sKoumePtr->actionFunc = BossTw_FlyTo;
                sKotakePtr->actionFunc = BossTw_FlyTo;
                sKoumePtr->targetPos.x = 600.0f;
                sKoumePtr->targetPos.y = 400.0f;
                sKoumePtr->targetPos.z = 0.0f;
                sKoumePtr->timers[0] = 100;
                sKotakePtr->targetPos.x = -600.0f;
                sKotakePtr->targetPos.y = 400.0f;
                sKotakePtr->targetPos.z = 0.0f;
                sKotakePtr->timers[0] = 100;
            }

            if (this->work[CS_TIMER_1] == 260) {
                Camera* cam = Play_GetCamera(play, CAM_ID_MAIN);

                cam->eye = this->subCamEye;
                cam->eyeNext = this->subCamEye;
                cam->at = this->subCamAt;
                func_800C08AC(play, this->subCamId, 0);
                this->subCamId = 0;
                this->csState2 = this->subCamId;
                func_80064534(play, &play->csCtx);
                Player_SetCsActionWithHaltedActors(play, &this->actor, 7);
                BossTw_SetupWait(this, play);
            }
            break;
    }

    if (this->subCamId != 0) {
        if (updateCam) {
            Math_ApproachF(&this->subCamEye.x, this->subCamEyeTarget.x, this->subCamDistStep,
                           this->subCamEyeStep.x * this->subCamUpdateRate);
            Math_ApproachF(&this->subCamEye.y, this->subCamEyeTarget.y, this->subCamDistStep,
                           this->subCamEyeStep.y * this->subCamUpdateRate);
            Math_ApproachF(&this->subCamEye.z, this->subCamEyeTarget.z, this->subCamDistStep,
                           this->subCamEyeStep.z * this->subCamUpdateRate);
            Math_ApproachF(&this->subCamAt.x, this->subCamAtTarget.x, this->subCamDistStep,
                           this->subCamAtStep.x * this->subCamUpdateRate);
            Math_ApproachF(&this->subCamAt.y, this->subCamAtTarget.y, this->subCamDistStep,
                           this->subCamAtStep.y * this->subCamUpdateRate);
            Math_ApproachF(&this->subCamAt.z, this->subCamAtTarget.z, this->subCamDistStep,
                           this->subCamAtStep.z * this->subCamUpdateRate);
        }

        Play_CameraSetAtEye(play, this->subCamId, &this->subCamAt, &this->subCamEye);
    }
}

void BossTw_DeathBall(BossTw* this, PlayState* play) {
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    s32 pad;
    s16 i;
    s16 yaw;

    if ((this->work[CS_TIMER_1] % 16) == 0) {
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FB_FLY);
    }

    if (sTwinrovaPtr->csState2 < 2) {
        if (this->timers[0] == 0) {
            this->timers[0] = 20;
            this->targetPos.x = Rand_CenteredFloat(100.0f) + sTwinrovaPtr->actor.world.pos.x;
            this->targetPos.y = Rand_CenteredFloat(50.0f) + 400.0f;
            this->targetPos.z = Rand_CenteredFloat(100.0f) + sTwinrovaPtr->actor.world.pos.z;
        }

        this->timers[1] = 10;
        this->rotateSpeed = 8192.0f;
        this->actor.speedXZ = 5.0f;
    } else {
        if (this->timers[1] == 9) {
            this->targetPos.y = 413.0f;
            this->actor.world.pos.z = 0.0f;
            this->actor.world.pos.x = 0.0f;
            for (i = 0; i < ARRAY_COUNT(this->blastTailPos); i++) {
                this->blastTailPos[i] = this->actor.world.pos;
            }
        }

        if (this->actor.params == 0x69) {
            this->targetPos.x = sKoumePtr->actor.world.pos.x;
            this->targetPos.z = sKoumePtr->actor.world.pos.z;
        } else {
            this->targetPos.x = sKotakePtr->actor.world.pos.x;
            this->targetPos.z = sKotakePtr->actor.world.pos.z;
        }

        Math_ApproachF(&this->targetPos.y, 263.0f, 1.0f, 2.0f);

        if (this->targetPos.y == 263.0f) {
            Math_ApproachF(&this->actor.speedXZ, 0.0f, 1.0f, 0.2f);
            if (sTwinrovaPtr->csState2 == 3) {
                Actor_Kill(&this->actor);
            }
        }
    }

    xDiff = this->targetPos.x - this->actor.world.pos.x;
    yDiff = this->targetPos.y - this->actor.world.pos.y;
    zDiff = this->targetPos.z - this->actor.world.pos.z;

    yaw = Math_FAtan2F(xDiff, zDiff) * (32768 / M_PI);
    Math_ApproachS(&this->actor.world.rot.x, Math_FAtan2F(yDiff, sqrtf(SQ(xDiff) + SQ(zDiff))) * (32768 / M_PI), 5,
                   this->rotateSpeed);
    Math_ApproachS(&this->actor.world.rot.y, yaw, 5, this->rotateSpeed);
    Actor_UpdateVelocityXYZ(&this->actor);
    Actor_UpdatePos(&this->actor);
}

void BossTw_TwinrovaSetupDeathCS(BossTw* this, PlayState* play) {
    BossTw_ClearSummonedEnemies(play);
    BossTw_ClearGroundHazards(play, true);
    BossTw_ClearPlayerFreeze(play);
    BossTw_ClearPlayerBurn(play);
    BossTw_ClearSiegeZones(this, play, false, TWINROVA_SIEGE_USE_ZONE_ELEMENT, false);
    BossTw_CancelPhaseTwoMagic(this, play, false);
    sEnvType = 0;
    this->actionFunc = BossTw_TwinrovaDeathCS;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaDamageAnim, -3.0f);
    this->actor.world.rot.y = this->actor.shape.rot.y;
    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
    this->csState2 = this->csState1 = 0;
    this->work[CS_TIMER_1] = this->work[CS_TIMER_2] = 0;
    this->work[INVINC_TIMER] = 10000;
    BossTw_SetupDeathCS(sKoumePtr, play);
    BossTw_SetupDeathCS(sKotakePtr, play);
    sKotakePtr->timers[0] = 8;
    this->workf[UNK_F19] = 1.0f;
}

void BossTw_DeathCSMsgSfx(BossTw* this, PlayState* play) {
    s32 pad;
    s32 pad2;
    s32 pad3;
    s16 msgId2;
    s16 msgId1;
    u8 kotakeAnim;
    u8 koumeAnim;
    u8 sp35;

    msgId2 = 0;
    msgId1 = 0;
    kotakeAnim = 0;
    koumeAnim = 0;
    sp35 = 0;

    // Skip ahead to last part of the cutscene in rando
    if (this->work[CS_TIMER_2] == 10 && (IS_RANDO || IS_BOSS_RUSH)) {
        this->work[CS_TIMER_2] = 860;
    }

    if (this->work[CS_TIMER_2] == 80) {
        koumeAnim = 1;
    }

    if (this->work[CS_TIMER_2] == 80) {
        msgId2 = 0x604B;
        sp35 = 50;
    }

    if (this->work[CS_TIMER_2] == 140) {
        kotakeAnim = koumeAnim = 2;
    }

    if (this->work[CS_TIMER_2] == 170) {
        kotakeAnim = 3;
        sKotakePtr->work[YAW_TGT] = -0x4000;
        sKotakePtr->rotateSpeed = 0.0f;
        Audio_PlayActorSound2(&sKotakePtr->actor, NA_SE_EN_TWINROBA_SENSE);
        msgId2 = 0x604C;
    }

    if (this->work[CS_TIMER_2] == 210) {
        D_8094C874 = 30;
    }

    if (this->work[CS_TIMER_2] == 270) {
        koumeAnim = 3;
        sKoumePtr->work[YAW_TGT] = 0x4000;
        sKoumePtr->rotateSpeed = 0.0f;
        Audio_PlayActorSound2(&sKoumePtr->actor, NA_SE_EN_TWINROBA_SENSE);
    }

    if (this->work[CS_TIMER_2] == 290) {
        msgId2 = 0x604D;
        sp35 = 35;
    }

    if (this->work[CS_TIMER_2] == 350) {
        koumeAnim = kotakeAnim = 2;
        sKoumePtr->work[YAW_TGT] = sKotakePtr->work[YAW_TGT] = 0;
        sKoumePtr->rotateSpeed = sKotakePtr->rotateSpeed = 0.0f;
    }

    if (this->work[CS_TIMER_2] == 380) {
        koumeAnim = kotakeAnim = 3;
    }

    if (this->work[CS_TIMER_2] == 400) {
        koumeAnim = kotakeAnim = 2;
    }

    if (this->work[CS_TIMER_2] == 430) {
        koumeAnim = 4;
        D_8094C874 = 435;
        D_8094C878 = 1;
    }

    if (this->work[CS_TIMER_2] > 440 && this->work[CS_TIMER_2] < 860) {
        Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_FIGHT - SFX_FLAG);
    }

    if (this->work[CS_TIMER_2] == 430) {
        msgId2 = 0x604E;
    }

    if (this->work[CS_TIMER_2] == 480) {
        kotakeAnim = 4;
        sKotakePtr->work[YAW_TGT] = -0x4000;
    }

    if (this->work[CS_TIMER_2] == 500) {
        koumeAnim = 2;
    }

    if (this->work[CS_TIMER_2] == 480) {
        msgId1 = 0x604F;
    }

    if (this->work[CS_TIMER_2] == 530) {
        koumeAnim = 4;
        sKoumePtr->work[YAW_TGT] = 0x4000;
        D_8094C87A = 335;
        D_8094C87E = 1;
    }

    if (this->work[CS_TIMER_2] == 530) {
        msgId2 = 0x6050;
    }

    if (this->work[CS_TIMER_2] == 580) {
        msgId1 = 0x6051;
    }

    if (this->work[CS_TIMER_2] == 620) {
        msgId2 = 0x6052;
    }

    if (this->work[CS_TIMER_2] == 660) {
        msgId1 = 0x6053;
    }

    if (this->work[CS_TIMER_2] == 700) {
        msgId2 = 0x6054;
    }

    if (this->work[CS_TIMER_2] == 740) {
        msgId1 = 0x6055;
    }

    if (this->work[CS_TIMER_2] == 780) {
        msgId2 = 0x6056;
    }

    if (this->work[CS_TIMER_2] == 820) {
        msgId1 = 0x6057;
        Audio_QueueSeqCmd(0x1 << 28 | SEQ_PLAYER_BGM_MAIN << 24 | 0x5000FF);
    }

    if (this->work[CS_TIMER_2] == 860) {
        koumeAnim = kotakeAnim = 3;
    }

    if (this->work[CS_TIMER_2] == 900) {
        Audio_PlayActorSound2(&sKoumePtr->actor, NA_SE_EN_TWINROBA_DIE);
        Audio_PlayActorSound2(&sKotakePtr->actor, NA_SE_EN_TWINROBA_DIE);
    }

    if (this->work[CS_TIMER_2] == 930) {
        msgId2 = 0x6058;
    }

    if (msgId2 != 0) {
        Message_StartTextbox(play, msgId2, NULL);

        if (sp35) {
            D_8094C876 = 10;
            D_8094C874 = sp35;
            D_8094C878 = 0;
        }
    }

    if (msgId1 != 0) {
        Message_StartTextbox(play, msgId1, NULL);
    }

    switch (kotakeAnim) {
        case 1:
            Animation_MorphToLoop(&sKotakePtr->skelAnime, &gTwinrovaKotakeKoumeShakeHandAnim, -5.0f);
            break;
        case 2:
            Animation_MorphToLoop(&sKotakePtr->skelAnime, &gTwinrovaKotakeKoumeFloatLookForwardAnim, -5.0f);
            break;
        case 3:
            Animation_MorphToLoop(&sKotakePtr->skelAnime, &gTwinrovaKotakeKoumeFloatLookUpAnim, -5.0f);
            break;
        case 4:
            Animation_MorphToLoop(&sKotakePtr->skelAnime, &gTwinrovaKotakeKoumeBickerAnim, -5.0f);
            break;
    }

    switch (koumeAnim) {
        case 1:
            Animation_MorphToLoop(&sKoumePtr->skelAnime, &gTwinrovaKotakeKoumeShakeHandAnim, -5.0f);
            break;
        case 2:
            Animation_MorphToLoop(&sKoumePtr->skelAnime, &gTwinrovaKotakeKoumeFloatLookForwardAnim, -5.0f);
            break;
        case 3:
            Animation_MorphToLoop(&sKoumePtr->skelAnime, &gTwinrovaKotakeKoumeFloatLookUpAnim, -5.0f);
            break;
        case 4:
            Animation_MorphToLoop(&sKoumePtr->skelAnime, &gTwinrovaKotakeKoumeBickerAnim, -5.0f);
            break;
    }

    if (this->work[CS_TIMER_2] >= 120 && this->work[CS_TIMER_2] < 500) {
        Math_ApproachF(&this->workf[UNK_F18], 255.0f, 0.1f, 5.0f);
    }

    // Add separate timings for the "beam" that opens and closes around the sisters
    // Needed because we skip ahead in cutscene timer value so it never gets called otherwise
    if (IS_RANDO || IS_BOSS_RUSH) {
        if (this->work[CS_TIMER_2] < 900) {
            Math_ApproachF(&this->workf[UNK_F18], 255.0f, 0.1f, 5.0f);
        } else if (this->work[CS_TIMER_2] > 910) {
            Math_ApproachF(&this->workf[UNK_F18], 0.0f, 1.0f, 3.0f);
        }
    }

    if (this->work[CS_TIMER_2] >= 150) {
        Math_ApproachF(&sKoumePtr->workf[UNK_F17], (Math_SinS(this->work[CS_TIMER_1] * 2000) * 0.05f) + 0.4f, 0.1f,
                       0.01f);
        Math_ApproachF(&sKotakePtr->workf[UNK_F17], (Math_CosS(this->work[CS_TIMER_1] * 1700) * 0.05f) + 0.4f, 0.1f,
                       0.01f);

        if (this->work[CS_TIMER_2] >= 880) {
            Math_ApproachF(&sKotakePtr->actor.world.pos.y, 2000.0f, 1.0f, this->actor.speedXZ);
            Math_ApproachF(&sKoumePtr->actor.world.pos.y, 2000.0f, 1.0f, this->actor.speedXZ);
            Math_ApproachF(&this->actor.speedXZ, 10.0f, 1.0f, 0.25f);

            if (this->work[CS_TIMER_2] >= 930) {
                Math_ApproachF(&this->workf[UNK_F19], 5.0f, 1.0f, 0.05f);
                Math_ApproachF(&this->workf[UNK_F18], 0.0f, 1.0f, 3.0f);
            }

            Audio_PlayActorSound2(&this->actor, NA_SE_EV_GOTO_HEAVEN - SFX_FLAG);
        } else {
            f32 yTarget = Math_CosS(this->work[CS_TIMER_2] * 1700) * 4.0f;
            Math_ApproachF(&sKotakePtr->actor.world.pos.y, 20.0f + (263.0f + yTarget), 0.1f, this->actor.speedXZ);
            yTarget = Math_SinS(this->work[CS_TIMER_2] * 1500) * 4.0f;
            Math_ApproachF(&sKoumePtr->actor.world.pos.y, 20.0f + (263.0f + yTarget), 0.1f, this->actor.speedXZ);
            Math_ApproachF(&this->actor.speedXZ, 1.0f, 1.0f, 0.05f);
        }
    }
}

void BossTw_TwinrovaDeathCS(BossTw* this, PlayState* play) {
    s16 i;
    Vec3f spD0;
    Player* player = GET_PLAYER(play);
    Camera* mainCam = Play_GetCamera(play, CAM_ID_MAIN);

    if (this->csState2 == 0) {
        if (!BossTw_TryAcquireCutsceneCamera(this, play, 8)) {
            return;
        }

        // Waiting for a camera must not consume the authored death timeline. Restart all three participants from
        // their setup poses once the cinematic can actually be shown.
        this->csState2 = 1;
        this->work[CS_TIMER_1] = this->work[CS_TIMER_2] = 0;
        this->work[UNK_S8] = 0;
        Animation_MorphToLoop(&this->skelAnime, &gTwinrovaDamageAnim, -3.0f);
        BossTw_SetupDeathCS(sKoumePtr, play);
        BossTw_SetupDeathCS(sKotakePtr, play);
        this->subCamEye = mainCam->eye;
        this->subCamAt = mainCam->at;
        Audio_QueueSeqCmd(0x1 << 28 | SEQ_PLAYER_BGM_MAIN << 24 | 0x100FF);
    }

    SkelAnime_Update(&this->skelAnime);
    this->work[UNK_S8] += 20;

    if (this->work[UNK_S8] > 255) {
        this->work[UNK_S8] = 255;
    }

    Math_ApproachF(&this->workf[UNK_F12], 0.0f, 1.0f, 0.05f);
    this->unk_5F8 = 1;

    switch (this->csState1) {
        case 0:
            if (this->work[CS_TIMER_1] == 15) {
                Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaDeathAnim, -3.0f);
            }

            if (this->work[CS_TIMER_1] >= 15) {
                Math_ApproachF(&this->actor.world.pos.y, 400.0f, 0.05f, 10.0f);
            }

            if (this->work[CS_TIMER_1] >= 55) {
                if (this->work[CS_TIMER_1] == 55) {
                    play->envCtx.unk_D8 = 0;
                }

                sEnvType = -1;
                play->envCtx.unk_BE = 5;
                play->envCtx.unk_BD = 0;
                Math_ApproachF(&play->envCtx.unk_D8, 1.0f, 1.0f, 0.015f);
                Math_ApproachF(&this->actor.scale.x, 0.00024999998f, 0.1f, 0.00005f);
                this->actor.shape.rot.y += (s16)this->actor.speedXZ;
                this->workf[UNK_F13] += this->actor.speedXZ;
                if (this->workf[UNK_F13] > 65536.0f) {
                    this->workf[UNK_F13] -= 65536.0f;
                    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_ROLL);
                }
                Math_ApproachF(&this->actor.speedXZ, 12288.0f, 1.0f, 256.0f);
                if (this->work[CS_TIMER_1] == 135) {
                    Vec3f spBC;
                    Vec3f spB0;
                    Vec3f spA4 = { 0.0f, 0.0f, 0.0f };
                    Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_TRANSFORM);
                    for (i = 0; i < 100; i++) {
                        spB0.x = Rand_CenteredFloat(5.0f);
                        spB0.y = Rand_CenteredFloat(5.0f);
                        spB0.z = Rand_CenteredFloat(5.0f);
                        spBC = this->actor.world.pos;
                        spBC.x += spB0.x;
                        spBC.y += spB0.y;
                        spBC.z += spB0.z;
                        BossTw_AddFlameEffect(play, &spBC, &spB0, &spA4, Rand_ZeroFloat(2.0f) + 5,
                                              Rand_ZeroFloat(1.99f));
                    }
                    this->csState1 = 1;
                    this->visible = false;
                    this->actor.scale.x = 0.0f;
                    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_BOSS_TW, this->actor.world.pos.x,
                                       this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, TW_DEATHBALL_KOUME);
                    Actor_SpawnAsChild(&play->actorCtx, &this->actor, play, ACTOR_BOSS_TW, this->actor.world.pos.x,
                                       this->actor.world.pos.y, this->actor.world.pos.z, 0, 0, 0, TW_DEATHBALL_KOTAKE);
                    this->actor.flags &= ~ACTOR_FLAG_ATTENTION_ENABLED;
                }
            }
            Actor_SetScale(&this->actor, this->actor.scale.x);
            break;
        case 1:
            break;
    }

    switch (this->csState2) {
        case 1:
            spD0.x = Math_SinS(this->actor.world.rot.y) * 200.0f;
            spD0.z = Math_CosS(this->actor.world.rot.y) * 200.0f;
            Math_ApproachF(&this->subCamEye.x, spD0.x + this->actor.world.pos.x, 0.1f, 50.0f);
            Math_ApproachF(&this->subCamEye.y, 300.0f, 0.1f, 50.0f);
            Math_ApproachF(&this->subCamEye.z, spD0.z + this->actor.world.pos.z, 0.1f, 50.0f);
            Math_ApproachF(&this->subCamAt.x, this->actor.world.pos.x, 0.1f, 50.0f);
            Math_ApproachF(&this->subCamAt.y, this->actor.world.pos.y, 0.1f, 50.0f);
            Math_ApproachF(&this->subCamAt.z, this->actor.world.pos.z, 0.1f, 50.0f);
            if (this->work[CS_TIMER_1] == 170) {
                this->csState2 = 2;
                this->work[CS_TIMER_2] = 0;
                this->subCamEye.z = 170.0f;
                this->subCamDist = 170.0f;
                this->subCamEye.x = 0.0f;
                this->subCamAt.x = 0.0f;
                this->subCamAt.z = 0.0f;
                this->subCamEye.y = 260.0f;
                player->actor.shape.rot.y = -0x8000;
                player->actor.world.pos.x = -40.0f;
                player->actor.world.pos.y = 240.0f;
                player->actor.world.pos.z = 90.0f;
                sKoumePtr->actor.world.pos.x = -37.0f;
                sKotakePtr->actor.world.pos.x = 37.0f;
                sKotakePtr->actor.world.pos.y = 263.0f;
                sKoumePtr->actor.world.pos.y = sKotakePtr->actor.world.pos.y;
                this->subCamAt.y = sKoumePtr->actor.world.pos.y + 17.0f;
                sKotakePtr->actor.world.pos.z = 0.0f;
                sKoumePtr->actor.world.pos.z = sKotakePtr->actor.world.pos.z;
                sKoumePtr->work[YAW_TGT] = sKotakePtr->work[YAW_TGT] = sKoumePtr->actor.shape.rot.x =
                    sKotakePtr->actor.shape.rot.x = sKoumePtr->actor.shape.rot.y = sKotakePtr->actor.shape.rot.y = 0;
                Player_SetCsActionWithHaltedActors(play, &sKoumePtr->actor, 1);
                sKoumePtr->actor.flags |= ACTOR_FLAG_ATTENTION_ENABLED;
            }
            break;
        case 2:
            if (this->work[CS_TIMER_2] == 100) {
                Vec3f pos;
                Vec3f velocity;
                Vec3f accel = { 0.0f, 0.0f, 0.0f };
                s32 zero = 0;

                for (i = 0; i < 50; i++) {
                    velocity.x = Rand_CenteredFloat(3.0f);
                    velocity.y = Rand_CenteredFloat(3.0f);
                    velocity.z = Rand_CenteredFloat(3.0f);
                    pos = sKoumePtr->actor.world.pos;
                    pos.x += velocity.x * 2.0f;
                    pos.y += velocity.y * 2.0f;
                    pos.z += velocity.z * 2.0f;
                    BossTw_AddFlameEffect(play, &pos, &velocity, &accel, Rand_ZeroFloat(2.0f) + 5, 1);

                    // fake code needed to match, tricks the compiler into allocating more stack
                    if (zero) {
                        accel.x *= 2.0;
                    }

                    velocity.x = Rand_CenteredFloat(3.0f);
                    velocity.y = Rand_CenteredFloat(3.0f);
                    velocity.z = Rand_CenteredFloat(3.0f);
                    pos = sKotakePtr->actor.world.pos;
                    pos.x += velocity.x * 2.0f;
                    pos.y += velocity.y * 2.0f;
                    pos.z += velocity.z * 2.0f;
                    BossTw_AddFlameEffect(play, &pos, &velocity, &accel, Rand_ZeroFloat(2.0f) + 5, 0);
                }

                Actor_SetScale(&sKoumePtr->actor, 0.0f);
                Actor_SetScale(&sKotakePtr->actor, 0.0f);
                sKoumePtr->visible = 1;
                sKotakePtr->visible = 1;
                Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_TRANSFORM);
                Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_KOTAKE_KOUME);
                this->csState2 = 3;
                this->work[CS_TIMER_2] = 0;
                this->subCamYaw = this->subCamYawStep = this->actor.speedXZ = this->subCamDistStep = 0.0f;
            }
            break;
        case 3:
            BossTw_DeathCSMsgSfx(this, play);
            if (this->work[CS_TIMER_2] < 150) {
                play->envCtx.unk_BE = 1;
                play->envCtx.unk_BD = 0;
                Math_ApproachZeroF(&play->envCtx.unk_D8, 1.0f, 0.1f);
            } else {
                play->envCtx.unk_BE = 1;
                play->envCtx.unk_BD = 6;
                Math_ApproachF(&play->envCtx.unk_D8, (Math_SinS(this->work[CS_TIMER_2] * 4096) / 4.0f) + 0.75f, 1.0f,
                               0.1f);
            }

            Math_ApproachF(&this->subCamAt.y, sKoumePtr->actor.world.pos.y + 17.0f, 0.05f, 10.0f);

            if (this->work[CS_TIMER_2] >= 50) {
                Math_ApproachF(&this->subCamDist, 110.0f, 0.05f, this->subCamDistStep);
                Math_ApproachF(&this->subCamDistStep, 1.0f, 1.0f, 0.025f);
                this->subCamEye.x = this->subCamDist * sinf(this->subCamYaw);
                this->subCamEye.z = this->subCamDist * cosf(this->subCamYaw);
                if (this->work[CS_TIMER_2] >= 151) {
                    this->subCamYaw += this->subCamYawStep;
                    if (this->work[CS_TIMER_2] >= 800) {
                        Math_ApproachF(&this->subCamYawStep, 0.0f, 1.0f, 0.0001f);
                    } else {
                        Math_ApproachF(&this->subCamYawStep, 0.015f, 1.0f, 0.0001f);
                    }
                }
            }
            Math_ApproachF(&sKoumePtr->actor.scale.x, 0.009999999f, 0.1f, 0.001f);
            Actor_SetScale(&sKoumePtr->actor, sKoumePtr->actor.scale.x);
            Actor_SetScale(&sKotakePtr->actor, sKoumePtr->actor.scale.x);
            if (this->work[CS_TIMER_2] >= 1020) {
                mainCam = Play_GetCamera(play, CAM_ID_MAIN);
                mainCam->eye = this->subCamEye;
                mainCam->eyeNext = this->subCamEye;
                mainCam->at = this->subCamAt;
                func_800C08AC(play, this->subCamId, 0);
                this->csState2 = 4;
                this->subCamId = 0;
                func_80064534(play, &play->csCtx);
                Player_SetCsActionWithHaltedActors(play, &this->actor, 7);
                Audio_QueueSeqCmd(SEQ_PLAYER_BGM_MAIN << 24 | NA_BGM_BOSS_CLEAR);
                this->defeatRewardsSpawned = BossTw_TrySpawnDefeatRewards(this, play);

                this->actor.world.pos.y = -2000.0f;
                this->workf[UNK_F18] = 0.0f;
                sKoumePtr->visible = sKotakePtr->visible = false;
                if (&this->subCamEye) {} // fixes regalloc, may be fake
                Flags_SetClear(play, play->roomCtx.curRoom.num);
            }
            break;
        case 4:
            sEnvType = 0;
            if (!this->defeatRewardsSpawned) {
                this->defeatRewardsSpawned = BossTw_TrySpawnDefeatRewards(this, play);
            }
            break;
    }

    if (this->subCamId) {
        Play_CameraSetAtEye(play, this->subCamId, &this->subCamAt, &this->subCamEye);
    }
}

static s16 D_8094A900[] = {
    0, 1, 2, 2, 1,
};

static s16 D_8094A90C[] = {
    0, 1, 2, 2, 2, 2, 2, 2, 1,
};

void BossTw_Update(Actor* thisx, PlayState* play) {
    BossTw* this = (BossTw*)thisx;
    BossTw* otherTw = (BossTw*)this->actor.parent;
    Player* player = GET_PLAYER(play);
    s16 i;
    s32 pad;
    s32 suppressContactDamage;

    this->collider.base.colType = COLTYPE_HIT3;
    Math_ApproachF(&this->fogR, play->lightCtx.fogColor[0], 1.0f, 10.0f);
    Math_ApproachF(&this->fogG, play->lightCtx.fogColor[1], 1.0f, 10.0f);
    Math_ApproachF(&this->fogB, play->lightCtx.fogColor[2], 1.0f, 10.0f);
    Math_ApproachF(&this->fogNear, play->lightCtx.fogNear, 1.0f, 10.0f);
    Math_ApproachF(&this->fogFar, 1000.0f, 1.0f, 10.0f);
    this->work[CS_TIMER_1]++;
    this->work[CS_TIMER_2]++;
    this->work[TAIL_IDX]++;

    if (this->work[TAIL_IDX] >= ARRAY_COUNT(this->blastTailPos)) {
        this->work[TAIL_IDX] = 0;
    }

    this->blastTailPos[this->work[TAIL_IDX]] = this->actor.world.pos;

    for (i = 0; i < 4; i++) {
        if (this->timers[i] != 0) {
            this->timers[i]--;
        }
    }

    if (sPhaseOneCooldownUpdateFrame != play->gameplayFrames) {
        // Both sisters share one scheduler, so tick both special cooldowns together once per gameplay frame. Hyper
        // Bosses may substep neutral movement, but it must not silently make the special repertoire recur sooner.
        sPhaseOneCooldownUpdateFrame = play->gameplayFrames;
        if (sKotakePtr != NULL && sKotakePtr->timers[4] != 0) {
            sKotakePtr->timers[4]--;
        }
        if (sKoumePtr != NULL && sKoumePtr->timers[4] != 0) {
            sKoumePtr->timers[4]--;
        }
    }

    if (this->work[INVINC_TIMER] != 0) {
        this->work[INVINC_TIMER]--;
    }

    if (this->work[FOG_TIMER] != 0) {
        this->work[FOG_TIMER]--;
    }

    if (this->timers[2] == 0 &&
        (this->actionFunc == BossTw_FlyTo || this->actionFunc == BossTw_TurnToPlayer) &&
        !BossTw_HasActivePhaseOneMagic(play) && !BossTw_HasActiveGroundPressure(play) &&
        BossTw_CountActiveSummons(play) <= TWINROVA_PHASE_ONE_DUAL_VOLLEY_SUMMON_LIMIT &&
        (otherTw == NULL || BossTw_IsNormalPhaseOneMovement(otherTw))) {
        if ((s16)(player->actor.shape.rot.y - this->actor.yawTowardsPlayer + 0x8000) < 0x1000 &&
            (s16)(player->actor.shape.rot.y - this->actor.yawTowardsPlayer + 0x8000) > -0x1000 &&
            BossTw_HasPhaseOneSpacing(this, TWINROVA_PHASE_ONE_BODY_SEPARATION) &&
            this->actor.xzDistToPlayer < TWINROVA_SPIN_TRIGGER_RANGE &&
            fabsf(this->actor.world.pos.y - player->actor.world.pos.y) < TWINROVA_SPIN_TRIGGER_HEIGHT &&
            player->unk_A73 != 0) {
            // Preserve the original immediate anti-projectile/melee counter independently of the camping detector.
            this->sisterUnderPlayerTimer = 0;
            BossTw_SetupSpin(this, play);
        } else if (BossTw_HasPhaseOneSpacing(this, TWINROVA_PHASE_ONE_BODY_SEPARATION) &&
            this->actor.xzDistToPlayer < TWINROVA_SPIN_TRIGGER_RANGE &&
            this->actor.world.pos.y - player->actor.world.pos.y >= TWINROVA_PILLAR_DIVE_MIN_HEIGHT &&
            this->actor.world.pos.y - player->actor.world.pos.y <= TWINROVA_PILLAR_DIVE_MAX_HEIGHT) {
            if (this->sisterUnderPlayerTimer < TWINROVA_PILLAR_DIVE_TRACK_TIME) {
                this->sisterUnderPlayerTimer++;
            }
            if (this->sisterUnderPlayerTimer >= TWINROVA_PILLAR_DIVE_TRACK_TIME) {
                BossTw_SetupPillarDive(this, play);
            }
        } else {
            this->sisterUnderPlayerTimer = 0;
        }
    } else if (this->actionFunc != BossTw_PillarDive) {
        this->sisterUnderPlayerTimer = 0;
    }

    this->actionFunc(this, play);
    BossTw_SeparatePhaseOneSisters(this);
    // timers[3] is collision-only grace in live phase-one combat. Test it independently of actionFunc because Beam
    // or Volley's final update may transition to FinishBeamShoot before this post-action collider registration.
    suppressContactDamage = BossTw_IsPlayerHardDisabled(play) || this->timers[3] != 0;

    if (this->actionFunc != BossTw_Wait && this->actionFunc != BossTw_PortalReposition &&
        this->actionFunc != BossTw_MergeCS) {
        this->collider.dim.radius = 45;

        if ((this->actionFunc == BossTw_Spin && this->timers[0] != 0 &&
             this->timers[0] <= TWINROVA_SPIN_ACTIVE_TIME) ||
            (this->actionFunc == BossTw_PillarDive && this->csState1 == 1 && this->timers[0] != 0)) {
            this->collider.dim.radius *= 2;
            // Preserve the original top edge while extending the counter down to grounded Link.
            this->collider.dim.height = TWINROVA_SPIN_COLLIDER_HEIGHT;
            this->collider.dim.yShift = TWINROVA_SPIN_COLLIDER_Y_SHIFT;
        } else {
            this->collider.dim.height = 120;
            this->collider.dim.yShift = -30;
        }

        if (this->work[INVINC_TIMER] == 0) {
            if (this->collider.base.acFlags & AC_HIT) {
                this->collider.base.acFlags &= ~AC_HIT;
            }

            Collider_UpdateCylinder(&this->actor, &this->collider);
            CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
            if (!suppressContactDamage &&
                ((this->actionFunc != BossTw_Spin && this->actionFunc != BossTw_PillarDive) ||
                 (this->actionFunc == BossTw_PillarDive && this->csState1 == 1 && this->timers[0] != 0) ||
                 (this->actionFunc == BossTw_Spin && this->timers[0] != 0 &&
                  this->timers[0] <= TWINROVA_SPIN_ACTIVE_TIME))) {
                CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
            } else {
                Collider_ResetCylinderAT(play, &this->collider.base);
            }
        } else if (suppressContactDamage) {
            Collider_ResetCylinderAT(play, &this->collider.base);
        }

        if (this->actor.params == 0) {
            this->workf[OUTR_CRWN_TX_X2] += 1.0f;
            this->workf[OUTR_CRWN_TX_Y2] -= 7.0f;
            this->workf[INNR_CRWN_TX_Y1] += 1.0f;
        } else {
            this->workf[OUTR_CRWN_TX_X2] += 0.0f;
            this->workf[INNR_CRWN_TX_X2] += 0.0f;
            this->workf[OUTR_CRWN_TX_Y2] += -15.0f;
            this->workf[INNR_CRWN_TX_Y2] += -10.0f;
        }

        if (((this->work[CS_TIMER_2] % 32) == 0) && (Rand_ZeroOne() < 0.3f)) {
            this->work[BLINK_IDX] = 4;
        }

        this->eyeTexIdx = D_8094A900[this->work[BLINK_IDX]];

        if (this->work[BLINK_IDX] != 0) {
            this->work[BLINK_IDX]--;
        }

        if (this->actionFunc != BossTw_MergeCS && this->unk_5F8 != 0) {
            Vec3f pos;
            Vec3f velocity = { 0.0f, 0.0f, 0.0f };
            Vec3f accel = { 0.0f, 0.0f, 0.0f };

            if (this->scepterAlpha > 0.0f) {
                for (i = 0; i <= 0; i++) {
                    pos = this->scepterFlamePos[0];
                    pos.x += Rand_CenteredFloat(70.0f);
                    pos.y += Rand_CenteredFloat(70.0f);
                    pos.z += Rand_CenteredFloat(70.0f);
                    accel.y = 0.4f;
                    accel.x = Rand_CenteredFloat(0.5f);
                    accel.z = Rand_CenteredFloat(0.5f);
                    BossTw_AddDotEffect(play, &pos, &velocity, &accel, (s16)Rand_ZeroFloat(2.0f) + 8,
                                        this->actor.params, 37);
                }
            }

            for (i = 0; i <= 0; i++) {
                pos = this->crownPos;
                pos.x += Rand_CenteredFloat(70.0f);
                pos.y += Rand_CenteredFloat(70.0f);
                pos.z += Rand_CenteredFloat(70.0f);
                accel.y = 0.4f;
                accel.x = Rand_CenteredFloat(0.5f);
                accel.z = Rand_CenteredFloat(0.5f);
                BossTw_AddDotEffect(play, &pos, &velocity, &accel, (s16)Rand_ZeroFloat(2.0f) + 8, this->actor.params,
                                    37);
            }
        }
    }
}

static s32 BossTw_IsFusedSpinActive(BossTw* this) {
    return this->actionFunc == BossTw_TwinrovaSpin &&
           this->timers[0] > TWINROVA_FUSED_SPIN_RECOVERY_TIME &&
           this->timers[0] <= TWINROVA_FUSED_SPIN_RECOVERY_TIME + TWINROVA_SPIN_ACTIVE_TIME;
}

void BossTw_TwinrovaUpdate(Actor* thisx, PlayState* play2) {
    s16 i;
    s16 scepterParticleCount;
    s32 holdBreakerCooldown;
    s32 holdCycloneCooldown;
    s32 holdFalseChargeCooldown;
    s32 holdMinefieldCooldown;
    PlayState* play = play2;
    BossTw* this = (BossTw*)thisx;
    Player* player = GET_PLAYER(play);

    BossTw_UpdateSummonedEnemies(play);
    this->actor.flags &= ~ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER;
    this->unk_5F8 = 0;
    this->collider.base.colType = COLTYPE_HIT3;

    Math_ApproachF(&this->fogR, play->lightCtx.fogColor[0], 1.0f, 10.0f);
    Math_ApproachF(&this->fogG, play->lightCtx.fogColor[1], 1.0f, 10.0f);
    Math_ApproachF(&this->fogB, play->lightCtx.fogColor[2], 1.0f, 10.0f);
    Math_ApproachF(&this->fogNear, play->lightCtx.fogNear, 1.0f, 10.0f);
    Math_ApproachF(&this->fogFar, 1000.0f, 1.0f, 10.0f);

    this->work[CS_TIMER_1]++;
    this->work[CS_TIMER_2]++;

    // Count the Breaker cooldown only during active combat. A successful reflection can spend most of the authored
    // cooldown in its charged-release, stun, and get-up sequence, otherwise allowing another Breaker immediately
    // after the player's earned punish window.
    holdBreakerCooldown = this->twinrovaStun != 0 || BossTw_HasCommittedFusedShieldRelease(play, this) ||
                          this->actionFunc == BossTw_TwinrovaStun || this->actionFunc == BossTw_TwinrovaGetUp;
    if (this->minefieldSequenceState == TWINROVA_MINEFIELD_SEQUENCE_OVERLAP &&
        !BossTw_HasActiveMinefieldMines(play, this) && BossTw_CountActiveMinefieldPools(play, this) == 0) {
        this->minefieldSequenceState = TWINROVA_MINEFIELD_SEQUENCE_NONE;
    }
    holdMinefieldCooldown = holdBreakerCooldown ||
                            this->minefieldSequenceState != TWINROVA_MINEFIELD_SEQUENCE_NONE ||
                            BossTw_HasActiveMinefieldMines(play, this) ||
                            BossTw_CountActiveMinefieldPools(play, this) != 0;
    holdCycloneCooldown = holdBreakerCooldown || this->actionFunc == BossTw_TwinrovaCyclone;
    holdFalseChargeCooldown = holdBreakerCooldown || this->actionFunc == BossTw_TwinrovaFalseCharge ||
                              (this->actionFunc == BossTw_TwinrovaChargeBlast &&
                               this->pendingFusedAttack == TWINROVA_FUSED_ATTACK_FALSE_CHARGE);
    for (i = 0; i < 4; i++) {
        if (this->timers[i] != 0) {
            this->timers[i]--;
        }
    }
    if (sTwinrovaCooldownUpdateFrame != play->gameplayFrames) {
        // Hyper Bosses may substep neutral movement, but attack cadence is a gameplay-frame clock. Tick every special
        // cooldown exactly once so higher movement speed cannot silently erase the intended repertoire spacing.
        sTwinrovaCooldownUpdateFrame = play->gameplayFrames;
        if (this->timers[4] != 0 && !holdBreakerCooldown) {
            this->timers[4]--;
        }
        if (this->cycloneCooldown != 0 && !holdCycloneCooldown) {
            this->cycloneCooldown--;
        }
        if (this->falseChargeCooldown != 0 && !holdFalseChargeCooldown) {
            this->falseChargeCooldown--;
        }
        if (this->minefieldCooldown != 0 && !holdMinefieldCooldown) {
            this->minefieldCooldown--;
        }
    }

    if (this->work[INVINC_TIMER] != 0) {
        // AC_HIT persists until explicitly consumed. Discard contacts received during invulnerability before the
        // timer ticks so an attack held through the window cannot become buffered damage on the first vulnerable
        // update. The collider remains registered below, allowing a genuinely active strike to connect afterward.
        Collider_ResetCylinderAC(play, &this->collider.base);
        this->work[INVINC_TIMER]--;
    }

    if (this->work[FOG_TIMER] != 0) {
        this->work[FOG_TIMER]--;
    }

    this->actionFunc(this, play);

    // The close-range counter waits only for active gameplay pressure. Extinguished pools may keep fading without
    // delaying the next move, and collision registration below suppresses damage while Link cannot act.
    if (this->timers[3] == 0 &&
        (this->actionFunc == BossTw_TwinrovaFly || this->actionFunc == BossTw_TwinrovaArriveAtTarget ||
         this->actionFunc == BossTw_TwinrovaDoneBlastShoot || this->actionFunc == BossTw_TwinrovaLaugh) &&
        !BossTw_HasCommittedFusedShieldRelease(play, this) && !BossTw_HasActiveFusedDirectBlast(play, this) &&
        !BossTw_HasActiveArenaControlPattern(this, play) &&
        BossTw_CountActiveSummons(play) <= 3 && !player->bodyIsBurning &&
        this->visible && this->unk_5F8 == 0 && this->actor.xzDistToPlayer < TWINROVA_SPIN_TRIGGER_RANGE &&
        fabsf(this->actor.world.pos.y - player->actor.world.pos.y) < TWINROVA_SPIN_TRIGGER_HEIGHT &&
        (s16)(player->actor.shape.rot.y - this->actor.yawTowardsPlayer + 0x8000) < 0x1000 &&
        (s16)(player->actor.shape.rot.y - this->actor.yawTowardsPlayer + 0x8000) > -0x1000 && player->unk_A73 != 0) {
        BossTw_TwinrovaSetupSpin(this, play);
    }

    this->eyeTexIdx = D_8094A900[this->work[BLINK_IDX]];
    if (this->work[BLINK_IDX] != 0) {
        this->work[BLINK_IDX]--;
    }

    if ((this->work[CS_TIMER_2] % 32) == 0) {
        if (this->actionFunc != BossTw_TwinrovaMergeCS) {
            if (Rand_ZeroOne() < 0.3f) {
                this->work[BLINK_IDX] = 4;
            }
        }
    }

    if (this->actionFunc == BossTw_TwinrovaMergeCS) {
        this->leftEyeTexIdx = D_8094A90C[this->work[TW_BLINK_IDX]];
        if (this->work[TW_BLINK_IDX] != 0) {
            this->work[TW_BLINK_IDX]--;
        }
    } else {
        if (this->actionFunc == BossTw_TwinrovaStun) {
            this->eyeTexIdx = 1;
        }

        if (this->actionFunc == BossTw_TwinrovaDeathCS) {
            this->eyeTexIdx = 2;
        }

        this->leftEyeTexIdx = this->eyeTexIdx;
    }

    if (this->visible && this->unk_5F8 == 0) {
        Vec3f pos;
        Vec3f velocity = { 0.0f, 0.0f, 0.0f };
        Vec3f accel;

        // Leave visual space for the authored shot tells and Cyclone's rapid orbit.
        scepterParticleCount =
            this->actionFunc == BossTw_TwinrovaBreakerSequence ? 1 : 2;

        if (this->work[UNK_S8] != 0) {
            this->work[UNK_S8] -= 20;
            if (this->work[UNK_S8] < 0) {
                this->work[UNK_S8] = 0;
            }
        }

        Math_ApproachF(&this->workf[UNK_F12], 1.0f, 1.0f, 0.05f);
        accel.y = 0.4f;

        for (i = 0; i < scepterParticleCount; i++) {
            pos = this->leftScepterPos;
            pos.x += Rand_CenteredFloat(30.0f);
            pos.y += Rand_CenteredFloat(30.0f);
            pos.z += Rand_CenteredFloat(30.0f);
            accel.x = Rand_CenteredFloat(0.5f);
            accel.z = Rand_CenteredFloat(0.5f);
            BossTw_AddDotEffect(play, &pos, &velocity, &accel, (s16)Rand_ZeroFloat(2.0f) + 7, 0, 75);
        }

        for (i = 0; i < scepterParticleCount; i++) {
            pos = this->rightScepterPos;
            pos.x += Rand_CenteredFloat(30.0f);
            pos.y += Rand_CenteredFloat(30.0f);
            pos.z += Rand_CenteredFloat(30.0f);
            accel.x = Rand_CenteredFloat(0.5f);
            accel.z = Rand_CenteredFloat(0.5f);
            BossTw_AddDotEffect(play, &pos, &velocity, &accel, (s16)Rand_ZeroFloat(2.0f) + 7, 1, 75);
        }
    }

    this->collider.dim.radius = 35;

    if (BossTw_IsFusedSpinActive(this)) {
        this->collider.dim.radius *= 2;
        this->collider.dim.height = TWINROVA_SPIN_COLLIDER_HEIGHT;
        this->collider.dim.yShift = TWINROVA_SPIN_COLLIDER_Y_SHIFT;
    } else {
        this->collider.dim.height = 150;
        this->collider.dim.yShift = -60;
    }
    Collider_UpdateCylinder(&this->actor, &this->collider);

    if (this->work[INVINC_TIMER] == 0) {
        if (this->actionFunc != BossTw_TwinrovaStun) {
            if (this->twinrovaStun != 0) {
                this->twinrovaStun = 0;
                this->work[FOG_TIMER] = 10;
                BossTw_TwinrovaDamage(this, play, 0);
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_YOUNG_DAMAGE);
            } else if (this->collider.base.acFlags & AC_HIT) {
                ColliderInfo* info = this->collider.info.acHitInfo;

                this->collider.base.acFlags &= ~AC_HIT;
                if (info->toucher.dmgFlags & (DMG_SLINGSHOT | DMG_ARROW)) {}
            }
        } else if (this->collider.base.acFlags & AC_HIT) {
            u8 damage;
            u8 swordDamage;
            ColliderInfo* info = this->collider.info.acHitInfo;

            this->collider.base.acFlags &= ~AC_HIT;
            if (this->csState2 >= TWINROVA_STUN_DAMAGE_BUDGET) {
                // The punish budget can be exhausted by ranged damage before landing. Consume further contacts
                // without replaying a zero-damage hurt animation or extending invulnerability during the fall.
                if (!(info->toucher.dmgFlags & DMG_HOOKSHOT)) {
                    Audio_PlayActorSound2(&this->actor, NA_SE_IT_SHIELD_BOUND);
                }
            } else {
                swordDamage = false;
                damage = CollisionCheck_GetSwordDamage(info->toucher.dmgFlags, play);

                if (damage == 0) {
                    damage = 2;
                } else {
                    swordDamage = true;
                }

                if (!(info->toucher.dmgFlags & DMG_HOOKSHOT)) {
                    if (((s8)this->actor.colChkInfo.health < 3) && !swordDamage) {
                        // The last blow requires a committed melee finisher. Reject lesser damage without consuming
                        // invulnerability or playing the false hurt reaction that would steal the earned stun window.
                        Audio_PlayActorSound2(&this->actor, NA_SE_IT_SHIELD_BOUND);
                    } else {
                        BossTw_TwinrovaDamage(this, play, damage);
                    }
                }
            }
        }
    }

    CollisionCheck_SetAC(play, &play->colChkCtx, &this->collider.base);
    osSyncPrintf("OooooooooooooooooooooooooooooooooCC\n");
    CollisionCheck_SetOC(play, &play->colChkCtx, &this->collider.base);

    play->envCtx.unk_DC = 2;

    if (sTwinrovaSharedUpdateFrame != play->gameplayFrames) {
        // Hyper Bosses may substep the hidden phase-one controller and fused neutral movement. The shared effect
        // pool, shield UI, and elemental lighting are frame clocks, not boss motion, and advance only once here.
        sTwinrovaSharedUpdateFrame = play->gameplayFrames;
        if (sMinefieldLightPulseTimer != 0) {
            f32 pulseTarget = 1.0f;

            // The final mine owns a brief brightness pulse, while any simultaneously committed normal shot keeps its
            // current color identity. This prevents a fire detonation from visually lying over an active ice tell (or
            // vice versa) now that ordinary pressure may resume before every mine has resolved.
            if (sEnvType == 1 || sEnvType == 3) {
                play->envCtx.unk_BD = 3;
            } else if (sEnvType == 2 || sEnvType == 4) {
                play->envCtx.unk_BD = 2;
            } else {
                play->envCtx.unk_BD = sMinefieldLightPulseElement == TWINROVA_MAGIC_FIRE ? 2 : 3;
            }
            if (sMinefieldLightPulseTimer <= TWINROVA_MINEFIELD_FINAL_LIGHT_PULSE_PEAK_TIME) {
                // Own the fade as well as the flash; falling back to the room's slow neutral decay would turn a
                // six-update accent into a lingering full-arena tint.
                if (sEnvType == 0) {
                    pulseTarget = 0.0f;
                } else if (sEnvType == 1 || sEnvType == 2) {
                    pulseTarget = 0.5f;
                }
            }
            Math_ApproachF(&play->envCtx.unk_D8, pulseTarget, 1.0f, 0.2f);
            sMinefieldLightPulseTimer--;
        } else {
            switch (sEnvType) {
                case 0:
                    Math_ApproachZeroF(&play->envCtx.unk_D8, 1.0f, 0.02f);
                    break;
                case 1:
                    play->envCtx.unk_BD = 3;
                    Math_ApproachF(&play->envCtx.unk_D8, 0.5f, 1.0f, 0.05f);
                    break;
                case 2:
                    play->envCtx.unk_BD = 2;
                    Math_ApproachF(&play->envCtx.unk_D8,
                                   (Math_SinS((s16)play->gameplayFrames * 0x3000) * 0.03f) + 0.5f, 1.0f, 0.05f);
                    break;
                case 3:
                    play->envCtx.unk_BD = 3;
                    Math_ApproachF(&play->envCtx.unk_D8, 1.0f, 1.0f, 0.1f);
                    break;
                case 4:
                    play->envCtx.unk_BD = 2;
                    Math_ApproachF(&play->envCtx.unk_D8,
                                   (Math_SinS((s16)play->gameplayFrames * 0x3E00) * 0.05f) + 0.95f, 1.0f, 0.1f);
                    break;
                case 5:
                    play->envCtx.unk_BD = 0;
                    Math_ApproachF(&play->envCtx.unk_D8, 1.0f, 1.0f, 0.05f);
                    break;
                case -1:
                    break;
            }
        }
        BossTw_UpdateEffects(play);
        BossTw_UpdateShieldChargeState();
    }
    if (!BossTw_IsPlayerHardDisabled(play) && this->twinrovaStun == 0 && BossTw_IsFusedSpinActive(this)) {
        CollisionCheck_SetAT(play, &play->colChkCtx, &this->collider.base);
    } else {
        Collider_ResetCylinderAT(play, &this->collider.base);
    }

    if (sFreezeState == 1) {
        if (BossTw_AddPlayerFreezeEffect(play, NULL)) {
            sFreezeState = 2;
            Sfx_PlaySfxAtPos(&player->actor.projectedPos, NA_SE_VO_LI_FREEZE);
            Sfx_PlaySfxAtPos(&player->actor.projectedPos, NA_SE_PL_FREEZE);

            if (sShieldFireCharge != 0) {
                BossTw_DisruptShieldCharge(play, TWINROVA_MAGIC_ICE);
            }
        } else {
            // Retry on a later hit instead of latching an invisible permanent freeze state.
            sFreezeState = 0;
        }
    }

}

s32 BossTw_OverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    BossTw* this = (BossTw*)thisx;

    if (limbIndex == 21) {
        if (this->unk_5F8 == 0) {
            if (this->actor.params == 0) {
                *dList = gTwinrovaKotakeHeadDL;
            } else {
                *dList = gTwinrovaKoumeHeadDL;
            }
        }
    }

    if (limbIndex == 14) {
        if (this->actionFunc == BossTw_DeathCS) {
            *dList = NULL;
        } else if (this->scepterAlpha == 0.0f) {
            if (this->actor.params == 0) {
                *dList = gTwinrovaKotakeBroomDL;
            } else {
                *dList = gTwinrovaKoumeBroomDL;
            }
        }
    }

    return false;
}

void BossTw_PostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    static Vec3f D_8094A944 = { 0.0f, 0.0f, 0.0f };
    static Vec3f D_8094A950 = { 0.0f, 2000.0f, -2000.0f };
    static Vec3f D_8094A95C[] = {
        { 0.0f, 0.0f, -10000.0f }, { 0.0f, 0.0f, -8000.0f },  { 0.0f, 0.0f, -9000.0f },
        { 0.0f, 0.0f, -11000.0f }, { 0.0f, 0.0f, -12000.0f },
    };
    BossTw* this = (BossTw*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    switch (limbIndex) {
        case 21:
            Matrix_MultVec3f(&D_8094A944, &this->actor.focus.pos);
            Matrix_MultVec3f(&D_8094A950, &this->crownPos);

            if (this->unk_5F8 != 0) {
                gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                          G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
                if (this->actor.params == 0) {
                    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeIceHairDL));
                } else {
                    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKoumeFireHairDL));
                }
            }
            break;
        case 14:
            Matrix_MultVec3f(&D_8094A95C[0], &this->scepterFlamePos[0]);
            Matrix_MultVec3f(&D_8094A95C[1], &this->scepterFlamePos[1]);
            Matrix_MultVec3f(&D_8094A95C[2], &this->scepterFlamePos[2]);
            Matrix_MultVec3f(&D_8094A95C[3], &this->scepterFlamePos[3]);
            Matrix_MultVec3f(&D_8094A95C[4], &this->scepterFlamePos[4]);

            if (this->scepterAlpha > 0.0f) {
                gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                          G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
                if (this->actor.params == 0) {
                    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 225, 255, (s16)this->scepterAlpha);
                    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeIceBroomHeadDL));
                    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, (s16)this->scepterAlpha);
                    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeIceBroomHeadOuterDL));
                } else {
                    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 100, 20, 0, (s16)this->scepterAlpha);
                    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeFireBroomHeadDL));
                    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 70, 0, (s16)this->scepterAlpha);
                    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeFireBroomHeadOuterDL));
                }
            }
            break;
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void func_80941BC0(BossTw* this, PlayState* play) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Push();
    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    Matrix_Translate(this->groundBlastPos2.x, this->groundBlastPos2.y, this->groundBlastPos2.z, MTXMODE_NEW);
    Matrix_Scale(this->workf[UNK_F12] * (TWINROVA_ICE_POOL_DAMAGE_RADIUS / TWINROVA_ICE_POOL_MESH_INRADIUS),
                 this->workf[UNK_F12] * (TWINROVA_ICE_POOL_DAMAGE_RADIUS / TWINROVA_ICE_POOL_MESH_INRADIUS),
                 this->workf[UNK_F12] * (TWINROVA_ICE_POOL_DAMAGE_RADIUS / TWINROVA_ICE_POOL_MESH_INRADIUS),
                 MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, (s16)this->workf[UNK_F11]);
    gDPSetEnvColor(POLY_XLU_DISP++, 0, 40, 30, 80);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIcePoolDL));
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 215, 215, 215, (s16)this->workf[UNK_F11] * this->workf[UNK_F14]);
    gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, 128);
    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 0x20, 0x40, 1, (u32)this->workf[UNK_F16] & 0x3F,
                                  (this->work[CS_TIMER_2] * 4) & 0x3F, 0x10, 0x10, 0, 0, 5, 4));
    Matrix_Push();
    Matrix_RotateY(this->workf[UNK_F15], MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIcePoolShineDL));
    Matrix_Pop();
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPSegment(POLY_XLU_DISP++, 0xD,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, this->work[CS_TIMER_2] & 0x7F,
                                  (this->work[CS_TIMER_2] * 8) & 0xFF, 0x20, 0x40, 1,
                                  (-this->work[CS_TIMER_2] * 2) & 0x3F, 0, 0x10, 0x10, 1, 8, 2, 0));
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, (s16)this->workf[UNK_F9]);
    gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, 128);
    gDPSetRenderMode(POLY_XLU_DISP++,
                     Z_CMP | IM_RD | CVG_DST_SAVE | ZMODE_DEC | FORCE_BL |
                         GBL_c1(G_BL_CLR_FOG, G_BL_A_SHADE, G_BL_CLR_IN, G_BL_1MA),
                     G_RM_ZB_OVL_SURF2);
    gSPSetGeometryMode(POLY_XLU_DISP++, G_CULL_BACK | G_FOG);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaEffectHaloDL));
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

void func_80942180(BossTw* this, PlayState* play) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Push();

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    Matrix_Translate(this->groundBlastPos2.x, this->groundBlastPos2.y, this->groundBlastPos2.z, MTXMODE_NEW);
    Matrix_Push();
    Matrix_Scale(
        this->workf[KM_GD_CRTR_SCL] * (TWINROVA_FIRE_POOL_DAMAGE_RADIUS / TWINROVA_FIRE_POOL_MESH_INRADIUS),
        this->workf[KM_GD_CRTR_SCL] * (TWINROVA_FIRE_POOL_DAMAGE_RADIUS / TWINROVA_FIRE_POOL_MESH_INRADIUS),
        this->workf[KM_GD_CRTR_SCL] * (TWINROVA_FIRE_POOL_DAMAGE_RADIUS / TWINROVA_FIRE_POOL_MESH_INRADIUS),
        MTXMODE_APPLY);
    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (-this->work[CS_TIMER_1]) & 0x7F, 0, 0x20, 0x20, 1,
                                  (this->work[CS_TIMER_1] * 2) & 0x7F, 0, 0x20, 0x20, -1, 0, 2, 0));
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 100, 40, 00, (s16)this->workf[KM_GRND_CRTR_A]);
    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 255, 245, 255, 128);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaFirePoolDL));
    Matrix_Pop();

    Matrix_Scale(this->workf[KM_GD_CRTR_SCL], this->workf[KM_GD_CRTR_SCL], this->workf[KM_GD_CRTR_SCL], MTXMODE_APPLY);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, this->work[CS_TIMER_1] & 0x7F,
                                  (-this->work[CS_TIMER_1] * 6) & 0xFF, 0x20, 0x40, 1,
                                  (this->work[CS_TIMER_1] * 2) & 0x7F, (-this->work[CS_TIMER_1] * 6) & 0xFF, 0x20, 0x40,
                                  1, -6, 2, -6));
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 80, 0, 0, (s16)this->workf[KM_GD_SMOKE_A]);
    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 0, 0, 0, 100);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaFireSmokeDL));

    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (-this->work[CS_TIMER_1] * 3) & 0x7F, 0, 0x20, 0x20, 1, 0,
                                  (-this->work[CS_TIMER_1] * 10) & 0xFF, 0x20, 0x40, -3, 0, 0, -10));
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 100, 50, 0, (s16)(this->workf[KM_GD_FLM_A] * 0.7f));
    gDPPipeSync(POLY_XLU_DISP++);
    gDPSetEnvColor(POLY_XLU_DISP++, 200, 235, 240, 128);
    Matrix_Scale(this->workf[KM_GD_FLM_SCL], this->workf[KM_GD_FLM_SCL], this->workf[KM_GD_FLM_SCL], MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaBigFlameDL));

    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

void func_809426F0(BossTw* this, PlayState* play) {
    s32 pad;
    s16 i;

    OPEN_DISPS(play->state.gfxCtx);

    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (u8)(-this->work[CS_TIMER_2] * 15), 0x20, 0x40, 1, 0, 0,
                                  0x40, 0x40, 0, -15, 0, 0));
    Matrix_Push();
    Matrix_Translate(0.0f, 0.0f, 5000.0f, MTXMODE_APPLY);
    Matrix_Scale(this->spawnPortalScale / 2000.0f, this->spawnPortalScale / 2000.0f, this->spawnPortalScale / 2000.0f,
                 MTXMODE_APPLY);
    Matrix_RotateZ(this->portalRotation, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);

    if (this->actor.params == 0) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 135, 175, 165, (s16)this->spawnPortalAlpha);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeMagicSigilDL));
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 0, (s16)this->spawnPortalAlpha);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKoumeMagicSigilDL));
    }

    Matrix_Pop();

    if (this->actor.params == 0) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, (s16)this->flameAlpha);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceMaterialDL));
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 200, 20, 0, (s16)this->flameAlpha);
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 215, 255, 128);
    }

    for (i = 0; i < 8; i++) {
        FrameInterpolation_RecordOpenChild("Twinrova 809426F0", i);

        Matrix_Push();
        Matrix_Translate(0.0f, 0.0f, 5000.0f, MTXMODE_APPLY);
        Matrix_RotateZ(((i * M_PI) * 2.0f * 0.125f) + this->flameRotation, MTXMODE_APPLY);
        Matrix_Translate(0.0f, this->spawnPortalScale * 1.5f, 0.0f, MTXMODE_APPLY);
        gSPSegment(POLY_XLU_DISP++, 8,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, ((this->work[CS_TIMER_2] * 3) + (i * 10)) & 0x7F,
                                      (u8)((-this->work[CS_TIMER_2] * 15) + (i * 50)), 0x20, 0x40, 1, 0, 0, 0x20, 0x20,
                                      3, -15, 0, 0));
        Matrix_Scale(0.4f, 0.4f, 0.4f, MTXMODE_APPLY);
        Matrix_ReplaceRotation(&play->billboardMtxF);
        gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaFireDL));
        Matrix_Pop();

        FrameInterpolation_RecordCloseChild();
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void func_80942C70(Actor* thisx, PlayState* play) {
    BossTw* this = (BossTw*)thisx;
    s16 alpha;

    OPEN_DISPS(play->state.gfxCtx);

    if (this->beamDist != 0.0f) {
        Matrix_Push();
        gSPSegment(POLY_XLU_DISP++, 0xC,
                   Gfx_TexScrollEx(play->state.gfxCtx, 0, (u8)(this->work[CS_TIMER_1] * -0xF), 0x20, 0x40, 0, -0xF));
        alpha = this->beamScale * 100.0f * 255.0f;

        if (this->actor.params == 1) {
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 60, alpha);
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 0, 0, 128);
        } else {
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, alpha);
            gDPSetEnvColor(POLY_XLU_DISP++, 100, 100, 255, 128);
        }

        Matrix_Translate(this->beamOrigin.x, this->beamOrigin.y, this->beamOrigin.z, MTXMODE_NEW);
        Matrix_RotateY(this->beamYaw, MTXMODE_APPLY);
        Matrix_RotateX(this->beamPitch, MTXMODE_APPLY);
        Matrix_RotateZ(this->beamRoll, MTXMODE_APPLY);
        Matrix_Scale(this->beamScale, this->beamScale, (this->beamDist * 0.01f * 98.0f) / 20000.0f, MTXMODE_APPLY);
        gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaBeamDL));

        if (this->beamReflectionDist > 10.0f) {
            Matrix_Translate(this->beamReflectionOrigin.x, this->beamReflectionOrigin.y, this->beamReflectionOrigin.z,
                             MTXMODE_NEW);
            Matrix_RotateY(this->beamReflectionYaw, MTXMODE_APPLY);
            Matrix_RotateX(this->beamReflectionPitch, MTXMODE_APPLY);
            Matrix_RotateZ(this->beamRoll, MTXMODE_APPLY);
            Matrix_Scale(this->beamScale, this->beamScale, (this->beamReflectionDist * 0.01f * 100.0f) / 20000.0f,
                         MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
            gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaBeamDL));
        }

        Matrix_Pop();
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void func_80943028(Actor* thisx, PlayState* play) {
    BossTw* this = (BossTw*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Push();
    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y + 57.0f, this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_Scale(this->workf[UNK_F17], this->workf[UNK_F17], this->workf[UNK_F17], MTXMODE_APPLY);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, 255);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaHaloDL));
    Gfx_SetupDL_44Xlu(play->state.gfxCtx);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 0, 0, 0, 200);
    Matrix_Translate(this->actor.world.pos.x, 240.0f, this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_Scale((this->actor.scale.x * 4000.0f) / 100.0f, 1.0f, (this->actor.scale.x * 4000.0f) / 100.0f,
                 MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gCircleShadowDL));
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void* sEyeTextures[] = {
    gTwinrovaKotakeKoumeEyeOpenTex,
    gTwinrovaKotakeKoumeEyeHalfTex,
    gTwinrovaKotakeKoumeEyeClosedTex,
};

static void BossTw_ElementalPortalDraw(BossTw* this, PlayState* play) {
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, -this->work[CS_TIMER_1] * 15, 0x20, 0x40, 1, 0, 0,
                                  0x40, 0x40, 0, -15, 0, 0));
    Matrix_Push();
    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_RotateZ(this->portalRotation, MTXMODE_APPLY);
    Matrix_Scale(this->workf[UNK_F17], this->workf[UNK_F17], this->workf[UNK_F17], MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);

    if (this->actor.params == TW_KOTAKE) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 135, 175, 255, (s16)this->workf[UNK_F18]);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeMagicSigilDL));
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 120, 0, (s16)this->workf[UNK_F18]);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKoumeMagicSigilDL));
    }

    Matrix_Pop();
    CLOSE_DISPS(play->state.gfxCtx);
}

void BossTw_Draw(Actor* thisx, PlayState* play2) {
    static Vec3f D_8094A9A4 = { 0.0f, 200.0f, 2000.0f };
    PlayState* play = play2;
    BossTw* this = (BossTw*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    if (this->visible) {
        gSPSegment(POLY_OPA_DISP++, 10, SEGMENTED_TO_VIRTUAL(sEyeTextures[this->eyeTexIdx]));
        gSPSegment(POLY_XLU_DISP++, 10, SEGMENTED_TO_VIRTUAL(sEyeTextures[this->eyeTexIdx]));
        gSPSegment(POLY_XLU_DISP++, 8,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (s16)this->workf[OUTR_CRWN_TX_X1] & 0x7F,
                                      (s16)this->workf[OUTR_CRWN_TX_Y1] & 0x7F, 0x20, 0x20, 1,
                                      (s16)this->workf[OUTR_CRWN_TX_X2] & 0x7F,
                                      (s16)this->workf[OUTR_CRWN_TX_Y2] & 0xFF, 0x20, 0x40, 0, 0,
                                      this->actor.params == 0 ? 1 : 0, this->actor.params == 0 ? -7 : -15));

        if (this->actor.params == TW_KOTAKE) {
            gSPSegment(POLY_XLU_DISP++, 9,
                       Gfx_TexScrollEx(play->state.gfxCtx, (s16)this->workf[INNR_CRWN_TX_X1] & 0x7F,
                                       (s16)this->workf[INNR_CRWN_TX_Y1] & 0xFF, 0x20, 0x40, 0,
                                       this->actor.params == 0 ? 1 : 0));
        } else {
            gSPSegment(POLY_XLU_DISP++, 9,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (s16)this->workf[INNR_CRWN_TX_X1] & 0x7F,
                                          (s16)this->workf[INNR_CRWN_TX_Y1] & 0x7F, 0x20, 0x20, 1,
                                          (s16)this->workf[INNR_CRWN_TX_X2] & 0x7F,
                                          (s16)this->workf[INNR_CRWN_TX_Y2] & 0xFF, 0x20, 0x40, 0, 0, 0,
                                          this->actor.params == 0 ? 0 : -10));
        }

        Gfx_SetupDL_25Opa(play->state.gfxCtx);
        Gfx_SetupDL_25Xlu(play->state.gfxCtx);

        if (this->work[FOG_TIMER] & 2) {
            POLY_OPA_DISP = Gfx_SetFog(POLY_OPA_DISP, 255, 50, 0, 0, 900, 1099);
        } else {
            POLY_OPA_DISP = Gfx_SetFog(POLY_OPA_DISP, (u32)this->fogR, (u32)this->fogG, (u32)this->fogB, 0,
                                       this->fogNear, this->fogFar);
        }

        Matrix_Push();
        SkelAnime_DrawSkeletonOpa(play, &this->skelAnime, BossTw_OverrideLimbDraw, BossTw_PostLimbDraw, this);
        Matrix_Pop();
        POLY_OPA_DISP = Play_SetFog(play, POLY_OPA_DISP);
    }

    if (this->actor.params == TW_KOTAKE) {
        if (this->workf[UNK_F9] > 0.0f) {
            func_80941BC0(this, play);
        }
    } else {
        func_80942180(this, play);
    }

    if (this->visible) {
        if (this->actionFunc == BossTw_DeathCS) {
            func_80943028(&this->actor, play);
        } else {
            func_809426F0(this, play);
            Matrix_MultVec3f(&D_8094A9A4, &this->beamOrigin);
            func_80942C70(&this->actor, play);
        }
    }

    if (this->actionFunc == BossTw_PortalReposition && this->workf[UNK_F18] > 0.0f) {
        BossTw_ElementalPortalDraw(this, play);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void* D_8094A9B0[] = {
    gTwinrovaEyeOpenTex,
    gTwinrovaEyeHalfTex,
    gTwinrovaEyeClosedTex,
};

s32 BossTw_TwinrovaOverrideLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3f* pos, Vec3s* rot, void* thisx) {
    BossTw* this = (BossTw*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    switch (limbIndex) {
        case 21:
            gSPSegment(POLY_OPA_DISP++, 0xC,
                       Gfx_TexScrollEx(play->state.gfxCtx, 0, (s16)(f32)this->work[CS_TIMER_1], 8, 8, 0, 1));
            gSPSegment(POLY_OPA_DISP++, 8, SEGMENTED_TO_VIRTUAL(D_8094A9B0[this->eyeTexIdx]));
            gSPSegment(POLY_OPA_DISP++, 9, SEGMENTED_TO_VIRTUAL(D_8094A9B0[this->leftEyeTexIdx]));
            gDPSetEnvColor(POLY_OPA_DISP++, 255, 255, 255, this->work[UNK_S8]);
            break;
        case 17:
        case 41:
            *dList = NULL;
            gSPSegment(POLY_XLU_DISP++, 0xA,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 0x20, 0x20, 1, 0, -this->work[CS_TIMER_1] * 0xF,
                                          0x20, 0x40, 0, 0, 0, -0xF));
            break;
        case 18:
        case 42:
            *dList = NULL;
            gSPSegment(POLY_XLU_DISP++, 0xB,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 0x20, 0x20, 1, 0, -this->work[CS_TIMER_1] * 0xA,
                                          0x20, 0x40, 0, 0, 0, -0xA));
            break;
        case 16:
        case 32:
            *dList = NULL;
            gSPSegment(POLY_XLU_DISP++, 8,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 0x20, 0x20, 1, this->work[CS_TIMER_1],
                                          -this->work[CS_TIMER_1] * 7, 0x20, 0x40, 0, 0, 1, -7));
            break;
        case 15:
        case 31:
            *dList = NULL;
            gSPSegment(POLY_XLU_DISP++, 9,
                       Gfx_TexScrollEx(play->state.gfxCtx, 0, this->work[CS_TIMER_1], 0x20, 0x40, 0, 1));
            break;
        case 19:
            if (this->unk_5F8 != 0) {
                *dList = gTwinrovaLeftHairBunDL;
            }
            break;

        case 20:
            if (this->unk_5F8 != 0) {
                *dList = gTwinrovaRightHairBunDL;
            }
            break;
    }

    if (this->unk_5F8 != 0 && ((limbIndex == 34) || (limbIndex == 40))) {
        *dList = NULL;
    }

    CLOSE_DISPS(play->state.gfxCtx);

    return false;
}

void BossTw_TwinrovaPostLimbDraw(PlayState* play, s32 limbIndex, Gfx** dList, Vec3s* rot, void* thisx) {
    static Vec3f D_8094A9BC = { 0.0f, 0.0f, 0.0f };
    static Vec3f D_8094A9C8 = { 0.0f, 2000.0f, -2000.0f };
    static Vec3f D_8094A9D4 = { 13000.0f, 0.0f, 0.0f };
    static Vec3f D_8094A9E0 = { 13000.0f, 0.0f, 0.0f };

    BossTw* this = (BossTw*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    switch (limbIndex) {
        case 34:
            Matrix_MultVec3f(&D_8094A9D4, &this->leftScepterPos);
            break;
        case 40:
            Matrix_MultVec3f(&D_8094A9E0, &this->rightScepterPos);
            break;
        case 21:
            Matrix_MultVec3f(&D_8094A9BC, &this->actor.focus.pos);
            Matrix_MultVec3f(&D_8094A9C8, &this->crownPos);
            break;
        case 15:
        case 16:
        case 17:
        case 18:
        case 31:
        case 32:
        case 41:
        case 42:
            Matrix_Push();
            Matrix_Scale(this->workf[UNK_F12], this->workf[UNK_F12], this->workf[UNK_F12], MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
            Matrix_Pop();
            gSPDisplayList(POLY_XLU_DISP++, *dList);
            break;
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void BossTw_ShieldChargeDraw(BossTw* this, PlayState* play) {
    s32 pad;
    Player* player = GET_PLAYER(play);
    s16 temp_t0;
    s16 temp_a0;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Push();

    temp_t0 = sShieldFireCharge | sShieldIceCharge;

    if (temp_t0 == 3) {
        temp_t0 *= 3;
    } else if (temp_t0 >= 4) {
        temp_t0 = 1;
    }

    if (Player_HasMirrorShieldEquipped(play)) {
        if (temp_t0 != 0) {
            Matrix_Mult(&player->shieldMf, MTXMODE_NEW);
            Matrix_RotateX(M_PI / 2.0f, MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
            temp_a0 = (Math_SinS(this->work[CS_TIMER_1] * 2730 * temp_t0) * D_8094C854 * 0.5f) + (D_8094C854 * 0.5f);
            if (sShieldFireCharge != 0) {
                gDPSetEnvColor(POLY_XLU_DISP++, 255, 245, 255, temp_a0);
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaMirrorShieldFireChargeSidesDL));
                gSPSegment(POLY_XLU_DISP++, 8,
                           Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (this->work[CS_TIMER_1] * 2) * temp_t0, 0, 0x20,
                                              0x20, 1, (-this->work[CS_TIMER_1] * 2) * temp_t0, 0, 0x20, 0x20, 0,
                                              2 * temp_t0, -2 * temp_t0, 0));
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 100, 20, 0, (s16)D_8094C854);
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaMirrorShieldFireChargeCenterDL));
            } else {
                gDPSetEnvColor(POLY_XLU_DISP++, 225, 255, 255, temp_a0);
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaMirrorShieldIceChargeSidesDL));
                gSPSegment(POLY_XLU_DISP++, 8,
                           Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, (-this->work[CS_TIMER_1] * 5) * temp_t0, 0x20,
                                              0x40, 1, (this->work[CS_TIMER_1] * 4) * temp_t0, 0, 0x20, 0x200, 0,
                                              -5 * temp_t0, 4 * temp_t0, 0));
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 175, 205, 195, (s16)D_8094C854);
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaMirrorShieldIceChargeCenterDL));
            }
        }
    }

    if (Player_HasMirrorShieldEquipped(play) && D_8094C858 > 0.0f) {
        f32 scale = D_8094C872 > 0 ? 1.3f : 1.0f;

        Matrix_Mult(&player->shieldMf, MTXMODE_NEW);
        Matrix_RotateX(M_PI / 2.0f, MTXMODE_APPLY);
        Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
        gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
        if (sShieldFireCharge != 0) {
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 220, 20, (s16)D_8094C858);
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 0, 20, 110);
        } else {
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, (s16)D_8094C858);
            gDPSetEnvColor(POLY_XLU_DISP++, 185, 225, 205, 150);
        }

        gSPSegment(POLY_XLU_DISP++, 8,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, this->work[CS_TIMER_1] * D_8094C872, 0x20, 0x40, 1, 0,
                                      this->work[CS_TIMER_1] * D_8094C872, 0x20, 0x20, 0, D_8094C872, 0, D_8094C872));
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaShieldAbsorbAndReflectEffectDL));
    }

    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

void BossTw_SpawnPortalDraw(BossTw* this, PlayState* play) {
    s32 pad;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, -this->work[CS_TIMER_1] * 15, 0x20, 0x40, 1, 0, 0, 0x40,
                                  0x40, 0, -15, 0, 0));

    Matrix_Push();

    Matrix_Translate(0.0f, 232.0f, -600.0f, MTXMODE_NEW);
    Matrix_Scale(this->spawnPortalScale, this->spawnPortalScale, this->spawnPortalScale, MTXMODE_APPLY);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 0, 0, 0, (s16)this->spawnPortalAlpha);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaSpawnPortalShadowDL));

    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 135, 175, 165, (s16)this->spawnPortalAlpha);
    Matrix_Translate(0.0f, 2.0f, 0.0f, MTXMODE_APPLY);
    Matrix_RotateX(M_PI / 2.0f, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeMagicSigilDL));

    Matrix_Translate(0.0f, 232.0f, 600.0f, MTXMODE_NEW);
    Matrix_Scale(this->spawnPortalScale, this->spawnPortalScale, this->spawnPortalScale, MTXMODE_APPLY);
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 0, 0, 0, (s16)this->spawnPortalAlpha);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaSpawnPortalShadowDL));

    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 0, (s16)this->spawnPortalAlpha);
    Matrix_Translate(0.0f, 2.0f, 0.0f, MTXMODE_APPLY);
    Matrix_RotateX(M_PI / 2.0f, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKoumeMagicSigilDL));

    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

void func_80944C50(BossTw* this, PlayState* play) {
    s32 pad;
    f32 scale;

    OPEN_DISPS(play->state.gfxCtx);

    Matrix_Push();
    Matrix_Translate(0.0f, 750.0f, 0.0f, MTXMODE_NEW);
    Matrix_Scale(0.35f, 0.35f, 0.35f, MTXMODE_APPLY);
    Matrix_Push();
    Matrix_Scale(this->workf[UNK_F19], this->workf[UNK_F19], this->workf[UNK_F19], MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaLightCircleDL));

    Matrix_Pop();
    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, -sKoumePtr->work[CS_TIMER_1] * 2, 0, 0x20, 0x20, 1,
                                  -sKoumePtr->work[CS_TIMER_1] * 2, 0, 0x20, 0x40, -2, 0, -2, 0));
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, (s16)this->workf[UNK_F18] / 2);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaLightRaysDL));

    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, -sKoumePtr->work[CS_TIMER_1] * 5,
                                  -sKoumePtr->work[CS_TIMER_1] * 2, 0x20, 0x40, 1, 0, -sKoumePtr->work[CS_TIMER_1] * 2,
                                  0x10, 0x10, -5, -2, 0, -2));
    gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 255, 255, (s16)(this->workf[UNK_F18] * 0.3f));

    scale = this->workf[UNK_F18] / 150.0f;
    scale = CLAMP_MAX(scale, 1.0f);

    Matrix_Scale(scale, 1.0f, scale, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaLightPillarDL));
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void BossTw_DrawFusedAttackPortalSigil(BossTw* this, PlayState* play) {
    TwinrovaMagicElement element = (TwinrovaMagicElement)sTwinrovaBlastType;
    f32 pulse = (Math_SinS(this->work[CS_TIMER_1] * 0x1800) * 0.5f) + 0.5f;
    f32 scale = 0.055f + (pulse * 0.015f);
    s16 alpha = 205 + (s16)(pulse * 50.0f);

    // This is only a compact launch-origin marker. Keep the arena-scale rune language exclusive to Cyclone's
    // overhead storm portal so a lower-route shot can never read as a second signature attack behind Twinrova.
    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, -this->work[CS_TIMER_1] * 15, 0x20, 0x40, 1, 0, 0,
                                  0x40, 0x40, 0, -15, 0, 0));
    Matrix_Push();
    Matrix_Translate(this->attackPortalPos.x, this->attackPortalPos.y, this->attackPortalPos.z, MTXMODE_NEW);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_RotateZ(this->work[CS_TIMER_1] * 0.08f, MTXMODE_APPLY);
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    if (element == TWINROVA_MAGIC_ICE) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 150, 220, 255, alpha);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeMagicSigilDL));
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 145, 0, alpha);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKoumeMagicSigilDL));
    }
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void BossTw_DrawCycloneStormSigil(BossTw* this, PlayState* play) {
    f32 scale;
    f32 alphaScale;
    f32 pulse = (Math_SinS(this->work[CS_TIMER_1] * 0x1000) * 0.5f) + 0.5f;
    s16 alpha;

    if (this->csState1 == 0) {
        alphaScale = 1.0f - (this->timers[0] / (f32)TWINROVA_CYCLONE_WINDUP_TIME);
        scale = TWINROVA_CYCLONE_SIGIL_SCALE * (0.25f + (alphaScale * 0.75f));
    } else if (this->csState1 == 1) {
        alphaScale = 0.88f + (pulse * 0.12f);
        scale = TWINROVA_CYCLONE_SIGIL_SCALE * (0.96f + (pulse * 0.08f));
    } else {
        alphaScale = this->timers[0] / (f32)TWINROVA_CYCLONE_RECOVERY_TIME;
        scale = TWINROVA_CYCLONE_SIGIL_SCALE * (1.0f + ((1.0f - alphaScale) * 0.12f));
    }
    alphaScale = CLAMP(alphaScale, 0.0f, 1.0f);
    alpha = (s16)(255.0f * alphaScale);

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, -this->work[CS_TIMER_1] * 12, 0x20, 0x40, 1, 0,
                                  this->work[CS_TIMER_1] * 3, 0x40, 0x40, 0, -12, 0, 3));
    gSPClearGeometryMode(POLY_XLU_DISP++, G_CULL_BACK);
    Matrix_Push();
    Matrix_Translate(0.0f, TWINROVA_CYCLONE_SIGIL_HEIGHT, 0.0f, MTXMODE_NEW);
    Matrix_RotateY(this->work[CS_TIMER_1] * this->csState2 * 0.025f, MTXMODE_APPLY);
    Matrix_RotateX(-M_PI / 2.0f, MTXMODE_APPLY);
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
    if (this->blastType == TWINROVA_MAGIC_ICE) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 160, 230, 255, alpha);
        gDPSetEnvColor(POLY_XLU_DISP++, 30, 80, 180, 160);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeMagicSigilDL));
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 145, 20, alpha);
        gDPSetEnvColor(POLY_XLU_DISP++, 180, 25, 0, 160);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKoumeMagicSigilDL));
    }
    Matrix_Pop();
    gSPSetGeometryMode(POLY_XLU_DISP++, G_CULL_BACK);

    CLOSE_DISPS(play->state.gfxCtx);
}

void BossTw_TwinrovaDraw(Actor* thisx, PlayState* play2) {
    static Vec3f D_8094A9EC = { 0.0f, 200.0f, 2000.0f };
    PlayState* play = play2;
    BossTw* this = (BossTw*)thisx;

    OPEN_DISPS(play->state.gfxCtx);

    if (this->visible) {
        Gfx_SetupDL_25Opa(play->state.gfxCtx);
        Gfx_SetupDL_25Xlu(play->state.gfxCtx);

        POLY_OPA_DISP = (this->work[FOG_TIMER] & 2) ? Gfx_SetFog2(POLY_OPA_DISP, 255, 50, 0, 0, 900, 1099)
                                                    : Gfx_SetFog2(POLY_OPA_DISP, (u32)this->fogR, (u32)this->fogG,
                                                                  (u32)this->fogB, 0, this->fogNear, this->fogFar);

        Matrix_Push();
        SkelAnime_DrawSkeletonOpa(play, &this->skelAnime, BossTw_TwinrovaOverrideLimbDraw, BossTw_TwinrovaPostLimbDraw,
                                  thisx);
        Matrix_Pop();

        Matrix_MultVec3f(&D_8094A9EC, &this->beamOrigin);
        POLY_OPA_DISP = Gfx_SetFog2(POLY_OPA_DISP, play->lightCtx.fogColor[0], play->lightCtx.fogColor[1],
                                    play->lightCtx.fogColor[2], 0, play->lightCtx.fogNear, 1000);

        if (this->attackPortalActive) {
            BossTw_DrawFusedAttackPortalSigil(this, play);
        }
        if (this->actionFunc == BossTw_TwinrovaCyclone) {
            BossTw_DrawCycloneStormSigil(this, play);
        }
    }

    BossTw_DrawEffects(play);
    BossTw_ShieldChargeDraw(this, play);

    if (this->spawnPortalAlpha > 0.0f) {
        BossTw_SpawnPortalDraw(this, play);
    }

    if (this->workf[UNK_F18] > 0.0f) {
        func_80944C50(this, play);
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void BossTw_SiegeZoneUpdate(Actor* thisx, PlayState* play) {
    BossTw* this = (BossTw*)thisx;
    Actor* ownerActor = BossTw_FindActorByAddress(play, this->actor.parent);
    Player* player = GET_PLAYER(play);
    f32 zoneRadius = BossTw_GetSiegeZoneRadius(this);
    f32 sigilScale = BossTw_GetSiegeSigilScale(this);
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;

    if (ownerActor == NULL || ownerActor->id != ACTOR_BOSS_TW || ownerActor->update == NULL ||
        ownerActor->params != TW_TWINROVA) {
        Actor_Kill(&this->actor);
        return;
    }

    this->work[CS_TIMER_1]++;
    this->portalRotation += 0.06f;
    if (this->timers[0] != 0) {
        this->timers[0]--;
    }
    if (this->work[BURN_TMR] != 0) {
        this->work[BURN_TMR]--;
    }

    switch (this->csState1) {
        case TWINROVA_SIEGE_STATE_TELEGRAPH:
            if (this->csState2 == TWINROVA_SIEGE_PATTERN_ROTATING_WEDGE) {
                BossTw_UpdateSiegeWedgeTelegraph(this);
            }

            if (this->timers[0] <= TWINROVA_SIEGE_FINAL_WARNING_TIME) {
                this->portalRotation += 0.1f;
                Math_ApproachF(&this->workf[UNK_F17], sigilScale, 1.0f, sigilScale / 12.0f);
                Math_ApproachF(&this->workf[UNK_F18], 255.0f, 1.0f, 30.0f);
            } else {
                // Reach the authoritative boundary early, then hold it for most of the warning. The final pulse
                // uses alpha and rotation so it never advertises a false rim that becomes safe on detonation.
                Math_ApproachF(&this->workf[UNK_F17], sigilScale, 1.0f, sigilScale / 12.0f);
                Math_ApproachF(&this->workf[UNK_F18], 225.0f, 1.0f, 18.0f);
            }

            if (this->timers[0] == 45 || this->timers[0] == 25 || this->timers[0] == 10) {
                s16 warningParticles = this->timers[0] == 10 ? 8 : (this->timers[0] == 25 ? 4 : 2);

                BossTw_SpawnSiegeBurst(this, play, warningParticles, TWINROVA_SIEGE_USE_ZONE_ELEMENT);
                if (this->csState2 != TWINROVA_SIEGE_PATTERN_ROTATING_WEDGE ||
                    this->timers[0] != TWINROVA_SIEGE_WEDGE_LOCK_TIME) {
                    BossTw_PlaySiegeCue(this, play, NA_SE_EN_TWINROBA_MASIC_SET);
                }
            }
            if (this->timers[0] == 0) {
                this->csState1 = TWINROVA_SIEGE_STATE_ACTIVE;
                this->timers[0] = TWINROVA_SIEGE_ACTIVE_TIME;
                this->workf[UNK_F17] = sigilScale;
                this->workf[UNK_F18] = 255.0f;
                play->envCtx.unk_D8 = 1.0f;
                BossTw_SpawnSiegeBurst(this, play, 10, TWINROVA_SIEGE_USE_ZONE_ELEMENT);
                BossTw_PlaySiegeCue(this, play, this->blastType == TWINROVA_MAGIC_FIRE
                                                   ? NA_SE_EN_TWINROBA_FIRE_EXP
                                                   : NA_SE_EV_ICE_FREEZE);
            }
            break;

        case TWINROVA_SIEGE_STATE_ACTIVE:
            Math_ApproachF(&this->workf[UNK_F17], sigilScale, 0.2f, 0.002f);
            Math_ApproachF(&this->workf[UNK_F18], 230.0f, 1.0f, 10.0f);

            if (BossTw_IsPlayerHardDisabled(play) && this->work[BURN_TMR] < TWINROVA_FROZEN_ATTACK_GRACE) {
                // Enemy-inflicted freezes use a different player flag. Give either freeze source the same escape
                // grace so a persistent zone cannot punish Link on the first controllable update.
                this->work[BURN_TMR] = TWINROVA_FROZEN_ATTACK_GRACE;
            }

            if ((this->work[CS_TIMER_1] & 7) == 0) {
                Vec3f pos = this->actor.world.pos;
                Vec3f velocity = { 0.0f, 2.0f, 0.0f };
                Vec3f accel = { 0.0f, 0.05f, 0.0f };
                f32 radius = sqrtf(Rand_ZeroOne()) * zoneRadius;
                s16 angle = (s16)Rand_ZeroFloat(65536.0f);

                pos.x += Math_SinS(angle) * radius;
                pos.z += Math_CosS(angle) * radius;
                BossTw_AddDotEffect(play, &pos, &velocity, &accel, Rand_ZeroFloat(3.0f) + 7.0f, this->blastType,
                                    ARRAY_COUNT(sEffects));
            }

            xDiff = player->actor.world.pos.x - this->actor.world.pos.x;
            yDiff = player->actor.world.pos.y - this->actor.world.pos.y;
            zDiff = player->actor.world.pos.z - this->actor.world.pos.z;
            if (!BossTw_IsPlayerHardDisabled(play) && this->work[BURN_TMR] == 0 &&
                (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
                fabsf(yDiff) < 30.0f && (SQ(xDiff) + SQ(zDiff)) < SQ(zoneRadius)) {
                if (this->blastType == TWINROVA_MAGIC_FIRE) {
                    if (sShieldIceCharge != 0) {
                        BossTw_DisruptShieldCharge(play, TWINROVA_MAGIC_FIRE);
                    }
                    if (!player->bodyIsBurning) {
                        s16 i;

                        for (i = 0; i < ARRAY_COUNT(player->bodyFlameTimers); i++) {
                            player->bodyFlameTimers[i] = Rand_S16Offset(0, 200);
                        }
                        player->bodyIsBurning = true;
                        Player_PlaySfx(&player->actor, player->ageProperties->unk_92 + NA_SE_VO_LI_DEMO_DAMAGE);
                    }
                    this->work[BURN_TMR] = 40;
                    if (this->blastBehavior != TWINROVA_BLAST_MINEFIELD_POOL) {
                        ((BossTw*)ownerActor)->timers[2] = 100;
                    }
                } else if (this->blastType == TWINROVA_MAGIC_ICE) {
                    if (sShieldFireCharge != 0) {
                        BossTw_DisruptShieldCharge(play, TWINROVA_MAGIC_ICE);
                    }
                    // Ice Siege is also an arena reset: never carry a fire burn into its forced freeze.
                    BossTw_BeginPlayerFreeze(play);
                    if (this->blastBehavior == TWINROVA_BLAST_MINEFIELD_POOL) {
                        // A mine patch owns only its local cooldown; it must not refresh every ice Siege sibling.
                        this->work[BURN_TMR] = TWINROVA_FROZEN_ATTACK_GRACE;
                    } else {
                        // Overlapping circles are one authored pattern, not independent freeze traps. Arm every ice
                        // sibling together so thaw always leaves a real escape window before any zone can freeze again.
                        BossTw_ArmIceSiegeFreezeCooldown((BossTw*)ownerActor, play);
                    }
                    if (this->blastBehavior != TWINROVA_BLAST_MINEFIELD_POOL) {
                        ((BossTw*)ownerActor)->timers[2] = 100;
                    }
                }
            }

            if (this->timers[0] == 0) {
                if (this->blastBehavior != TWINROVA_BLAST_MINEFIELD_POOL &&
                    (((BossTw*)ownerActor)->actionFunc == BossTw_TwinrovaBreakerSequence ||
                     BossTw_HasCommittedFusedShieldRelease(play, (BossTw*)ownerActor))) {
                    // Never let the visible arena rule expire mid-pattern or while the player's charged cleanse is
                    // still travelling through it.
                    this->timers[0] = 1;
                } else {
                    this->csState1 = TWINROVA_SIEGE_STATE_FADE;
                    this->timers[0] = this->blastBehavior == TWINROVA_BLAST_MINEFIELD_POOL
                                          ? TWINROVA_MINEFIELD_POOL_FADE_TIME
                                          : TWINROVA_SIEGE_FADE_TIME;
                }
            }
            break;

        case TWINROVA_SIEGE_STATE_FADE: {
            f32 fadeTime = this->blastBehavior == TWINROVA_BLAST_MINEFIELD_POOL
                               ? TWINROVA_MINEFIELD_POOL_FADE_TIME
                               : TWINROVA_SIEGE_FADE_TIME;

            // Use the full authored fade instead of becoming transparent halfway through it.
            Math_ApproachF(&this->workf[UNK_F17], 0.0f, 1.0f, sigilScale / fadeTime);
            Math_ApproachF(&this->workf[UNK_F18], 0.0f, 1.0f, 8.0f);
            if (this->timers[0] == 0) {
                Actor_Kill(&this->actor);
            }
            break;
        }
    }
}

void BossTw_RainMarkerUpdate(Actor* thisx, PlayState* play) {
    BossTw* this = (BossTw*)thisx;
    Actor* parentActor = BossTw_FindActorByAddress(play, this->actor.parent);
    BossTw* rainBlast = (BossTw*)parentActor;
    f32 sigilScale;

    if (parentActor == NULL || parentActor->update == NULL || parentActor->id != ACTOR_BOSS_TW ||
        rainBlast->blastBehavior != TWINROVA_BLAST_RAIN ||
        (rainBlast->csState1 != 0 && rainBlast->csState1 != 1)) {
        Actor_Kill(&this->actor);
        return;
    }

    sigilScale = BossTw_IsCycloneRain(rainBlast) ? TWINROVA_CYCLONE_RAIN_SIGIL_SCALE
                                                 : TWINROVA_RAIN_SIGIL_SCALE;

    this->work[CS_TIMER_1]++;
    this->portalRotation += this->timers[0] <= TWINROVA_RAIN_MARKER_FINAL_PULSE_TIME ? 0.2f : 0.08f;
    if (this->timers[0] != 0) {
        this->timers[0]--;
    }

    // Establish the exact impact boundary in twelve updates, then hold it for most of the projectile's descent.
    Math_ApproachF(&this->workf[UNK_F17], sigilScale, 1.0f, sigilScale / 12.0f);
    if (this->timers[0] <= TWINROVA_RAIN_MARKER_FINAL_PULSE_TIME) {
        // Keep the marked footprint authoritative; urgency comes from the faster rotation and alpha pulse.
        Math_ApproachF(&this->workf[UNK_F18], 255.0f, 1.0f, 30.0f);
    } else {
        Math_ApproachF(&this->workf[UNK_F18], 220.0f, 1.0f, 20.0f);
    }

    if (this->timers[0] == TWINROVA_RAIN_MARKER_FINAL_PULSE_TIME &&
        (!BossTw_IsCycloneRain(rainBlast) || rainBlast->work[TWINROVA_RAIN_PLAY_CYCLONE_CUE])) {
        // One positional cue supports players whose camera is tracking the circling sisters rather than the floor.
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
    }

    if (this->timers[0] == 0) {
        Actor_Kill(&this->actor);
    }
}

void BossTw_SiegeZoneDraw(Actor* thisx, PlayState* play) {
    BossTw* this = (BossTw*)thisx;
    s32 isWarningSigil = this->actor.params == TW_FIRE_WARNING_SIGIL ||
                           this->actor.params == TW_ICE_WARNING_SIGIL;
    f32 poolMeshInradius = this->blastType == TWINROVA_MAGIC_FIRE ? TWINROVA_FIRE_POOL_MESH_INRADIUS
                                                                 : TWINROVA_ICE_POOL_MESH_INRADIUS;
    // The fire octagon and ice decagon have different native inradii. Scale their nearest edges against the same
    // sigil boundary so neither element leaves an invisible damaging crescent between polygon corners.
    f32 poolScale = (this->workf[UNK_F17] * TWINROVA_SIGIL_MESH_RADIUS) / poolMeshInradius;
    s16 alpha = this->workf[UNK_F18];
    s16 poolAlpha = alpha;
    s32 finalPulse = false;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    gSPSegment(POLY_XLU_DISP++, 8,
               Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, -this->work[CS_TIMER_1] * 2, 0, 0x20, 0x20, 1,
                                  this->work[CS_TIMER_1] * 3, 0, 0x20, 0x20, -2, 0, 3, 0));
    Matrix_Push();

    if (this->csState1 == TWINROVA_SIEGE_STATE_TELEGRAPH) {
        // Keep the authoritative footprint readable against the bright stone floor for the entire tell. It remains
        // substantially dimmer than the active pool, so detonation still has an unmistakable visual state change.
        poolAlpha = (s16)(poolAlpha * (isWarningSigil ? TWINROVA_RAIN_TELEGRAPH_POOL_ALPHA_SCALE
                                                     : TWINROVA_SIEGE_TELEGRAPH_POOL_ALPHA_SCALE));
    }
    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y + 1.0f, this->actor.world.pos.z, MTXMODE_NEW);
    if (this->blastType == TWINROVA_MAGIC_ICE) {
        Matrix_Scale(poolScale, poolScale, poolScale, MTXMODE_APPLY);
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 220, 245, 255, poolAlpha);
        gDPSetEnvColor(POLY_XLU_DISP++, 40, 100, 150, 128);
        gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                  G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIcePoolDL));
    } else {
        Matrix_Scale(poolScale, poolScale, poolScale, MTXMODE_APPLY);
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 150, 35, 0, poolAlpha);
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 220, 80, 128);
        gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                  G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaFirePoolDL));
    }

    if (this->blastBehavior != TWINROVA_BLAST_MINEFIELD_POOL) {
        // Mine patches retain their truthful elemental footprint but omit Siege's stationary rotating rune.
        if (this->csState1 == TWINROVA_SIEGE_STATE_TELEGRAPH) {
            finalPulse = isWarningSigil ? this->timers[0] <= TWINROVA_RAIN_MARKER_FINAL_PULSE_TIME
                                        : this->timers[0] <= TWINROVA_SIEGE_FINAL_WARNING_TIME;
            s16 pulseSpeed = finalPulse ? 0x2000 : 0x1000;

            // Retain a strong floor-readable core at the pulse trough. The previous 40% minimum disappeared against
            // the Spirit Temple's bright stone even though the marker actor itself was still active and authoritative.
            alpha = (s16)(alpha * (0.85f + (Math_SinS(this->work[CS_TIMER_1] * pulseSpeed) * 0.15f)));
        }
        gSPSegment(POLY_XLU_DISP++, 8,
                   Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, -this->work[CS_TIMER_1] * 15, 0x20, 0x40, 1, 0,
                                      0, 0x40, 0x40, 0, -15, 0, 0));
        // Lift the rune above the translucent pool to avoid z-fighting on sloped/raised tiles without moving its pool
        // or changing the exact gameplay radius advertised by the mesh.
        Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y + 6.0f, this->actor.world.pos.z,
                         MTXMODE_NEW);
        Matrix_RotateY(this->portalRotation, MTXMODE_APPLY);
        Matrix_RotateX(M_PI / 2.0f, MTXMODE_APPLY);
        Matrix_Scale(this->workf[UNK_F17], this->workf[UNK_F17], this->workf[UNK_F17], MTXMODE_APPLY);
        gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_LOAD | G_MTX_MODELVIEW | G_MTX_NOPUSH);
        if (this->blastType == TWINROVA_MAGIC_ICE) {
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, finalPulse ? 210 : 135, finalPulse ? 245 : 210, 255, alpha);
            gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKotakeMagicSigilDL));
        } else {
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, finalPulse ? 210 : 120, finalPulse ? 120 : 0, alpha);
            gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaKoumeMagicSigilDL));
        }
    }

    Matrix_Pop();
    CLOSE_DISPS(play->state.gfxCtx);
}

static void BossTw_SetupMovingBlast(BossTw* this, Player* player) {
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    f32 xzDist;
    f32 aimDistance;
    s16 requiredLifetime;
    s16 i;

    if (BossTw_IsCycloneRain(this)) {
        Actor_SetScale(&this->actor, TWINROVA_CYCLONE_RAIN_PROJECTILE_SCALE);
        this->collider.dim.radius = TWINROVA_CYCLONE_RAIN_FLIGHT_RADIUS;
        this->collider.dim.height = TWINROVA_CYCLONE_RAIN_FLIGHT_HEIGHT;
        this->collider.dim.yShift = TWINROVA_CYCLONE_RAIN_FLIGHT_Y_SHIFT;
    } else {
        Actor_SetScale(&this->actor, 0.03f);
    }
    this->csState1 = 1;

    if (this->blastBehavior == TWINROVA_BLAST_RAIN) {
        if (sTwinrovaPtr != NULL && this->actor.parent == &sTwinrovaPtr->actor) {
            // Phase one's barrage is launched upward by the flying sisters. Cyclone's projectiles instead emerge from
            // the overhead sigil, so solve a dedicated downward arc that begins falling immediately and still reaches
            // its exact marker on the shared 64-update warning clock.
            this->actor.velocity.y = TWINROVA_CYCLONE_RAIN_INITIAL_FALL_SPEED;
            this->actor.gravity =
                (this->targetPos.y - this->actor.world.pos.y -
                 (this->actor.velocity.y * TWINROVA_RAIN_FLIGHT_FRAMES)) /
                (TWINROVA_RAIN_FLIGHT_FRAMES * (TWINROVA_RAIN_FLIGHT_FRAMES + 1.0f) * 0.5f);
        } else {
            this->actor.gravity = TWINROVA_RAIN_GRAVITY;
            this->actor.velocity.y =
                (this->targetPos.y - this->actor.world.pos.y -
                 (this->actor.gravity * TWINROVA_RAIN_FLIGHT_FRAMES * (TWINROVA_RAIN_FLIGHT_FRAMES + 1.0f) * 0.5f)) /
                TWINROVA_RAIN_FLIGHT_FRAMES;
        }
        this->actor.velocity.x = (this->targetPos.x - this->actor.world.pos.x) / TWINROVA_RAIN_FLIGHT_FRAMES;
        this->actor.velocity.z = (this->targetPos.z - this->actor.world.pos.z) / TWINROVA_RAIN_FLIGHT_FRAMES;
        xzDist = sqrtf(SQ(this->actor.velocity.x) + SQ(this->actor.velocity.z));
        this->actor.world.rot.y = Math_FAtan2F(this->actor.velocity.x, this->actor.velocity.z) * (32768.0f / M_PI);
        this->actor.world.rot.x = Math_FAtan2F(this->actor.velocity.y, xzDist) * (32768.0f / M_PI);
        this->actor.speedXZ = 0.0f;
    } else {
        xDiff = player->actor.world.pos.x - this->actor.world.pos.x;
        yDiff = (player->actor.world.pos.y + 30.0f) - this->actor.world.pos.y;
        zDiff = player->actor.world.pos.z - this->actor.world.pos.z;
        if (BossTw_IsSignatureProjectile(this)) {
            // Signature shots deliberately commit to this point. Remember it so a successful dodge can resolve as
            // soon as the shot has safely passed, instead of waiting out an invisible four-second arena crossing.
            this->targetPos = player->actor.world.pos;
            this->targetPos.y += 30.0f;
            this->work[TWINROVA_SIGNATURE_PASSED_AIM] = false;

            // Ninety updates cover every normal route, but a committed lower portal and Link may legally end up on
            // opposite arena edges. Extend only those long shots far enough to cross their sampled aim point plus the
            // delayed collision-result update; passed-aim resolution still prevents any extra tail travel.
            aimDistance = sqrtf(SQ(xDiff) + SQ(yDiff) + SQ(zDiff));
            requiredLifetime = (s16)(aimDistance / TWINROVA_DIRECT_BLAST_SPEED) + 2;
            requiredLifetime = CLAMP_MAX(requiredLifetime, TWINROVA_SIGNATURE_PROJECTILE_MAX_LIFETIME);
            if (this->timers[1] < requiredLifetime) {
                this->timers[1] = requiredLifetime;
            }
        }
        this->actor.world.rot.y = Math_FAtan2F(xDiff, zDiff) * (32768.0f / M_PI);
        xzDist = sqrtf(SQ(xDiff) + SQ(zDiff));
        this->actor.world.rot.x = Math_FAtan2F(yDiff, xzDist) * (32768.0f / M_PI);
        this->actor.speedXZ = TWINROVA_DIRECT_BLAST_SPEED;

        if (this->blastBehavior == TWINROVA_BLAST_CURVING) {
            this->actor.world.rot.y +=
                Rand_ZeroOne() < 0.5f ? -TWINROVA_CURVE_YAW_OFFSET : TWINROVA_CURVE_YAW_OFFSET;
            this->timers[2] = TWINROVA_CURVE_STEER_FRAMES;
        }
    }

    for (i = 0; i < ARRAY_COUNT(this->blastTailPos); i++) {
        this->blastTailPos[i] = this->actor.world.pos;
    }
    this->workf[TAIL_ALPHA] = 255.0f;
}

static s32 BossTw_ShouldSuppressDirectProjectileCollision(BossTw* this, PlayState* play) {
    s32 isHostileDirectBlast = BossTw_IsPhaseOneVolleyBlast(this) ||
                               (sTwinrovaPtr != NULL && BossTw_IsFusedDirectBlast(this, sTwinrovaPtr));

    if (!isHostileDirectBlast || this->blastBehavior == TWINROVA_BLAST_RAIN) {
        return false;
    }

    if (this->blastBehavior == TWINROVA_BLAST_MINEFIELD_FOLLOWUP && this->minefieldFollowupSuppressed) {
        // The cast still releases and resolves visibly while Link is disabled, but this authored retry slot must never
        // become a fourth actionable shield shot after its short generic thaw grace expires.
        return true;
    }

    // Action functions may change the projectile to its impact/fade state before collision registration. Carry the
    // same grace through that transition so continuous movement cannot expose a one-update frozen-player hit.
    if (BossTw_IsPlayerHardDisabled(play) && this->timers[3] < TWINROVA_FROZEN_ATTACK_GRACE) {
        this->timers[3] = TWINROVA_FROZEN_ATTACK_GRACE;
    }
    return this->timers[3] != 0;
}

static s32 BossTw_UpdateMovingBlast(BossTw* this, PlayState* play) {
    if (BossTw_IsSignatureProjectile(this) && this->work[TWINROVA_SIGNATURE_PASSED_AIM]) {
        // Remain at the crossing point for one update. BlastUpdate consumes the collision result registered on the
        // crossing update before resolving this as a miss, preserving the last legal shield-intercept frame.
        return true;
    }

    if (this->blastBehavior == TWINROVA_BLAST_RAIN) {
        this->actor.velocity.y += this->actor.gravity;
        Actor_UpdatePos(&this->actor);
        return false;
    }

    if (this->blastBehavior == TWINROVA_BLAST_CURVING && this->timers[2] != 0) {
        Player* player = GET_PLAYER(play);
        f32 xDiff = player->actor.world.pos.x - this->actor.world.pos.x;
        f32 yDiff = (player->actor.world.pos.y + 30.0f) - this->actor.world.pos.y;
        f32 zDiff = player->actor.world.pos.z - this->actor.world.pos.z;
        f32 xzDist = sqrtf(SQ(xDiff) + SQ(zDiff));
        s16 yawTarget = Math_FAtan2F(xDiff, zDiff) * (32768.0f / M_PI);
        s16 pitchTarget = Math_FAtan2F(yDiff, xzDist) * (32768.0f / M_PI);

        Math_ApproachS(&this->actor.world.rot.y, yawTarget, 4, TWINROVA_CURVE_YAW_STEP);
        Math_ApproachS(&this->actor.world.rot.x, pitchTarget, 4, TWINROVA_CURVE_PITCH_STEP);
    }

    Actor_UpdateVelocityXYZ(&this->actor);
    Actor_UpdatePos(&this->actor);
    if (BossTw_IsSignatureProjectile(this)) {
        f32 xToAim = this->targetPos.x - this->actor.world.pos.x;
        f32 yToAim = this->targetPos.y - this->actor.world.pos.y;
        f32 zToAim = this->targetPos.z - this->actor.world.pos.z;

        if (((xToAim * this->actor.velocity.x) + (yToAim * this->actor.velocity.y) +
             (zToAim * this->actor.velocity.z)) <= 0.0f) {
            this->work[TWINROVA_SIGNATURE_PASSED_AIM] = true;
        }
    }
    return false;
}

static s32 BossTw_BlastReachedFloor(BossTw* this, f32 floorY) {
    if (this->blastBehavior == TWINROVA_BLAST_RAIN) {
        if (this->work[CS_TIMER_1] < TWINROVA_RAIN_FLIGHT_FRAMES) {
            return false;
        }

        // The sigil is authoritative: finish the authored ballistic flight at its exact marked center instead of
        // overshooting one update while the generic strict floor checks wait for the projectile to cross the plane.
        this->actor.world.pos = this->targetPos;
        this->collider.dim.radius = BossTw_IsCycloneRain(this) ? TWINROVA_CYCLONE_RAIN_IMPACT_RADIUS
                                                              : TWINROVA_RAIN_IMPACT_RADIUS;
        if (this->blastType == TWINROVA_MAGIC_FIRE) {
            if (!BossTw_IsCycloneRain(this) || this->work[TWINROVA_RAIN_PLAY_CYCLONE_CUE]) {
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FIRE_EXP);
            }
        } else {
            if (!BossTw_IsCycloneRain(this) || this->work[TWINROVA_RAIN_PLAY_CYCLONE_CUE]) {
                Audio_PlayActorSound2(&this->actor, NA_SE_EV_ICE_FREEZE);
            }
        }
        return true;
    }

    if (this->blastBehavior == TWINROVA_BLAST_SIEGE || this->blastBehavior == TWINROVA_BLAST_BREAKER ||
        this->blastBehavior == TWINROVA_BLAST_LOWER_ROUTE ||
        this->blastBehavior == TWINROVA_BLAST_MINEFIELD_FOLLOWUP) {
        // The legacy 35-unit result is a no-floor sentinel. Lower-routed and Siege shots must pass beneath raised
        // platforms, then resolve exactly on a real floor instead of exploding in empty space.
        if (floorY < 0.0f || floorY == 35.0f || this->actor.world.pos.y > floorY) {
            return false;
        }
        this->actor.world.pos.y = floorY;
        return true;
    }

    if (floorY < 0.0f) {
        return false;
    }
    return true;
}

static s16 BossTw_GetBlastImpactEffectCount(BossTw* this) {
    if (this->blastBehavior == TWINROVA_BLAST_RAIN) {
        return BossTw_IsCycloneRain(this) ? TWINROVA_CYCLONE_RAIN_IMPACT_EFFECTS
                                         : TWINROVA_RAIN_IMPACT_EFFECTS;
    }
    if (this->blastBehavior == TWINROVA_BLAST_SIEGE || this->blastBehavior == TWINROVA_BLAST_BREAKER ||
        this->blastBehavior == TWINROVA_BLAST_MINEFIELD_FOLLOWUP) {
        return TWINROVA_SIEGE_IMPACT_EFFECTS;
    }
    if (BossTw_IsPhaseOneVolleyBlast(this)) {
        return TWINROVA_PHASE_ONE_VOLLEY_IMPACT_EFFECTS;
    }
    return 50;
}

static s32 BossTw_BlastCreatesGroundHazard(BossTw* this) {
    return !BossTw_IsPhaseOneVolleyBlast(this) && this->blastBehavior != TWINROVA_BLAST_RAIN &&
           this->blastBehavior != TWINROVA_BLAST_SIEGE && this->blastBehavior != TWINROVA_BLAST_BREAKER &&
           this->blastBehavior != TWINROVA_BLAST_MINEFIELD_FOLLOWUP;
}

static void BossTw_PlayBlastMissImpactSfx(BossTw* this) {
    // Rain announces its exact detonation in BossTw_BlastReachedFloor. Other non-hazard shots need one discrete,
    // positional resolution cue so an intentional dodge never reads as a looping projectile sound simply vanishing.
    if (this->blastBehavior == TWINROVA_BLAST_RAIN) {
        return;
    }

    Audio_PlayActorSound2(&this->actor,
                          this->blastType == TWINROVA_MAGIC_FIRE ? NA_SE_EN_TWINROBA_FIRE_EXP : NA_SE_EV_ICE_FREEZE);
}

static void BossTw_ResolveBlastMiss(BossTw* this) {
    BossTw_PlayBlastMissImpactSfx(this);
    sEnvType = 0;
    this->csState1 = 2;
    this->timers[0] = 20;
}

void BossTw_BlastFire(BossTw* this, PlayState* play) {
    s16 i;
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    Player* player = GET_PLAYER(play);
    Player* player2 = player;

    switch (this->actor.params) {
        case TW_FIRE_BLAST:
            switch (this->csState1) {
                case 0:
                    BossTw_SetupMovingBlast(this, player);
                    // fallthrough
                case 1:
                case 10:
                    if (this->csState1 == 1 && this->timers[1] == 0 &&
                        (this->blastBehavior == TWINROVA_BLAST_STRAIGHT ||
                         this->blastBehavior == TWINROVA_BLAST_SIEGE ||
                         this->blastBehavior == TWINROVA_BLAST_BREAKER ||
                         this->blastBehavior == TWINROVA_BLAST_LOWER_ROUTE ||
                         this->blastBehavior == TWINROVA_BLAST_MINEFIELD_FOLLOWUP)) {
                        BossTw_ResolveBlastMiss(this);
                        break;
                    }
                    this->blastActive = true;
                    if (this->timers[0] == 0) {
                        if (BossTw_UpdateMovingBlast(this, play)) {
                            break;
                        }
                        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_FIRE & ~SFX_FLAG);
                    } else {
                        Vec3f velocity;
                        Vec3f velDir;
                        Vec3s blastDir;
                        s16 alpha;
                        s32 shieldBlastSpawned = false;

                        this->actor.world.pos = player2->bodyPartsPos[15];
                        this->actor.world.pos.y = -2000.0f;
                        Matrix_MtxFToYXZRotS(&player2->shieldMf, &blastDir, 0);
                        blastDir.x = -blastDir.x;
                        blastDir.y = blastDir.y + 0x8000;
                        Math_ApproachS(&this->magicDir.x, blastDir.x, 0xA, 0x800);
                        Math_ApproachS(&this->magicDir.y, blastDir.y, 0xA, 0x800);

                        if (this->timers[0] == 50) {
                            D_8094C86F = 10;
                            D_8094C872 = 7;
                            play->envCtx.unk_D8 = 1.0f;
                        }

                        if (this->timers[0] <= 50) {
                            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_FIRE & ~SFX_FLAG);
                            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_REFL_FIRE & ~SFX_FLAG);
                            Matrix_RotateY((this->magicDir.y / 32678.0f) * M_PI, MTXMODE_NEW);
                            Matrix_RotateX((this->magicDir.x / 32678.0f) * M_PI, MTXMODE_APPLY);
                            velDir.x = 0.0f;
                            velDir.y = 0.0f;
                            velDir.z = 50.0f;
                            Matrix_MultVec3f(&velDir, &velocity);
                            alpha = this->timers[0] * 10;
                            alpha = CLAMP_MAX(alpha, 255);

                            shieldBlastSpawned = BossTw_AddShieldBlastEffect(
                                play, &player2->bodyPartsPos[15], &velocity, &sZeroVector, 10.0f, 80.0f, alpha,
                                TWINROVA_MAGIC_FIRE, BossTw_GetShieldBlastTarget(this));
                        }

                        if (this->timers[0] == 1) {
                            if (!shieldBlastSpawned) {
                                this->timers[0] = 2;
                                return;
                            }

                            sEnvType = 0;
                            // The release is already committed. Mark the charge as consumed even if another
                            // elemental source disrupted the globals during the firing animation.
                            sShieldFireCharge = 4;
                            sShieldIceCharge = 0;
                            Actor_Kill(&this->actor);
                        }

                        return;
                    }

                    this->groundBlastPos.y = BossTw_GetFloorY(&this->actor.world.pos);

                    if (BossTw_BlastReachedFloor(this, this->groundBlastPos.y)) {
                        if (this->groundBlastPos.y != 35.0f && BossTw_BlastCreatesGroundHazard(this)) {
                            this->groundBlastPos.x = this->actor.world.pos.x;
                            this->groundBlastPos.z = this->actor.world.pos.z;
                            BossTw_SpawnGroundBlast(this, play, 1);
                        } else {
                            Vec3f velocity;
                            Vec3f accel;
                            s16 effectCount = BossTw_GetBlastImpactEffectCount(this);

                            BossTw_PlayBlastMissImpactSfx(this);
                            for (i = 0; i < effectCount; i++) {
                                velocity.x = Rand_CenteredFloat(20.0f);
                                velocity.y = Rand_CenteredFloat(20.0f);
                                velocity.z = Rand_CenteredFloat(20.0f);
                                accel.x = 0.0f;
                                accel.y = 0.0f;
                                accel.z = 0.0f;
                                BossTw_AddFlameEffect(play, &this->actor.world.pos, &velocity, &accel,
                                                      Rand_ZeroFloat(10.0f) + 25.0f, this->blastType);
                            }

                            play->envCtx.unk_D8 = 0.5f;
                            if (this->blastBehavior == TWINROVA_BLAST_SIEGE ||
                                this->blastBehavior == TWINROVA_BLAST_BREAKER ||
                                this->blastBehavior == TWINROVA_BLAST_MINEFIELD_FOLLOWUP) {
                                sEnvType = 0;
                            }
                        }

                        this->csState1 = 2;
                        this->timers[0] = 20;
                    } else if (this->blastBehavior != TWINROVA_BLAST_RAIN) {
                        Vec3f pos;
                        Vec3f velocity = { 0.0f, 0.0f, 0.0f };
                        Vec3f accel = { 0.0f, 0.0f, 0.0f };

                        for (i = 0; i < 10; i++) {
                            pos = this->blastTailPos[(s16)Rand_ZeroFloat(29.9f)];
                            pos.x += Rand_CenteredFloat(40.0f);
                            pos.y += Rand_CenteredFloat(40.0f);
                            pos.z += Rand_CenteredFloat(40.0f);
                            accel.y = 0.4f;
                            accel.x = Rand_CenteredFloat(0.5f);
                            accel.z = Rand_CenteredFloat(0.5f);
                            BossTw_AddDotEffect(play, &pos, &velocity, &accel, (s16)Rand_ZeroFloat(2.0f) + 8, 1, 75);
                        }
                    }
                    break;
                case 2:
                    Math_ApproachF(&this->workf[TAIL_ALPHA], 0.0f, 1.0f, 15.0f);
                    if (this->timers[0] == 0) {
                        Actor_Kill(&this->actor);
                    }
                    break;
            }
            break;

        case TW_FIRE_BLAST_GROUND:
            if (this->timers[0] != 0) {
                if (sGroundBlastType != 1) {
                    this->timers[0] = 0;
                    break;
                }

                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FIRE_EXP - SFX_FLAG);

                if (BossTw_IsPlayerHardDisabled(play) && this->work[BURN_TMR] < TWINROVA_FROZEN_ATTACK_GRACE) {
                    this->work[BURN_TMR] = TWINROVA_FROZEN_ATTACK_GRACE;
                }

                xDiff = sKoumePtr->groundBlastPos2.x - player->actor.world.pos.x;
                yDiff = sKoumePtr->groundBlastPos2.y - player->actor.world.pos.y;
                zDiff = sKoumePtr->groundBlastPos2.z - player->actor.world.pos.z;

                if (!BossTw_IsPlayerHardDisabled(play) && this->work[BURN_TMR] == 0 &&
                    (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
                    fabsf(yDiff) < 10.0f &&
                    (SQ(xDiff) + SQ(zDiff)) <
                        SQ(sKoumePtr->workf[UNK_F13] * TWINROVA_FIRE_POOL_DAMAGE_RADIUS)) {
                    if (sShieldIceCharge != 0) {
                        BossTw_DisruptShieldCharge(play, TWINROVA_MAGIC_FIRE);
                    }
                    if (!player->bodyIsBurning) {
                        s16 j;

                        for (j = 0; j < ARRAY_COUNT(player->bodyFlameTimers); j++) {
                            player->bodyFlameTimers[j] = Rand_S16Offset(0, 200);
                        }
                        player->bodyIsBurning = true;
                        Player_PlaySfx(&player->actor, player->ageProperties->unk_92 + NA_SE_VO_LI_DEMO_DAMAGE);
                    }
                    this->work[BURN_TMR] = 40;
                    sTwinrovaPtr->timers[2] = 100;
                }

                Math_ApproachF(&sKoumePtr->workf[UNK_F13], 0.04f, 0.1f, 0.002f);
                if (this->timers[0] == 1 && sGroundBlastType == 1) {
                    // The visible crater may fade, but its damage and elemental authority end with the active timer.
                    sGroundBlastType = 0;
                    sEnvType = 0;
                    sKoumePtr->workf[KM_GD_FLM_A] = 150.0f;
                    sKoumePtr->workf[KM_GD_SMOKE_A] = 150.0f;
                    sKoumePtr->workf[KM_GRND_CRTR_A] = 150.0f;
                    Audio_PlayActorSound2(&this->actor, NA_SE_EN_EXTINCT);
                }
                break;
            }

            {
                f32 sp4C = sGroundBlastType == 2 ? 3.0f : 1.0f;

                Math_ApproachF(&sKoumePtr->workf[UNK_F9], 0.0f, 1.0f, 10.0f * sp4C);
                Math_ApproachF(&sKoumePtr->workf[UNK_F12], 0.0f, 1.0f, 0.03f * sp4C);
                Math_ApproachF(&sKoumePtr->workf[TAIL_ALPHA], 0.0f, 1.0f, 3.0f * sp4C);
                Math_ApproachF(&sKoumePtr->workf[UNK_F11], 0.0f, 1.0f, 6.0f * sp4C);
            }

            if (sKoumePtr->workf[TAIL_ALPHA] <= 0.0f) {
                if (sGroundBlastType == 1) {
                    sGroundBlastType = 0;
                    sEnvType = 0;
                }
                Actor_Kill(&this->actor);
            }

            break;
    }
}

void BossTw_BlastIce(BossTw* this, PlayState* play) {
    s16 i;
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    Player* player = GET_PLAYER(play);
    Player* player2 = player;

    switch (this->actor.params) {
        case TW_ICE_BLAST:
            switch (this->csState1) {
                case 0:
                    BossTw_SetupMovingBlast(this, player);
                    // fallthrough
                case 1:
                case 10:
                    if (this->csState1 == 1 && this->timers[1] == 0 &&
                        (this->blastBehavior == TWINROVA_BLAST_STRAIGHT ||
                         this->blastBehavior == TWINROVA_BLAST_SIEGE ||
                         this->blastBehavior == TWINROVA_BLAST_BREAKER ||
                         this->blastBehavior == TWINROVA_BLAST_LOWER_ROUTE ||
                         this->blastBehavior == TWINROVA_BLAST_MINEFIELD_FOLLOWUP)) {
                        BossTw_ResolveBlastMiss(this);
                        break;
                    }
                    this->blastActive = true;

                    if (this->timers[0] == 0) {
                        if (BossTw_UpdateMovingBlast(this, play)) {
                            break;
                        }
                        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_FREEZE - SFX_FLAG);
                    } else {
                        Vec3f velocity;
                        Vec3f spF4;
                        Vec3s reflDir;
                        s16 alpha;
                        s32 shieldBlastSpawned = false;

                        this->actor.world.pos = player2->bodyPartsPos[15];
                        this->actor.world.pos.y = -2000.0f;
                        Matrix_MtxFToYXZRotS(&player2->shieldMf, &reflDir, 0);
                        reflDir.x = -reflDir.x;
                        reflDir.y += 0x8000;
                        Math_ApproachS(&this->magicDir.x, reflDir.x, 0xA, 0x800);
                        Math_ApproachS(&this->magicDir.y, reflDir.y, 0xA, 0x800);

                        if (this->timers[0] == 50) {
                            D_8094C86F = 10;
                            D_8094C872 = 7;
                            play->envCtx.unk_D8 = 1.0f;
                        }

                        if (this->timers[0] <= 50) {
                            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SHOOT_FREEZE - SFX_FLAG);
                            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_REFL_FREEZE - SFX_FLAG);
                            Matrix_RotateY((this->magicDir.y / 32678.0f) * M_PI, MTXMODE_NEW);
                            Matrix_RotateX((this->magicDir.x / 32678.0f) * M_PI, MTXMODE_APPLY);
                            spF4.x = 0.0f;
                            spF4.y = 0.0f;
                            spF4.z = 50.0f;
                            Matrix_MultVec3f(&spF4, &velocity);
                            alpha = this->timers[0] * 10;
                            alpha = CLAMP_MAX(alpha, 255);

                            shieldBlastSpawned = BossTw_AddShieldBlastEffect(
                                play, &player2->bodyPartsPos[15], &velocity, &sZeroVector, 10.0f, 80.0f, alpha,
                                TWINROVA_MAGIC_ICE, BossTw_GetShieldBlastTarget(this));
                        }

                        if (this->timers[0] == 1) {
                            if (!shieldBlastSpawned) {
                                this->timers[0] = 2;
                                break;
                            }

                            sEnvType = 0;
                            // The release is already committed. Mark the charge as consumed even if another
                            // elemental source disrupted the globals during the firing animation.
                            sShieldIceCharge = 4;
                            sShieldFireCharge = 0;
                            Actor_Kill(&this->actor);
                        }

                        break;
                    }

                    this->groundBlastPos.y = BossTw_GetFloorY(&this->actor.world.pos);

                    if (BossTw_BlastReachedFloor(this, this->groundBlastPos.y)) {
                        if (this->groundBlastPos.y != 35.0f && BossTw_BlastCreatesGroundHazard(this)) {
                            this->groundBlastPos.x = this->actor.world.pos.x;
                            this->groundBlastPos.z = this->actor.world.pos.z;
                            BossTw_SpawnGroundBlast(this, play, 0);
                        } else {
                            s16 effectCount = BossTw_GetBlastImpactEffectCount(this);

                            BossTw_PlayBlastMissImpactSfx(this);
                            for (i = 0; i < effectCount; i++) {
                                Vec3f velocity;
                                Vec3f accel;

                                velocity.x = Rand_CenteredFloat(20.0f);
                                velocity.y = Rand_CenteredFloat(20.0f);
                                velocity.z = Rand_CenteredFloat(20.0f);
                                accel.x = 0.0f;
                                accel.y = 0.0f;
                                accel.z = 0.0f;
                                BossTw_AddFlameEffect(play, &this->actor.world.pos, &velocity, &accel,
                                                      Rand_ZeroFloat(10.0f) + 25.0f, this->blastType);
                            }

                            play->envCtx.unk_D8 = 0.5f;
                            if (this->blastBehavior == TWINROVA_BLAST_SIEGE ||
                                this->blastBehavior == TWINROVA_BLAST_BREAKER ||
                                this->blastBehavior == TWINROVA_BLAST_MINEFIELD_FOLLOWUP) {
                                sEnvType = 0;
                            }
                        }

                        this->csState1 = 2;
                        this->timers[0] = 20;
                    } else if (this->blastBehavior != TWINROVA_BLAST_RAIN) {
                        Vec3f pos;
                        Vec3f velocity = { 0.0f, 0.0f, 0.0f };
                        Vec3f accel = { 0.0f, 0.0f, 0.0f };

                        for (i = 0; i < 10; i++) {
                            pos = this->blastTailPos[(s16)Rand_ZeroFloat(29.9f)];
                            pos.x += Rand_CenteredFloat(40.0f);
                            pos.y += Rand_CenteredFloat(40.0f);
                            pos.z += Rand_CenteredFloat(40.0f);
                            accel.y = 0.4f;
                            accel.x = Rand_CenteredFloat(0.5f);
                            accel.z = Rand_CenteredFloat(0.5f);
                            BossTw_AddDotEffect(play, &pos, &velocity, &accel, ((s16)Rand_ZeroFloat(2.0f) + 8), 0, 75);
                        }
                    }
                    break;

                case 2:
                    Math_ApproachF(&this->workf[TAIL_ALPHA], 0.0f, 1.0f, 15.0f);
                    if (this->timers[0] == 0) {
                        Actor_Kill(&this->actor);
                    }
                    break;
            }
            break;

        case TW_ICE_BLAST_GROUND:
            if (this->timers[0] != 0) {
                if (sGroundBlastType != 2) {
                    this->timers[0] = 0;
                    break;
                }

                Audio_PlayActorSound2(&this->actor, NA_SE_EV_ICE_FREEZE - SFX_FLAG);

                if (BossTw_IsPlayerHardDisabled(play) && this->work[BURN_TMR] < TWINROVA_FROZEN_ATTACK_GRACE) {
                    this->work[BURN_TMR] = TWINROVA_FROZEN_ATTACK_GRACE;
                }

                if (this->timers[0] > (sTwinrovaPtr->actionFunc == BossTw_Wait ? 70 : 20)) {
                    s32 pad;
                    Vec3f pos;
                    Vec3f velocity;
                    Vec3f accel;

                    pos.x = sKotakePtr->groundBlastPos2.x + Rand_CenteredFloat(320.0f);
                    pos.z = sKotakePtr->groundBlastPos2.z + Rand_CenteredFloat(320.0f);
                    pos.y = sKotakePtr->groundBlastPos2.y;
                    velocity.x = 0.0f;
                    velocity.y = 0.0f;
                    velocity.z = 0.0f;
                    accel.x = 0.0f;
                    accel.y = 0.13f;
                    accel.z = 0.0f;
                    BossTw_AddDmgCloud(play, 3, &pos, &velocity, &accel, Rand_ZeroFloat(5.0f) + 20.0f, 0, 0, 80);
                    velocity.x = Rand_CenteredFloat(10.0f);
                    velocity.z = Rand_CenteredFloat(10.0f);
                    velocity.y = Rand_ZeroFloat(3.0f) + 3.0f;
                    pos.x = sKotakePtr->groundBlastPos2.x + (velocity.x * 0.5f);
                    pos.z = sKotakePtr->groundBlastPos2.z + (velocity.z * 0.5f);
                    BossTw_AddDmgCloud(play, 3, &pos, &velocity, &accel, Rand_ZeroFloat(5.0f) + 15.0f, 255, 2, 130);
                }

                Math_ApproachF(&sKotakePtr->workf[UNK_F9], 80.0f, 1.0f, 3.0f);
                Math_ApproachF(&sKotakePtr->workf[UNK_F11], 255.0f, 1.0f, 10.0f);
                Math_ApproachF(&sKotakePtr->workf[UNK_F12], 0.04f, 0.1f, 0.002f);
                Math_ApproachF(&sKotakePtr->workf[UNK_F16], 70.0f, 1.0f, -5.0f);

                xDiff = sKotakePtr->groundBlastPos2.x - player->actor.world.pos.x;
                yDiff = sKotakePtr->groundBlastPos2.y - player->actor.world.pos.y;
                zDiff = sKotakePtr->groundBlastPos2.z - player->actor.world.pos.z;
                if (sGroundBlastType == 2 && !BossTw_IsPlayerHardDisabled(play) && this->work[BURN_TMR] == 0 &&
                    sKotakePtr->workf[UNK_F11] > 200.0f &&
                    (player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && fabsf(yDiff) < 10.0f &&
                    (SQ(xDiff) + SQ(zDiff)) <
                        SQ(sKotakePtr->workf[UNK_F12] * TWINROVA_ICE_POOL_DAMAGE_RADIUS)) {
                    if (sShieldFireCharge != 0) {
                        BossTw_DisruptShieldCharge(play, TWINROVA_MAGIC_ICE);
                    }
                    BossTw_BeginPlayerFreeze(play);
                    sTwinrovaPtr->timers[2] = 100;
                }

                if ((this->timers[0] == 70) || (this->timers[0] == 30)) {
                    sKotakePtr->workf[UNK_F16] = 10.0f;
                }

                if ((this->timers[0] % 4) == 0) {
                    sKotakePtr->workf[UNK_F15] = (2.0f * (s16)Rand_ZeroFloat(9.9f) * M_PI) / 10.0f;
                }
                if (this->timers[0] == 1 && sGroundBlastType == 2) {
                    sGroundBlastType = 0;
                    sEnvType = 0;
                    sKotakePtr->workf[UNK_F11] = 150.0f;
                    sKotakePtr->workf[UNK_F14] = 0.6f;
                    Audio_PlayActorSound2(&this->actor, NA_SE_EV_ICE_MELT);
                }
            } else {
                f32 sp80;

                if (sGroundBlastType == 1) {
                    if (sKotakePtr->workf[UNK_F11] > 1.0f) {
                        for (i = 0; i < 3; i++) {
                            Vec3f pos;
                            Vec3f velocity;
                            Vec3f accel;
                            pos.x = Rand_CenteredFloat(280.0f) + sKotakePtr->groundBlastPos2.x;
                            pos.z = Rand_CenteredFloat(280.0f) + sKotakePtr->groundBlastPos2.z;
                            pos.y = sKotakePtr->groundBlastPos2.y + 30.0f;
                            velocity.x = 0.0f;
                            velocity.y = 0.0f;
                            velocity.z = 0.0f;
                            accel.x = 0.0f;
                            accel.y = 0.13f;
                            accel.z = 0.0f;
                            BossTw_AddDmgCloud(play, 3, &pos, &velocity, &accel, Rand_ZeroFloat(5.0f) + 20, 0, 0, 80);
                        }
                    }
                    sp80 = 3.0f;
                } else {
                    sp80 = 1.0f;
                }

                Math_ApproachF(&sKotakePtr->workf[UNK_F14], 0.0f, 1.0f, 0.2f * sp80);
                Math_ApproachF(&sKotakePtr->workf[UNK_F11], 0.0f, 1.0f, 5.0f * sp80);
                Math_ApproachF(&sKotakePtr->workf[UNK_F9], 0.0f, 1.0f, sp80);

                if (sKotakePtr->workf[UNK_F9] <= 0.0f) {
                    if (sGroundBlastType == 2) {
                        sGroundBlastType = 0;
                    }
                    Actor_Kill(&this->actor);
                }
            }
            break;
    }
}

s32 BossTw_BlastShieldCheck(BossTw* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    s32 ret = false;
    s32 hadShieldCharge;
    ColliderInfo* info;

    if (this->csState1 == 1) {
        if (this->collider.base.acFlags & AC_HIT) {
            this->collider.base.acFlags &= ~AC_HIT;
            this->collider.base.atFlags &= ~AT_HIT;
            info = this->collider.info.acHitInfo;

            if (info->toucher.dmgFlags & DMG_SHIELD) {
                this->work[INVINC_TIMER] = 7;
                play->envCtx.unk_D8 = 1.0f;
                Rumble_Request(0.0f, 100, 5, 4);

                if (BossTw_IsCycloneRain(this)) {
                    // Cyclone is movement pressure, not part of the Mirror Shield puzzle. A block visibly dissipates
                    // this drop but must preserve any charge Link carried into the storm and can never release it.
                    // Use the local shield-impact burst, not the vanilla deflection arc: that arc eventually creates
                    // a real elemental ground pool, which would turn dense charge-neutral rain into hidden pressure.
                    BossTw_AddShieldHitEffect(play, 6.0f, this->blastType);
                    Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_REFLECT_MG2);
                    this->csState1 = 2;
                    this->timers[0] = 20;
                    return true;
                }

                if (Player_HasMirrorShieldEquipped(play)) {
                    if (BossTw_IsPhaseOneVolleyBlast(this) && BossTw_HasCommittedPhaseOneShieldRelease(play) &&
                        ((this->blastType == TWINROVA_MAGIC_FIRE && sShieldFireCharge >= 3) ||
                         (this->blastType == TWINROVA_MAGIC_ICE && sShieldIceCharge >= 3))) {
                        // Spiral supplies four shots per element. Once a release is visibly committed, absorb the
                        // redundant fourth matching shot without creating a second blast that would hit immunity.
                        BossTw_AddShieldHitEffect(play, 6.0f, this->blastType);
                        Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_REFLECT_MG2);
                        this->csState1 = 2;
                        this->timers[0] = 20;
                        sEnvType = 0;
                        return true;
                    }

                    if (this->blastBehavior == TWINROVA_BLAST_BREAKER) {
                        // The breaker resets the lesson; it must never become a source of the opposing charge.
                        // This leaves all three authored follow-up shots available to rebuild from neutral.
                        hadShieldCharge = sShieldFireCharge != 0 || sShieldIceCharge != 0;
                        BossTw_DisruptShieldCharge(play, (TwinrovaMagicElement)this->blastType);
                        if (!hadShieldCharge) {
                            BossTw_AddShieldHitEffect(play, 16.0f, this->blastType);
                            play->envCtx.unk_D8 = 1.0f;
                            Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_REFLECT_MG2);
                        }
                    } else if (this->blastType == 1) {
                        if (sShieldIceCharge != 0) {
                            BossTw_DisruptShieldCharge(play, TWINROVA_MAGIC_FIRE);
                        } else {
                            BossTw_AddShieldHitEffect(play, 10.0f, 1);
                            sShieldFireCharge++;
                            D_8094C86F = (sShieldFireCharge * 2) + 8;
                            D_8094C872 = -7;
                        }
                    } else {
                        if (sShieldFireCharge != 0) {
                            BossTw_DisruptShieldCharge(play, TWINROVA_MAGIC_ICE);
                        } else {
                            BossTw_AddShieldHitEffect(play, 10.0f, 0);
                            sShieldIceCharge++;
                            D_8094C86F = (sShieldIceCharge * 2) + 8;
                            D_8094C872 = -7;
                        }
                    }

                    if ((sShieldIceCharge >= 3) || (sShieldFireCharge >= 3)) {
                        if (sTwinrovaPtr != NULL && BossTw_IsFusedDirectBlast(this, sTwinrovaPtr)) {
                            // A committed release consumes the current package even if the player aims it wide.
                            // Restart with three clean shots of the opposite element after recovery.
                            sFixedBlastType = !this->blastType;
                            sFixedBlatSeq = 0;
                        }
                        this->timers[0] = 80;
                        this->csState1 = 10;
                        Matrix_MtxFToYXZRotS(&player->shieldMf, &this->magicDir, 0);
                        this->magicDir.y += 0x8000;
                        this->magicDir.x = -this->magicDir.x;
                        D_8094C86F = 8;
                    } else {
                        this->csState1 = 2;
                        this->timers[0] = 20;
                        sEnvType = 0;
                    }
                } else {
                    BossTw_AddShieldDeflectEffect(play, 10.0f, this->blastType);
                    this->csState1 = 2;
                    this->timers[0] = 20;
                    sEnvType = 0;
                    sShieldIceCharge = 0;
                    sShieldFireCharge = 0;
                    Sfx_PlaySfxCentered(NA_SE_IT_SHIELD_REFLECT_MG2);
                }

                ret = true;
            }
        }
    }

    return ret;
}

static void BossTw_BeginMinefieldFade(BossTw* this) {
    this->csState1 = TWINROVA_MINE_STATE_FADE;
    this->timers[0] = TWINROVA_MINEFIELD_FADE_TIME;
    this->actor.velocity.x = this->actor.velocity.y = this->actor.velocity.z = 0.0f;
}

static s32 BossTw_IsNextActiveMinefieldMine(BossTw* mine, PlayState* play) {
    Actor* actor = play->actorCtx.actorLists[ACTORCAT_BOSS].head;

    while (actor != NULL) {
        BossTw* other = (BossTw*)actor;

        if (actor != &mine->actor && actor->id == ACTOR_BOSS_TW && actor->update != NULL &&
            actor->parent == mine->actor.parent && other->blastBehavior == TWINROVA_BLAST_MINEFIELD &&
            other->csState1 < TWINROVA_MINE_STATE_FADE &&
            other->work[TWINROVA_MINE_DETONATION_ORDER] < mine->work[TWINROVA_MINE_DETONATION_ORDER]) {
            return false;
        }
        actor = actor->next;
    }

    return true;
}

static void BossTw_PlayFinalMinefieldPulse(BossTw* mine) {
    mine->work[TWINROVA_MINE_FINAL_PULSE_PLAYED] = true;
    sMinefieldLightPulseElement = mine->blastType;
    sMinefieldLightPulseTimer = TWINROVA_MINEFIELD_FINAL_LIGHT_PULSE_TIME;
}

static void BossTw_ApplyMinefieldDetonationHit(BossTw* this, PlayState* play) {
    Player* player = GET_PLAYER(play);
    f32 radius = this->blastType == TWINROVA_MAGIC_FIRE ? TWINROVA_MINEFIELD_FIRE_POOL_RADIUS
                                                        : TWINROVA_MINEFIELD_ICE_PATCH_RADIUS;
    f32 xDiff = player->actor.world.pos.x - this->targetPos.x;
    f32 yDiff = player->actor.world.pos.y - this->targetPos.y;
    f32 zDiff = player->actor.world.pos.z - this->targetPos.z;
    s16 i;

    if (BossTw_IsPlayerHardDisabled(play) || !(player->actor.bgCheckFlags & BGCHECKFLAG_GROUND) ||
        fabsf(yDiff) >= TWINROVA_MINEFIELD_BURST_HEIGHT || (SQ(xDiff) + SQ(zDiff)) >= SQ(radius)) {
        return;
    }

    if ((this->blastType == TWINROVA_MAGIC_FIRE && sShieldIceCharge != 0) ||
        (this->blastType == TWINROVA_MAGIC_ICE && sShieldFireCharge != 0)) {
        BossTw_DisruptShieldCharge(play, (TwinrovaMagicElement)this->blastType);
    }

    // The detonation itself owns one truthful damage event. A residual pool is optional pressure, never the burst's
    // collision substitute, so actor-pool limits cannot turn a visible explosion harmless.
    Actor_SetPlayerKnockbackLarge(play, &this->actor, 3.0f, this->actor.yawTowardsPlayer, 2.0f, 0x10);
    if (this->blastType == TWINROVA_MAGIC_FIRE) {
        if (!player->bodyIsBurning) {
            for (i = 0; i < ARRAY_COUNT(player->bodyFlameTimers); i++) {
                player->bodyFlameTimers[i] = Rand_S16Offset(0, 200);
            }
            player->bodyIsBurning = true;
            Player_PlaySfx(&player->actor, player->ageProperties->unk_92 + NA_SE_VO_LI_DEMO_DAMAGE);
        }
    } else {
        BossTw_BeginPlayerFreeze(play);
    }
}

static void BossTw_UpdateMinefieldBlast(BossTw* this, PlayState* play) {
    Actor* ownerActor = BossTw_FindActorByAddress(play, this->actor.parent);
    BossTw* pool;
    s16 i;

    if (ownerActor == NULL || ownerActor->update == NULL || ownerActor->id != ACTOR_BOSS_TW ||
        ownerActor->params != TW_TWINROVA) {
        Actor_Kill(&this->actor);
        return;
    }

    switch (this->csState1) {
        case TWINROVA_MINE_STATE_LAUNCH:
            Actor_SetScale(&this->actor, 0.03f);
            this->csState1 = TWINROVA_MINE_STATE_FLOAT;
            this->timers[0] = TWINROVA_MINEFIELD_FLIGHT_TIME;
            this->actor.velocity.x = (this->targetPos.x - this->actor.world.pos.x) / TWINROVA_MINEFIELD_FLIGHT_TIME;
            this->actor.velocity.y =
                ((this->targetPos.y + 30.0f) - this->actor.world.pos.y) / TWINROVA_MINEFIELD_FLIGHT_TIME;
            this->actor.velocity.z = (this->targetPos.z - this->actor.world.pos.z) / TWINROVA_MINEFIELD_FLIGHT_TIME;
            for (i = 0; i < ARRAY_COUNT(this->blastTailPos); i++) {
                this->blastTailPos[i] = this->actor.world.pos;
            }
            this->workf[TAIL_ALPHA] = 255.0f;
            break;

        case TWINROVA_MINE_STATE_FLOAT:
            Actor_UpdatePos(&this->actor);
            if (BossTw_IsNextActiveMinefieldMine(this, play)) {
                // One positional loop follows the next unresolved mine. Refresh it continuously as required by the
                // audio engine without four simultaneous enemy-bank loops masking the boss's release and impact cues.
                Audio_PlayActorSound2(&this->actor, this->blastType == TWINROVA_MAGIC_FIRE
                                                        ? NA_SE_EN_TWINROBA_MS_FIRE - SFX_FLAG
                                                        : NA_SE_EN_TWINROBA_MS_FREEZE - SFX_FLAG);
            }
            if (this->timers[0] == 0) {
                this->actor.world.pos = this->targetPos;
                this->actor.world.pos.y += 30.0f;
                this->actor.velocity.x = this->actor.velocity.y = this->actor.velocity.z = 0.0f;
                this->csState1 = TWINROVA_MINE_STATE_ARM;
                // Launch cadence already separates arrivals; extend each armed fuse slightly by its authored order so
                // detonation is clearly read as a sequence even when several mines settle close together.
                this->timers[0] = TWINROVA_MINEFIELD_SETTLE_TIME +
                                  this->work[TWINROVA_MINE_DETONATION_ORDER] * TWINROVA_MINEFIELD_DETONATE_STAGGER;
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
            }
            break;

        case TWINROVA_MINE_STATE_ARM:
            {
                s16 pulseInterval = this->timers[0] <= 20 ? 4 : (this->timers[0] <= 45 ? 6 :
                                                                 (this->timers[0] <= 90 ? 8 : 16));

                if (BossTw_IsNextActiveMinefieldMine(this, play) && (this->timers[0] % pulseInterval) == 0) {
                    // Only the next detonation owns the countdown bus. Its cadence accelerates without four mines
                    // retriggering the same cue over one another.
                    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
                }
            }
            this->actor.world.pos.y = this->targetPos.y + 30.0f + Math_SinS(this->work[CS_TIMER_1] * 0x800) * 8.0f;
            if (this->timers[0] == 0) {
                this->csState1 = TWINROVA_MINE_STATE_DETONATE;
                this->timers[0] = TWINROVA_MINEFIELD_DETONATE_TIME;
                BossTw_AddRingEffect(play, &this->targetPos, 0.45f, 3.6f, 255, this->blastType, 1,
                                     ARRAY_COUNT(sEffects));
                BossTw_SpawnMagicLaunchEffects(play, &this->targetPos, this->blastType,
                                                14 + this->work[TWINROVA_MINE_DETONATION_ORDER]);
                Audio_PlayActorSound2(&this->actor, this->blastType == TWINROVA_MAGIC_FIRE
                                                         ? NA_SE_EN_TWINROBA_FIRE_EXP
                                                         : NA_SE_EV_ICE_FREEZE);
                BossTw_ApplyMinefieldDetonationHit(this, play);
                // The falloff is handled by the rumble system, so only mines close to Link feel substantial.
                Rumble_Request(this->actor.xyzDistToPlayerSq, 130, 8, 5);
                if (BossTw_IsFinalMinefieldDetonation(this, play)) {
                    // One brief arena flash closes the authored sequence without turning every mine into a full flash.
                    BossTw_PlayFinalMinefieldPulse(this);
                }
                pool = BossTw_SpawnMinefieldPool(this, play);
                if (pool != NULL) {
                    // The shared burst routine renders fire as a quick ember burst and ice as bright crystal shards.
                    BossTw_SpawnSiegeBurst(pool, play, 10, TWINROVA_SIEGE_USE_ZONE_ELEMENT);
                }
            }
            break;

        case TWINROVA_MINE_STATE_DETONATE:
            this->actor.world.pos.y = this->targetPos.y + 30.0f;
            if (this->timers[0] == 0) {
                if (!this->work[TWINROVA_MINE_FINAL_PULSE_PLAYED] &&
                    BossTw_IsFinalMinefieldDetonation(this, play)) {
                    // Re-evaluate after the burst so the final actual detonation always receives the authored closing
                    // pulse even if another mine resolved earlier in the same update.
                    BossTw_PlayFinalMinefieldPulse(this);
                }
                BossTw_BeginMinefieldFade(this);
            }
            break;

        case TWINROVA_MINE_STATE_FADE:
            this->actor.world.pos.y = this->targetPos.y + 30.0f;
            Math_ApproachF(&this->workf[TAIL_ALPHA], 0.0f, 1.0f, 18.0f);
            if (this->timers[0] == 0) {
                Actor_Kill(&this->actor);
            }
            break;
    }

    if (this->timers[0] != 0) {
        this->timers[0]--;
    }
}

void BossTw_BlastUpdate(Actor* thisx, PlayState* play) {
    BossTw* this = (BossTw*)thisx;
    ColliderCylinder* collider;
    s32 suppressDirectProjectileCollision;
    s32 resolvePassedAim;
    s32 shieldContactHandled;
    s16 i;

    this->work[CS_TIMER_1]++;
    this->work[CS_TIMER_2]++;
    this->work[TAIL_IDX]++;

    if (this->work[TAIL_IDX] > 29) {
        this->work[TAIL_IDX] = 0;
    }

    this->blastTailPos[this->work[TAIL_IDX]] = this->actor.world.pos;

    if (this->blastBehavior == TWINROVA_BLAST_MINEFIELD) {
        // Minefield orbs are positional hazards, not shield-puzzle shots. Keep their flight and settle lifecycle
        // separate from the direct-blast collision path so they never aim at, charge from, or strike Link in flight.
        BossTw_UpdateMinefieldBlast(this, play);
        return;
    }

    suppressDirectProjectileCollision = BossTw_ShouldSuppressDirectProjectileCollision(this, play);
    if (suppressDirectProjectileCollision) {
        // Collision flags are delayed by one update. Discard them before shield processing so a contact registered
        // immediately before the freeze cannot charge, reset, or release the Mirror Shield during the lockout.
        Collider_ResetCylinderAC(play, &this->collider.base);
        Collider_ResetCylinderAT(play, &this->collider.base);
    }

    // Collision results belong to the previous update. Consume a pending shield contact before movement can turn a
    // live projectile into a floor impact; otherwise an exact last-frame block would be discarded with state 1.
    shieldContactHandled =
        !suppressDirectProjectileCollision && this->work[INVINC_TIMER] == 0 && this->csState1 == 1 &&
        (this->collider.base.acFlags & AC_HIT) && BossTw_BlastShieldCheck(this, play);

    // Capture this before the action: crossing happens inside the action, and its newly registered collision result
    // is not readable until the following update.
    resolvePassedAim = this->csState1 == 1 && BossTw_IsSignatureProjectile(this) &&
                       this->work[TWINROVA_SIGNATURE_PASSED_AIM];
    this->actionFunc(this, play);

    if (((this->actor.params == TW_FIRE_BLAST_GROUND) && (this->timers[0] == 0 || sGroundBlastType != 1)) ||
        ((this->actor.params == TW_ICE_BLAST_GROUND) && (this->timers[0] == 0 || sGroundBlastType != 2))) {
        // A replacement, elemental cleanse, or natural timeout ends this hazard's gameplay authority. Its visual
        // fade may continue, but a committed emergence must never fire from that harmless remnant.
        if (this->beamShootState != 0) {
            TwinrovaMagicElement element = this->actor.params == TW_FIRE_BLAST_GROUND ? TWINROVA_MAGIC_FIRE
                                                                                      : TWINROVA_MAGIC_ICE;

            // A replacement or elemental cleanse can revoke the pool before its summon resolves. Cancel gameplay
            // authority immediately while still resolving the visible warning at its advertised origin.
            if (this->timers[0] == 1) {
                // Natural expiry already played the pool's extinguish/melt cue inside the action function. Add only
                // the missing visual collapse here so that cue is not doubled at the same world position.
                BossTw_AddRingEffect(play, &this->actor.world.pos, 0.3f, 1.8f, 180, element, 1,
                                     ARRAY_COUNT(sEffects));
                BossTw_SpawnMagicLaunchEffects(play, &this->actor.world.pos, element, 6);
            } else {
                BossTw_ShowFailedMagicCast(play, &this->actor.world.pos, element);
            }
        }
        BossTw_CancelGroundSummonWarning(this, play, false);
    }

    if (suppressDirectProjectileCollision && this->csState1 == 1 && !resolvePassedAim &&
        !BossTw_IsPlayerHardDisabled(play) && this->timers[3] == TWINROVA_FROZEN_ATTACK_GRACE - 1) {
        // Renew the moving orb's element and position when Link thaws. Collision remains disabled through the grace
        // window, but flight, steering, impacts, and lifetime never stop.
        BossTw_AddRingEffect(play, &this->actor.world.pos, 0.45f, 3.2f, 255, this->blastType, 1,
                             ARRAY_COUNT(sEffects));
        BossTw_SpawnMagicLaunchEffects(play, &this->actor.world.pos, this->blastType, 8);
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
    }

    if (this->blastBehavior == TWINROVA_BLAST_BREAKER && this->csState1 == 1 &&
        (this->work[CS_TIMER_1] % TWINROVA_BREAKER_TRAIL_INTERVAL) == 0) {
        // The breaker changes the shield rule, so its identity must remain visible after the pre-cast flash.
        BossTw_AddRingEffect(play, &this->actor.world.pos, 0.35f, 2.4f, 220, this->blastType, 1,
                             ARRAY_COUNT(sEffects));
    }

    if (this->work[CAN_SHOOT] && this->beamShootState == 0 &&
        this->timers[3] == TWINROVA_SUMMON_WARNING_TIME &&
        (this->actor.params == TW_FIRE_BLAST_GROUND || this->actor.params == TW_ICE_BLAST_GROUND)) {
        TwinrovaMagicElement element = this->actor.params == TW_FIRE_BLAST_GROUND ? TWINROVA_MAGIC_FIRE
                                                                                  : TWINROVA_MAGIC_ICE;

        if (BossTw_TryReserveSummonGroup(this, play)) {
            this->beamShootState = TWINROVA_SUMMON_PENDING;
            // Commit the group before its warning so a Wolfos/Red Bubble emergence has a distinct, stronger tell.
            this->csState2 = (s16)Rand_ZeroFloat(TWINROVA_SUMMON_GROUP_MAX);
            if (!BossTw_ShowCommittedSummonWarning(this, play, element)) {
                // The persistent marker is authoritative. Under effect-pool saturation, cancel visibly rather than
                // letting the reserved enemies emerge from an unmarked ground hazard thirty updates later.
                BossTw_CancelGroundSummonWarning(this, play, true);
            }
        } else {
            // Do not play a strong summon tell when the current arena pressure has denied this emergence.
            this->work[CAN_SHOOT] = false;
        }
    }

    if (this->work[CAN_SHOOT] && this->beamShootState != 0 && this->timers[3] == 0 &&
        (this->actor.params == TW_FIRE_BLAST_GROUND || this->actor.params == TW_ICE_BLAST_GROUND)) {
        TwinrovaMagicElement element = this->actor.params == TW_FIRE_BLAST_GROUND ? TWINROVA_MAGIC_FIRE
                                                                                  : TWINROVA_MAGIC_ICE;

        this->work[CAN_SHOOT] = false;
        this->beamShootState = 0;
        BossTw_SpawnSummonGroup(play, &this->actor.world.pos, element, (TwinrovaSummonGroup)this->csState2);
    }

    for (i = 0; i < 5; i++) {
        if (this->timers[i] != 0) {
            this->timers[i]--;
        }
    }

    if (this->work[INVINC_TIMER] != 0) {
        this->work[INVINC_TIMER]--;
    }

    if (this->work[BURN_TMR] != 0) {
        this->work[BURN_TMR]--;
    }

    this->actor.focus.pos = this->actor.world.pos;
    collider = &this->collider;
    Collider_UpdateCylinder(&this->actor, collider);

    if (suppressDirectProjectileCollision) {
        // Link cannot move or raise the shield during the long ice lockout. Let the projectile continue through its
        // full simulation, but remove damage and shield interaction until the authored thaw grace has elapsed.
        this->blastActive = false;
        Collider_ResetCylinderAC(play, &collider->base);
        Collider_ResetCylinderAT(play, &collider->base);
        if (resolvePassedAim) {
            // Signature shots normally wait one update at their committed aim point for delayed shield collision.
            // Collision was suppressed on both updates, so resolve the miss instead of pinning the orb until thaw.
            BossTw_ResolveBlastMiss(this);
        }
    }

    if (!shieldContactHandled && this->blastActive && this->work[INVINC_TIMER] == 0) {
        if (!BossTw_BlastShieldCheck(this, play)) {
            if (resolvePassedAim) {
                // Link has already received the crossing update's full shield window. End the authored dodge here;
                // the at-least-90-update lifetime remains an emergency fallback for any route that never crosses.
                BossTw_ResolveBlastMiss(this);
            } else {
                CollisionCheck_SetAC(play, &play->colChkCtx, &collider->base);
                // Rain's floor sigil promises an exact detonation. Keep AC live during the descent so an attentive
                // player can still intercept the projectile with the Mirror Shield, but do not let its small flight
                // collider deal damage a frame or two before the marked impact expands it to the authoritative radius.
                if ((this->blastBehavior != TWINROVA_BLAST_RAIN || this->csState1 == 2) &&
                    !(this->blastBehavior == TWINROVA_BLAST_RAIN && BossTw_IsPlayerHardDisabled(play))) {
                    // Rain still resolves on its marked center while Link is frozen, but it cannot chain-hit before
                    // he regains control. Later volleys keep their full visual and positional warning.
                    CollisionCheck_SetAT(play, &play->colChkCtx, &collider->base);
                }
            }
        }
    }

    if (!suppressDirectProjectileCollision && (collider->base.atFlags & AT_HIT) &&
        collider->base.at == &GET_PLAYER(play)->actor) {
        s32 opposingChargeHit =
            (this->blastType == TWINROVA_MAGIC_FIRE && sShieldIceCharge != 0) ||
            (this->blastType == TWINROVA_MAGIC_ICE && sShieldFireCharge != 0);

        collider->base.atFlags &= ~AT_HIT;
        if (!BossTw_IsCycloneRain(this) &&
            (this->blastBehavior == TWINROVA_BLAST_BREAKER || opposingChargeHit)) {
            BossTw_DisruptShieldCharge(play, (TwinrovaMagicElement)this->blastType);
        }
    }

    this->blastActive = false;
}

static void BossTw_DrawMinefieldOrb(BossTw* this, PlayState* play, f32 baseScale, s16 alpha) {
    OPEN_DISPS(play->state.gfxCtx);

    // Minefield deliberately has one silhouette: a large, colorized Phantom Ganon energy ball.
    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    Matrix_Push();
    Matrix_Translate(this->actor.world.pos.x, this->actor.world.pos.y, this->actor.world.pos.z, MTXMODE_NEW);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_Scale(baseScale, baseScale, baseScale, MTXMODE_APPLY);
    if (this->blastType == TWINROVA_MAGIC_FIRE) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 245, 190, alpha);
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 30, 0, (s16)(alpha * 0.8f));
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 215, 245, 255, alpha);
        gDPSetEnvColor(POLY_XLU_DISP++, 25, 120, 255, (s16)(alpha * 0.8f));
    }
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, gPhantomEnergyBallDL);
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void BossTw_DrawMinefieldDetonation(BossTw* this, PlayState* play) {
    f32 burstStrength = this->timers[0] / (f32)TWINROVA_MINEFIELD_DETONATE_TIME;
    f32 scale = TWINROVA_MINEFIELD_DETONATION_LIGHTNING_SCALE * (0.75f + ((1.0f - burstStrength) * 0.35f));
    s16 alpha = (s16)(burstStrength * 255.0f);

    OPEN_DISPS(play->state.gfxCtx);

    // A short, colorized Phantom lightning blast is the Minefield's payoff. It is intentionally confined to the burst
    // state, leaving the armed silhouette clean and preventing a permanent lightning wall over the arena.
    Gfx_SetupDL_25Xlu(play->state.gfxCtx);
    Matrix_Push();
    Matrix_Translate(this->targetPos.x, this->targetPos.y + 30.0f, this->targetPos.z, MTXMODE_NEW);
    Matrix_ReplaceRotation(&play->billboardMtxF);
    Matrix_RotateZ(this->work[CS_TIMER_1] * 0.28f, MTXMODE_APPLY);
    Matrix_Scale(scale, scale, scale, MTXMODE_APPLY);
    if (this->blastType == TWINROVA_MAGIC_FIRE) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 220, 90, alpha);
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 35, 0, (s16)(alpha * 0.8f));
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 190, 240, 255, alpha);
        gDPSetEnvColor(POLY_XLU_DISP++, 35, 120, 255, (s16)(alpha * 0.8f));
    }
    gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
    gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gPhantomLightningBlastDL));
    Matrix_Pop();

    CLOSE_DISPS(play->state.gfxCtx);
}

static void BossTw_DrawMinefieldFuse(BossTw* this, PlayState* play) {
    // Match the slightly larger ice patch with a proportionally larger orb. These settled silhouettes are more than
    // twice the travelling-orb scale so their visible footprint honestly previews the residual 105/115-unit hazard.
    f32 baseScale = this->blastType == TWINROVA_MAGIC_FIRE ? TWINROVA_MINEFIELD_FIRE_ORB_ARM_SCALE
                                                           : TWINROVA_MINEFIELD_ICE_ORB_ARM_SCALE;

    BossTw_DrawMinefieldOrb(this, play, baseScale, 255);
}

void BossTw_BlastDraw(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    BossTw* this = (BossTw*)thisx;
    f32 scaleFactor;
    s16 tailIdx;
    s16 i;

    if (this->blastBehavior == TWINROVA_BLAST_MINEFIELD) {
        if (this->csState1 == TWINROVA_MINE_STATE_ARM) {
            BossTw_DrawMinefieldFuse(this, play);
            return;
        }
        if (this->csState1 == TWINROVA_MINE_STATE_DETONATE) {
            BossTw_DrawMinefieldDetonation(this, play);
            return;
        }
        if (this->csState1 == TWINROVA_MINE_STATE_FADE) {
            // The burst and residual pool own the resolved read. Never regrow the fuse from its fade timer.
            return;
        }
        if (this->csState1 == TWINROVA_MINE_STATE_FLOAT) {
            BossTw_DrawMinefieldOrb(this, play, TWINROVA_MINEFIELD_ORB_FLOAT_SCALE, 230);
            return;
        }
    }

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);

    switch (this->actor.params) {
        case TW_FIRE_BLAST:
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 200, 20, 0, (s8)this->workf[TAIL_ALPHA]);
            gDPSetEnvColor(POLY_XLU_DISP++, 255, 215, 255, 128);
            for (i = 9; i >= 0; i--) {
                FrameInterpolation_RecordOpenChild("Twinrova Fire Blast", i);

                gSPSegment(POLY_XLU_DISP++, 8,
                           Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, ((this->work[CS_TIMER_1] * 3) + (i * 10)) & 0x7F,
                                              ((-this->work[CS_TIMER_1] * 15) + (i * 50)) & 0xFF, 0x20, 0x40, 1, 0, 0,
                                              0x20, 0x20, 3, -15, 0, 0));
                tailIdx = ((this->work[TAIL_IDX] - i) + 30) % 30;
                Matrix_Translate(this->blastTailPos[tailIdx].x, this->blastTailPos[tailIdx].y,
                                 this->blastTailPos[tailIdx].z, MTXMODE_NEW);
                scaleFactor = 1.0f - (i * 0.09f);
                Matrix_Scale(this->actor.scale.x * scaleFactor, this->actor.scale.y * scaleFactor,
                             this->actor.scale.z * scaleFactor, MTXMODE_APPLY);
                Matrix_ReplaceRotation(&play->billboardMtxF);
                gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaFireDL));

                FrameInterpolation_RecordCloseChild();
            }
            break;

        case TW_FIRE_BLAST_GROUND:
            break;

        case TW_ICE_BLAST:
            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, (s8)this->workf[TAIL_ALPHA]);
            gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceMaterialDL));
            for (i = 9; i >= 0; i--) {
                FrameInterpolation_RecordOpenChild("Twinrova Ice Blast", i);

                gSPSegment(POLY_XLU_DISP++, 8,
                           Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, ((this->work[CS_TIMER_1] * 3) + (i * 0xA)) & 0x7F,
                                              (u8)((-this->work[CS_TIMER_1] * 0xF) + (i * 50)), 0x20, 0x40, 1, 0, 0,
                                              0x20, 0x20, 3, -0xF, 0, 0));
                tailIdx = ((this->work[TAIL_IDX] - i) + 30) % 30;
                Matrix_Translate(this->blastTailPos[tailIdx].x, this->blastTailPos[tailIdx].y,
                                 this->blastTailPos[tailIdx].z, MTXMODE_NEW);
                scaleFactor = 1.0f - (i * 0.09f);
                Matrix_Scale(this->actor.scale.x * scaleFactor, this->actor.scale.y * scaleFactor,
                             this->actor.scale.z * scaleFactor, MTXMODE_APPLY);
                Matrix_ReplaceRotation(&play->billboardMtxF);
                gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx),
                          G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceModelDL));

                FrameInterpolation_RecordCloseChild();
            }
            break;

        case TW_ICE_BLAST_GROUND:
            break;
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void BossTw_DrawDeathBall(Actor* thisx, PlayState* play2) {
    PlayState* play = play2;
    BossTw* this = (BossTw*)thisx;
    f32 scaleFactor;
    s16 tailIdx;
    s16 i;

    OPEN_DISPS(play->state.gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);

    if (this->actor.params == TW_DEATHBALL_KOUME) {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 200, 20, 0, (s8)this->workf[TAIL_ALPHA]);
        gDPSetEnvColor(POLY_XLU_DISP++, 255, 215, 255, 128);

        for (i = 9; i >= 0; i--) {
            FrameInterpolation_RecordOpenChild("Twinrova Death Ball 0", i);

            gSPSegment(POLY_XLU_DISP++, 8,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (((this->work[CS_TIMER_1] * 3) + (i * 0xA))) & 0x7F,
                                          (u8)((-this->work[CS_TIMER_1] * 0xF) + (i * 50)), 0x20, 0x40, 1, 0, 0, 0x20,
                                          0x20, 3, -0xF, 0, 0));
            tailIdx = ((this->work[TAIL_IDX] - i) + 30) % 30;
            Matrix_Translate(this->blastTailPos[tailIdx].x, this->blastTailPos[tailIdx].y,
                             this->blastTailPos[tailIdx].z, MTXMODE_NEW);
            scaleFactor = (1.0f - (i * 0.09f));
            Matrix_Scale(this->actor.scale.x * scaleFactor, this->actor.scale.y * scaleFactor,
                         this->actor.scale.z * scaleFactor, MTXMODE_APPLY);
            Matrix_ReplaceRotation(&play->billboardMtxF);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaFireDL));

            FrameInterpolation_RecordCloseChild();
        }
    } else {
        gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, (s8)this->workf[TAIL_ALPHA]);
        gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceMaterialDL));

        for (i = 9; i >= 0; i--) {
            FrameInterpolation_RecordOpenChild("Twinrova Death Ball 1", i);

            gSPSegment(POLY_XLU_DISP++, 8,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (((this->work[CS_TIMER_1] * 3) + (i * 0xA))) & 0x7F,
                                          (u8)((-this->work[CS_TIMER_1] * 0xF) + (i * 50)), 0x20, 0x40, 1, 0, 0, 0x20,
                                          0x20, 3, -0xF, 0, 0));
            tailIdx = ((this->work[TAIL_IDX] - i) + 30) % 30;
            Matrix_Translate(this->blastTailPos[tailIdx].x, this->blastTailPos[tailIdx].y,
                             this->blastTailPos[tailIdx].z, MTXMODE_NEW);
            scaleFactor = (1.0f - (i * 0.09f));
            Matrix_Scale(this->actor.scale.x * scaleFactor, this->actor.scale.y * scaleFactor,
                         this->actor.scale.z * scaleFactor, MTXMODE_APPLY);
            Matrix_ReplaceRotation(&play->billboardMtxF);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(play->state.gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceModelDL));

            FrameInterpolation_RecordCloseChild();
        }
    }

    CLOSE_DISPS(play->state.gfxCtx);
}

void BossTw_UpdateEffects(PlayState* play) {
    static Color_RGB8 sDotColors[] = {
        { 255, 128, 0 },   { 255, 0, 0 },     { 255, 255, 0 },   { 255, 0, 0 },
        { 100, 100, 100 }, { 255, 255, 255 }, { 150, 150, 150 }, { 255, 255, 255 },
    };
    Vec3f sp11C;
    BossTwEffect* eff = play->specialEffects;
    Player* player = GET_PLAYER(play);
    u8 sp113 = 0;
    s16 i;
    s16 j;
    s16 colorIdx;
    Vec3f off;
    Vec3f spF4;
    Vec3f spE8;
    Vec3f spDC;
    Vec3f spD0;
    f32 phi_f22;
    Vec3f spC0;
    Vec3f spB4;
    Vec3f spA8;
    s16 spA6;
    f32 phi_f0;
    Actor* unk44;

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (eff->type != 0) {
            eff->pos.x += eff->curSpeed.x;
            eff->pos.y += eff->curSpeed.y;
            eff->pos.z += eff->curSpeed.z;
            eff->frame++;
            eff->curSpeed.x += eff->accel.x;
            eff->curSpeed.y += eff->accel.y;
            eff->curSpeed.z += eff->accel.z;

            if (eff->type == 1) {
                colorIdx = eff->frame % 4;

                if (eff->work[EFF_ARGS] == 0) {
                    colorIdx += 4;
                }

                eff->color.r = sDotColors[colorIdx].r;
                eff->color.g = sDotColors[colorIdx].g;
                eff->color.b = sDotColors[colorIdx].b;
                eff->alpha -= 20;

                if (eff->alpha <= 0) {
                    eff->alpha = 0;
                    eff->type = TWEFF_NONE;
                }

            } else if ((eff->type == 3) || (eff->type == 2)) {
                if (eff->work[EFF_ARGS] == 2) {
                    eff->alpha -= 20;
                    if (eff->alpha <= 0) {
                        eff->alpha = 0;
                        eff->type = TWEFF_NONE;
                    }
                } else if (eff->work[EFF_ARGS] == 0) {
                    eff->alpha += 10;
                    if (eff->alpha >= 100) {
                        eff->work[EFF_ARGS]++;
                    }
                } else {
                    eff->alpha -= 3;
                    if (eff->alpha <= 0) {
                        eff->alpha = 0;
                        eff->type = TWEFF_NONE;
                    }
                }
            } else if (eff->type == TWEFF_FLAME) {
                if (eff->work[EFF_UNKS1] != 0) {
                    eff->alpha = (eff->alpha - (i & 7)) - 0xD;
                    if (eff->alpha <= 0) {
                        eff->alpha = 0;
                        eff->type = TWEFF_NONE;
                    }
                } else {
                    eff->alpha += 300;
                    if (eff->alpha >= 255) {
                        eff->alpha = 255;
                        eff->work[EFF_UNKS1]++;
                    }
                }
            } else if (eff->type == TWEFF_SHLD_BLST) {
                BossTw* shieldTarget = eff->target != NULL ? (BossTw*)eff->target : sTwinrovaPtr;

                D_8094C870 = 1;
                eff->work[EFF_UNKS1]++;
                if (eff->work[EFF_UNKS1] > 30) {
                    eff->alpha -= 10;
                    if (eff->alpha <= 0) {
                        eff->alpha = 0;
                        eff->type = TWEFF_NONE;
                        eff->target = NULL;
                    }
                }

                if (eff->type == TWEFF_SHLD_BLST) {
                    Math_ApproachF(&eff->workf[EFF_SCALE], eff->workf[EFF_DIST], 0.1f, 0.003f);

                    if (shieldTarget != NULL) {
                        if (shieldTarget == sTwinrovaPtr) {
                            BossTw_ClearSiegeZonesNearShieldBlast(shieldTarget, play, &eff->pos,
                                                                  (TwinrovaMagicElement)eff->work[EFF_ARGS]);
                        }
                        off.x = shieldTarget->actor.world.pos.x - eff->pos.x;
                        off.y = (shieldTarget->actor.world.pos.y - eff->pos.y) * 0.5f;
                        off.z = shieldTarget->actor.world.pos.z - eff->pos.z;

                        if (shieldTarget == sTwinrovaPtr && shieldTarget->actionFunc != BossTw_TwinrovaStun &&
                            shieldTarget->actionFunc != BossTw_TwinrovaDeathCS && shieldTarget->actor.update != NULL &&
                            (SQ(off.x) + SQ(off.y) + SQ(off.z)) < SQ(60.0f)) {
                            for (j = 0; j < 50; j++) {
                                spF4.x = shieldTarget->actor.world.pos.x + Rand_CenteredFloat(35.0f);
                                spF4.y = shieldTarget->actor.world.pos.y + Rand_CenteredFloat(70.0f);
                                spF4.z = shieldTarget->actor.world.pos.z + Rand_CenteredFloat(35.0f);
                                spE8.x = Rand_CenteredFloat(20.0f);
                                spE8.y = Rand_CenteredFloat(20.0f);
                                spE8.z = Rand_CenteredFloat(20.0f);
                                spDC.x = 0.0f;
                                spDC.y = 0.0f;
                                spDC.z = 0.0f;
                                BossTw_AddFlameEffect(play, &spF4, &spE8, &spDC, Rand_ZeroFloat(10.0f) + 25.0f,
                                                      eff->work[EFF_ARGS]);
                            }

                            sTwinrovaBlastType = eff->work[EFF_ARGS];
                            BossTw_ClearSiegeZones(sTwinrovaPtr, play, true, eff->work[EFF_ARGS], true);
                            BossTw_CancelPhaseTwoMagic(sTwinrovaPtr, play, true);
                            sEnvType = 0;
                            sTwinrovaPtr->twinrovaStun = 1;
                            play->envCtx.unk_D8 = 1.0f;
                        } else if (shieldTarget != sTwinrovaPtr &&
                                   (SQ(off.x) + SQ(off.y) + SQ(off.z)) < SQ(60.0f) &&
                                   BossTw_TryHitSisterWithMagic(shieldTarget, play,
                                                               (TwinrovaMagicElement)eff->work[EFF_ARGS])) {
                            eff->type = TWEFF_NONE;
                            eff->target = NULL;
                        }
                    }
                }
            } else if (eff->type == TWEFF_MERGEFLAME) {
                sp11C.x = 0.0f;
                sp11C.y = eff->pos.y;
                sp11C.z = eff->workf[EFF_DIST];
                Matrix_RotateY(sTwinrovaPtr->workf[UNK_F9] + eff->workf[EFF_ROLL], MTXMODE_NEW);
                Matrix_MultVec3f(&sp11C, &eff->pos);

                if (eff->work[EFF_UNKS1] != 0) {
                    eff->alpha -= 60;
                    if (eff->alpha <= 0) {
                        eff->alpha = 0;
                        eff->type = TWEFF_NONE;
                    }
                } else {
                    eff->alpha += 60;
                    if (eff->alpha >= 255) {
                        eff->alpha = 255;
                        eff->work[EFF_UNKS1]++;
                    }
                }
            } else if (eff->type == TWEFF_SHLD_DEFL) {
                eff->work[EFF_UNKS1]++;
                sp11C.x = 0.0f;
                sp11C.y = 0.0f;
                sp11C.z = -eff->workf[EFF_DIST];
                Matrix_RotateY((sShieldHitYaw / 32768.0f) * M_PI, MTXMODE_NEW);
                Matrix_RotateX(-0.2f, MTXMODE_APPLY);
                Matrix_RotateZ(eff->workf[EFF_ROLL], MTXMODE_APPLY);
                Matrix_RotateY(eff->workf[EFF_YAW], MTXMODE_APPLY);
                Matrix_MultVec3f(&sp11C, &eff->pos);
                eff->pos.x += sShieldHitPos.x;
                eff->pos.y += sShieldHitPos.y;
                eff->pos.z += sShieldHitPos.z;

                if (eff->work[EFF_UNKS1] < 10) {
                    Math_ApproachF(&eff->workf[EFF_DIST], 50.0f, 0.5f, 100.0f);
                } else {
                    Math_ApproachF(&eff->workf[EFF_YAW], 0.0f, 0.5f, 10.0f);
                    Math_ApproachF(&eff->workf[EFF_DIST], 1000.0f, 1.0f, 10.0f);
                    if (eff->work[EFF_UNKS1] >= 0x10) {
                        if ((eff->work[EFF_UNKS1] == 16) && (sp113 == 0)) {
                            sp113 = 1;
                            spD0 = eff->pos;
                            if (eff->pos.y > 40.0f) {
                                spD0.y = 220.0f;
                            } else {
                                spD0.y = -50.0f;
                            }
                            sTwinrovaPtr->groundBlastPos.y = phi_f0 = BossTw_GetFloorY(&spD0);
                            if (phi_f0 >= 0.0f) {
                                if (sTwinrovaPtr->groundBlastPos.y != 35.0f) {
                                    sTwinrovaPtr->groundBlastPos.x = eff->pos.x;
                                    sTwinrovaPtr->groundBlastPos.z = eff->pos.z;
                                    BossTw_SpawnGroundBlast(sTwinrovaPtr, play, eff->work[EFF_ARGS]);
                                }
                            }
                        }
                        eff->alpha -= 300;
                        if (eff->alpha <= 0) {
                            eff->alpha = 0;
                            eff->type = TWEFF_NONE;
                        }
                    }
                }

                BossTw_AddFlameEffect(play, &eff->pos, &sZeroVector, &sZeroVector, 10, eff->work[EFF_ARGS]);
            } else if (eff->type == TWEFF_SHLD_HIT) {
                eff->work[EFF_UNKS1]++;
                sp11C.x = 0.0f;
                sp11C.y = 0.0f;
                sp11C.z = -eff->workf[EFF_DIST];
                Matrix_RotateY((sShieldHitYaw / 32768.0f) * M_PI, MTXMODE_NEW);
                Matrix_RotateX(-0.2f, MTXMODE_APPLY);
                Matrix_RotateZ(eff->workf[EFF_ROLL], MTXMODE_APPLY);
                Matrix_RotateY(eff->workf[EFF_YAW], MTXMODE_APPLY);
                Matrix_MultVec3f(&sp11C, &eff->pos);
                eff->pos.x += sShieldHitPos.x;
                eff->pos.y += sShieldHitPos.y;
                eff->pos.z += sShieldHitPos.z;

                if (eff->work[EFF_UNKS1] < 5) {
                    Math_ApproachF(&eff->workf[EFF_DIST], 40.0f, 0.5f, 100.0f);
                } else {
                    Math_ApproachF(&eff->workf[EFF_DIST], 0.0f, 0.2f, 5.0f);
                    if (eff->work[EFF_UNKS1] >= 11) {
                        eff->alpha -= 30;
                        if (eff->alpha <= 0) {
                            eff->alpha = 0;
                            eff->type = TWEFF_NONE;
                        }
                    }
                }

                BossTw_AddFlameEffect(play, &eff->pos, &sZeroVector, &sZeroVector, 10, eff->work[EFF_ARGS]);
            } else if (eff->type == 4) {
                if (eff->work[EFF_UNKS1] == 0) {
                    Math_ApproachF(&eff->workf[EFF_SCALE], eff->workf[EFF_DIST], 0.05f, 1.0f);

                    if (eff->frame >= 16) {
                        eff->alpha -= 10;
                        if (eff->alpha <= 0) {
                            eff->alpha = 0;
                            eff->type = TWEFF_NONE;
                        }
                    }
                } else {
                    Math_ApproachF(&eff->workf[EFF_SCALE], eff->workf[EFF_DIST], 0.1f, 2.0f);
                    eff->alpha -= 15;

                    if (eff->alpha <= 0) {
                        eff->alpha = 0;
                        eff->type = TWEFF_NONE;
                    }
                }
            } else if (eff->type == TWEFF_PLYR_FRZ) {
                if (eff->work[EFF_ARGS] < eff->frame) {
                    phi_f0 = 1.0f;

                    if (eff->target != NULL || sGroundBlastType == 1) {
                        phi_f0 *= 3.0f;
                    }

                    Math_ApproachF(&eff->workf[EFF_SCALE], 0.0f, 1.0f, 0.0005f * phi_f0);

                    if (eff->workf[EFF_SCALE] == 0.0f) {
                        eff->type = TWEFF_NONE;
                        if (eff->target == NULL) {
                            player->stateFlags2 &= ~PLAYER_STATE2_PAUSE_MOST_UPDATING;
                            sFreezeState = 0;
                        }
                    }
                } else {
                    if (sGroundBlastType == 1) {
                        eff->frame = 100;
                    }
                    Math_ApproachF(&eff->workf[EFF_DIST], 0.8f, 0.2f, 0.04f);

                    if (eff->target == NULL) {
                        Math_ApproachF(&eff->workf[EFF_SCALE], 0.012f, 1.0f, 0.002f);
                        eff->workf[EFF_ROLL] += eff->workf[EFF_DIST];

                        if (eff->workf[EFF_ROLL] >= 0.8f) {
                            eff->workf[EFF_ROLL] -= 0.8f;
                            player->stateFlags2 |= PLAYER_STATE2_PAUSE_MOST_UPDATING;
                        } else {
                            player->stateFlags2 &= ~PLAYER_STATE2_PAUSE_MOST_UPDATING;
                        }

                        if ((sKotakePtr->workf[UNK_F11] > 10.0f) && (sKotakePtr->workf[UNK_F11] < 200.0f)) {
                            eff->frame = 100;
                        }

                        if (!(play->gameplayFrames & 1)) {
                            play->damagePlayer(play, -1);
                        }
                    } else {
                        Math_ApproachF(&eff->workf[EFF_SCALE], 0.042f, 1.0f, 0.002f);
                    }

                    if ((eff->workf[EFF_DIST] > 0.4f) && ((eff->frame & 7) == 0)) {
                        spA6 = Rand_ZeroFloat(17.9f);

                        if (eff->target == NULL) {
                            spC0.x = player->bodyPartsPos[spA6].x + Rand_CenteredFloat(5.0f);
                            spC0.y = player->bodyPartsPos[spA6].y + Rand_CenteredFloat(5.0f);
                            spC0.z = player->bodyPartsPos[spA6].z + Rand_CenteredFloat(5.0f);
                            phi_f22 = 10.0f;
                        } else {
                            unk44 = eff->target;
                            spC0.x = unk44->world.pos.x + Rand_CenteredFloat(40.0f);
                            spC0.y = unk44->world.pos.y + Rand_CenteredFloat(40.0f);
                            spC0.z = unk44->world.pos.z + Rand_CenteredFloat(40.0f);
                            phi_f22 = 20.0f;
                        }

                        spB4.x = 0.0f;
                        spB4.y = 0.0f;
                        spB4.z = 0.0f;
                        spA8.x = 0.0f;
                        spA8.y = 0.1f;
                        spA8.z = 0.0f;

                        BossTw_AddDmgCloud(play, 3, &spC0, &spB4, &spA8, phi_f22 + Rand_ZeroFloat(phi_f22 * 0.5f), 0, 0,
                                           150);
                    }
                }
            }
        }
        eff++;
    }
}

static s32 sRandSeed0;
static s32 sRandSeed1;
static s32 sRandSeed2;

void BossTw_InitRand(s32 seed0, s32 seed1, s32 seed2) {
    sRandSeed0 = seed0;
    sRandSeed1 = seed1;
    sRandSeed2 = seed2;
}

f32 BossTw_RandZeroOne(void) {
    f32 rand;

    // Wichmann-Hill algorithm
    sRandSeed0 = (sRandSeed0 * 171) % 30269;
    sRandSeed1 = (sRandSeed1 * 172) % 30307;
    sRandSeed2 = (sRandSeed2 * 170) % 30323;

    rand = (sRandSeed0 / 30269.0f) + (sRandSeed1 / 30307.0f) + (sRandSeed2 / 30323.0f);
    while (rand >= 1.0f) {
        rand -= 1.0f;
    }

    return fabsf(rand);
}

void BossTw_DrawEffects(PlayState* play) {
    u8 sp18F = 0;
    s16 i;
    s16 j;
    s32 pad;
    Player* player = GET_PLAYER(play);
    s16 phi_s4;
    BossTwEffect* currentEffect = play->specialEffects;
    BossTwEffect* effectHead;
    GraphicsContext* gfxCtx = play->state.gfxCtx;

    effectHead = currentEffect;

    OPEN_DISPS(gfxCtx);

    Gfx_SetupDL_25Xlu(play->state.gfxCtx);

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (currentEffect->type == 1) {
            FrameInterpolation_RecordOpenChild(currentEffect, currentEffect->epoch);

            if (sp18F == 0) {
                gSPDisplayList(POLY_XLU_DISP++, gTwinrovaMagicParticleMaterialDL);
                sp18F++;
            }

            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, currentEffect->color.r, currentEffect->color.g,
                            currentEffect->color.b, currentEffect->alpha);
            Matrix_Translate(currentEffect->pos.x, currentEffect->pos.y, currentEffect->pos.z, MTXMODE_NEW);
            Matrix_ReplaceRotation(&play->billboardMtxF);
            Matrix_Scale(currentEffect->workf[EFF_SCALE], currentEffect->workf[EFF_SCALE], 1.0f, MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, gTwinrovaMagicParticleModelDL);

            FrameInterpolation_RecordCloseChild();
        }
        currentEffect++;
    }

    sp18F = 0;
    currentEffect = effectHead;

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (currentEffect->type == 3) {
            FrameInterpolation_RecordOpenChild(currentEffect, currentEffect->epoch);

            if (sp18F == 0) {
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceMaterialDL));
                sp18F++;
            }

            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, currentEffect->alpha);
            gSPSegment(POLY_XLU_DISP++, 8,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (currentEffect->frame * 3) & 0x7F,
                                          (currentEffect->frame * 15) & 0xFF, 0x20, 0x40, 1, 0, 0, 0x20, 0x20, 3, 15, 0,
                                          0));
            Matrix_Translate(currentEffect->pos.x, currentEffect->pos.y, currentEffect->pos.z, MTXMODE_NEW);
            Matrix_ReplaceRotation(&play->billboardMtxF);
            Matrix_Scale(currentEffect->workf[EFF_SCALE], currentEffect->workf[EFF_SCALE], 1.0f, MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceModelDL));

            FrameInterpolation_RecordCloseChild();
        }
        currentEffect++;
    }

    sp18F = 0;
    currentEffect = effectHead;

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (currentEffect->type == 2) {
            FrameInterpolation_RecordOpenChild(currentEffect, currentEffect->epoch);

            if (sp18F == 0) {
                gDPPipeSync(POLY_XLU_DISP++);
                gDPSetEnvColor(POLY_XLU_DISP++, 255, 215, 255, 128);
                sp18F++;
            }

            gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 200, 20, 0, currentEffect->alpha);
            gSPSegment(POLY_XLU_DISP++, 8,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (currentEffect->frame * 3) & 0x7F,
                                          (currentEffect->frame * 15) & 0xFF, 0x20, 0x40, 1, 0, 0, 0x20, 0x20, 3, 15, 0,
                                          0));
            Matrix_Translate(currentEffect->pos.x, currentEffect->pos.y, currentEffect->pos.z, MTXMODE_NEW);
            Matrix_ReplaceRotation(&play->billboardMtxF);
            Matrix_Scale(currentEffect->workf[EFF_SCALE], currentEffect->workf[EFF_SCALE], 1.0f, MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaFireDL));

            FrameInterpolation_RecordCloseChild();
        }

        currentEffect++;
    }

    sp18F = 0;
    currentEffect = effectHead;

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (currentEffect->type == 4) {
            FrameInterpolation_RecordOpenChild(currentEffect, currentEffect->epoch);

            if (sp18F == 0) {
                sp18F++;
            }

            gSPSegment(POLY_XLU_DISP++, 0xD,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, currentEffect->frame & 0x7F,
                                          (currentEffect->frame * 8) & 0xFF, 0x20, 0x40, 1,
                                          (currentEffect->frame * -2) & 0x7F, 0, 0x10, 0x10, 1, 8, -2, 0));

            if (currentEffect->work[EFF_ARGS] == 1) {
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 255, 65, 0, currentEffect->alpha);
                gDPPipeSync(POLY_XLU_DISP++);
                gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 0, 128);
            } else {
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, currentEffect->alpha);
                gDPSetEnvColor(POLY_XLU_DISP++, 255, 255, 255, 128);
            }

            Matrix_Translate(currentEffect->pos.x, currentEffect->pos.y, currentEffect->pos.z, MTXMODE_NEW);
            Matrix_ReplaceRotation(&play->billboardMtxF);

            if (currentEffect->work[EFF_UNKS1] == 0) {
                Matrix_Translate(0.0f, 0.0f, 60.0f, MTXMODE_APPLY);
            } else {
                Matrix_Translate(0.0f, 0.0f, 0.0f, MTXMODE_APPLY);
            }

            Matrix_RotateZ(currentEffect->workf[EFF_ROLL], MTXMODE_APPLY);
            Matrix_RotateX(M_PI / 2.0f, MTXMODE_APPLY);
            Matrix_Scale(currentEffect->workf[EFF_SCALE], 1.0f, currentEffect->workf[EFF_SCALE], MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
            gDPSetRenderMode(POLY_XLU_DISP++, G_RM_PASS, G_RM_AA_ZB_XLU_SURF2);
            gSPClearGeometryMode(POLY_XLU_DISP++, G_CULL_BACK | G_FOG);
            gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaEffectHaloDL));

            FrameInterpolation_RecordCloseChild();
        }

        currentEffect++;
    }

    sp18F = 0;
    currentEffect = effectHead;

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        Actor* actor;
        Vec3f off;

        if (currentEffect->type == TWEFF_PLYR_FRZ) {
            FrameInterpolation_RecordOpenChild(currentEffect, currentEffect->epoch);

            if (sp18F == 0) {
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceSurroundingPlayerMaterialDL));
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, 255);
                gSPSegment(
                    POLY_XLU_DISP++, 8,
                    Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, 0, 0, 0x20, 0x40, 1, 0, 0, 0x20, 0x20, 0, 0, 0, 0));
                sp18F++;
                BossTw_InitRand(1, 0x71AC, 0x263A);
            }

            actor = currentEffect->target;
            phi_s4 = actor == NULL ? 70 : 20;

            for (j = 0; j < phi_s4; j++) {
                off.x = (BossTw_RandZeroOne() - 0.5f) * 30.0f;
                off.y = currentEffect->workf[EFF_DIST] * j;
                off.z = (BossTw_RandZeroOne() - 0.5f) * 30.0f;

                if (actor != NULL) {
                    Matrix_Translate(actor->world.pos.x + off.x, actor->world.pos.y + off.y, actor->world.pos.z + off.z,
                                     MTXMODE_NEW);
                } else {
                    Matrix_Translate(player->actor.world.pos.x + off.x, player->actor.world.pos.y + off.y,
                                     player->actor.world.pos.z + off.z, MTXMODE_NEW);
                }

                Matrix_Scale(currentEffect->workf[EFF_SCALE], currentEffect->workf[EFF_SCALE],
                             currentEffect->workf[EFF_SCALE], MTXMODE_APPLY);
                Matrix_RotateY(BossTw_RandZeroOne() * M_PI, MTXMODE_APPLY);
                Matrix_RotateX((BossTw_RandZeroOne() - 0.5f) * M_PI * 0.5f, MTXMODE_APPLY);
                gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceModelDL));
            }

            FrameInterpolation_RecordCloseChild();
        }

        currentEffect++;
    }

    sp18F = 0;
    currentEffect = effectHead;

    for (i = 0; i < ARRAY_COUNT(sEffects); i++) {
        if (currentEffect->type >= 6) {
            FrameInterpolation_RecordOpenChild(currentEffect, currentEffect->epoch);

            if (currentEffect->work[EFF_ARGS] == 0) {
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 195, 225, 235, currentEffect->alpha);
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceMaterialDL));
            } else {
                gDPSetPrimColor(POLY_XLU_DISP++, 0, 0, 200, 20, 0, currentEffect->alpha);
                gDPPipeSync(POLY_XLU_DISP++);
                gDPSetEnvColor(POLY_XLU_DISP++, 255, 215, 255, 128);
            }

            gSPSegment(POLY_XLU_DISP++, 8,
                       Gfx_TwoTexScrollEx(play->state.gfxCtx, 0, (currentEffect->frame * 3) & 0x7F,
                                          (-currentEffect->frame * 15) & 0xFF, 0x20, 0x40, 1, 0, 0, 0x20, 0x20, 3, -15,
                                          0, 0));
            Matrix_Translate(currentEffect->pos.x, currentEffect->pos.y, currentEffect->pos.z, MTXMODE_NEW);
            Matrix_ReplaceRotation(&play->billboardMtxF);
            Matrix_Scale(currentEffect->workf[EFF_SCALE], currentEffect->workf[EFF_SCALE], 1.0f, MTXMODE_APPLY);
            gSPMatrix(POLY_XLU_DISP++, MATRIX_NEWMTX(gfxCtx), G_MTX_NOPUSH | G_MTX_LOAD | G_MTX_MODELVIEW);

            if (currentEffect->work[EFF_ARGS] == 0) {
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaIceModelDL));
            } else {
                gSPDisplayList(POLY_XLU_DISP++, SEGMENTED_TO_VIRTUAL(gTwinrovaFireDL));
            }

            FrameInterpolation_RecordCloseChild();
        }

        currentEffect++;
    }

    CLOSE_DISPS(gfxCtx);
}

static s32 BossTw_IsFusedFinalCycle(BossTw* this) {
    return this->actor.colChkInfo.health > 0 && this->actor.colChkInfo.health <= TWINROVA_FINAL_CYCLE_HEALTH;
}

static s16 BossTw_GetBreakerSequenceCooldown(BossTw* this) {
    return BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_BREAKER_SEQUENCE_COOLDOWN
                                         : TWINROVA_BREAKER_SEQUENCE_COOLDOWN;
}

static s16 BossTw_GetCycloneCooldown(BossTw* this) {
    return BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_CYCLONE_COOLDOWN : TWINROVA_CYCLONE_COOLDOWN;
}

static s16 BossTw_GetMinefieldCooldown(BossTw* this) {
    return BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_MINEFIELD_COOLDOWN : TWINROVA_MINEFIELD_COOLDOWN;
}

static void BossTw_LockMinefieldLayout(BossTw* this, Player* player) {
    this->attackPortalPos.x = 0.0f;
    this->attackPortalPos.z = 0.0f;

    if (player->actor.floorHeight < TWINROVA_UPPER_FLOOR_MIN_Y) {
        // The lower layout uses the open diagonal lanes between the center and side platforms.
        this->attackPortalPos.y = TWINROVA_SIEGE_LOWER_FLOOR_Y;
    } else if (fabsf(player->actor.world.pos.x) < 350.0f && fabsf(player->actor.world.pos.z) < 350.0f) {
        this->attackPortalPos.y = 240.0f;
    } else if (fabsf(player->actor.world.pos.x) >= fabsf(player->actor.world.pos.z)) {
        this->attackPortalPos.x = player->actor.world.pos.x >= 0.0f ? 600.0f : -600.0f;
        this->attackPortalPos.y = 230.0f;
    } else {
        this->attackPortalPos.z = player->actor.world.pos.z >= 0.0f ? 600.0f : -600.0f;
        this->attackPortalPos.y = 230.0f;
    }
}

static void BossTw_GetMinefieldLandingPoint(BossTw* this, s16 index, Vec3f* point) {
    s32 lowerLayout = this->attackPortalPos.y < TWINROVA_UPPER_FLOOR_MIN_Y;
    s16 slot = index % 4;
    s16 angle = slot * 0x4000 + (lowerLayout ? 0x2000 : 0);

    // The layout is locked when Minefield begins. Every destination is a known playable surface and remains fixed
    // even if Link changes floors while the orbs are in flight.
    if (lowerLayout) {
        point->x = Math_SinS(angle) * 600.0f;
        point->y = TWINROVA_SIEGE_LOWER_FLOOR_Y;
        point->z = Math_CosS(angle) * 600.0f;
    } else if (this->attackPortalPos.x == 0.0f && this->attackPortalPos.z == 0.0f) {
        point->x = Math_SinS(angle) * 180.0f;
        point->y = 240.0f;
        point->z = Math_CosS(angle) * 180.0f;
    } else if (this->attackPortalPos.x != 0.0f) {
        if (slot < 2) {
            point->x = this->attackPortalPos.x;
            point->y = TWINROVA_SIEGE_SIDE_FLOOR_Y;
            point->z = slot == 0 ? 65.0f : -65.0f;
        } else {
            point->x = this->attackPortalPos.x > 0.0f ? 230.0f : -230.0f;
            point->y = 240.0f;
            point->z = slot == 2 ? 120.0f : -120.0f;
        }
    } else {
        if (slot < 2) {
            point->x = slot == 0 ? 65.0f : -65.0f;
            point->y = TWINROVA_SIEGE_SIDE_FLOOR_Y;
            point->z = this->attackPortalPos.z;
        } else {
            point->x = slot == 2 ? 120.0f : -120.0f;
            point->y = 240.0f;
            point->z = this->attackPortalPos.z > 0.0f ? 230.0f : -230.0f;
        }
    }
}

static s32 BossTw_CanStartMinefield(BossTw* this, PlayState* play) {
    return this->actor.colChkInfo.health < TWINROVA_FUSED_MAX_HEALTH && this->minefieldCooldown == 0 &&
           sShieldFireCharge == 0 && sShieldIceCharge == 0 && !BossTw_HasActiveArenaControlPattern(this, play) &&
           !BossTw_HasActiveFusedDirectBlast(play, this) &&
           !BossTw_HasActiveMinefieldMines(play, this) &&
           BossTw_CountActiveSummons(play) <= TWINROVA_FUSED_SUMMON_SOFT_LIMIT && !GET_PLAYER(play)->bodyIsBurning;
}

static void BossTw_TwinrovaSetupMinefield(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_TwinrovaMinefield;
    this->csState1 = 0;
    // The first Minefield teaches the cadence with three destinations; the final cycle earns the fourth beat.
    this->csState2 = BossTw_IsFusedFinalCycle(this) ? Rand_S16Offset(3, 2) : 3;
    BossTw_LockMinefieldLayout(this, GET_PLAYER(play));
    this->work[TW_PLLR_IDX] = Rand_S16Offset(0, ARRAY_COUNT(sTwinrovaPillarPos));
    this->minefieldSequenceState = TWINROVA_MINEFIELD_SEQUENCE_PENDING;
    this->work[CAN_SHOOT] = false;
    this->timers[0] = TWINROVA_MINEFIELD_WINDUP_TIME;
    this->minefieldCooldown = BossTw_GetMinefieldCooldown(this);
    this->actor.speedXZ = 0.0f;
    this->actor.velocity.x = this->actor.velocity.y = this->actor.velocity.z = 0.0f;
    this->attackPortalActive = false;
    Animation_MorphToPlayOnceSetSpeed(&this->skelAnime, &gTwinrovaWindUpAnim, TWINROVA_FUSED_ATTACK_ANIM_MORPH_TIME,
                                      TWINROVA_FUSED_CHARGE_ANIM_SPEED);

    BossTw_AddRingEffect(play, &this->leftScepterPos, 0.55f, 3.5f, 220, TWINROVA_MAGIC_ICE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_AddRingEffect(play, &this->rightScepterPos, 0.55f, 3.5f, 220, TWINROVA_MAGIC_FIRE, 1,
                         ARRAY_COUNT(sEffects));
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
    Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_POWERUP);
    // Deployment is an arena-wide commitment cue, not a proximity hit. Keep it light but guaranteed.
    Rumble_Request(0.0f, 50, 5, 3);
}

void BossTw_TwinrovaMinefield(BossTw* this, PlayState* play) {
    BossTw* mine;
    Vec3f point;
    Vec3f* spawnPos;
    AnimationHeader* animation;
    TwinrovaMagicElement element;
    s16 magicParams;

    SkelAnime_Update(&this->skelAnime);
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1000);

    if (this->timers[0] != 0) {
        if (this->csState1 == 0 && (this->timers[0] & 3) == 0) {
            BossTw_SpawnMagicLaunchEffects(play, &this->leftScepterPos, TWINROVA_MAGIC_ICE, 2);
            BossTw_SpawnMagicLaunchEffects(play, &this->rightScepterPos, TWINROVA_MAGIC_FIRE, 2);
        }
        return;
    }

    // Mine launches use the complete throw clip. This preserves the rapid cadence without hard-cutting a cast at its
    // release frame; the short recovery gap begins only once the previous throw has visibly completed.
    if (this->work[CAN_SHOOT] == TWINROVA_MINEFIELD_CAST_RECOVER) {
        if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
            this->work[CAN_SHOOT] = TWINROVA_MINEFIELD_CAST_GAP;
            this->timers[0] = TWINROVA_MINEFIELD_LAUNCH_GAP;
        }
        return;
    }
    if (this->work[CAN_SHOOT] == TWINROVA_MINEFIELD_CAST_GAP) {
        if (this->timers[0] == 0) {
            this->work[CAN_SHOOT] = TWINROVA_MINEFIELD_CAST_READY;
        }
        return;
    }

    if (this->csState1 >= this->csState2) {
        sEnvType = 0;
        BossTw_TwinrovaSetupDoneBlastShoot(this, play);
        return;
    }

    element = (TwinrovaMagicElement)((sTwinrovaBlastType + this->csState1) & 1);
    magicParams = element == TWINROVA_MAGIC_FIRE ? TW_FIRE_BLAST : TW_ICE_BLAST;
    spawnPos = element == TWINROVA_MAGIC_FIRE ? &this->rightScepterPos : &this->leftScepterPos;
    if (this->work[CAN_SHOOT] == TWINROVA_MINEFIELD_CAST_READY) {
        animation = element == TWINROVA_MAGIC_FIRE ? &gTwinrovaFireAttackAnim : &gTwinrovaIceAttackAnim;
        this->work[CAN_SHOOT] = TWINROVA_MINEFIELD_CAST_ACTIVE;
        Animation_MorphToPlayOnceSetSpeed(&this->skelAnime, animation, TWINROVA_FUSED_ATTACK_ANIM_MORPH_TIME,
                                          TWINROVA_FUSED_ATTACK_ANIM_SPEED);
        this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(animation);
        BossTw_SpawnMagicLaunchEffects(play, spawnPos, element, 4);
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
        return;
    }
    if (Animation_OnFrame(&this->skelAnime, 8.0f)) {
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_THROW_MASIC);
    }
    if (!Animation_OnFrame(&this->skelAnime, 12.0f)) {
        return;
    }

    BossTw_GetMinefieldLandingPoint(this, this->work[TW_PLLR_IDX] + this->csState1, &point);
    mine = BossTw_CountActiveMinefieldMines(play, this) >= TWINROVA_MINEFIELD_MAX_ACTIVE_MINES
               ? NULL
               : BossTw_SpawnMagicBlast(this, play, spawnPos, magicParams, TWINROVA_BLAST_MINEFIELD);
    if (mine != NULL) {
        // Consume Minefield's first-use guarantee only after the arena actually receives a mine. A full actor pool
        // should produce a visible failed cast and leave the move eligible to retry later.
        this->minefieldSeen = true;
        mine->targetPos = point;
        mine->work[TWINROVA_MINE_DETONATION_ORDER] = this->csState1;
        mine->work[TWINROVA_MINE_FINAL_PULSE_PLAYED] = false;
        BossTw_SpawnMagicLaunchEffects(play, spawnPos, element, 10);
    } else {
        BossTw_ShowFailedMagicCast(play, spawnPos, element);
    }

    this->work[CAN_SHOOT] = TWINROVA_MINEFIELD_CAST_RECOVER;
    this->csState1++;
}

static s32 BossTw_CanStartCyclone(BossTw* this, PlayState* play) {
    return this->actor.colChkInfo.health < TWINROVA_FUSED_MAX_HEALTH && this->cycloneCooldown == 0 &&
           sShieldFireCharge == 0 && sShieldIceCharge == 0 && !BossTw_HasActiveArenaControlPattern(this, play) &&
           !GET_PLAYER(play)->bodyIsBurning &&
           !BossTw_HasActiveFusedDirectBlast(play, this) && !BossTw_HasActiveMinefieldMines(play, this);
}

static void BossTw_GetCycloneRoutePosition(BossTw* this, Vec3f* routePos) {
    routePos->x = Math_SinS(this->work[YAW_TGT]) * TWINROVA_CYCLONE_ORBIT_RADIUS;
    routePos->y = TWINROVA_CYCLONE_FLIGHT_HEIGHT;
    routePos->z = Math_CosS(this->work[YAW_TGT]) * TWINROVA_CYCLONE_ORBIT_RADIUS;
}

static void BossTw_TwinrovaSetupCyclone(BossTw* this, PlayState* play) {
    f32 orbitRadius = sqrtf(SQ(this->actor.world.pos.x) + SQ(this->actor.world.pos.z));
    TwinrovaMagicElement element = (TwinrovaMagicElement)sFixedBlastType;
    Vec3f* tellPos = element == TWINROVA_MAGIC_FIRE ? &this->rightScepterPos : &this->leftScepterPos;

    this->actionFunc = BossTw_TwinrovaCyclone;
    this->csState1 = 0;
    this->csState2 = Rand_ZeroOne() < 0.5f ? 1 : -1;
    this->timers[0] = TWINROVA_CYCLONE_WINDUP_TIME;
    this->timers[1] = 0;
    this->cycloneCooldown = BossTw_GetCycloneCooldown(this);
    this->work[YAW_TGT] = orbitRadius > 1.0f
                              ? (s16)(Math_FAtan2F(this->actor.world.pos.x, this->actor.world.pos.z) *
                                      (32768.0f / M_PI))
                              : 0;
    this->actor.speedXZ = 0.0f;
    this->actor.velocity.x = this->actor.velocity.y = this->actor.velocity.z = 0.0f;
    this->attackPortalActive = false;
    this->cycloneSeen = true;
    this->blastType = element;
    this->beamShootState = (s16)Rand_CenteredFloat(65535.0f);
    this->work[CAN_SHOOT] = 0;
    // Cyclone rain is charge-neutral, so do not touch the authored element or sequence of the next normal shield
    // package. The storm may visually use that element without silently spending or restarting a puzzle shot.
    sEnvType = element + 1;
    BossTw_GetCycloneRoutePosition(this, &this->targetPos);
    Animation_MorphToLoopSetSpeed(&this->skelAnime, &gTwinrovaHoverAnim, TWINROVA_FUSED_HOVER_ANIM_MORPH_TIME,
                                  TWINROVA_CYCLONE_HOVER_SPEED);

    BossTw_AddRingEffect(play, tellPos, 0.9f, 4.5f, 255, element, 1, ARRAY_COUNT(sEffects));
    BossTw_SpawnMagicLaunchEffects(play, tellPos, element, 24);
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
    Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_POWERUP);
    Rumble_Request(0.0f, 70, 8, 4);
    play->envCtx.unk_D8 = 1.0f;
}

void BossTw_TwinrovaCyclone(BossTw* this, PlayState* play) {
    Vec3f routePos;
    s16 tangentYaw = this->work[YAW_TGT] + (this->csState2 > 0 ? 0x4000 : -0x4000);
    s16 rainInterval = BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_CYCLONE_RAIN_INTERVAL
                                                      : TWINROVA_CYCLONE_RAIN_INTERVAL;
    s16 rainProjectileCap = BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_CYCLONE_RAIN_MAX_PROJECTILES
                                                           : TWINROVA_CYCLONE_RAIN_MAX_PROJECTILES;

    if (this->twinrovaStun != 0) {
        return;
    }

    SkelAnime_Update(&this->skelAnime);
    BossTw_GetCycloneRoutePosition(this, &routePos);
    sEnvType = this->blastType + 1;

    switch (this->csState1) {
        case 0:
            Math_ApproachF(&this->actor.world.pos.x, routePos.x, 0.2f, 18.0f);
            Math_ApproachF(&this->actor.world.pos.y, routePos.y, 0.2f, 14.0f);
            Math_ApproachF(&this->actor.world.pos.z, routePos.z, 0.2f, 18.0f);
            Math_ApproachS(&this->actor.world.rot.y, tangentYaw, 5, 0x1800);
            Math_ApproachS(&this->actor.shape.rot.y, tangentYaw, 5, 0x1800);
            Math_ApproachS(&this->actor.world.rot.x, TWINROVA_CYCLONE_TILT, 5, 0x300);
            Math_ApproachS(&this->actor.shape.rot.x, TWINROVA_CYCLONE_TILT, 5, 0x300);
            if ((this->timers[0] & 7) == 0) {
                Vec3f* tellPos = this->blastType == TWINROVA_MAGIC_FIRE ? &this->rightScepterPos
                                                                       : &this->leftScepterPos;

                BossTw_SpawnMagicLaunchEffects(play, tellPos, this->blastType, 5);
            }
            if (this->timers[0] == 0) {
                this->csState1 = 1;
                this->timers[0] = TWINROVA_CYCLONE_ACTIVE_TIME;
                this->timers[1] = TWINROVA_CYCLONE_RAIN_OPENING_DELAY;
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_ROLL);
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_TRANSFORM);
                Rumble_Request(0.0f, 110, 8, 4);
            }
            break;

        case 1:
            // The path is authored from one orbit angle, so its speed and gaps remain stable across save/load and
            // Hyper Bosses. The overhead sigil owns the storm while Twinrova's body remains pure movement pressure.
            this->actor.world.pos = routePos;
            this->actor.world.rot.y = tangentYaw;
            this->actor.shape.rot.y = tangentYaw;
            this->actor.world.rot.x = this->actor.shape.rot.x = TWINROVA_CYCLONE_TILT;
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FB_FLY - SFX_FLAG);
            this->work[YAW_TGT] += this->csState2 * TWINROVA_CYCLONE_ANGULAR_SPEED;
            if (this->timers[1] == 0 && this->timers[0] > TWINROVA_CYCLONE_RAIN_STOP_TIME &&
                !BossTw_HasCommittedFusedShieldRelease(play, this)) {
                if (BossTw_CountCycloneRainProjectiles(play, this) <= rainProjectileCap - 2) {
                    // Pair staging is atomic: allocation pressure rolls both the projectile and its marker back. The
                    // explicit live cap also includes impact fades, keeping the denser storm bounded in the actor pool.
                    BossTw_SpawnCycloneRainPair(this, play, this->work[CAN_SHOOT]);
                }
                this->work[CAN_SHOOT]++;
                this->timers[1] = rainInterval;
            }
            if (this->timers[0] == 0) {
                this->csState1 = 2;
                this->timers[0] = TWINROVA_CYCLONE_RECOVERY_TIME;
                // Cyclone has remained on the hover animation throughout the orbit. Restarting the same clip here
                // visibly hitches the exit, so recovery simply coasts the existing motion down.
                BossTw_AddRingEffect(play, &this->actor.world.pos, 0.7f, 3.6f, 255, this->blastType, 1,
                                     ARRAY_COUNT(sEffects));
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
            }
            break;

        case 2: {
            f32 recoveryStrength = this->timers[0] / (f32)TWINROVA_CYCLONE_RECOVERY_TIME;

            // Carry the authored orbit into recovery and bleed its angular speed away. The attack resolves as a
            // deliberate coast rather than snapping from full velocity to a stationary hover.
            this->actor.world.pos = routePos;
            this->actor.world.rot.y = this->actor.shape.rot.y = tangentYaw;
            this->work[YAW_TGT] +=
                (s16)(this->csState2 * TWINROVA_CYCLONE_ANGULAR_SPEED * recoveryStrength);
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FB_FLY - SFX_FLAG);
            Math_ApproachS(&this->actor.world.rot.x, 0, 5, 0x500);
            Math_ApproachS(&this->actor.shape.rot.x, 0, 5, 0x500);
            if (this->timers[0] == 0) {
                sEnvType = 0;
                if (BossTw_HasCommittedFusedShieldRelease(play, this)) {
                    // Respect a release committed by another legal source; Cyclone rain itself cannot create one.
                    BossTw_TwinrovaSetupDoneBlastShoot(this, play);
                } else {
                    BossTw_TwinrovaSetupFly(this, play);
                }
            }
            break;
        }
    }
}

void BossTw_TwinrovaSetupArriveAtTarget(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_TwinrovaArriveAtTarget;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaHoverAnim, -3.0f);
    this->work[CS_TIMER_1] = Rand_ZeroFloat(100.0f);
    this->timers[1] = BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_ARRIVAL_WAIT : TWINROVA_ARRIVAL_WAIT;
    this->rotateSpeed = 0.0f;
}

void BossTw_TwinrovaArriveAtTarget(BossTw* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.1f, fabsf(this->actor.velocity.x) * 1.5f);
    Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.1f, fabsf(this->actor.velocity.y) * 1.5f);
    Math_ApproachF(&this->targetPos.y, 380.0f, 1.0f, 2.0f);
    Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.1f, fabsf(this->actor.velocity.z) * 1.5f);

    if (this->timers[1] == 1) {
        // Twinrova does not share Link's freeze. Begin the visible wind-up on schedule; airborne projectiles keep
        // moving while their collision remains suppressed through the authored thaw grace period.
        BossTw_TwinrovaSetupChargeBlast(this, play);
    }

    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, this->rotateSpeed);
    Math_ApproachF(&this->rotateSpeed, 4096.0f, 1.0f, 350.0f);
}

static void BossTw_EmitFusedElementTell(BossTw* this, PlayState* play, TwinrovaMagicElement element, s32 strongTell) {
    Vec3f* tellPos = element == TWINROVA_MAGIC_FIRE ? &this->rightScepterPos : &this->leftScepterPos;

    sEnvType = element + 1;
    BossTw_AddRingEffect(play, tellPos, strongTell ? 0.8f : 0.5f, strongTell ? 4.0f : 3.2f, 255, element,
                         strongTell ? 0 : 1, ARRAY_COUNT(sEffects));
    BossTw_SpawnMagicLaunchEffects(play, tellPos, element, strongTell ? 30 : 16);
    Audio_PlayActorSound2(&this->actor,
                          strongTell ? NA_SE_EN_TWINROBA_TRANSFORM : NA_SE_EN_TWINROBA_MASIC_SET);
}

static void BossTw_ShowFailedMagicCast(PlayState* play, Vec3f* castPos, TwinrovaMagicElement element) {
    BossTw_AddRingEffect(play, castPos, 0.3f, 1.8f, 180, element, 1, ARRAY_COUNT(sEffects));
    BossTw_SpawnMagicLaunchEffects(play, castPos, element, 6);
    SoundSource_PlaySfxAtFixedWorldPos(play, castPos, 20,
                                      element == TWINROVA_MAGIC_FIRE ? NA_SE_EN_EXTINCT : NA_SE_EV_ICE_MELT);
}

static Vec3f* BossTw_GetFusedAttackTellPos(BossTw* this, TwinrovaMagicElement element) {
    if (this->attackPortalActive) {
        return &this->attackPortalPos;
    }
    return element == TWINROVA_MAGIC_FIRE ? &this->rightScepterPos : &this->leftScepterPos;
}

static s32 BossTw_ShouldRelockFusedAttackOrigin(BossTw* this, Player* player) {
    s32 playerOnLowerDeck = player->actor.floorHeight < TWINROVA_UPPER_FLOOR_MIN_Y;
    s32 portalOnLowerDeck;

    if (!this->attackPortalActive) {
        return playerOnLowerDeck;
    }

    portalOnLowerDeck = this->attackPortalPos.y <= TWINROVA_SIEGE_LOWER_PORTAL_HEIGHT;
    return playerOnLowerDeck != portalOnLowerDeck;
}

static void BossTw_EmitFusedAttackOriginTell(BossTw* this, PlayState* play, TwinrovaMagicElement element,
                                             s32 playCue) {
    Vec3f* tellPos = BossTw_GetFusedAttackTellPos(this, element);

    BossTw_AddRingEffect(play, tellPos, 0.45f, 3.2f, 255, element, 1, ARRAY_COUNT(sEffects));
    BossTw_SpawnMagicLaunchEffects(play, tellPos, element, 8);
    if (this->attackPortalActive) {
        // A remote launch point must announce itself from the ring, even when the scepter has already supplied the
        // element cue. This also makes a deck-change origin relock truthful without duplicating actor-local audio.
        SoundSource_PlaySfxAtFixedWorldPos(play, tellPos, 20, NA_SE_EN_TWINROBA_MASIC_SET);
    } else if (playCue) {
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
    }
}

static void BossTw_LockFusedAttackPortal(BossTw* this, PlayState* play, TwinrovaMagicElement element,
                                          s32 usesLowerRoute, s32 playCue) {
    Player* player = GET_PLAYER(play);
    s32 playerOnLowerDeck = player->actor.floorHeight < TWINROVA_UPPER_FLOOR_MIN_Y;
    s32 usesCommittedWedgeOrigin = this->actionFunc == BossTw_TwinrovaBreakerSequence &&
                                   this->lastSiegePattern == TWINROVA_SIEGE_PATTERN_ROTATING_WEDGE;

    this->attackPortalActive = usesLowerRoute || playerOnLowerDeck;
    if (!this->attackPortalActive) {
        if (playCue) {
            // Returning to the upper deck moves the committed origin back to the visible scepter. Re-announce it
            // just as strongly as a newly-created remote portal before restarting the release animation.
            BossTw_EmitFusedAttackOriginTell(this, play, element, true);
        }
        return;
    }

    if (usesLowerRoute || usesCommittedWedgeOrigin) {
        // A Wedge promises one launch sector on both floors. Preserve the authored target through a deck swap instead
        // of replacing it with the generic player-relative portal and silently changing the attack's stated rule.
        this->attackPortalPos = this->targetPos;
        this->attackPortalPos.y = playerOnLowerDeck ? TWINROVA_SIEGE_LOWER_PORTAL_HEIGHT
                                                    : TWINROVA_SIEGE_UPPER_PORTAL_HEIGHT;
    } else {
        BossTw_GetLowerMagicPortalPos(this, player, &this->attackPortalPos);
    }
    BossTw_EmitFusedAttackOriginTell(this, play, element, playCue);
}

static s16 BossTw_GetFalseChargeCooldown(BossTw* this) {
    return BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_FALSE_CHARGE_COOLDOWN : TWINROVA_FALSE_CHARGE_COOLDOWN;
}

static s32 BossTw_CanArmFalseCharge(BossTw* this, PlayState* play) {
    return this->actor.colChkInfo.health < TWINROVA_FUSED_MAX_HEALTH && this->falseChargeCooldown == 0 &&
           sShieldFireCharge == 0 && sShieldIceCharge == 0 && !BossTw_HasActiveArenaControlPattern(this, play) &&
           !BossTw_HasActiveMinefieldMines(play, this) && !GET_PLAYER(play)->bodyIsBurning &&
           !BossTw_HasActiveFusedDirectBlast(play, this);
}

static s32 BossTw_CanStartBreakerSequence(BossTw* this, PlayState* play) {
    return this->actor.colChkInfo.health < TWINROVA_FUSED_MAX_HEALTH && this->timers[4] == 0 &&
           sShieldFireCharge == 0 && sShieldIceCharge == 0 && !BossTw_IsPlayerHardDisabled(play) &&
           !BossTw_HasActiveArenaControlPattern(this, play) && !GET_PLAYER(play)->bodyIsBurning &&
           !BossTw_HasActiveFusedDirectBlast(play, this) && !BossTw_HasActiveMinefieldMines(play, this);
}

static TwinrovaFusedAttack BossTw_SelectFusedAttack(BossTw* this, PlayState* play) {
    s32 canFalseCharge = BossTw_CanArmFalseCharge(this, play);
    s32 canBreaker = BossTw_CanStartBreakerSequence(this, play);
    s32 canMinefield = BossTw_CanStartMinefield(this, play);
    s32 canCyclone = this->breakerSequenceSeen && BossTw_CanStartCyclone(this, play);
    s16 weights[TWINROVA_FUSED_ATTACK_MAX];
    s16 totalWeight = 0;
    s16 roll;
    s16 i;

    weights[TWINROVA_FUSED_ATTACK_NORMAL] =
        BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_FUSED_NORMAL_WEIGHT : TWINROVA_FUSED_NORMAL_WEIGHT;
    weights[TWINROVA_FUSED_ATTACK_FALSE_CHARGE] = canFalseCharge ? TWINROVA_FUSED_FALSE_CHARGE_WEIGHT : 0;
    weights[TWINROVA_FUSED_ATTACK_BREAKER] = canBreaker ? TWINROVA_FUSED_BREAKER_WEIGHT : 0;
    weights[TWINROVA_FUSED_ATTACK_MINEFIELD] = canMinefield ? TWINROVA_FUSED_MINEFIELD_WEIGHT : 0;
    weights[TWINROVA_FUSED_ATTACK_CYCLONE] =
        canCyclone ? (BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_FUSED_CYCLONE_WEIGHT
                                                     : TWINROVA_FUSED_CYCLONE_WEIGHT)
                   : 0;

    // Teach and then exercise every major phase-two rule before the mature weighted rotation takes over. Context
    // gates remain authoritative, so a temporarily illegal move waits for the next clean scheduler opportunity.
    if (!this->breakerSequenceSeen && canBreaker) {
        return TWINROVA_FUSED_ATTACK_BREAKER;
    }
    if (this->breakerSequenceSeen && !this->cycloneSeen && canCyclone) {
        return TWINROVA_FUSED_ATTACK_CYCLONE;
    }
    if (this->breakerSequenceSeen && this->cycloneSeen && !this->minefieldSeen && canMinefield) {
        return TWINROVA_FUSED_ATTACK_MINEFIELD;
    }
    if (this->breakerSequenceSeen && this->cycloneSeen && this->minefieldSeen && !this->falseChargeSeen &&
        canFalseCharge) {
        return TWINROVA_FUSED_ATTACK_FALSE_CHARGE;
    }

    for (i = 0; i < TWINROVA_FUSED_ATTACK_MAX; i++) {
        totalWeight += weights[i];
    }
    roll = (s16)Rand_ZeroFloat(totalWeight);
    for (i = 0; i < TWINROVA_FUSED_ATTACK_MAX; i++) {
        if (roll < weights[i]) {
            return (TwinrovaFusedAttack)i;
        }
        roll -= weights[i];
    }
    return TWINROVA_FUSED_ATTACK_NORMAL;
}

static void BossTw_TwinrovaSetupFalseCharge(BossTw* this, PlayState* play) {
    TwinrovaMagicElement actualElement = (TwinrovaMagicElement)!sTwinrovaBlastType;

    // The original wind-up is deliberately false. Count the opposite release as the first real shot in its new
    // three-shot package, so the feint tests tracking rather than corrupting the Mirror Shield's charge lesson.
    sTwinrovaBlastType = sFixedBlastType = actualElement;
    sFixedBlatSeq = 1;
    this->pendingFusedAttack = TWINROVA_FUSED_ATTACK_NORMAL;
    this->actionFunc = BossTw_TwinrovaFalseCharge;
    this->timers[0] = TWINROVA_FALSE_CHARGE_REPOSITION_TIME;
    this->work[TW_PLLR_IDX] = (this->work[TW_PLLR_IDX] + Rand_S16Offset(1, 3)) % 4;
    this->targetPos = sTwinrovaPillarPos[this->work[TW_PLLR_IDX]];
    this->targetPos.y = 400.0f;
    this->actor.speedXZ = 0.0f;
    this->actor.velocity.x = this->actor.velocity.y = this->actor.velocity.z = 0.0f;
    this->attackPortalActive = false;
    Animation_MorphToLoopSetSpeed(&this->skelAnime, &gTwinrovaHoverAnim, TWINROVA_FUSED_HOVER_ANIM_MORPH_TIME,
                                  TWINROVA_FALSE_CHARGE_ANIM_SPEED);
    // The abandoned origin gets a sharp color-switch flash, not the long-lived strong ring used by trustworthy
    // commitments. The normal release setup will mark the real post-reposition scepter/portal through its shot frame.
    BossTw_EmitFusedElementTell(this, play, actualElement, false);
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_TRANSFORM);
    play->envCtx.unk_D8 = 0.8f;
    Rumble_Request(0.0f, 65, 6, 3);
}

void BossTw_TwinrovaFalseCharge(BossTw* this, PlayState* play) {
    TwinrovaMagicElement actualElement = (TwinrovaMagicElement)sTwinrovaBlastType;

    SkelAnime_Update(&this->skelAnime);
    Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.35f, TWINROVA_FALSE_CHARGE_MOVE_STEP);
    Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.35f, TWINROVA_FALSE_CHARGE_MOVE_STEP * 0.75f);
    Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.35f, TWINROVA_FALSE_CHARGE_MOVE_STEP);
    Math_ApproachS(&this->actor.world.rot.y, this->actor.yawTowardsPlayer, 5, 0x1800);
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1800);

    if ((this->timers[0] & 3) == 0) {
        Vec3f* tellPos = actualElement == TWINROVA_MAGIC_FIRE ? &this->rightScepterPos : &this->leftScepterPos;

        BossTw_SpawnMagicLaunchEffects(play, tellPos, actualElement, 4);
    }

    if (this->timers[0] == 0) {
        BossTw_TwinrovaSetupShootBlast(this, play);
    }
}

void BossTw_TwinrovaSetupChargeBlast(BossTw* this, PlayState* play) {
    TwinrovaFusedAttack attack = BossTw_SelectFusedAttack(this, play);

    this->pendingFusedAttack = attack;
    if (attack == TWINROVA_FUSED_ATTACK_CYCLONE) {
        this->pendingFusedAttack = TWINROVA_FUSED_ATTACK_NORMAL;
        BossTw_TwinrovaSetupCyclone(this, play);
        return;
    }
    if (attack == TWINROVA_FUSED_ATTACK_MINEFIELD) {
        this->pendingFusedAttack = TWINROVA_FUSED_ATTACK_NORMAL;
        BossTw_TwinrovaSetupMinefield(this, play);
        return;
    }

    this->actionFunc = BossTw_TwinrovaChargeBlast;
    this->csState1 = 0;
    BossTw_SelectTwinrovaBlastType();
    if (attack == TWINROVA_FUSED_ATTACK_FALSE_CHARGE) {
        this->falseChargeSeen = true;
        this->falseChargeCooldown = BossTw_GetFalseChargeCooldown(this);
    }
    Animation_MorphToPlayOnceSetSpeed(&this->skelAnime, &gTwinrovaWindUpAnim, TWINROVA_FUSED_CHARGE_ANIM_MORPH_TIME,
                                      attack == TWINROVA_FUSED_ATTACK_FALSE_CHARGE
                                          ? TWINROVA_FALSE_CHARGE_ANIM_SPEED
                                          : TWINROVA_FUSED_CHARGE_ANIM_SPEED);
    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaWindUpAnim);
    BossTw_EmitFusedElementTell(this, play, (TwinrovaMagicElement)sTwinrovaBlastType, false);
}

static void BossTw_SelectTwinrovaBlastType(void) {
    // Authored three-shot packages guarantee that mastery, rather than low-health RNG, earns a release.
    if (++sFixedBlatSeq >= 4) {
        sFixedBlatSeq = 1;
        sFixedBlastType = !sFixedBlastType;
    }

    sTwinrovaBlastType = sFixedBlastType;
}

static void BossTw_SetTwinrovaAttackAnimation(BossTw* this, TwinrovaMagicElement element) {
    AnimationHeader* animation = element == TWINROVA_MAGIC_ICE ? &gTwinrovaIceAttackAnim : &gTwinrovaFireAttackAnim;

    // Preserve the end of a reposition/hover pose while letting the actual cast run faster. The old zero-frame switch
    // was especially visible when False Charge jumped straight from its fake-out into the real release.
    Animation_MorphToPlayOnceSetSpeed(&this->skelAnime, animation, TWINROVA_FUSED_ATTACK_ANIM_MORPH_TIME,
                                      TWINROVA_FUSED_ATTACK_ANIM_SPEED);
    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(animation);
}

static void BossTw_BeginMinefieldFollowupShot(BossTw* this, PlayState* play) {
    TwinrovaMagicElement element = (TwinrovaMagicElement)this->csState2;

    this->work[CAN_SHOOT] = false;
    this->minefieldFollowupSuppressed = false;
    this->beamShootState = TWINROVA_BREAKER_STATE_SHOOT;
    BossTw_EmitFusedElementTell(this, play, element, false);
    BossTw_LockFusedAttackPortal(this, play, element, false, false);
    BossTw_SetTwinrovaAttackAnimation(this, element);
}

static void BossTw_TwinrovaSetupMinefieldFollowup(BossTw* this, PlayState* play) {
    // Three matching ordinary shots preserve the vanilla shield lesson and create an offensive opportunity while the
    // independent Minefield timer keeps applying arena pressure. The resulting blast cannot erase mines or their pools.
    TwinrovaMagicElement element = (TwinrovaMagicElement)!sTwinrovaBlastType;

    this->minefieldSequenceState = TWINROVA_MINEFIELD_SEQUENCE_ACTIVE;
    this->actionFunc = BossTw_TwinrovaMinefieldFollowup;
    this->csState1 = 0;
    this->csState2 = element;
    this->actor.speedXZ = 0.0f;
    this->actor.velocity.x = this->actor.velocity.y = this->actor.velocity.z = 0.0f;
    this->attackPortalActive = false;
    sTwinrovaBlastType = sFixedBlastType = element;
    sFixedBlatSeq = 0;
    BossTw_BeginMinefieldFollowupShot(this, play);
}

static void BossTw_TwinrovaMinefieldFollowup(BossTw* this, PlayState* play) {
    BossTw* magic;
    Player* player = GET_PLAYER(play);
    TwinrovaMagicElement element = (TwinrovaMagicElement)this->csState2;
    Vec3f* spawnPos;
    s16 magicParams = element == TWINROVA_MAGIC_FIRE ? TW_FIRE_BLAST : TW_ICE_BLAST;

    SkelAnime_Update(&this->skelAnime);
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1000);

    if (!BossTw_HasActiveMinefieldMines(play, this)) {
        // Allocation pressure may exhaust every mine before a retry succeeds. Leave the authored package cleanly
        // instead of trapping Twinrova in a permanent cast loop; already-released shots continue independently.
        this->minefieldSequenceState = BossTw_CountActiveMinefieldPools(play, this) != 0
                                           ? TWINROVA_MINEFIELD_SEQUENCE_OVERLAP
                                           : TWINROVA_MINEFIELD_SEQUENCE_NONE;
        this->work[CAN_SHOOT] = false;
        BossTw_TwinrovaSetupDoneBlastShoot(this, play);
        return;
    }

    if (this->beamShootState == TWINROVA_BREAKER_STATE_GAP) {
        if (this->timers[0] == 0) {
            BossTw_BeginMinefieldFollowupShot(this, play);
        }
        return;
    }

    if (!this->work[CAN_SHOOT] && BossTw_ShouldRelockFusedAttackOrigin(this, player)) {
        // A floor change before commitment restarts the short cast from a newly marked, truthful origin.
        BossTw_LockFusedAttackPortal(this, play, element, false, true);
        BossTw_SetTwinrovaAttackAnimation(this, element);
        return;
    }

    if (Animation_OnFrame(&this->skelAnime, 8.0f)) {
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_THROW_MASIC);
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_YOUNG_SHOOTVC);
    }

    if (Animation_OnFrame(&this->skelAnime, 12.0f)) {
        spawnPos = element == TWINROVA_MAGIC_FIRE ? &this->rightScepterPos : &this->leftScepterPos;
        if (this->attackPortalActive) {
            spawnPos = &this->attackPortalPos;
        }

        magic = BossTw_SpawnMagicBlast(this, play, spawnPos, magicParams, TWINROVA_BLAST_MINEFIELD_FOLLOWUP);
        if (magic != NULL) {
            this->work[CAN_SHOOT] = true;
            this->minefieldFollowupSuppressed = BossTw_IsPlayerHardDisabled(play);
            magic->minefieldFollowupSuppressed = this->minefieldFollowupSuppressed;
            BossTw_SpawnMagicLaunchEffects(play, spawnPos, element, TWINROVA_NORMAL_BLAST_SPAWN_EFFECTS);
        } else {
            BossTw_ShowFailedMagicCast(play, spawnPos, element);
        }
    }

    if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
        if (this->work[CAN_SHOOT] && !this->minefieldFollowupSuppressed) {
            this->csState1++;
        }

        if (this->csState1 >= TWINROVA_MINEFIELD_FOLLOWUP_SHOTS) {
            // If Link has not already completed the package, resume the ordinary scheduler at its normal element
            // boundary. A very close third intercept may already have committed the shield release and authored the
            // opposite next package in BossTw_BlastShieldCheck; never overwrite that earned state here.
            if (sShieldFireCharge < 3 && sShieldIceCharge < 3 &&
                !BossTw_HasCommittedFusedShieldRelease(play, this)) {
                sFixedBlatSeq = 3;
            }
            BossTw_AccelerateMinefieldFuses(play, this);
            this->minefieldSequenceState = TWINROVA_MINEFIELD_SEQUENCE_OVERLAP;
            this->work[CAN_SHOOT] = false;
            BossTw_TwinrovaSetupDoneBlastShoot(this, play);
            return;
        }

        // Launches are compact and may coexist in flight, but their full attack animations keep each shield beat clear.
        // Allocation failure retries the same slot after the gap rather than silently shortening the promised package.
        this->work[CAN_SHOOT] = false;
        this->beamShootState = TWINROVA_BREAKER_STATE_GAP;
        this->timers[0] = TWINROVA_MINEFIELD_FOLLOWUP_GAP;
        Animation_MorphToLoop(&this->skelAnime, &gTwinrovaHoverAnim, -3.0f);
    }
}

static TwinrovaMagicElement BossTw_GetBreakerShotElement(BossTw* this) {
    if (this->csState1 == TWINROVA_BREAKER_SHOT_INDEX) {
        return (TwinrovaMagicElement)!this->csState2;
    }
    return (TwinrovaMagicElement)this->csState2;
}

static void BossTw_ArmBreakerCooldown(BossTw* this) {
    if (this->actionFunc == BossTw_TwinrovaBreakerSequence) {
        this->timers[4] = BossTw_GetBreakerSequenceCooldown(this);
    }
}

static void BossTw_BeginBreakerShot(BossTw* this, PlayState* play) {
    TwinrovaMagicElement element = BossTw_GetBreakerShotElement(this);
    s32 usesLowerRoute = this->targetPos.y == TWINROVA_SIEGE_LOWER_ATTACK_HEIGHT;

    sTwinrovaBlastType = element;
    this->work[CAN_SHOOT] = false;
    BossTw_EmitFusedElementTell(this, play, element, this->csState1 == TWINROVA_BREAKER_SHOT_INDEX);
    if (this->csState1 == TWINROVA_BREAKER_SHOT_INDEX) {
        this->beamShootState = TWINROVA_BREAKER_STATE_TELL;
        this->timers[0] = TWINROVA_BREAKER_TELL_TIME;
        Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaWindUpAnim, -3.0f);
        this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaWindUpAnim);
        play->envCtx.unk_D8 = 1.0f;
    } else {
        this->beamShootState = TWINROVA_BREAKER_STATE_SHOOT;
        BossTw_SetTwinrovaAttackAnimation(this, element);
    }
    // Commit any remote launch point before the shot. Its ring now communicates both element and origin.
    BossTw_LockFusedAttackPortal(this, play, element, usesLowerRoute, false);
}

static void BossTw_TwinrovaSetupBreakerSequence(BossTw* this, PlayState* play) {
    TwinrovaMagicElement siegeElement = (TwinrovaMagicElement)!sTwinrovaBlastType;

    this->actionFunc = BossTw_TwinrovaBreakerSequence;
    this->pendingFusedAttack = TWINROVA_FUSED_ATTACK_NORMAL;
    this->actor.speedXZ = 0.0f;
    this->csState1 = 0;
    this->csState2 = sTwinrovaBlastType;
    this->beamShootState = TWINROVA_BREAKER_STATE_SIEGE_TELEGRAPH;
    this->timers[0] = TWINROVA_SIEGE_TELEGRAPH_TIME;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaHoverAnim, -5.0f);
    if (!BossTw_SpawnSiegeZones(this, play, siegeElement)) {
        // Fall back to the already-telegraphed normal shot and avoid immediately retrying under actor pressure.
        this->timers[4] = BossTw_GetBreakerSequenceCooldown(this);
        BossTw_TwinrovaSetupShootBlast(this, play);
        return;
    }
    this->breakerSequenceSeen = true;
    sEnvType = siegeElement + 1;
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
}

void BossTw_TwinrovaChargeBlast(BossTw* this, PlayState* play) {
    Vec3f* tellPos = sTwinrovaBlastType == TWINROVA_MAGIC_FIRE ? &this->rightScepterPos : &this->leftScepterPos;

    SkelAnime_Update(&this->skelAnime);

    Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.03f, fabsf(this->actor.velocity.x) * 1.5f);
    Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.03f, fabsf(this->actor.velocity.y) * 1.5f);
    Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.03f, fabsf(this->actor.velocity.z) * 1.5f);
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1000);

    if ((this->work[CS_TIMER_1] & 7) == 0) {
        BossTw_SpawnMagicLaunchEffects(play, tellPos, sTwinrovaBlastType, 2);
    }

    if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
        if (this->pendingFusedAttack == TWINROVA_FUSED_ATTACK_FALSE_CHARGE) {
            BossTw_TwinrovaSetupFalseCharge(this, play);
            return;
        }
        if (this->pendingFusedAttack == TWINROVA_FUSED_ATTACK_BREAKER &&
            BossTw_CanStartBreakerSequence(this, play)) {
            BossTw_TwinrovaSetupBreakerSequence(this, play);
        } else {
            this->pendingFusedAttack = TWINROVA_FUSED_ATTACK_NORMAL;
            BossTw_TwinrovaSetupShootBlast(this, play);
        }
    }
}

void BossTw_TwinrovaSetupShootBlast(BossTw* this, PlayState* play) {
    TwinrovaMagicElement element = (TwinrovaMagicElement)sTwinrovaBlastType;

    this->actionFunc = BossTw_TwinrovaShootBlast;
    this->work[CAN_SHOOT] = false;
    BossTw_SetTwinrovaAttackAnimation(this, element);
    // The original wind-up outlives its setup ring; renew the selected element at commitment.
    BossTw_EmitFusedElementTell(this, play, element, false);
    BossTw_LockFusedAttackPortal(this, play, element, false, false);
}

void BossTw_TwinrovaShootBlast(BossTw* this, PlayState* play) {
    BossTw* twMagic;
    Player* player = GET_PLAYER(play);
    Vec3f* magicSpawnPos;
    TwinrovaMagicElement element = (TwinrovaMagicElement)sTwinrovaBlastType;
    TwinrovaBlastBehavior behavior;
    s32 useLowerPortal;
    s32 magicParams;

    if (!this->work[CAN_SHOOT] && BossTw_ShouldRelockFusedAttackOrigin(this, player)) {
        // A deck change invalidates either the scepter or portal origin. Re-author it and restart the complete attack
        // animation so Link always gets the same twelve-update commitment window in both directions.
        BossTw_LockFusedAttackPortal(this, play, element, false, true);
        BossTw_SetTwinrovaAttackAnimation(this, element);
        return;
    }

    SkelAnime_Update(&this->skelAnime);

    if (Animation_OnFrame(&this->skelAnime, 8.0f)) {
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_THROW_MASIC);
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_YOUNG_SHOOTVC);
    }

    if (Animation_OnFrame(&this->skelAnime, 12.0f)) {
        // Origin re-authoring is legal only before this commitment point. Mark the slot spent even if actor
        // allocation fails, so a later deck change cannot retry or duplicate the deterministic package shot.
        this->work[CAN_SHOOT] = true;
        behavior = TWINROVA_BLAST_STRAIGHT;
        if (sTwinrovaBlastType != 0) {
            magicParams = TW_FIRE_BLAST;
            magicSpawnPos = &this->rightScepterPos;
        } else {
            magicParams = TW_ICE_BLAST;
            magicSpawnPos = &this->leftScepterPos;
        }

        useLowerPortal = this->attackPortalActive;
        if (useLowerPortal) {
            magicSpawnPos = &this->attackPortalPos;
            behavior = TWINROVA_BLAST_LOWER_ROUTE;
        }
        if (this->minefieldSequenceState == TWINROVA_MINEFIELD_SEQUENCE_OVERLAP) {
            // Ordinary shots keep Twinrova aggressive during the mine tail, but they resolve cleanly on a dodge. Do
            // not stack a full ground pool and another summon warning onto the already-authoritative mine pattern.
            behavior = TWINROVA_BLAST_MINEFIELD_FOLLOWUP;
        }

        twMagic = BossTw_SpawnMagicBlast(this, play, magicSpawnPos, magicParams, behavior);
        if (twMagic != NULL) {
            if (useLowerPortal) {
                BossTw_AddRingEffect(play, magicSpawnPos, 0.5f, 3.2f, 255, twMagic->blastType, 1,
                                     ARRAY_COUNT(sEffects));
                Audio_PlayActorSound2(&twMagic->actor, NA_SE_EN_TWINROBA_MASIC_SET);
            }
            // Keep routine releases below the signature sequence's visual hierarchy and preserve shared effect slots
            // for shield, portal, and arena-sigil feedback.
            BossTw_SpawnMagicLaunchEffects(play, magicSpawnPos, twMagic->blastType,
                                            TWINROVA_NORMAL_BLAST_SPAWN_EFFECTS);
        } else {
            // Actor-pool pressure can cancel the projectile, but never the player's read of what happened.
            BossTw_ShowFailedMagicCast(play, magicSpawnPos, element);
            // Preserve the guaranteed three-successful-shot elemental package under actor-pool pressure.
            if (sFixedBlatSeq != 0) {
                sFixedBlatSeq--;
            }
            sEnvType = 0;
        }
    }

    if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
        BossTw_TwinrovaSetupDoneBlastShoot(this, play);
    }

    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1000);
}

void BossTw_TwinrovaBreakerSequence(BossTw* this, PlayState* play) {
    BossTw* twMagic;
    Player* player = GET_PLAYER(play);
    Vec3f* magicSpawnPos;
    TwinrovaMagicElement element;
    TwinrovaBlastBehavior behavior;
    f32 xzMoveStep;
    f32 yTarget;
    s32 usesLowerRoute;
    s16 magicParams;

    if (this->twinrovaStun != 0) {
        return;
    }
    if (BossTw_HasCommittedFusedShieldRelease(play, this)) {
        BossTw_TwinrovaSetupDoneBlastShoot(this, play);
        return;
    }

    usesLowerRoute = this->targetPos.y == TWINROVA_SIEGE_LOWER_ATTACK_HEIGHT;
    if (this->beamShootState == TWINROVA_BREAKER_STATE_SHOOT && !this->work[CAN_SHOOT] &&
        BossTw_ShouldRelockFusedAttackOrigin(this, player)) {
        // As with an ordinary shot, changing decks moves the launch origin and earns a fresh tell in either direction.
        element = BossTw_GetBreakerShotElement(this);
        BossTw_LockFusedAttackPortal(this, play, element, usesLowerRoute, true);
        BossTw_SetTwinrovaAttackAnimation(this, element);
        return;
    }

    SkelAnime_Update(&this->skelAnime);
    if (this->targetPos.y == TWINROVA_SIEGE_LOWER_ATTACK_HEIGHT) {
        xzMoveStep = TWINROVA_SIEGE_LOWER_MOVE_STEP;
    } else if (this->beamShootState == TWINROVA_BREAKER_STATE_SIEGE_TELEGRAPH) {
        // The layout promises that Twinrova will attack from its remaining side. Use the tell as committed movement
        // time so even an opposite-pillar start reaches that authored origin before the sigils become dangerous.
        xzMoveStep = TWINROVA_SIEGE_UPPER_MOVE_STEP;
    } else {
        xzMoveStep = 12.0f;
    }
    Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.06f, xzMoveStep);
    Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.06f, xzMoveStep);
    yTarget = this->targetPos.y;
    if (yTarget == TWINROVA_SIEGE_LOWER_ATTACK_HEIGHT &&
        (SQ(this->actor.world.pos.x) + SQ(this->actor.world.pos.z)) < SQ(TWINROVA_SIEGE_LOWER_DESCEND_RADIUS)) {
        // Cross the raised center at normal hover height, then descend only after reaching the clear outer floor.
        yTarget = 420.0f;
    }
    Math_ApproachF(&this->actor.world.pos.y, yTarget, 0.06f, 8.0f);
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1000);

    switch (this->beamShootState) {
        case TWINROVA_BREAKER_STATE_SIEGE_TELEGRAPH:
            if (this->timers[0] == TWINROVA_SIEGE_WEDGE_LOCK_TIME &&
                this->lastSiegePattern == TWINROVA_SIEGE_PATTERN_ROTATING_WEDGE) {
                // Moving child zones update sequentially, so nearest-zone audio can change identity on their lock
                // frame. Author this one cue from the fused boss while every zone still emits its positional burst.
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
            }
            if (this->timers[0] == TWINROVA_SIEGE_FINAL_WARNING_TIME) {
                BossTw_EmitFusedElementTell(this, play, (TwinrovaMagicElement)this->csState2, false);
                // The scepter previews the opening shot, but the arena lighting must keep identifying the opposing
                // Siege zones until they actually detonate. BeginBreakerShot switches the ambience at commitment.
                sEnvType = !this->csState2 + 1;
            }
            if (this->timers[0] == 0) {
                BossTw_BeginBreakerShot(this, play);
            }
            break;

        case TWINROVA_BREAKER_STATE_TELL:
            element = BossTw_GetBreakerShotElement(this);
            magicSpawnPos = BossTw_GetFusedAttackTellPos(this, element);
            if ((this->timers[0] & 3) == 0) {
                Vec3f velocity = { Rand_CenteredFloat(10.0f), Rand_CenteredFloat(10.0f), Rand_CenteredFloat(10.0f) };

                BossTw_AddDotEffect(play, magicSpawnPos, &velocity, &sZeroVector, Rand_ZeroFloat(4.0f) + 10.0f,
                                    element, ARRAY_COUNT(sEffects));
            }
            if (this->timers[0] == 0) {
                if (BossTw_ShouldRelockFusedAttackOrigin(this, player)) {
                    // Relocking emits the commitment pulse from the newly-authoritative origin.
                    BossTw_LockFusedAttackPortal(this, play, element, usesLowerRoute, true);
                } else {
                    // Bridge the final two visual updates before release with a smaller commitment pulse at the
                    // already-locked launch point. The long color-switch warning remains unchanged.
                    BossTw_EmitFusedAttackOriginTell(this, play, element, true);
                }
                this->beamShootState = TWINROVA_BREAKER_STATE_SHOOT;
                BossTw_SetTwinrovaAttackAnimation(this, element);
            }
            break;

        case TWINROVA_BREAKER_STATE_SHOOT:
            if (Animation_OnFrame(&this->skelAnime, 8.0f)) {
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_THROW_MASIC);
                Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_YOUNG_SHOOTVC);
            }

            if (Animation_OnFrame(&this->skelAnime, 12.0f)) {
                element = BossTw_GetBreakerShotElement(this);
                if (element == TWINROVA_MAGIC_FIRE) {
                    magicParams = TW_FIRE_BLAST;
                    magicSpawnPos = &this->rightScepterPos;
                } else {
                    magicParams = TW_ICE_BLAST;
                    magicSpawnPos = &this->leftScepterPos;
                }
                if (this->attackPortalActive) {
                    magicSpawnPos = &this->attackPortalPos;
                }
                behavior = this->csState1 == TWINROVA_BREAKER_SHOT_INDEX ? TWINROVA_BLAST_BREAKER
                                                                         : TWINROVA_BLAST_SIEGE;
                twMagic = BossTw_SpawnMagicBlast(this, play, magicSpawnPos, magicParams, behavior);
                if (twMagic != NULL) {
                    this->work[CAN_SHOOT] = true;
                    if (this->attackPortalActive) {
                        BossTw_AddRingEffect(play, magicSpawnPos, 0.45f, 3.2f, 255, twMagic->blastType, 1,
                                             ARRAY_COUNT(sEffects));
                    }
                    BossTw_SpawnMagicLaunchEffects(play, magicSpawnPos, twMagic->blastType, 24);
                } else {
                    BossTw_ShowFailedMagicCast(play, magicSpawnPos, element);
                }
            }

            if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
                if (!this->work[CAN_SHOOT]) {
                    // Actor-pool exhaustion must not trap Twinrova in a retry loop with permanent siege zones.
                    if (this->csState1 == 0 && sFixedBlatSeq != 0) {
                        // No shot was delivered, so retry this package slot on the next attack cycle.
                        sFixedBlatSeq--;
                        // The first signature sequence is guaranteed until its opening projectile actually exists.
                        this->breakerSequenceSeen = false;
                    } else if (this->csState1 != 0) {
                        // Preserve a coherent matching-element rebuild whether allocation failed before or after
                        // the breaker. The next normal package supplies three clean shots of the base element.
                        sFixedBlastType = this->csState2;
                        sFixedBlatSeq = 0;
                    }
                    // Extinguish the already-promised layout visibly when the projectile cannot be allocated.
                    BossTw_ClearSiegeZones(this, play, true, TWINROVA_SIEGE_USE_ZONE_ELEMENT, false);
                    sEnvType = 0;
                    BossTw_TwinrovaSetupDoneBlastShoot(this, play);
                } else {
                    this->csState1++;
                    // Start cadence and recovery only after the released projectile has resolved.
                    this->beamShootState = TWINROVA_BREAKER_STATE_GAP;
                    this->timers[0] = 0;
                    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaHoverAnim, -3.0f);
                }
            }
            break;

        case TWINROVA_BREAKER_STATE_GAP:
            if (BossTw_HasActiveBreakerProjectile(play, this)) {
                break;
            }
            if (this->work[CAN_SHOOT]) {
                this->work[CAN_SHOOT] = false;
                if (this->csState1 >= TWINROVA_BREAKER_SEQUENCE_SHOTS) {
                    BossTw_TwinrovaSetupDoneBlastShoot(this, play);
                    break;
                }
                this->timers[0] = this->csState1 == TWINROVA_BREAKER_SHOT_INDEX + 1
                                      ? TWINROVA_BREAKER_RECOVERY_GAP
                                      : TWINROVA_BREAKER_SHOT_GAP;
                break;
            }
            if (this->timers[0] == 0) {
                BossTw_BeginBreakerShot(this, play);
            }
            break;
    }
}

void BossTw_TwinrovaSetupDoneBlastShoot(BossTw* this, PlayState* play) {
    s32 useSpecialRecovery = this->actionFunc == BossTw_TwinrovaBreakerSequence ||
                             BossTw_HasCommittedFusedShieldRelease(play, this);
    s16 normalRecovery =
        BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_NORMAL_SHOT_RECOVERY : TWINROVA_NORMAL_SHOT_RECOVERY;
    s16 specialRecovery =
        BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_SPECIAL_SHOT_RECOVERY : TWINROVA_SPECIAL_SHOT_RECOVERY;

    BossTw_ArmBreakerCooldown(this);
    this->attackPortalActive = false;
    this->actionFunc = BossTw_TwinrovaDoneBlastShoot;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaHoverAnim, -10.0f);
    this->timers[1] = useSpecialRecovery ? specialRecovery : normalRecovery;
}

void BossTw_TwinrovaDoneBlastShoot(BossTw* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if (this->timers[1] == 0 && this->minefieldSequenceState == TWINROVA_MINEFIELD_SEQUENCE_PENDING &&
        BossTw_AreAllMinefieldMinesArmed(play, this)) {
        // This is the sole intentional overlap: an authored ordinary package begins only after every destination and
        // fuse is readable. Signature arena patterns remain gated by the live mines.
        BossTw_TwinrovaSetupMinefieldFollowup(this, play);
        return;
    }
    if (this->minefieldSequenceState == TWINROVA_MINEFIELD_SEQUENCE_PENDING &&
        !BossTw_HasActiveMinefieldMines(play, this)) {
        // Actor allocation may have prevented the layout entirely; do not carry a stale follow-up into another cycle.
        this->minefieldSequenceState = BossTw_CountActiveMinefieldPools(play, this) != 0
                                           ? TWINROVA_MINEFIELD_SEQUENCE_OVERLAP
                                           : TWINROVA_MINEFIELD_SEQUENCE_NONE;
    }

    if (this->minefieldSequenceState == TWINROVA_MINEFIELD_SEQUENCE_OVERLAP) {
        if (!BossTw_HasActiveMinefieldMines(play, this) && BossTw_CountActiveMinefieldPools(play, this) == 0) {
            this->minefieldSequenceState = TWINROVA_MINEFIELD_SEQUENCE_NONE;
        } else if (this->timers[1] == 0 && !BossTw_HasCommittedFusedShieldRelease(play, this) &&
                   !BossTw_HasActiveFusedNonMineBlast(play, this)) {
            // Once the authored shield package has resolved, mines become background pressure. Ordinary movement and
            // shots resume while every other arena-control special remains gated by the live mine/pool predicates.
            BossTw_TwinrovaSetupFly(this, play);
            return;
        }
    }

    if (this->timers[1] == 0 && !BossTw_HasCommittedFusedShieldRelease(play, this) &&
        !BossTw_HasActiveFusedDirectBlast(play, this) && !BossTw_HasActiveArenaControlPattern(this, play)) {
        BossTw_TwinrovaSetupFly(this, play);
    }

    D_8094C870 = 0;
    Math_ApproachS(&this->actor.shape.rot.y, this->actor.yawTowardsPlayer, 5, 0x1000);
}

static void BossTw_EmitStunRecoveryTell(BossTw* this, PlayState* play) {
    BossTw_AddRingEffect(play, &this->leftScepterPos, 0.5f, 3.5f, 255, TWINROVA_MAGIC_ICE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_AddRingEffect(play, &this->rightScepterPos, 0.5f, 3.5f, 255, TWINROVA_MAGIC_FIRE, 1,
                         ARRAY_COUNT(sEffects));
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_SENSE);
}

static void BossTw_EmitStunBudgetWarning(BossTw* this, PlayState* play) {
    BossTw_AddRingEffect(play, &this->leftScepterPos, 0.35f, 3.0f, 220, TWINROVA_MAGIC_ICE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_AddRingEffect(play, &this->rightScepterPos, 0.35f, 3.0f, 220, TWINROVA_MAGIC_FIRE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_SpawnMagicLaunchEffects(play, &this->leftScepterPos, TWINROVA_MAGIC_ICE, 4);
    BossTw_SpawnMagicLaunchEffects(play, &this->rightScepterPos, TWINROVA_MAGIC_FIRE, 4);
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_MASIC_SET);
}

static void BossTw_ShowFusedFinalCycleEscalation(BossTw* this, PlayState* play) {
    s16 breakerCooldown = BossTw_GetBreakerSequenceCooldown(this);
    s16 cycloneCooldown = BossTw_GetCycloneCooldown(this);
    s16 falseChargeCooldown = BossTw_GetFalseChargeCooldown(this);
    s16 minefieldCooldown = BossTw_GetMinefieldCooldown(this);

    BossTw_AddRingEffect(play, &this->leftScepterPos, 0.8f, 4.0f, 255, TWINROVA_MAGIC_ICE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_AddRingEffect(play, &this->rightScepterPos, 0.8f, 4.0f, 255, TWINROVA_MAGIC_FIRE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_SpawnMagicLaunchEffects(play, &this->leftScepterPos, TWINROVA_MAGIC_ICE, 16);
    BossTw_SpawnMagicLaunchEffects(play, &this->rightScepterPos, TWINROVA_MAGIC_FIRE, 16);
    Sfx_PlaySfxCentered(NA_SE_EN_TWINROBA_POWERUP);
    play->envCtx.unk_D8 = 1.0f;
    Rumble_Request(0.0f, 160, 10, 4);

    // An interrupted Breaker may have armed the first-cycle cooldown before this damage crossed the threshold.
    if (this->timers[4] > breakerCooldown) {
        this->timers[4] = breakerCooldown;
    }
    if (this->cycloneCooldown > cycloneCooldown) {
        this->cycloneCooldown = cycloneCooldown;
    }
    if (this->falseChargeCooldown > falseChargeCooldown) {
        this->falseChargeCooldown = falseChargeCooldown;
    }
    if (this->minefieldCooldown > minefieldCooldown) {
        this->minefieldCooldown = minefieldCooldown;
    }
}

static s32 BossTw_IsNearStunLanding(BossTw* this) {
    f32 footHeight;

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        return true;
    }
    if (this->actor.floorHeight <= BGCHECK_Y_MIN) {
        return false;
    }

    // Stun's bg check is sampled from thirty units below the actor origin. Begin the existing sixteen-frame laydown
    // one terminal-fall animation before contact instead of letting Twinrova become prone high above the arena.
    footHeight = this->actor.world.pos.y - 30.0f;
    return footHeight - this->actor.floorHeight <= TWINROVA_STUN_LAYDOWN_LEAD_HEIGHT;
}

void BossTw_TwinrovaDamage(BossTw* this, PlayState* play, u8 damage) {
    if (this->actionFunc != BossTw_TwinrovaStun) {
        s32 playerOnUpperDeck = GET_PLAYER(play)->actor.floorHeight >= TWINROVA_UPPER_FLOOR_MIN_Y;
        s32 twinrovaWillLandOnUpperDeck = BossTw_IsOverRaisedPlatform(&this->actor.world.pos);

        BossTw_ArmBreakerCooldown(this);
        BossTw_ClearSummonedEnemies(play);
        BossTw_ClearGroundHazards(play, true);
        BossTw_ClearPlayerFreeze(play);
        BossTw_ClearPlayerBurn(play);
        BossTw_ClearSiegeZones(this, play, false, TWINROVA_SIEGE_USE_ZONE_ELEMENT, true);
        BossTw_CancelPhaseTwoMagic(this, play, true);
        sEnvType = 0;
        Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaChargedAttackHitAnim, -15.0f);
        this->timers[0] = TWINROVA_STUN_TIME;
        if (playerOnUpperDeck != twinrovaWillLandOnUpperDeck) {
            // A legal reflection can cross floors after the cast committed. Preserve the full melee punish window
            // after Link spends the extra traversal time reaching the platform where Twinrova will actually fall.
            this->timers[0] += TWINROVA_CROSS_DECK_STUN_BONUS;
        }
        this->timers[1] = 20;
        this->csState1 = 0;
        this->csState2 = 0;
        this->actor.velocity.y = 0.0f;
    } else {
        u8 remainingDamage = this->csState2 < TWINROVA_STUN_DAMAGE_BUDGET
                                 ? TWINROVA_STUN_DAMAGE_BUDGET - this->csState2
                                 : 0;
        u8 accumulatedDamageBefore = this->csState2;
        u8 healthBeforeDamage;
        s32 enteredFinalCycle;
        s32 nearingDamageBudget;
        s32 preserveLandingTransition = this->csState1 == 0;

        if (damage > remainingDamage) {
            damage = remainingDamage;
        }
        this->work[FOG_TIMER] = 10;
        this->work[INVINC_TIMER] = 20;
        Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaDamageAnim, -3.0f);
        this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaDamageAnim);
        if (!preserveLandingTransition) {
            this->csState1 = 1;
        }
        this->csState2 += damage;
        nearingDamageBudget = accumulatedDamageBefore < TWINROVA_STUN_DAMAGE_WARNING &&
                              this->csState2 >= TWINROVA_STUN_DAMAGE_WARNING &&
                              this->csState2 < TWINROVA_STUN_DAMAGE_BUDGET;
        healthBeforeDamage = this->actor.colChkInfo.health;

        if ((s8)(this->actor.colChkInfo.health -= damage) < 0) {
            this->actor.colChkInfo.health = 0;
        }

        enteredFinalCycle = healthBeforeDamage > TWINROVA_FINAL_CYCLE_HEALTH &&
                            BossTw_IsFusedFinalCycle(this);
        if (enteredFinalCycle) {
            BossTw_ShowFusedFinalCycleEscalation(this, play);
        } else if (nearingDamageBudget) {
            // The cap is an authored punish-window rule. Preview it before the final hit closes the window instead
            // of teaching it through an apparently abrupt get-up.
            BossTw_EmitStunBudgetWarning(this, play);
        }

        if ((s8)this->actor.colChkInfo.health <= 0) {
            BossTw_TwinrovaSetupDeathCS(this, play);
            Enemy_StartFinishingBlow(play, &this->actor);
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_YOUNG_DEAD);
            GameInteractor_ExecuteOnBossDefeat(&this->actor);
            return;
        }

        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_YOUNG_DAMAGE2);
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_CUTBODY);
        if (this->csState2 >= TWINROVA_STUN_DAMAGE_BUDGET) {
            if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
                if (!enteredFinalCycle) {
                    BossTw_EmitStunRecoveryTell(this, play);
                }
                BossTw_TwinrovaSetupGetUp(this, play);
                return;
            }
            // Ranged damage can exhaust the punish budget during the fall. Keep the stun action/collider alive and
            // defer the recovery until actual floor contact; a get-up animation in midair is as misleading as a
            // premature prone loop.
        }
    }

    this->actionFunc = BossTw_TwinrovaStun;
}

void BossTw_TwinrovaStun(BossTw* this, PlayState* play) {
    s16 cloudType;
    f32 fallVelocity = this->actor.velocity.y;

    this->unk_5F8 = 1;
    this->actor.flags |= ACTOR_FLAG_HOOKSHOT_PULLS_PLAYER;

    cloudType = sTwinrovaBlastType == 0 ? 3 : 2;

    if ((this->work[CS_TIMER_1] % 8) == 0) {
        Vec3f pos;
        Vec3f velocity;
        Vec3f accel;
        pos.x = this->actor.world.pos.x + Rand_CenteredFloat(20.0f);
        pos.y = this->actor.world.pos.y + Rand_CenteredFloat(40.0f) + 20;
        pos.z = this->actor.world.pos.z + Rand_CenteredFloat(20.0f);
        velocity.x = 0.0f;
        velocity.y = 0.0f;
        velocity.z = 0.0f;
        accel.x = 0.0f;
        accel.y = 0.1f;
        accel.z = 0.0f;
        BossTw_AddDmgCloud(play, cloudType, &pos, &velocity, &accel, Rand_ZeroFloat(5.0f) + 10.0f, 0, 0, 150);
    }

    if (this->timers[0] == TWINROVA_STUN_RECOVERY_WARNING_TIME) {
        BossTw_EmitStunRecoveryTell(this, play);
    }
    if (this->timers[0] != 0 && this->timers[0] <= TWINROVA_STUN_RECOVERY_WARNING_TIME &&
        (this->timers[0] % 5) == 0) {
        BossTw_SpawnMagicLaunchEffects(play, &this->leftScepterPos, TWINROVA_MAGIC_ICE, 2);
        BossTw_SpawnMagicLaunchEffects(play, &this->rightScepterPos, TWINROVA_MAGIC_FIRE, 2);
    }

    SkelAnime_Update(&this->skelAnime);
    this->work[UNK_S8] += 20;

    if (this->work[UNK_S8] > 255) {
        this->work[UNK_S8] = 255;
    }

    Math_ApproachF(&this->workf[UNK_F12], 0.0f, 1.0f, 0.05f);
    this->actor.world.pos.y += this->actor.velocity.y;
    Math_ApproachF(&this->actor.velocity.y, -5.0f, 1.0f, 0.5f);
    this->actor.world.pos.y -= 30.0f;
    Actor_UpdateBgCheckInfo(play, &this->actor, 50.0f, 50.0f, 100.0f, 4);
    this->actor.world.pos.y += 30.0f;

    if ((this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) && fallVelocity < -1.0f) {
        Vec3f landingPos = this->actor.world.pos;

        landingPos.y = this->actor.floorHeight + 10.0f;
        BossTw_AddRingEffect(play, &landingPos, 0.45f, 3.2f, 255, TWINROVA_MAGIC_ICE, 1,
                             ARRAY_COUNT(sEffects));
        BossTw_AddRingEffect(play, &landingPos, 0.6f, 3.6f, 255, TWINROVA_MAGIC_FIRE, 1,
                             ARRAY_COUNT(sEffects));
        BossTw_SpawnMagicLaunchEffects(play, &landingPos, TWINROVA_MAGIC_ICE, 8);
        BossTw_SpawnMagicLaunchEffects(play, &landingPos, TWINROVA_MAGIC_FIRE, 8);
        SoundSource_PlaySfxAtFixedWorldPos(play, &landingPos, 20, NA_SE_EV_OBJECT_FALL);
        Rumble_Request(0.0f, 140, 8, 4);
        play->envCtx.unk_D8 = 0.75f;
    }

    if (this->csState1 == 0) {
        if (this->timers[1] == 0 && BossTw_IsNearStunLanding(this)) {
            this->csState1 = 1;
            this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaStunStartAnim);
            Animation_Change(&this->skelAnime, &gTwinrovaStunStartAnim, 1.0f, 0.0f, this->workf[ANIM_SW_TGT], 3, 0.0f);
        }
    } else if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
        this->workf[ANIM_SW_TGT] = 1000.0f;
        Animation_MorphToLoop(&this->skelAnime, &gTwinrovaStunLoopAnim, 0.0f);
    }

    if (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) {
        this->actor.velocity.y = 0.0f;
        if (this->csState2 >= TWINROVA_STUN_DAMAGE_BUDGET) {
            BossTw_EmitStunRecoveryTell(this, play);
            BossTw_TwinrovaSetupGetUp(this, play);
            return;
        }
    }

    if (this->timers[0] == 0 && (this->actor.bgCheckFlags & BGCHECKFLAG_GROUND) &&
        !(this->collider.base.acFlags & AC_HIT)) {
        // Collision results arrive one update late. Keep the final visible stun contact in the stun damage path
        // instead of changing state first and silently discarding an attack that already connected.
        BossTw_TwinrovaSetupGetUp(this, play);
    }
}

void BossTw_TwinrovaSetupGetUp(BossTw* this, PlayState* play) {
    Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaStunEndAnim, 0.0f);
    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaStunEndAnim);
    this->actionFunc = BossTw_TwinrovaGetUp;
    this->timers[0] =
        BossTw_IsFusedFinalCycle(this) ? TWINROVA_FINAL_GET_UP_RECOVERY : TWINROVA_GET_UP_RECOVERY;
}

void BossTw_TwinrovaGetUp(BossTw* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.05f, 5.0f);

    if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
        this->workf[ANIM_SW_TGT] = 1000.0f;
        Animation_MorphToLoop(&this->skelAnime, &gTwinrovaHoverAnim, 0.0f);
    }

    if (this->timers[0] == 0) {
        BossTw_TwinrovaSetupFly(this, play);
    }
}

void BossTw_TwinrovaSetupFly(BossTw* this, PlayState* play) {
    f32 xDiff;
    f32 zDiff;
    f32 yDiff;
    f32 xzDist;
    Player* player = GET_PLAYER(play);

    do {
        this->work[TW_PLLR_IDX] += (s16)(((s16)Rand_ZeroFloat(2.99f)) + 1);
        this->work[TW_PLLR_IDX] %= 4;
        this->targetPos = sTwinrovaPillarPos[this->work[TW_PLLR_IDX]];
        xDiff = this->targetPos.x - player->actor.world.pos.x;
        zDiff = this->targetPos.z - player->actor.world.pos.z;
        xzDist = SQ(xDiff) + SQ(zDiff);
    } while (!(xzDist > SQ(300.0f)));

    this->targetPos.y = 480.0f;
    xDiff = this->targetPos.x - this->actor.world.pos.x;
    yDiff = this->targetPos.y - this->actor.world.pos.y;
    zDiff = this->targetPos.z - this->actor.world.pos.z;
    this->actionFunc = BossTw_TwinrovaFly;
    this->rotateSpeed = 0.0f;
    this->actor.speedXZ = 0.0f;
    this->actor.world.rot.y = Math_FAtan2F(xDiff, zDiff) * (32768 / M_PI);
    xzDist = sqrtf(SQ(xDiff) + SQ(zDiff));
    this->actor.world.rot.x = Math_FAtan2F(yDiff, xzDist) * (32768 / M_PI);
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaHoverAnim, -10.0f);
}

void BossTw_TwinrovaFly(BossTw* this, PlayState* play) {
    f32 xDiff;
    f32 yDiff;
    f32 zDiff;
    s32 pad;
    f32 yaw;
    f32 xzDist;

    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_FLY - SFX_FLAG);
    SkelAnime_Update(&this->skelAnime);
    xDiff = this->targetPos.x - this->actor.world.pos.x;
    yDiff = this->targetPos.y - this->actor.world.pos.y;
    zDiff = this->targetPos.z - this->actor.world.pos.z;
    // Convert from radians to degrees, then degrees to binary angle
    yaw = (s16)(Math_FAtan2F(xDiff, zDiff) * ((180.0f / M_PI) * (65536.0f / 360.0f)));
    xzDist = sqrtf(SQ(xDiff) + SQ(zDiff));
    Math_ApproachS(&this->actor.world.rot.x,
                   (f32)(s16)(Math_FAtan2F(yDiff, xzDist) * ((180.0f / M_PI) * (65536.0f / 360.0f))), 0xA,
                   this->rotateSpeed);
    Math_ApproachS(&this->actor.world.rot.y, yaw, 0xA, this->rotateSpeed);
    Math_ApproachS(&this->actor.shape.rot.y, yaw, 0xA, this->rotateSpeed);
    Math_ApproachF(&this->rotateSpeed, 2600.0f, 1.0f, 150.0f);
    Math_ApproachF(&this->actor.speedXZ, 44.0f, 1.0f, 4.0f);
    Actor_UpdateVelocityXYZ(&this->actor);
    Math_ApproachF(&this->actor.world.pos.x, this->targetPos.x, 0.1f, fabsf(this->actor.velocity.x) * 1.5f);
    Math_ApproachF(&this->actor.world.pos.y, this->targetPos.y, 0.1f, fabsf(this->actor.velocity.y) * 1.5f);
    Math_ApproachF(&this->targetPos.y, 380.0f, 1.0f, 2.0f);
    Math_ApproachF(&this->actor.world.pos.z, this->targetPos.z, 0.1f, fabsf(this->actor.velocity.z) * 1.5f);

    if (xzDist < 200.0f) {
        BossTw_TwinrovaSetupArriveAtTarget(this, play);
    }
}

void BossTw_TwinrovaSetupSpin(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_TwinrovaSpin;
    Animation_MorphToLoop(&this->skelAnime, &gTwinrovaHoverAnim, 0.0f);
    this->timers[0] =
        TWINROVA_SPIN_TELL_TIME + TWINROVA_SPIN_ACTIVE_TIME + TWINROVA_FUSED_SPIN_RECOVERY_TIME;
    this->actor.speedXZ = 0.0f;
    // The damage is a body-centered cylinder, so the primary tell must originate there rather than only at the two
    // scepters. Concentric elemental rings identify the fused counter and remain visible through the full wind-up.
    BossTw_AddRingEffect(play, &this->actor.world.pos, 0.45f, 3.2f, 255, TWINROVA_MAGIC_ICE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_AddRingEffect(play, &this->actor.world.pos, 0.6f, 3.6f, 255, TWINROVA_MAGIC_FIRE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_AddRingEffect(play, &this->leftScepterPos, 0.35f, 3.0f, 255, TWINROVA_MAGIC_ICE, 1,
                         ARRAY_COUNT(sEffects));
    BossTw_AddRingEffect(play, &this->rightScepterPos, 0.35f, 3.0f, 255, TWINROVA_MAGIC_FIRE, 1,
                         ARRAY_COUNT(sEffects));
    Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_ROLL);
}

void BossTw_TwinrovaSpin(BossTw* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);
    if (this->timers[0] > TWINROVA_FUSED_SPIN_RECOVERY_TIME + TWINROVA_SPIN_ACTIVE_TIME) {
        return;
    }

    if (this->timers[0] > TWINROVA_FUSED_SPIN_RECOVERY_TIME) {
        this->collider.base.colType = COLTYPE_METAL;
        this->actor.shape.rot.y -= 0x3000;

        if ((this->timers[0] % 4) == 0) {
            Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_ROLL);
        }
    } else if (this->timers[0] != 0) {
        // The damaging rotation is over. Visibly settle back to neutral before movement resumes so spacing the
        // counter earns a short, legible reset instead of an immediate fly-away transition.
        Math_ApproachS(&this->actor.shape.rot.y, this->actor.world.rot.y, 3, 0x2000);
    } else {
        // The generic proximity counter runs after the action function. Without a cooldown, SetupFly makes this
        // actor eligible again on the exact recovery frame and a close attacking player can loop spins forever.
        // Keep enough time for Twinrova to commit to elemental pressure before this punish is legal again.
        this->timers[3] = TWINROVA_FUSED_SPIN_RETRIGGER_COOLDOWN;
        BossTw_TwinrovaSetupFly(this, play);
    }
}

void BossTw_TwinrovaSetupLaugh(BossTw* this, PlayState* play) {
    this->actionFunc = BossTw_TwinrovaLaugh;
    Animation_MorphToPlayOnce(&this->skelAnime, &gTwinrovaLaughAnim, 0.0f);
    this->workf[ANIM_SW_TGT] = Animation_GetLastFrame(&gTwinrovaLaughAnim);
    this->actor.speedXZ = 0.0f;
}

void BossTw_TwinrovaLaugh(BossTw* this, PlayState* play) {
    SkelAnime_Update(&this->skelAnime);

    if (Animation_OnFrame(&this->skelAnime, 10.0f)) {
        Audio_PlayActorSound2(&this->actor, NA_SE_EN_TWINROBA_YOUNG_LAUGH);
    }

    if (Animation_OnFrame(&this->skelAnime, this->workf[ANIM_SW_TGT])) {
        BossTw_TwinrovaSetupFly(this, play);
    }
}

void BossTw_Reset(void) {
    sTwInitialized = false;
    sKotakePtr = NULL;
    sKoumePtr = NULL;
    sTwinrovaPtr = NULL;
    BossTw_ResetShieldCharge();
    D_8094C870 = 0;
    sEnvType = 0;
    sGroundBlastType = 0;
    sFreezeState = 0;
    sMinefieldLightPulseTimer = 0;
    sMinefieldLightPulseElement = TWINROVA_MAGIC_ICE;
    memset(sEffects, 0, sizeof(sEffects));
    memset(sTwinrovaSummons, 0, sizeof(sTwinrovaSummons));
    sTwinrovaSummonUpdateFrame = 0;
    sTwinrovaSharedUpdateFrame = 0;
    sTwinrovaCooldownUpdateFrame = 0;
    sPhaseOneSchedulerDecisionFrame = 0;
    sPhaseOneCooldownUpdateFrame = 0;
    BossTw_ResetPhaseOneScheduler();
}
