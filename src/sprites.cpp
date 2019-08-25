
struct Animation
{
    float duration = 1.f;
    ivec2 start = {0, 0};
    u32 size = 1;
    b32 loop = true;
};

struct Sprite
{
    GLTilemapTexture const * tilemap = 0;
    ivec2 size = {64, 64};
    ivec2 position = {0,0};
    float rotation = 0.f;
    AABox collision_box = {0,0,64,64};
    ivec2 mirror = {false, false};
    ivec2 velocity = {0,0};
    
    i32 current_frame = 0;
    u32 current_animation = 0;
    float time_accumulator = 0.f;
    Animation* animation_set;
    
    AABox get_transformed_AABox() const
    {
        return this->collision_box.translate(this->position);
    }
};

internal void 
Sprite_update_animation(Sprite *const sprite, float dt)
{
    Animation* anim_ref;
    
    anim_ref = &sprite->animation_set[sprite->current_animation];
    if (anim_ref->size == 0)
        return;
    
    sprite->time_accumulator += dt;
    
    if (sprite->time_accumulator >= anim_ref->duration)
    {
        if (sprite->current_frame < anim_ref->size)
            sprite->current_frame++;
        else if (sprite->current_frame >= anim_ref->size && anim_ref->loop)
            sprite->current_frame = 0;
        
        sprite->time_accumulator = 0.f;
    }
    
}

internal void
Sprite_draw(Sprite const * sprite, i32 frame)
{
    assert(sprite->tilemap);
    
    gl_slow_tilemap_draw(
        sprite->tilemap,
        {(float)sprite->position.x, (float)sprite->position.y},
        {(float)sprite->size.x, (float)sprite->size.y},
        sprite->rotation,
        frame,
        sprite->mirror.x, sprite->mirror.y);
    
}

internal void
Sprite_draw_anim(Sprite const * sprite)
{
    assert(sprite->tilemap);
    Animation* anim_ref = &sprite->animation_set[sprite->current_animation];
    i32 frame_to_render = anim_ref->start.y * sprite->tilemap->cols
        + anim_ref->start.x + sprite->current_frame;
    //Sprite_draw(sprite, frame_to_render);
    
    gl_slow_tilemap_draw(
        sprite->tilemap,
        {(float)sprite->position.x, (float)sprite->position.y},
        {(float)sprite->size.x, (float)sprite->size.y},
        sprite->rotation,
        frame_to_render,
        sprite->mirror.x, sprite->mirror.y);
    
}

internal void
Sprite_set_anim(Sprite *const sprite, u32 new_anim)
{
    sprite->time_accumulator = 0.f;
    sprite->current_frame = 0;
    sprite->current_animation = new_anim;
}
