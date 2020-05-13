/* date = May 13th 2020 10:00 am */
#ifndef PICKUPS_H
#define PICKUPS_H

enum class PickupType {
    // Bonus Points Items
    Bag100,
    Bag200,
    Bag500,
    Bag1000,
    Bag2000 = 4,
    Bag5000,
    Bag10000,
    Bag20000,
    Jewel100 = 8,
    Jewel200,
    Jewel500,
    Jewel1000,
    Jewel2000 = 12,
    Jewel5000,
    Jewel10000,
    Jewel20000,
    Jewel50000 = 16,
    // Magic Items
    Bell,
    Bell2,
    JewelChange,
    JewelChange2 = 20,
    PotionFire,
    PotionSuperFire,
    PotionFireGrowth,
    ScrollExtension = 24,
    ScrollExtension2,
    JewelRange,
    
    Count
};

internal inline bool pickup_type_is_valid(PickupType type) {
    return(u32(type) >= 0 && type < PickupType::Count);
}

internal inline bool pickup_type_is_non_effect(PickupType type) {
    return pickup_type_is_valid(type) && (type <= PickupType::Jewel50000);
}

internal inline bool pickup_is_bell(PickupType type) {
    return pickup_type_is_valid(type) && (type == PickupType::Bell || type == PickupType::Bell2);
}

internal long get_pickup_worth(PickupType type) {
    assert(pickup_type_is_valid(type));
    
    switch(type) {
        case PickupType::Bag100:
        case PickupType::Jewel100: {
            return 100;
        }break;
        
        case PickupType::Bag200:
        case PickupType::Jewel200: {
            return 200;
        }break;
        
        case PickupType::Bag500:
        case PickupType::Jewel500: {
            return 500;
        }break;
        
        case PickupType::Bag1000:
        case PickupType::Jewel1000: {
            return 1000;
        }break;
        
        case PickupType::Bag2000:
        case PickupType::Jewel2000: {
            return 2000;
        }break;
        
        case PickupType::Bag5000:
        case PickupType::Jewel5000: {
            return 5000;
        }break;
        
        case PickupType::Bag10000:
        case PickupType::Jewel10000: {
            return 10000;
        }break;
        
        case PickupType::Bag20000:
        case PickupType::Jewel20000: {
            return 20000;
        }break;
        case PickupType::Jewel50000: {
            return 50000;
        }break;
    }
    assert(0);
    return -1;
}

#endif //PICKUPS_H
