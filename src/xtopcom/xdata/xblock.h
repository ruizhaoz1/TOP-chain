// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>
#include <vector>
#include <map>
#include "json/json.h"
#include "xbasic/xns_macro.h"
#include "xbase/xvblock.h"
#include "xdata/xdata_common.h"
#include "xbasic/xobject_ptr.h"
#include "xdata/xnative_property.h"
#include "xdata/xheader_cert.h"
#include "xdata/xpropertylog.h"
#include "xdata/xaccount_mstate.h"
#include "xdata/xcons_transaction.h"
#include "xdata/xlightunit_info.h"
#include "xdata/xtableindex.h"

NS_BEG2(top, data)

class xblock_consensus_para_t;

class xblockheader_extra_data_t : public xserializable_based_on<void> {
 protected:
    enum xblockheader_extra_data_type : uint16_t {
        enum_extra_data_type_tgas_total_lock_amount_property_height = 0,
    };

 public:
    int32_t do_write(base::xstream_t & stream) const override;
    int32_t do_read(base::xstream_t & stream) override;

    int32_t deserialize_from_string(const std::string & extra_data);
    int32_t serialize_to_string(std::string & extra_data);

 public:
    uint64_t get_tgas_total_lock_amount_property_height() const;
    void     set_tgas_total_lock_amount_property_height(uint64_t height);

 private:
    std::map<uint16_t, std::string>  m_paras;
};

class xblock_t : public base::xvblock_t {
 public:
    static bool check_merkle_path(const std::string &leaf, const xmerkle_path_256_t &hash_path, const std::string & root);
    static std::string get_block_base_path(base::xvblock_t* block) {return block->get_account() + ':' + std::to_string(block->get_height());}
    static xobject_ptr_t<xblock_t> raw_vblock_to_object_ptr(base::xvblock_t* block);
public:
    xblock_t(enum_xdata_type type);
    xblock_t(base::xvheader_t & header, xblockcert_t & cert, enum_xdata_type type);
    // xblock_t(base::xvheader_t & header, xblockcert_t & cert, const std::string & input, const std::string & output, enum_xdata_type type);
    xblock_t(base::xvheader_t & header, xblockcert_t & cert, const xinput_ptr_t & input, const xoutput_ptr_t & output, enum_xdata_type type);


#ifdef XENABLE_PSTACK  // tracking memory
    virtual int32_t add_ref() override;
    virtual int32_t release_ref() override;
#endif

 protected:
    virtual ~xblock_t();
 private:
    xblock_t(const xblock_t &);
    xblock_t & operator = (const xblock_t &);

 protected:
    xblockcert_t*       get_blockcert() const {return (xblockcert_t*)get_cert();}

 public:
    virtual int32_t     full_block_serialize_to(base::xstream_t & stream);  // for block sync
    static  base::xvblock_t*    full_block_read_from(base::xstream_t & stream);  // for block sync
    void        set_parent_cert_and_path(base::xvqcert_t* parent_cert, const xmerkle_path_256_t & path);
    bool        calc_input_merkle_path(const std::string & leaf, xmerkle_path_256_t& hash_path) const;
    bool        calc_output_merkle_path(const std::string & leaf, xmerkle_path_256_t& hash_path) const;
    bool        check_block_hash();

 public:
    void            set_consensus_para(const xblock_consensus_para_t & para);
    virtual bool set_full_offstate(const xtable_mbt_ptr_t & offstate) {return false;}
    virtual xtable_mbt_ptr_t get_full_offstate() const {return nullptr;}
    virtual bool is_full_state_block() const {return false;}

 public:
    inline base::enum_xvblock_level get_block_level() const {return get_header()->get_block_level();}
    inline base::enum_xvblock_class get_block_class() const {return get_header()->get_block_class();}
    inline bool     is_unitblock() const {return get_block_level() == base::enum_xvblock_level_unit;}
    inline bool     is_tableblock() const {return get_block_level() == base::enum_xvblock_level_table;}
    inline bool     is_fullblock() const {return get_block_class() == base::enum_xvblock_class_full;}
    inline bool     is_lightunit() const {return get_block_level() == base::enum_xvblock_level_unit && get_block_class() == base::enum_xvblock_class_light;}
    inline bool     is_fullunit() const {return get_block_level() == base::enum_xvblock_level_unit && get_block_class() == base::enum_xvblock_class_full;}
    inline bool     is_emptyblock() const {return get_block_class() == base::enum_xvblock_class_nil;}
    inline bool     is_fulltable() const {return get_block_level() == base::enum_xvblock_level_table && get_block_class() == base::enum_xvblock_class_full;}
    inline bool     is_lighttable() const {return get_block_level() == base::enum_xvblock_level_table && get_block_class() == base::enum_xvblock_class_light;}
    inline bool     is_emptytable() const {return get_block_level() == base::enum_xvblock_level_table && get_block_class() == base::enum_xvblock_class_nil;}
    std::string     get_block_hash_hex_str() const;
    std::string     dump_header() const;
    std::string     dump_cert() const;
    virtual std::string     dump_body() const;
    std::string     dump_cert(base::xvqcert_t* qcert) const;

 public:
    virtual const std::map<std::string, std::string> & get_property_hash_map() const {return m_empty_map;}
    virtual std::string                 get_property_hash(const std::string & prop_name) const {return m_empty_string;}
    virtual const xnative_property_t &  get_native_property() const {return m_empty_native;}
    virtual const std::vector<xlightunit_tx_info_ptr_t> & get_txs() const { return m_empty_txs;}
    virtual xlightunit_tx_info_ptr_t    get_tx_info(const std::string & txhash) const;
    virtual xaccount_binlog_t*          get_property_log() const {return nullptr;}
    virtual int64_t                     get_pledge_balance_change_tgas() const {return 0;}
    virtual uint32_t                    get_txs_count() const {return 0;}
    virtual int64_t                     get_balance_change() const {return 0;}
    virtual int64_t                     get_burn_balance_change() const {return 0;}
    virtual const xaccount_mstate2*     get_fullunit_mstate() const {return nullptr;}
    virtual const std::map<std::string, std::string> * get_fullunit_propertys() const {return nullptr;}
    virtual const std::vector<xobject_ptr_t<xblock_t>> & get_tableblock_units(bool need_parent_cert) const {return m_empty_blocks;}
    virtual void                        dump_block_data(xJson::Value & json) const {return;}
    virtual uint16_t                    get_unconfirm_sendtx_num() const {return 0;}
    virtual bool                        is_prev_sendtx_confirmed() const {return false;}
    virtual std::map<std::string, xaccount_index_t> get_units_index() const {return {};}

 public:
    uint64_t    get_timerblock_height() const {return get_clock();}
    std::string get_block_owner()const {return get_account();}
    uint64_t    get_timestamp() {return get_cert()->get_gmtime();}

 private:
    static std::map<std::string, std::string>      m_empty_map;
    static xnative_property_t                      m_empty_native;
    static std::vector<xlightunit_tx_info_ptr_t>   m_empty_txs;
    static uint256_t                               m_empty_uint256;
    static std::string                             m_empty_string;
    static std::vector<xobject_ptr_t<xblock_t>>    m_empty_blocks;

};

using xblock_ptr_t = xobject_ptr_t<xblock_t>;
using xvblock_ptr_t = xobject_ptr_t<base::xvblock_t>;
using xvheader_ptr_t = xobject_ptr_t<base::xvheader_t>;
class xblock_consensus_para_t {
 public:
    xblock_consensus_para_t() = default;

    xblock_consensus_para_t(const std::string & _account, uint64_t _clock, uint64_t _viewid, uint32_t _viewtoken, uint64_t _proposal_height);
    xblock_consensus_para_t(const xvip2_t & validator, base::xvblock_t* prev_block);

 public:
    void    set_xip(const xvip2_t & _validator_xip, const xvip2_t & _auditor_xip);
    void    set_timer_block(base::xvblock_t* _timer_block);
    void    set_drand_block(base::xvblock_t* _drand_block);
    void    set_latest_blocks(const base::xblock_mptrs & latest_blocks);
    void    update_latest_cert_block(const xblock_ptr_t & proposal_prev_block) {m_latest_cert_block = proposal_prev_block;}
    void    set_common_consensus_para(uint64_t clock,
                                   const xvip2_t & validator,
                                   const xvip2_t & auditor,
                                   uint64_t viewid,
                                   uint32_t m_viewtoken,
                                   uint64_t drand_height);
    void    set_tableblock_consensus_para(uint64_t drand_height,
                                       const std::string & random_seed,
                                       uint64_t total_lock_tgas_token,
                                       const std::string & extra_data);
    void    set_justify_cert_hash(const std::string & justify_cert_hash) const {m_justify_cert_hash = justify_cert_hash;}
    void    set_parent_height(uint64_t height) const {m_parent_height = height;}

 public:
    const std::string &     get_extra_data() const {return m_extra_data;}
    const std::string &     get_random_seed() const {return m_random_seed;}
    uint64_t                get_clock() const {return m_clock;}
    uint64_t                get_viewid() const {return m_viewid;}
    uint32_t                get_viewtoken() const {return m_viewtoken;}
    uint64_t                get_timestamp() const {return (uint64_t)(m_clock * 10) + base::TOP_BEGIN_GMTIME;}
    uint64_t                get_drand_height() const {return m_drand_height;}
    const xblock_ptr_t &    get_timer_block() const {return m_timer_block;}
    const xblock_ptr_t &    get_drand_block() const {return m_drand_block;}
    const xblock_ptr_t &    get_latest_cert_block() const {return m_latest_cert_block;}
    const xblock_ptr_t &    get_latest_locked_block() const {return m_latest_locked_block;}
    const xblock_ptr_t &    get_latest_committed_block() const {return m_latest_committed_block;}
    const xvip2_t &         get_validator() const {return m_validator;}
    const xvip2_t &         get_auditor() const {return m_auditor;}
    uint64_t                get_total_lock_tgas_token() const {return m_total_lock_tgas_token;}
    const std::string &     get_justify_cert_hash() const {return m_justify_cert_hash;}
    uint64_t                get_proposal_height() const {return m_proposal_height;}
    uint64_t                get_parent_height() const {return m_parent_height;}
    const std::string &     dump() const {return m_dump_str;}

 private:
    std::string     m_account;
    uint64_t        m_clock{0};
    uint32_t        m_viewtoken{0};
    uint64_t        m_viewid{0};
    uint64_t        m_proposal_height{0};

    xvip2_t         m_validator;
    xvip2_t         m_auditor;
    uint64_t        m_drand_height{0};
    std::string     m_random_seed;
    uint64_t        m_total_lock_tgas_token{0};
    std::string     m_extra_data;
    std::string     m_dump_str;
    xblock_ptr_t    m_timer_block{nullptr};
    xblock_ptr_t    m_drand_block{nullptr};
    xblock_ptr_t    m_latest_cert_block{nullptr};
    xblock_ptr_t    m_latest_locked_block{nullptr};
    xblock_ptr_t    m_latest_committed_block{nullptr};
    mutable std::string     m_justify_cert_hash;  // may changed by unit
    mutable uint64_t        m_parent_height{0};  // may changed by unit
};

NS_END2