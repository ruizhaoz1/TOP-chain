#pragma once

#include "xcontract_common/xcontract_execution_result.h"
#include "xcontract_common/xcontract_execution_context.h"
#include "xsystem_contracts/xbasic_system_contract.h"
#include "xcontract_common/xproperties/xproperty_token.h"
#include "xcontract_runtime/xsystem/xsystem_contract_runtime_helper.h"

NS_BEG2(top, system_contracts)

class xtop_transfer_contract : public xbasic_system_contract_t {

private:
    contract_common::properties::xtoken_property_t m_balance{"balance", this};

public:
    xtop_transfer_contract();

    BEGIN_CONTRACT_API()
    DECLARE_API(xtop_transfer_contract::transfer);
    END_CONTRACT_API

private:
    void transfer(uint64_t const amount);
};
using xtransfer_contract_t = xtop_transfer_contract;

NS_END2
