
struct Animation
{
    float duration = 1.f;
    ivec2 start = {0, 0};
    u32 size = 1;
    b32 loop = true;
};

// AnimatedSprite: load sprite from tilemap and 
struct AnimatedSprite
{
    GLTilemapTexture const * tilemap = 0;
    ivec2 size = {64, 64};
    ivec2 position = {0,0};
    float rotation = 0.f;
    ivec2 mirror = {false, false};
    
};

internal void
draw_sprite(AnimatedSprite const * sprite, i32 frame)
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