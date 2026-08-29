#include "item_loader.h"

void ItemLoader::_bind_methods() {
    BIND_METHOD(ItemLoader, generate_directional_variant, "block", "direction", "generate_texture");
    BIND_METHOD(ItemLoader, get_rotated_texture, "block", "direction");
}

DEFINE_CONSTRUCTORS(ItemLoader);

Ref<Block> ItemLoader::generate_directional_variant(const Ref<Block> &block, Vector3i direction, bool generate_texture) {
    Ref<Block> new_block = block->duplicate();
    if (new_block->can_drop && block->drop_item == nullptr) {
        new_block->set_drop_item(block);
    }
    
    new_block->set_essence(new_block->get_essence()->duplicate());
    new_block->get_essence()->set("fuseable", false);
    new_block->set_internal(true);

    // Direction-based id / name
    String suffix;
    int id_offset = 0;

    if (direction == Vector3i(0, 1, 0)) { suffix = " y+"; id_offset = 1; }
    else if (direction == Vector3i(0, -1, 0)) { suffix = " y-"; id_offset = 2; }
    else if (direction == Vector3i(0, 0, 1)) { suffix = " z+"; id_offset = 3; }
    else if (direction == Vector3i(0, 0, -1)) { suffix = " z-"; id_offset = 4; }
    else if (direction == Vector3i(1, 0, 0)) { suffix = " x+"; id_offset = 5; }
    else if (direction == Vector3i(-1, 0, 0)) { suffix = " x-"; id_offset = 6; }

    new_block->set_internal_name(block->get_internal_name() + suffix);
    new_block->set_display_name(block->get_display_name() + suffix);
    new_block->set_id(block->get_id() + id_offset);
    new_block->set_direction(direction);

    // Rotate textures
    if (generate_texture) {
        new_block->set_texture(get_rotated_texture(block, direction));
    } else {
        new_block->set_texture(block->get_texture());
    }

    return new_block;
}

Ref<ImageTexture> ItemLoader::get_rotated_texture(const Ref<Block> &block, Vector3i direction) {
    if (block.is_null() || block->get_texture().is_null()) {
        UtilityFunctions::printerr("get_rotated_texture called with null block or texture");
        return Ref<ImageTexture>();
    }

    Ref<Image> base_img = block->get_texture()->get_image();
    if (base_img.is_null()) {
        UtilityFunctions::printerr("get_rotated_texture failed to get image");
        return Ref<ImageTexture>();
    }

    auto region = [](Ref<Image> img, int x, int y) {
        return img->get_region(Rect2i(x, y, 15, 15));
    };

    Ref<Image> y_p = region(base_img, 0, 0);
    Ref<Image> y_n = region(base_img, 15, 0);
    Ref<Image> z_n = region(base_img, 30, 0);
    Ref<Image> z_p = region(base_img, 45, 0);
    Ref<Image> x_n = region(base_img, 60, 0);
    Ref<Image> x_p = region(base_img, 75, 0);

    Ref<Image> ny_p = y_p, ny_n = y_n, nz_p = z_p, nz_n = z_n, nx_p = x_p, nx_n = x_n;

    if (direction == Vector3i(0, 0, 1)) {
        z_n->rotate_180();
        ny_p = z_n; ny_n = z_p;
        nz_p = y_p;
        y_n->rotate_180(); nz_n = y_n;
        x_p->rotate_90(ClockDirection::CLOCKWISE);
        nx_p = x_p;
        x_n->rotate_90(ClockDirection::COUNTERCLOCKWISE);
        nx_n = x_n;
    } else if (direction == Vector3i(0, 0, -1)) {
        ny_p = z_p;
        z_n->rotate_180(); ny_n = z_n;
        nz_p = y_n;
        y_p->rotate_180(); nz_n = y_p;
        x_p->rotate_90(ClockDirection::COUNTERCLOCKWISE); nx_p = x_p;
        x_n->rotate_90(ClockDirection::CLOCKWISE); nx_n = x_n;
    } else if (direction == Vector3i(0, -1, 0)) {
        y_n->rotate_180();
        ny_p = y_n; ny_n = y_p;
        z_n->rotate_180(); nz_p = z_n;
        z_p->rotate_180(); nz_n = z_p;
        x_p->rotate_180(); nx_p = x_p;
        x_n->rotate_180(); nx_n = x_n;
    } else if (direction == Vector3i(1, 0, 0)) {
        x_n->rotate_90(ClockDirection::CLOCKWISE); ny_p = x_n;
        x_p->rotate_90(ClockDirection::CLOCKWISE); ny_n = x_p;
        y_p->rotate_90(ClockDirection::CLOCKWISE); nx_p = y_p;
        y_n->rotate_90(ClockDirection::CLOCKWISE); nx_n = y_n;
        z_p->rotate_90(ClockDirection::CLOCKWISE); nz_p = z_p;
        z_n->rotate_90(ClockDirection::COUNTERCLOCKWISE); nz_n = z_n;
    } else if (direction == Vector3i(-1, 0, 0)) {
        x_p->rotate_90(ClockDirection::COUNTERCLOCKWISE); ny_p = x_p;
        x_n->rotate_90(ClockDirection::COUNTERCLOCKWISE); ny_n = x_n;
        y_n->rotate_90(ClockDirection::COUNTERCLOCKWISE); nx_p = y_n;
        y_p->rotate_90(ClockDirection::COUNTERCLOCKWISE); nx_n = y_p;
        z_p->rotate_90(ClockDirection::COUNTERCLOCKWISE); nz_p = z_p;
        z_n->rotate_90(ClockDirection::CLOCKWISE); nz_n = z_n;
    }

    Ref<Image> new_img = base_img->duplicate();
    new_img->blit_rect(ny_p, Rect2i(0, 0, 15, 15), Vector2i(0, 0));
    new_img->blit_rect(ny_n, Rect2i(0, 0, 15, 15), Vector2i(15, 0));
    new_img->blit_rect(nz_n, Rect2i(0, 0, 15, 15), Vector2i(30, 0));
    new_img->blit_rect(nz_p, Rect2i(0, 0, 15, 15), Vector2i(45, 0));
    new_img->blit_rect(nx_n, Rect2i(0, 0, 15, 15), Vector2i(60, 0));
    new_img->blit_rect(nx_p, Rect2i(0, 0, 15, 15), Vector2i(75, 0));

    return ImageTexture::create_from_image(new_img);
}
