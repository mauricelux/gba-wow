#ifndef GW_MENU_LAYOUT_H
#define GW_MENU_LAYOUT_H

// Rows and columns shared by the pages of the pause menu (gw_menu*.cpp).
namespace gw::menu_layout
{
    constexpr int title_row = 1;
    constexpr int content_top = 3;
    constexpr int content_rows = 14;
    constexpr int list_rows = 9;        // lists with a details box under them
    constexpr int details_top = 13;
    constexpr int details_rows = 4;
    constexpr int hint_row = 18;
    constexpr int page_x = 2;
    constexpr int page_width = 26;
}

#endif
