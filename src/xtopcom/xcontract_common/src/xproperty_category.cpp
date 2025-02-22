// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xcontract_common/xproperties/xproperty_category.h"

#include <cassert>
#if !defined(XCXX14_OR_ABOVE)
#    include <type_traits>
#endif

NS_BEG3(top, contract_common, properties)

char category_character(xproperty_category_t const c) noexcept {
    switch (c) {
    case xproperty_category_t::system:
        return '$';

    case xproperty_category_t::user:
        return '#';

    default:
        assert(false);
        return '\0';
    }
}

NS_END3

#if !defined(XCXX14_OR_ABOVE)
NS_BEG1(std)

size_t hash<top::contract_common::properties::xproperty_category_t>::operator()(top::contract_common::properties::xproperty_category_t const property_category) const noexcept {
    return static_cast<size_t>(static_cast<std::underlying_type<top::contract_common::properties::xproperty_category_t>::type>(property_category));
}

NS_END1
#endif
