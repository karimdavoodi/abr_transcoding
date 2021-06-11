#pragma once


#define LOG(level) BOOST_LOG_TRIVIAL(level) \
        << "\033[0;32m[" << __func__ << ":" <<__LINE__ << "]\033[0m "
