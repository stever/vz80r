//------------------------------------------------------------------------------
//  v6502r.c
//  Main source file.
//------------------------------------------------------------------------------
#include "v6502r.h"
#include "sokol_args.h"

app_state_t app;

/* example program from visual6502.org:
                   * = 0000
 0000   A9 00      LDA #$00
 0002   20 10 00   JSR $0010
 0005   4C 02 00   JMP $0002
 0008   00         BRK
 0009   00         BRK
 000A   00         BRK
 000B   00         BRK
 000C   00         BRK
 000D   00         BRK
 000E   00         BRK
 000F   43         ???
 0010   E8         INX
 0011   88         DEY
 0012   E6 0F      INC $0F
 0014   38         SEC
 0015   69 02      ADC #$02
 0017   60         RTS
 0018              .END
*/
static uint8_t test_prog[] = {
    0xa9, 0x00, 0x20, 0x10, 0x00, 0x4c, 0x02, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x43,
    0xe8, 0x88, 0xe6, 0x0f, 0x38, 0x69, 0x02, 0x60
};

static void app_init(void) {
    util_init();
    gfx_init(&(gfx_desc_t){
        .seg_vertices = {
            [0] = SG_RANGE(seg_vertices_0),
            [1] = SG_RANGE(seg_vertices_1),
            [2] = SG_RANGE(seg_vertices_2),
            [3] = SG_RANGE(seg_vertices_3),
            [4] = SG_RANGE(seg_vertices_4),
            [5] = SG_RANGE(seg_vertices_5),
        },
        .seg_max_x = seg_max_x,
        .seg_max_y = seg_max_y,
    });
    ui_init();
    pick_init(&(pick_desc_t){
        .seg_max_x = seg_max_x,
        .seg_max_y = seg_max_y,
        .layers = {
            [0] = {
                .verts = (const pick_vertex_t*) seg_vertices_0,
                .num_verts = sizeof(seg_vertices_0) / 8,
            },
            [1] = {
                .verts = (const pick_vertex_t*) seg_vertices_1,
                .num_verts = sizeof(seg_vertices_1) / 8,
            },
            [2] = {
                .verts = (const pick_vertex_t*) seg_vertices_2,
                .num_verts = sizeof(seg_vertices_2) / 8,
            },
            [3] = {
                .verts = (const pick_vertex_t*) seg_vertices_3,
                .num_verts = sizeof(seg_vertices_3) / 8,
            },
            [4] = {
                .verts = (const pick_vertex_t*) seg_vertices_4,
                .num_verts = sizeof(seg_vertices_4) / 8,
            },
            [5] = {
                .verts = (const pick_vertex_t*) seg_vertices_5,
                .num_verts = sizeof(seg_vertices_5) / 8,
            },
        },
        .mesh = {
            .tris = (const pick_triangle_t*) pick_tris,
            .num_tris = sizeof(pick_tris) / 8,
        },
        .grid = {
            .num_cells_x = grid_cells,
            .num_cells_y = grid_cells,
            .cells = (const pick_cell_t*) pick_grid,
            .num_cells = sizeof(pick_grid) / 8,
        }
    });
    trace_init(&app.trace);
    sim_init_or_reset(&app.sim);
    sim_write(0x0000, sizeof(test_prog), test_prog);
    sim_start(&app.sim, &app.trace);
}

static void app_frame(void) {
    gfx_new_frame(sapp_widthf(), sapp_heightf());
    ui_frame();
    sim_frame(&app.sim, &app.trace);
    const pick_result_t pick_result = pick_dopick(
        app.input.mouse,
        gfx_get_display_size(),
        gfx_get_offset(),
        gfx_get_aspect(),
        gfx_get_scale());
    for (int i = 0; i < pick_result.num_hits; i++) {
        gfx_highlight_node(pick_result.node_index[i]);
    }
    gfx_begin();
    gfx_draw();
    ui_draw();
    gfx_end();
}

static void app_input(const sapp_event* ev) {
    if (ui_input(ev)) {
        app.input.mouse.x = 0;
        app.input.mouse.y = 0;
        app.input.dragging = false;
        return;
    }
    float w = (float) ev->framebuffer_width * gfx_get_scale();
    float h = (float) ev->framebuffer_height * gfx_get_scale() * gfx_get_aspect();
    switch (ev->type) {
        case SAPP_EVENTTYPE_MOUSE_SCROLL:
            gfx_add_scale(ev->scroll_y * 0.25f);
            app.input.mouse.x = ev->mouse_x;
            app.input.mouse.y = ev->mouse_y;
            break;
        case SAPP_EVENTTYPE_MOUSE_DOWN:
            app.input.dragging = true;
            app.input.drag_start = (float2_t){ev->mouse_x, ev->mouse_y};
            app.input.offset_start = gfx_get_offset();
            app.input.mouse.x = ev->mouse_x;
            app.input.mouse.y = ev->mouse_y;
            break;
        case SAPP_EVENTTYPE_MOUSE_MOVE:
            if (app.input.dragging) {
                float dx = ((ev->mouse_x - app.input.drag_start.x) * 2.0f) / w;
                float dy = ((ev->mouse_y - app.input.drag_start.y) * -2.0f) / h;
                gfx_set_offset((float2_t){
                    .x = app.input.offset_start.x + dx,
                    .y = app.input.offset_start.y + dy,
                });
            }
            app.input.mouse.x = ev->mouse_x;
            app.input.mouse.y = ev->mouse_y;
            break;
        case SAPP_EVENTTYPE_MOUSE_UP:
            app.input.dragging = false;
            break;
        case SAPP_EVENTTYPE_MOUSE_LEAVE:
            app.input.dragging = false;
            break;
        default:
            break;
    }
}

static void app_cleanup(void) {
    sim_shutdown(&app.sim);
    trace_shutdown(&app.trace);
    ui_shutdown();
    gfx_shutdown();
}

sapp_desc sokol_main(int argc, char* argv[]) {
    sargs_setup(&(sargs_desc){ .argc = argc, .argv = argv });
    return (sapp_desc){
        .init_cb = app_init,
        .frame_cb = app_frame,
        .event_cb = app_input,
        .cleanup_cb = app_cleanup,
        .width = 900,
        .height = 700,
        .window_title = "Visual6502 Remix",
        .enable_clipboard = true,
        .clipboard_size = 16*1024,
        .icon = {
            .sokol_default = true
        }
    };
}