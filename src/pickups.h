/* date = May 13th 2020 10:00 am */
#ifndef PICKUPS_H
#define PICKUPS_H

internal inline bool pickup_type_is_non_effect(PickupType type) {
    // TODO(miked): the enum has been changed. can't just do a range check
    // now
    return pickup_type_is_valid(type) && (type <= PT_Jewel50000);
}

internal inline bool pickup_is_bell(PickupType type) {
    return pickup_type_is_valid(type) && (type == PT_Bell);
}

internal long get_pickup_worth(PickupType type) {
    assert(pickup_type_is_valid(type));
    
    switch(type) {
        case PT_Bag100:
        case PT_Jewel100: {
            return 100;
        }break;
        
        case PT_Bag200:
        case PT_Jewel200: {
            return 200;
        }break;
        
        case PT_Bag500:
        case PT_Jewel500: {
            return 500;
        }break;
        
        case PT_Bag1000:
        case PT_Jewel1000: {
            return 1000;
        }break;
        
        case PT_Bag2000:
        case PT_Jewel2000: {
            return 2000;
        }break;
        
        case PT_Bag5000:
        case PT_Jewel5000: {
            return 5000;
        }break;
        
        case PT_Bag10000:
        case PT_Jewel10000: {
            return 10000;
        }break;
        
        case PT_Bag20000:
        case PT_Jewel20000: {
            return 20000;
        }break;
        case PT_Jewel50000: {
            return 50000;
        }break;
    }
    assert(0);
    return -1;
}

#endif //PICKUPS_H
