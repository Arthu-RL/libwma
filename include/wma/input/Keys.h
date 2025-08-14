#ifndef KEYS_H
#define KEYS_H

#include <ink/ink_base.hpp>

#ifdef WMA_ENABLE_GLFW
#include <GLFW/glfw3.h>
#endif

#ifdef WMA_ENABLE_SDL
#include <SDL2/SDL.h>
#endif

namespace wma {

enum Key : i32 {
    // Letters
    KEY_A = 0,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,

    // Numbers
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,

    // Function keys
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,

    // Control keys
    KEY_ESCAPE,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_INSERT,
    KEY_DELETE,
    KEY_RIGHT,
    KEY_LEFT,
    KEY_DOWN,
    KEY_UP,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_HOME,
    KEY_END,

    // Modifier keys
    KEY_CAPS_LOCK,
    KEY_SCROLL_LOCK,
    KEY_NUM_LOCK,
    KEY_LEFT_SHIFT,
    KEY_RIGHT_SHIFT,
    KEY_LEFT_CTRL,
    KEY_RIGHT_CTRL,
    KEY_LEFT_ALT,
    KEY_RIGHT_ALT,
    KEY_LEFT_SUPER,  // Windows/Command
    KEY_RIGHT_SUPER,

    // Symbols
    KEY_SPACE,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_LEFT_BRACKET,
    KEY_RIGHT_BRACKET,
    KEY_BACKSLASH,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_GRAVE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,

    // Keypad
    KEY_KP_0,
    KEY_KP_1,
    KEY_KP_2,
    KEY_KP_3,
    KEY_KP_4,
    KEY_KP_5,
    KEY_KP_6,
    KEY_KP_7,
    KEY_KP_8,
    KEY_KP_9,
    KEY_KP_DECIMAL,
    KEY_KP_DIVIDE,
    KEY_KP_MULTIPLY,
    KEY_KP_SUBTRACT,
    KEY_KP_ADD,
    KEY_KP_ENTER,

    KEY_UNKNOWN = -1
};

#ifdef WMA_ENABLE_GLFW

//! Map keys to GLFW
inline Key mapGLFWKey(int glfwKey) {
    switch (glfwKey) {
    // Letters
    case GLFW_KEY_A: return Key::KEY_A;
    case GLFW_KEY_B: return Key::KEY_B;
    case GLFW_KEY_C: return Key::KEY_C;
    case GLFW_KEY_D: return Key::KEY_D;
    case GLFW_KEY_E: return Key::KEY_E;
    case GLFW_KEY_F: return Key::KEY_F;
    case GLFW_KEY_G: return Key::KEY_G;
    case GLFW_KEY_H: return Key::KEY_H;
    case GLFW_KEY_I: return Key::KEY_I;
    case GLFW_KEY_J: return Key::KEY_J;
    case GLFW_KEY_K: return Key::KEY_K;
    case GLFW_KEY_L: return Key::KEY_L;
    case GLFW_KEY_M: return Key::KEY_M;
    case GLFW_KEY_N: return Key::KEY_N;
    case GLFW_KEY_O: return Key::KEY_O;
    case GLFW_KEY_P: return Key::KEY_P;
    case GLFW_KEY_Q: return Key::KEY_Q;
    case GLFW_KEY_R: return Key::KEY_R;
    case GLFW_KEY_S: return Key::KEY_S;
    case GLFW_KEY_T: return Key::KEY_T;
    case GLFW_KEY_U: return Key::KEY_U;
    case GLFW_KEY_V: return Key::KEY_V;
    case GLFW_KEY_W: return Key::KEY_W;
    case GLFW_KEY_X: return Key::KEY_X;
    case GLFW_KEY_Y: return Key::KEY_Y;
    case GLFW_KEY_Z: return Key::KEY_Z;

        // Numbers
    case GLFW_KEY_0: return Key::KEY_0;
    case GLFW_KEY_1: return Key::KEY_1;
    case GLFW_KEY_2: return Key::KEY_2;
    case GLFW_KEY_3: return Key::KEY_3;
    case GLFW_KEY_4: return Key::KEY_4;
    case GLFW_KEY_5: return Key::KEY_5;
    case GLFW_KEY_6: return Key::KEY_6;
    case GLFW_KEY_7: return Key::KEY_7;
    case GLFW_KEY_8: return Key::KEY_8;
    case GLFW_KEY_9: return Key::KEY_9;

        // Function keys
    case GLFW_KEY_F1: return Key::KEY_F1;
    case GLFW_KEY_F2: return Key::KEY_F2;
    case GLFW_KEY_F3: return Key::KEY_F3;
    case GLFW_KEY_F4: return Key::KEY_F4;
    case GLFW_KEY_F5: return Key::KEY_F5;
    case GLFW_KEY_F6: return Key::KEY_F6;
    case GLFW_KEY_F7: return Key::KEY_F7;
    case GLFW_KEY_F8: return Key::KEY_F8;
    case GLFW_KEY_F9: return Key::KEY_F9;
    case GLFW_KEY_F10: return Key::KEY_F10;
    case GLFW_KEY_F11: return Key::KEY_F11;
    case GLFW_KEY_F12: return Key::KEY_F12;

        // Controls
    case GLFW_KEY_ESCAPE: return Key::KEY_ESCAPE;
    case GLFW_KEY_ENTER: return Key::KEY_ENTER;
    case GLFW_KEY_TAB: return Key::KEY_TAB;
    case GLFW_KEY_BACKSPACE: return Key::KEY_BACKSPACE;
    case GLFW_KEY_INSERT: return Key::KEY_INSERT;
    case GLFW_KEY_DELETE: return Key::KEY_DELETE;
    case GLFW_KEY_RIGHT: return Key::KEY_RIGHT;
    case GLFW_KEY_LEFT: return Key::KEY_LEFT;
    case GLFW_KEY_DOWN: return Key::KEY_DOWN;
    case GLFW_KEY_UP: return Key::KEY_UP;
    case GLFW_KEY_PAGE_UP: return Key::KEY_PAGE_UP;
    case GLFW_KEY_PAGE_DOWN: return Key::KEY_PAGE_DOWN;
    case GLFW_KEY_HOME: return Key::KEY_HOME;
    case GLFW_KEY_END: return Key::KEY_END;

        // Modifiers
    case GLFW_KEY_LEFT_SHIFT: return Key::KEY_LEFT_SHIFT;
    case GLFW_KEY_RIGHT_SHIFT: return Key::KEY_RIGHT_SHIFT;
    case GLFW_KEY_LEFT_CONTROL: return Key::KEY_LEFT_CTRL;
    case GLFW_KEY_RIGHT_CONTROL: return Key::KEY_RIGHT_CTRL;
    case GLFW_KEY_LEFT_ALT: return Key::KEY_LEFT_ALT;
    case GLFW_KEY_RIGHT_ALT: return Key::KEY_RIGHT_ALT;
    case GLFW_KEY_LEFT_SUPER: return Key::KEY_LEFT_SUPER;
    case GLFW_KEY_RIGHT_SUPER: return Key::KEY_RIGHT_SUPER;
    case GLFW_KEY_CAPS_LOCK: return Key::KEY_CAPS_LOCK;
    case GLFW_KEY_SCROLL_LOCK: return Key::KEY_SCROLL_LOCK;
    case GLFW_KEY_NUM_LOCK: return Key::KEY_NUM_LOCK;

        // Symbols
    case GLFW_KEY_SPACE: return Key::KEY_SPACE;
    case GLFW_KEY_MINUS: return Key::KEY_MINUS;
    case GLFW_KEY_EQUAL: return Key::KEY_EQUAL;
    case GLFW_KEY_LEFT_BRACKET: return Key::KEY_LEFT_BRACKET;
    case GLFW_KEY_RIGHT_BRACKET: return Key::KEY_RIGHT_BRACKET;
    case GLFW_KEY_BACKSLASH: return Key::KEY_BACKSLASH;
    case GLFW_KEY_SEMICOLON: return Key::KEY_SEMICOLON;
    case GLFW_KEY_APOSTROPHE: return Key::KEY_APOSTROPHE;
    case GLFW_KEY_GRAVE_ACCENT: return Key::KEY_GRAVE;
    case GLFW_KEY_COMMA: return Key::KEY_COMMA;
    case GLFW_KEY_PERIOD: return Key::KEY_PERIOD;
    case GLFW_KEY_SLASH: return Key::KEY_SLASH;

        // Keypad
    case GLFW_KEY_KP_0: return Key::KEY_KP_0;
    case GLFW_KEY_KP_1: return Key::KEY_KP_1;
    case GLFW_KEY_KP_2: return Key::KEY_KP_2;
    case GLFW_KEY_KP_3: return Key::KEY_KP_3;
    case GLFW_KEY_KP_4: return Key::KEY_KP_4;
    case GLFW_KEY_KP_5: return Key::KEY_KP_5;
    case GLFW_KEY_KP_6: return Key::KEY_KP_6;
    case GLFW_KEY_KP_7: return Key::KEY_KP_7;
    case GLFW_KEY_KP_8: return Key::KEY_KP_8;
    case GLFW_KEY_KP_9: return Key::KEY_KP_9;
    case GLFW_KEY_KP_DECIMAL: return Key::KEY_KP_DECIMAL;
    case GLFW_KEY_KP_DIVIDE: return Key::KEY_KP_DIVIDE;
    case GLFW_KEY_KP_MULTIPLY: return Key::KEY_KP_MULTIPLY;
    case GLFW_KEY_KP_SUBTRACT: return Key::KEY_KP_SUBTRACT;
    case GLFW_KEY_KP_ADD: return Key::KEY_KP_ADD;
    case GLFW_KEY_KP_ENTER: return Key::KEY_KP_ENTER;

    default: return Key::KEY_UNKNOWN;
    }
}

#endif


#ifdef WMA_ENABLE_SDL
//! Map keys to SDL2
inline Key mapSDLKey(SDL_Keycode sdlKey) {
    switch (sdlKey) {
    // Letters
    case SDLK_a: return Key::KEY_A;
    case SDLK_b: return Key::KEY_B;
    case SDLK_c: return Key::KEY_C;
    case SDLK_d: return Key::KEY_D;
    case SDLK_e: return Key::KEY_E;
    case SDLK_f: return Key::KEY_F;
    case SDLK_g: return Key::KEY_G;
    case SDLK_h: return Key::KEY_H;
    case SDLK_i: return Key::KEY_I;
    case SDLK_j: return Key::KEY_J;
    case SDLK_k: return Key::KEY_K;
    case SDLK_l: return Key::KEY_L;
    case SDLK_m: return Key::KEY_M;
    case SDLK_n: return Key::KEY_N;
    case SDLK_o: return Key::KEY_O;
    case SDLK_p: return Key::KEY_P;
    case SDLK_q: return Key::KEY_Q;
    case SDLK_r: return Key::KEY_R;
    case SDLK_s: return Key::KEY_S;
    case SDLK_t: return Key::KEY_T;
    case SDLK_u: return Key::KEY_U;
    case SDLK_v: return Key::KEY_V;
    case SDLK_w: return Key::KEY_W;
    case SDLK_x: return Key::KEY_X;
    case SDLK_y: return Key::KEY_Y;
    case SDLK_z: return Key::KEY_Z;

        // Numbers
    case SDLK_0: return Key::KEY_0;
    case SDLK_1: return Key::KEY_1;
    case SDLK_2: return Key::KEY_2;
    case SDLK_3: return Key::KEY_3;
    case SDLK_4: return Key::KEY_4;
    case SDLK_5: return Key::KEY_5;
    case SDLK_6: return Key::KEY_6;
    case SDLK_7: return Key::KEY_7;
    case SDLK_8: return Key::KEY_8;
    case SDLK_9: return Key::KEY_9;

        // Function keys
    case SDLK_F1: return Key::KEY_F1;
    case SDLK_F2: return Key::KEY_F2;
    case SDLK_F3: return Key::KEY_F3;
    case SDLK_F4: return Key::KEY_F4;
    case SDLK_F5: return Key::KEY_F5;
    case SDLK_F6: return Key::KEY_F6;
    case SDLK_F7: return Key::KEY_F7;
    case SDLK_F8: return Key::KEY_F8;
    case SDLK_F9: return Key::KEY_F9;
    case SDLK_F10: return Key::KEY_F10;
    case SDLK_F11: return Key::KEY_F11;
    case SDLK_F12: return Key::KEY_F12;

        // Controls
    case SDLK_ESCAPE: return Key::KEY_ESCAPE;
    case SDLK_RETURN: return Key::KEY_ENTER;
    case SDLK_TAB: return Key::KEY_TAB;
    case SDLK_BACKSPACE: return Key::KEY_BACKSPACE;
    case SDLK_INSERT: return Key::KEY_INSERT;
    case SDLK_DELETE: return Key::KEY_DELETE;
    case SDLK_RIGHT: return Key::KEY_RIGHT;
    case SDLK_LEFT: return Key::KEY_LEFT;
    case SDLK_DOWN: return Key::KEY_DOWN;
    case SDLK_UP: return Key::KEY_UP;
    case SDLK_PAGEUP: return Key::KEY_PAGE_UP;
    case SDLK_PAGEDOWN: return Key::KEY_PAGE_DOWN;
    case SDLK_HOME: return Key::KEY_HOME;
    case SDLK_END: return Key::KEY_END;

        // Modifiers
    case SDLK_LSHIFT: return Key::KEY_LEFT_SHIFT;
    case SDLK_RSHIFT: return Key::KEY_RIGHT_SHIFT;
    case SDLK_LCTRL: return Key::KEY_LEFT_CTRL;
    case SDLK_RCTRL: return Key::KEY_RIGHT_CTRL;
    case SDLK_LALT: return Key::KEY_LEFT_ALT;
    case SDLK_RALT: return Key::KEY_RIGHT_ALT;
    case SDLK_LGUI: return Key::KEY_LEFT_SUPER;
    case SDLK_RGUI: return Key::KEY_RIGHT_SUPER;
    case SDLK_CAPSLOCK: return Key::KEY_CAPS_LOCK;
    case SDLK_NUMLOCKCLEAR: return Key::KEY_NUM_LOCK;
    case SDLK_SCROLLLOCK: return Key::KEY_SCROLL_LOCK;

        // Symbols
    case SDLK_SPACE: return Key::KEY_SPACE;
    case SDLK_MINUS: return Key::KEY_MINUS;
    case SDLK_EQUALS: return Key::KEY_EQUAL;
    case SDLK_LEFTBRACKET: return Key::KEY_LEFT_BRACKET;
    case SDLK_RIGHTBRACKET: return Key::KEY_RIGHT_BRACKET;
    case SDLK_BACKSLASH: return Key::KEY_BACKSLASH;
    case SDLK_SEMICOLON: return Key::KEY_SEMICOLON;
    case SDLK_QUOTE: return Key::KEY_APOSTROPHE;
    case SDLK_BACKQUOTE: return Key::KEY_GRAVE;
    case SDLK_COMMA: return Key::KEY_COMMA;
    case SDLK_PERIOD: return Key::KEY_PERIOD;
    case SDLK_SLASH: return Key::KEY_SLASH;

        // Keypad
    case SDLK_KP_0: return Key::KEY_KP_0;
    case SDLK_KP_1: return Key::KEY_KP_1;
    case SDLK_KP_2: return Key::KEY_KP_2;
    case SDLK_KP_3: return Key::KEY_KP_3;
    case SDLK_KP_4: return Key::KEY_KP_4;
    case SDLK_KP_5: return Key::KEY_KP_5;
    case SDLK_KP_6: return Key::KEY_KP_6;
    case SDLK_KP_7: return Key::KEY_KP_7;
    case SDLK_KP_8: return Key::KEY_KP_8;
    case SDLK_KP_9: return Key::KEY_KP_9;
    case SDLK_KP_PERIOD: return Key::KEY_KP_DECIMAL;
    case SDLK_KP_DIVIDE: return Key::KEY_KP_DIVIDE;
    case SDLK_KP_MULTIPLY: return Key::KEY_KP_MULTIPLY;
    case SDLK_KP_MINUS: return Key::KEY_KP_SUBTRACT;
    case SDLK_KP_PLUS: return Key::KEY_KP_ADD;
    case SDLK_KP_ENTER: return Key::KEY_KP_ENTER;

    default: return Key::KEY_UNKNOWN;
    }
}

#endif

} // namespace wma

#endif // KEYS_H
