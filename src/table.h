#ifndef __TABLE_H__
#define __TABLE_H__

#include <memory>
#include <vector>

#include <memory>
#include <vector>

#include "log/log.h"
#include "player.h"

class Table
{
public:

private:
    int                                     id_ = 0;
    int                                     player_count_ = 0;
    int                                     player_count_max_ = 0;
    std::vector<std::shared_ptr<Player>>    player_list_;
};

#endif