#pragma once

#include <memory>

#include "xpbase/base/top_config.h"
#include "xpbase/base/kad_key/kadmlia_key.h"
#include "xelect_net/include/routing_manager.h"
#include "xelect_net/include/node_manager_base.h"

namespace top {

namespace kadmlia {
class RoutingTable;
}

namespace wrouter {
    class RootRoutingManager;
}

namespace elect {

// now chain genesis by root election committee
class ElectNode : public NodeManagerBase {
public:
    ElectNode(
            std::shared_ptr<transport::Transport>,
            const base::Config&);
    ~ElectNode();
    /**
     * @brief init ElectNode
     * 
     * @return int 
     */
    virtual int Init() override;
    /**
     * @brief join elect p2p network
     * 
     * @param xip xip is similar to network type
     * @return int 
     */
    virtual int Join(const base::XipParser& xip) override;
    /**
     * @brief quit elect p2p network
     * 
     * @param xip xip is similar to network type
     * @return int 
     */
    virtual int Quit(const base::XipParser& xip) override;
    /**
     * @brief drop nodes of network
     * 
     * @param xip xip is similar to network typ
     * @param drop_accounts list of accounts
     * @return int 
     */
    virtual int DropNodes(const base::XipParser& xip, const std::vector<std::string>& drop_accounts) override;

private:
    int AddCommitteeRole(base::KadmliaKeyPtr& kad_key);
    int AddBackupRole(base::KadmliaKeyPtr& kad_key);

    std::map<uint64_t, std::shared_ptr<RoutingManager>> ec_manager_map_;
    std::mutex ec_manager_map_mutex_;

    DISALLOW_COPY_AND_ASSIGN(ElectNode);
};

}  // namespace elect

}  // namespace top
