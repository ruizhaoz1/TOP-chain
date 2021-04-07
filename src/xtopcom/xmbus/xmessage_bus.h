// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <thread>
#include "xbasic/xns_macro.h"
#include "xmbus/xevent_queue.h"

NS_BEG2(top, mbus)

class xmessage_bus_t;

// very light timer implementation
// just for message bus timer event
class xmessage_bus_timer_t {
public:
    xmessage_bus_timer_t(xmessage_bus_t* bus,
            int timer_interval_milliseconds);
    virtual ~xmessage_bus_timer_t();

    void start();
    void stop();

private:
    std::atomic<bool> m_running {false};
    int m_interval_milliseconds;
    std::thread m_thread;
    xmessage_bus_t* m_message_bus;
};

class xtop_message_bus_face {
public:
    XDECLARE_DEFAULTED_DEFAULT_CONSTRUCTOR(xtop_message_bus_face);
    XDECLARE_DELETED_COPY_DEFAULTED_MOVE_SEMANTICS(xtop_message_bus_face);
    XDECLARE_DEFAULTED_VIRTULA_DESTRUCTOR(xtop_message_bus_face);

    virtual uint32_t add_sourcer(int major_type, xevent_queue_cb_t cb) = 0;
    virtual void remove_sourcer(int major_type, uint32_t id) = 0;
    virtual uint32_t add_listener(int major_type, xevent_queue_cb_t cb) = 0;
    virtual void remove_listener(int major_type, uint32_t id) = 0;

    virtual void push_event(const xevent_ptr_t& e) = 0;
    virtual void clear() = 0;

    virtual int size() = 0;
    virtual int sourcers_size() = 0;
    virtual int listeners_size() = 0;
    virtual xevent_queue_ptr_t get_queue(int major_type) = 0;

};
using xmessage_bus_face_t = xtop_message_bus_face;

class xmessage_bus_t final : public xmessage_bus_face_t {
public:

    xmessage_bus_t(bool enable_timer = false,
            int timer_interval_seconds = 1);
    ~xmessage_bus_t() override;

    uint32_t add_sourcer(int major_type, xevent_queue_cb_t cb) override;
    void remove_sourcer(int major_type, uint32_t id) override;
    uint32_t add_listener(int major_type, xevent_queue_cb_t cb) override;
    void remove_listener(int major_type, uint32_t id) override;

    void push_event(const xevent_ptr_t& e) override;
    void clear() override;

    // monitor functions
    int size() override;
    int sourcers_size() override;
    int listeners_size() override;
    xevent_queue_ptr_t get_queue(int major_type) override;

private:
    std::vector<xevent_queue_ptr_t> m_queues;
    xmessage_bus_timer_t m_timer;
};

NS_END2

