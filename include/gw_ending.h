#ifndef GW_ENDING_H
#define GW_ENDING_H

namespace gw
{

// The story's end, shown after turning in Bazil Thredd: a few pages of epilogue and the
// player's numbers. The world stays open afterwards.
class ending
{

public:
    void open();

    [[nodiscard]] bool is_open() const
    {
        return _open;
    }

    // Reads input and redraws. Returns false once it closed.
    bool update();

private:
    bool _open = false;
    bool _dirty = true;
    int _page = 0;

    void _draw();
};

}

#endif
