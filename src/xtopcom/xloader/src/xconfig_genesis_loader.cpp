// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string>
#include <fstream>

#include "json/json.h"
#include "json/value.h"
#include "xdata/xdata_common.h"
#include "xloader/xconfig_genesis_loader.h"
#include "xloader/src/xgenesis_info.h"

NS_BEG2(top, loader)

xconfig_genesis_loader_t::xconfig_genesis_loader_t(const std::string& config_file) {
    if (!config_file.empty()) {
        m_genesis_info = get_file_content(config_file);
        if (!m_genesis_info.empty()) {
            m_genesis_info = extract_genesis_content(m_genesis_info);
        }
    }
    if (m_genesis_info.empty()) {
        m_genesis_info = get_genesis_info();
        xinfo("xconfig_genesis_loader_t::xconfig_genesis_loader_t use default genesis config.");
    } else {
        xinfo("xconfig_genesis_loader_t::xconfig_genesis_loader_t use user genesis config.");
    }
}

void xconfig_genesis_loader_t::start() {

}

void xconfig_genesis_loader_t::stop() {

}

bool xconfig_genesis_loader_t::save_conf(const std::map<std::string, std::string>& map) {
    return true;
}

bool xconfig_genesis_loader_t::fetch_all(std::map<std::string, std::string>& map) {
    // genesis config no need set to config center, so return nothing
    return true;
}

std::string xconfig_genesis_loader_t::get_file_content(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) {
        return "";
    }
    std::ostringstream tmp;
    tmp << in.rdbuf();
    in.close();
    return std::move(tmp.str());
}

std::string xconfig_genesis_loader_t::extract_genesis_content(const std::string& filecontent) {
    xJson::Reader reader;
    xJson::Value json_root;

    xdbg("xconfig_genesis_loader_t::extract_genesis_content begin to parse");

    bool ret = reader.parse(filecontent, json_root);
    if (!ret) {
        xerror("xconfig_genesis_loader_t::extract_genesis_content parse file fail.");
        return {};
    }
    const auto members = json_root.getMemberNames();
    for (const auto& member : members) {
        if (member == "genesis") {
            xdbg("xconfig_genesis_loader_t::extract_genesis_content find genesis");
            return filecontent;
        }
    }
    xdbg("xconfig_genesis_loader_t::extract_genesis_content not find genesis");
    return {};
}

bool xconfig_genesis_loader_t::extract_genesis_para_accounts(const xJson::Value & json_root, data::xrootblock_para_t & para) {
    const auto members = json_root.getMemberNames();
    for (const auto& member : members) {
        if (member == "accounts") {
            xJson::Value arrayObj = json_root[member];
            const auto account_members = arrayObj.getMemberNames();
            for (auto const& account_member: account_members) {
                if ( "tcc" == account_member ) {
                    xJson::Value tccAccounts = arrayObj[account_member];
                    const auto tcc_members = tccAccounts.getMemberNames();
                    for (auto const& tcc_member: tcc_members) {
                        xJson::Value tccAccount = tccAccounts[tcc_member];
                        std::string balance_str = tccAccount["balance"].asString();
                        uint64_t balance = base::xstring_utl::touint64(balance_str);
                        para.m_account_balances[tcc_member] = balance;
                        para.m_tcc_accounts.emplace_back(tcc_member);
                        xdbg("xconfig_genesis_loader_t::extract_genesis_para tcc account=%s balance=%ld", tcc_member.c_str(), balance);
                    }
                } else if ("genesis_funds_account" == account_member) {
                    xJson::Value fundsAccounts = arrayObj[account_member];
                    const auto funds_members = fundsAccounts.getMemberNames();
                    for (auto const& fund_member: funds_members) {
                        xJson::Value fundAccount = fundsAccounts[fund_member];
                        std::string balance_str = fundAccount["balance"].asString();
                        uint64_t balance = base::xstring_utl::touint64(balance_str);
                        para.m_account_balances[fund_member] = balance;
                        para.m_geneis_funds_accounts.emplace_back(fund_member);
                        xdbg("xconfig_genesis_loader_t::extract_genesis_para genesis account=%s balance=%ld", fund_member.c_str(), balance);
                    }
                }
            }

            return true;
        }
    }
    xerror("xconfig_genesis_loader_t::extract_genesis_para not find accounts");
    return false;
}

bool xconfig_genesis_loader_t::extract_genesis_para_seedNodes(const xJson::Value & json_root, data::xrootblock_para_t & para) {
    const auto members = json_root.getMemberNames();
    for (const auto& member : members) {
        if (member == "seedNodes") {
            std::vector<data::node_info_t> genesis_nodes;
            xJson::Value arrayObj = json_root[member];
            const auto sub_members = arrayObj.getMemberNames();
            for (const auto& sub_member : sub_members) {
                std::string publickey = arrayObj[sub_member].asString();
                data::node_info_t node;
                node.m_account = common::xaccount_address_t{ sub_member };
                // node.m_publickey = base::xstring_utl::base64_decode(publickey);
                node.m_publickey = xpublic_key_t{publickey};
                genesis_nodes.push_back(node);
                xdbg("xconfig_genesis_loader_t::extract_genesis_para seedNode account=%s publickey=%s", sub_member.c_str(), publickey.c_str());
            }
            para.m_genesis_nodes = genesis_nodes;
            return true;
        }
    }
    xerror("xconfig_genesis_loader_t::extract_genesis_para not find seedNodes");
    return false;
}

bool xconfig_genesis_loader_t::extract_genesis_para_genesis_timestamp(const xJson::Value & json_root, data::xrootblock_para_t & para) {
    const auto members = json_root.getMemberNames();
    for (const auto & member : members) {
        if (member == "timestamp") {
            para.m_genesis_time_stamp = json_root[member].asUInt64();
            return true;
        }
    }
    xerror("xconfig_genesis_loader_t::extract_genesis_para not find timestamp");
    return false;
}

bool xconfig_genesis_loader_t::extract_genesis_para(data::xrootblock_para_t & para) {
    try {
        xJson::Reader reader;
        xJson::Value json_root;

        xdbg("xconfig_genesis_loader_t::extract_genesis_para begin to parse.");

        bool ret = reader.parse(m_genesis_info, json_root);
        if (!ret) {
            xerror("xconfig_genesis_loader_t::extract_genesis_para parse file fail. %s", m_genesis_info.c_str());
            return false;
        }

        xJson::Value genesis_root;
        bool is_find_genesis = false;
        const auto members = json_root.getMemberNames();
        for (const auto& member : members) {
            if (member == "genesis") {
                genesis_root = json_root[member];
                is_find_genesis = true;
                break;
            }
        }
        if (!is_find_genesis) {
            genesis_root = json_root;
        }

        if (false == extract_genesis_para_accounts(genesis_root, para)) {
            return false;
        }
        if (false == extract_genesis_para_seedNodes(genesis_root, para)) {
            return false;
        }
        if (false == extract_genesis_para_genesis_timestamp(genesis_root, para)) {
            return false;
        }

        return true;
    }catch(xJson::LogicError) {
        xerror("xconfig_genesis_loader_t::extract_genesis_para json logic error");
        return false;
    }catch(const std::exception& e) {
        xerror("xconfig_genesis_loader_t::extract_genesis_para other error");
        return false;
    }
}

NS_END2
