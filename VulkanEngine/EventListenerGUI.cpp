
#include "VEInclude.h"

namespace ve {

    void EventListenerGUI::onDrawOverlay(veEvent event) {
        VESubrenderFW_Nuklear* pSubrender = (VESubrenderFW_Nuklear*)getRendererPointer()->getOverlay();
        if (pSubrender == nullptr) return;

        struct nk_context* ctx = pSubrender->getContext();

        enum { EASY, HARD };
        static int op = EASY;
        static float value = 0.6f;
        static int i = 20;

        if (nk_begin(ctx, "Show", nk_rect(50, 50, 220, 220),
            NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE))
        {
            nk_layout_row_static(ctx, 30, 80, 1); /* fixed widget pixel width */
            if (nk_button_label(ctx, "button")) {
                /* event handling */
            }
            nk_layout_row_dynamic(ctx, 30, 2); /* fixed widget window ratio width */
            if (nk_option_label(ctx, "easy", op == EASY)) { op = EASY; }
            if (nk_option_label(ctx, "hard", op == HARD)) { op = HARD; }
            /* custom widget pixel width */
            nk_layout_row_begin(ctx, NK_STATIC, 30, 2);
            {
                nk_layout_row_push(ctx, 50);
                nk_label(ctx, "Volume:", NK_TEXT_LEFT);
                nk_layout_row_push(ctx, 110);
                nk_slider_float(ctx, 0, &value, 1.0f, 0.1f);
            }
            nk_layout_row_end(ctx);
        }
        nk_end(ctx);

    }

}