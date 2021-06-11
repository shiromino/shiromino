#pragma once
namespace Shiro {
    struct MetaGameOptionData {
        MetaGameOptionData();
        int mode;
        int submode;
        int num_args;
        int num_subargs;
        void **args;
        void **sub_args;
    };
}