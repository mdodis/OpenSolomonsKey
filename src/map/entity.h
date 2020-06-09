/* date = May 13th 2020 10:06 pm */
#ifndef ENTITY_H
#define ENTITY_H

enum EntityType {
    ET_EmptySpace = 0,
    ET_BlockFrail = 1,
    ET_BlockSolid = 2,
    ET_BlockFrailHalf = 3,
    ET_PlayerSpawnPoint = 4,
    ET_Player = 5,
    ET_Enemy = 6,
    ET_Door = 7,
    ET_Key = 8,
    ET_Pickup = 9,
    
    ET_Fairie,
    ET_Effect,
    ET_DFireball,
    ET_Count,
};

static inline bool is_valid_tilemap_object(EntityType type) {
    return type <= ET_BlockSolid;
}

static inline bool tile_is_empty(EntityType type) {
    return (type == ET_Door || type == ET_EmptySpace);
}

enum PickupType {
    // Bonus Points Items
    PT_Bag10000 = 0,
    PT_Bag20000,
    PT_Bag100,
    PT_Bag200 = 3,
    PT_Bag500,
    PT_Bag1000,
    PT_Bag2000,
    PT_Bag5000 = 7,
    PT_Bell,
    PT_Jewel20000,
    PT_Jewel1000,
    PT_Jewel2000 = 11,
    PT_Jewel10000,
    PT_PotionDestruction,
    PT_Hourglass,
    PT_Jewel5000 = 15,
    PT_Jewel50000,
    PT_Jewel100,
    PT_Jewel200,
    PT_Jewel500 = 19,
    PT_JewelChange,
    PT_JewelRange,
    PT_JewelRange2,
    PT_PotionFire = 23,
    PT_PotionSuperFire,
    PT_PotionFireGrowth,
    PT_PotionLife,
    PT_PotionTime1 = 27,
    PT_PotionTime2,
    PT_ScrollExtension,
    PT_SolomonSeal,
    PT_Sphinx = 31,
    PT_PaperCrane,
    PT_SolomonsKey,
    
    PT_Count
};

static inline bool pickup_type_is_valid(PickupType t) {
    return (t >= 0 && t < PT_Count);
}

enum EnemyType {
    
    MT_Chimera = 0,
    MT_Demonhead,
    MT_Dragon,
    MT_Gargoyle = 3,
    MT_Ghost,
    MT_Goblin,
    MT_Nuel,
    MT_Salamander = 7,
    MT_Wyvern,
    MT_PanelMonster,
    MT_EarthMage,
    MT_SparkBall = 11,
    MT_BlueFlame,
    MT_DevilsEmblem,
    MT_Slimegreen,
    MT_Slimeorange = 15,
    MT_Slimepurple,
    MT_KMirror,
    
    // enemies that can't be in level
    MT_PanelMonsterFlame,
    MT_DragonFire,
    
    MT_Count
};

#endif //ENTITY_H