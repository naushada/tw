#ifndef __io_adapter_hpp__
#define __io_adapter_hpp__

#include "evt_adapter.hpp"

class io_evt : public evt {
  public:
    io_evt(const evt::events& _event, const std::chrono::seconds& secs) : evt(_event, secs) {}

    virtual ~io_evt() = default;
  private:
};

class timer_evt : public evt {
  public:
    timer_evt(const bool& is_periodic, const std::chrono::seconds& secs): evt(is_periodic, secs) {}
    virtual ~timer_evt() = default;
  private:
};

#endif
