/* date = June 30th 2020 9:29 am */

#ifndef RESOURCES_H
#define RESOURCES_H

/* Tilemaps
Define all of the tilemaps by associating them with a name, and pass
the extra required information. All of the rendering is done through
tilemaps.
*/
#define ALL_TILEMAPS \
/*          NAME               PATH IN FOLDER                             rows cols*/\
DEF_TILEMAP(TM_logo            ,"res/logo3.png"                           ,1    ,1  )\
DEF_TILEMAP(TM_essentials      ,"res/essentials.png"                      ,5    ,5  )\
DEF_TILEMAP(TM_dana            ,"res/dana_all.png"                        ,5    ,6  )\
DEF_TILEMAP(TM_dana_fire       ,"res/dana_fireball.png"                   ,4    ,1  )\
DEF_TILEMAP(TM_fairies         ,"res/fairies_all.png"                     ,2    ,2  )\
DEF_TILEMAP(TM_demonhead       ,"res/demon_head.png"                      ,2    ,8  )\
DEF_TILEMAP(TM_goblin          ,"res/goblin_all.png"                      ,4    ,7  )\
DEF_TILEMAP(TM_blueflame       ,"res/blue_flame.png"                      ,2    ,7  )\
DEF_TILEMAP(TM_ghost           ,"res/ghost_all.png"                       ,2    ,3  )\
DEF_TILEMAP(TM_panelmonster    ,"res/panel_monster_all.png"               ,3    ,5  )\
DEF_TILEMAP(TM_wyvern          ,"res/wyvern_all.png"                      ,2    ,3  )\
DEF_TILEMAP(TM_dragon          ,"res/dragon_all.png"                      ,4    ,5  )\
DEF_TILEMAP(TM_spark_ball      ,"res/spark_ball_all.png"                  ,1    ,8  )\
DEF_TILEMAP(TM_gargoyle        ,"res/gargoyle_all.png"                    ,3    ,6  )\
DEF_TILEMAP(TM_effects         ,"res/effects.png"                         ,5    ,4  )\
DEF_TILEMAP(font               ,"res/font.png"                            ,6    ,16 )\
DEF_TILEMAP(misc               ,"res/misc.png"                            ,4    ,4  )\
DEF_TILEMAP(TM_pickups         ,"res/pickups.png"                         ,5    ,7  )\
DEF_TILEMAP(TM_ribbon          ,"res/ribbons.png"                         ,1    ,3  )\
DEF_TILEMAP(TM_ribbon_fire     ,"res/ribbons_fire.png"                    ,1    ,6  )\

/* Backgrounds
Lol
*/
#define ALL_BACKGROUNDS \
B("res/backgrounds/0.png")\
B("res/backgrounds/1.png")\
B("res/backgrounds/2.png")\
B("res/backgrounds/3.png")\
B("res/backgrounds/4.png")\
B("res/backgrounds/5.png")\
B("res/backgrounds/6.png")\
B("res/backgrounds/7.png")\
B("res/backgrounds/8.png")\
B("res/backgrounds/9.png")\
B("res/backgrounds/10.png")\
B("res/backgrounds/11.png")\
B("res/backgrounds/12.png")\
B("res/backgrounds/13.png")\
B("res/backgrounds/14.png")\
B("res/backgrounds/15.png")\
B("res/backgrounds/16.png")\
B("res/backgrounds/17.png")\
B("res/backgrounds/19.png")\
B("res/backgrounds/20.png")\
B("res/backgrounds/21.png")\
B("res/backgrounds/22.png")\
B("res/backgrounds/23.png")\
B("res/backgrounds/24.png")\
B("res/backgrounds/25.png")\
B("res/backgrounds/26.png")\
B("res/backgrounds/27.png")\


/* ANIMATIONS
You can group animations by "character", and the macro ugliness here
should prevent name collisions, if you input different character names.
*/
#define ALL_CHARACTERS                                                              \
/*            Name,        Tilemap,  Animation Count */                         \
DEF_CHARACTER(BlockFrail,            TM_essentials,      1,                     \
DEF_ANIM(BlockFrail,            Default, .1f,      {0,0}, 1,      true )   \
)                                                                 \
DEF_CHARACTER(BlockFrailHalf,        TM_essentials,      1,                     \
DEF_ANIM(BlockFrailHalf,        Default, .1f,      {4,0}, 1,      true )   \
)                                                                 \
DEF_CHARACTER(BlockSolid,            TM_essentials,      1,                     \
DEF_ANIM(BlockSolid,            Default, .1f,      {0,1}, 1,      true )   \
)                                                                 \
DEF_CHARACTER(Dana,                  TM_dana,            7,                     \
/*       Character,             Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(Dana,                  Idle,    .1f,      {0,0}, 1,      false)   \
DEF_ANIM(Dana,                  Run ,    .1f,      {0,1}, 5,      true )   \
DEF_ANIM(Dana,                  Cast,    .08f,     {0,2}, 3,      false)   \
DEF_ANIM(Dana,                  Crouch,  .05f,     {0,3}, 1,      true )   \
DEF_ANIM(Dana,                  Die,     .1f,      {2,4}, 2,      true )   \
DEF_ANIM(Dana,                  JumpWait,.1f,      {1,4}, 1,      false)   \
DEF_ANIM(Dana,                  JumpTurn,.1f,      {5,1}, 1,      true )   \
)                                                                 \
DEF_CHARACTER(Goblin,                TM_goblin,          5,                     \
/*       Character,             Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(Goblin,                Walk,    .2f,      {0,0}, 6,      true )   \
DEF_ANIM(Goblin,                Punch,   .2f ,     {0,1}, 4,      false)   \
DEF_ANIM(Goblin,                Chase,   .15f,     {0,2}, 7,      true )   \
DEF_ANIM(Goblin,                Wait,    1.f,      {5,0}, 1,      false)   \
DEF_ANIM(Goblin,                Fall,    .1f,      {0,3}, 2,      true )   \
)                                                                 \
DEF_CHARACTER(Ghost,                 TM_ghost,           2,                     \
/*       Character,             Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(Ghost,                 Fly,     .1f,      {0,0}, 3,      true )   \
DEF_ANIM(Ghost,                 Punch,   .1f,      {0,1}, 3,      false)   \
)                                                                 \
DEF_CHARACTER(BlueFlame,             TM_blueflame,      2,                      \
DEF_ANIM(BlueFlame,             Normal,  .1f,      {0,0}, 7,      true )   \
DEF_ANIM(BlueFlame,             Tame,    .1f,      {0,1}, 4,      true )   \
)                                                                 \
DEF_CHARACTER(DemonHead,             TM_demonhead,      2,                      \
DEF_ANIM(DemonHead,             Normal,  .1f,      {0,0}, 8,      true )   \
DEF_ANIM(DemonHead,             Fade,    .1f,      {0,1}, 8,      false)   \
)                                                                 \
DEF_CHARACTER(PanelMonster,          TM_panelmonster,   2,                      \
DEF_ANIM(PanelMonster,          Wait,    0.0f,     {0,0}, 1,      true )   \
DEF_ANIM(PanelMonster,          Fire,    0.1f,     {0,0}, 5,      false)   \
)                                                                 \
DEF_CHARACTER(PanelMonsterFlame,     TM_panelmonster,   3,                      \
DEF_ANIM(PanelMonsterFlame,     Create,  0.2f,     {0,1}, 3,      false)   \
DEF_ANIM(PanelMonsterFlame,     Default, 0.2f,     {0,2}, 4,      true )   \
DEF_ANIM(PanelMonsterFlame,     Hit,     0.2f,     {3,1}, 2,      false)   \
)                                                                 \
DEF_CHARACTER(Wyvern,                TM_wyvern,         2,                      \
DEF_ANIM(Wyvern,                Default, 0.1f,     {0,0}, 3,      true )   \
DEF_ANIM(Wyvern,                Hit,     0.1f,     {0,1}, 2,      false)   \
)                                                                 \
DEF_CHARACTER(Dragon,                TM_dragon,         6,                      \
DEF_ANIM(Dragon,                Walk,    0.2f,     {0,0}, 5,      true )   \
DEF_ANIM(Dragon,                Fire,    1.2f,     {3,1}, 1,      false)   \
DEF_ANIM(Dragon,                Turn,    0.2f,     {4,3}, 1,      false)   \
DEF_ANIM(Dragon,                TurnWait,2.2f,     {4,0}, 1,      false)   \
DEF_ANIM(Dragon,                FireWait,0.34f,    {0,1}, 3,      false)   \
DEF_ANIM(Dragon,                Die,     0.2f,     {0,3}, 2,      true )   \
)                                                                 \
DEF_CHARACTER(DragonFire,            TM_dragon,         1,                      \
DEF_ANIM(DragonFire,            Default, 0.2f,     {0,2}, 4,      true )   \
)                                                                 \
DEF_CHARACTER(SparkBall,             TM_spark_ball,     1,                      \
DEF_ANIM(SparkBall,             Default, 0.2f,     {0,0}, 8,      true )   \
)                                                                 \
DEF_CHARACTER(DFireball,             TM_dana_fire,      4,                      \
/*       Character,             Name     Duration, Start, Frames, Loop*/   \
DEF_ANIM(DFireball,             Down,    .1f,      {0,0}, 1,      true )   \
DEF_ANIM(DFireball,             Middle,  .1f,      {0,1}, 1,      true )   \
DEF_ANIM(DFireball,             Up,      .1f,      {0,2}, 1,      true )   \
DEF_ANIM(DFireball,             Decay,   .1f,      {0,3}, 1,      true )   \
)                                                                 \
DEF_CHARACTER(Gargoyle,              TM_gargoyle,       5,                      \
DEF_ANIM(Gargoyle,              Walk,    .1f,      {0,2}, 6,      true )   \
DEF_ANIM(Gargoyle,              Wait,    1.f,      {1,1}, 1,      false)   \
DEF_ANIM(Gargoyle,              FireWait,.1f,      {0,1}, 2,      true )   \
DEF_ANIM(Gargoyle,              Fire,    1.f,      {2,1}, 1,      false)   \
DEF_ANIM(Gargoyle,              Fall,    .1f,      {0,0}, 6,      false)   \
)                                                                 \
DEF_CHARACTER(Effect,                TM_effects,        7,                      \
DEF_ANIM(Effect,                Smoke,   .1f,      {0,0}, 4,      false)   \
DEF_ANIM(Effect,                Star,    .1f,      {0,1}, 3,      true )   \
DEF_ANIM(Effect,                Flash,   .1f,      {0,2}, 2,      false)   \
DEF_ANIM(Effect,                FlashBlk,.1f,      {0,2}, 2,      false)   \
DEF_ANIM(Effect,                Flash2,  .1f,      {0,3}, 3,      false)   \
DEF_ANIM(Effect,                Hit,     .1f,      {0,4}, 4,      false)   \
DEF_ANIM(Effect,                DanaDie, .5f,      {2,2}, 1,      false)   \
)                                                                 \
DEF_CHARACTER(Door,                  TM_essentials,     2,                      \
DEF_ANIM(Door,                  Close,   .1f,      {0,2}, 5,      false)   \
DEF_ANIM(Door,                  Open,    .1f,      {0,3}, 5,      false)   \
)                                                                 \
DEF_CHARACTER(Key,                   TM_essentials,     1,                      \
DEF_ANIM(Key,                   Default, .1f,      {0,4}, 1,      false)   \
)                                                                 \
DEF_CHARACTER(Fairie,                TM_fairies,        2,                      \
DEF_ANIM(Fairie,                Default, .1f,      {0,0}, 2,      true )   \
DEF_ANIM(Fairie,                Boye,    .1f,      {0,1}, 2,      true )   \
)                                                                 \
DEF_CHARACTER(KMirror,               TM_essentials,     1,                      \
DEF_ANIM(KMirror,               Default, .1f,      {3,1}, 1,      false)   \
)                                                                 \

#define ALL_SOUNDS \
DEF_SOUND(SND_background, "res/audio/bgm1.wav" ) \
DEF_SOUND(SND_jump,       "res/audio/splat.wav") \
DEF_SOUND(SND_show_key,   "res/audio/key_appear.wav") \
DEF_SOUND(SND_show_player,"res/audio/player_appear.wav") \
DEF_SOUND(SND_hurry,      "res/audio/hurry.wav") \
DEF_SOUND(SND_splat,      "res/audio/splat.wav") \
DEF_SOUND(SND_boueip,     "res/audio/boueip.wav") \
DEF_SOUND(SND_get_key,    "res/audio/get_key.wav") \
DEF_SOUND(SND_win,        "res/audio/win.wav") \
DEF_SOUND(SND_rest_bonus, "res/audio/rest_bonus.wav") \
DEF_SOUND(SND_item0,      "res/audio/item0.wav") \

#endif //RESOURCES_H
