#include "gw_looks_data.h"

namespace gw
{

const look_def& get_look(look_id look)
{
    return look_table[int(look)];
}

}
