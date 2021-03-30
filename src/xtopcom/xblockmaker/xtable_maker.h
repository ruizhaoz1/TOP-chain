﻿// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include "xdata/xtableblock.h"
#include "xdata/xfull_tableblock.h"
#include "xblockmaker/xunit_maker.h"
#include "xblockmaker/xblock_maker_para.h"
#include "xblockmaker/xblockmaker_face.h"
#include "xunit_service/xcons_face.h"

NS_BEG2(top, blockmaker)

using data::xblock_t;
using data::xblock_consensus_para_t;
using data::xtable_block_para_t;
using data::xfulltable_block_para_t;


class xtable_maker_t : public xblock_maker_t {
 public:
    explicit xtable_maker_t(const std::string & account, const xblockmaker_resources_ptr_t & resources);
    virtual ~xtable_maker_t() {}

 public:
    int32_t                 default_check_latest_state();
    xblock_ptr_t            make_proposal(xtablemaker_para_t & table_para, const data::xblock_consensus_para_t & cs_para, xtablemaker_result_t & result);
    int32_t                 verify_proposal(base::xvblock_t* proposal_block, const xtablemaker_para_t & table_para, const data::xblock_consensus_para_t & cs_para);
    bool                    can_make_next_block() const;

 protected:
    int32_t                 check_latest_state(base::xvblock_t* latest_cert_block); // check table latest block and state
    bool                    can_make_next_empty_block() const;
    bool                    can_make_next_full_block() const;
    bool                    can_make_next_light_block() const;
    xunit_maker_ptr_t       create_unit_maker(const std::string & account);
    xunit_maker_ptr_t       pop_unit_maker(const std::string & account);

    xblock_ptr_t            leader_make_light_table(const xtableblock_proposal_input_t & proposal_input, const data::xblock_consensus_para_t & cs_para, xtablemaker_result_t & table_result);
    xblock_ptr_t            backup_make_light_table(const xtableblock_proposal_input_t & proposal_input, const data::xblock_consensus_para_t & cs_para, xtablemaker_result_t & table_result);
    xblock_ptr_t            make_full_table(const xblock_consensus_para_t & cs_para, int32_t & error_code);
    void                    clear_old_unit_makers();

    xunit_maker_ptr_t       get_unit_maker(const std::string & account);
    void                    set_unit_maker(const xunit_maker_ptr_t & unitmaker);
    void                    update_uncommit_unit_makers();
    bool                    update_latest_blocks(const xblock_ptr_t & latest_block);
    bool                    unpack_table_and_update_units(const xblock_ptr_t & block);
    void                    clear_table_units(const xblock_ptr_t & block);
    bool                    verify_proposal_with_local(base::xvblock_t *proposal_block, base::xvblock_t *local_block) const;
    bool                    verify_proposal_class(base::xvblock_t *proposal_block) const;
    std::string             dump() const;
    xblock_ptr_t            leader_make_unit(const xunit_maker_ptr_t & unitmaker, const xunit_proposal_input_t & unit_input, const data::xblock_consensus_para_t & cs_para, xunitmaker_result_t & unit_result);
    bool                    load_table_blocks_from_last_full(const xblock_ptr_t & prev_block, std::vector<xblock_ptr_t> & blocks);

 private:
    std::map<std::string, xunit_maker_ptr_t>    m_unit_makers;
    static constexpr uint32_t                   m_keep_latest_blocks_max{50};
    static constexpr uint32_t                   m_empty_block_max_num{2};
    uint32_t                                    m_full_table_interval_num{100};
    int32_t                                     m_max_account_num{0};
    xblock_builder_face_ptr_t                   m_fulltable_builder;
    xblock_builder_face_ptr_t                   m_lighttable_builder;
    store::xindexstore_face_ptr_t               m_indexstore;
    mutable std::mutex                          m_lock;
};

using xtable_maker_ptr_t = xobject_ptr_t<xtable_maker_t>;

NS_END2