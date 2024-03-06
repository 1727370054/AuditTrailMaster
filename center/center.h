#ifndef __CENTER_H__
#define __CENTER_H__

#include <string>

#include "../orion_link_db/orion_link_db.h"

class Center
{
public:
    ~Center();

    static Center* GetInstance()
    {
        static Center c;
        return &c;
    }

    bool Install(std::string ip);

    /// @brief 
    bool Init();
private:
    Center();
    Center(const Center&) = delete;
    Center& operator=(const Center&) = delete;

private:
    ol::OrionLinkDB* db_ = NULL;
};

#endif // __CENTER_H__

