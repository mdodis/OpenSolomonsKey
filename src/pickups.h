/* date = May 13th 2020 10:00 am */
#ifndef PICKUPS_H
#define PICKUPS_H

internal inline bool pickup_is_bell(PickupType type) {
    return pickup_type_is_valid(type) && (type == PT_Bell);
}

internal long get_pickup_worth(PickupType type) {
    assert(pickup_type_is_valid(type));
    
    switch(type) {
        case PT_Bag100:
        case PT_JewelChange:
        case PT_JewelRange:
        case PT_PotionTime1:
        case PT_Hourglass:
        case PT_Jewel100: return 100;
        
        case PT_Bag200:
        case PT_PotionFire:
        case PT_PotionFireGrowth:
        case PT_ScrollExtension:
        case PT_JewelRange2:
        case PT_Jewel200: return 200;
        
        case PT_Bag500:
        case PT_PotionSuperFire:
        case PT_PotionTime2:
        case PT_PotionDestruction:
        case PT_Jewel500: return 500;
        
        case PT_Bag1000:
        case PT_PotionLife:
        case PT_Jewel1000: return 1000;
        
        case PT_Bag2000:
        case PT_Jewel2000: return 2000;
        
        case PT_Bag5000:
        case PT_Jewel5000: return 5000;
        
        case PT_Bag10000:
        case PT_Jewel10000: return 10000;
        
        case PT_Bag20000:
        case PT_Jewel20000: return 20000;
        
        case PT_Jewel50000: return 50000;
        
        default: return 0;
    }
}

#endif //PICKUPS_H
