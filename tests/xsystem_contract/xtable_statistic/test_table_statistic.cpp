// #define private public
#include "xbase/xobject_ptr.h"
#include "xbasic/xasio_io_context_wrapper.h"
#include "xbasic/xtimer_driver.h"
#include "xblockstore/xblockstore_face.h"
#include "xchain_timer/xchain_timer.h"
#include "xchain_upgrade/xchain_upgrade_center.h"
#include "xdata/xblocktool.h"
#include "xdata/xgenesis_data.h"
#include "xloader/xconfig_onchain_loader.h"
#include "xstore/xstore_face.h"
#include "xvm/manager/xcontract_manager.h"
#include "xvm/xsystem_contracts/xregistration/xrec_registration_contract.h"
#include "xvm/xsystem_contracts/xreward/xtable_vote_contract.h"
#include "xvm/xsystem_contracts/xreward/xzec_reward_contract.h"
#include "xvm/xsystem_contracts/xreward/xzec_vote_contract.h"
#include "xvm/xsystem_contracts/xworkload/xzec_workload_contract_v2.h"
#include "xvm/xvm_service.h"
#include "xvm/xvm_trace.h"

#include "tests/xelection/xmocked_vnode_service.h"
#include "xelection/xvnode_house.h"
#include "xvm/xsystem_contracts/xslash/xtable_statistic_info_collection_contract.h"

#include <gtest/gtest.h>

#include <cinttypes>
#include <fstream>
#include <string>

using namespace top;
using namespace top::xvm;
using namespace top::xvm::xcontract;
using namespace top::xstake;
using namespace top::contract;
using namespace top::tests::election;

NS_BEG3(top, xvm, system_contracts)

class xtop_hash_t : public top::base::xhashplugin_t {
public:
    xtop_hash_t()
      : base::xhashplugin_t(-1)  //-1 = support every hash types
    {
    }

private:
    xtop_hash_t(const xtop_hash_t &) = delete;
    xtop_hash_t & operator=(const xtop_hash_t &) = delete;

public:
    virtual ~xtop_hash_t(){};
    virtual const std::string hash(const std::string & input, enum_xhash_type type) override {
        xassert(type == enum_xhash_type_sha2_256);
        auto hash = utl::xsha2_256_t::digest(input);
        return std::string(reinterpret_cast<char *>(hash.data()), hash.size());
    }
};

static std::vector<std::string> test_account = {
    "T00000LWtyNvjRk2Z2tcH4j6n27qPnM8agwf9ZJv",
    "T00000LcQEbtEyFirJepvE3ausXnDqPFuZexuCPH",
    "T00000LaB8gJxcm7MSspERF1LoBZjvo12tDub3F5",
    "T00000LciNfQZ5pkovANnuLt38ig9TCgS9sQX5JZ",
    "T00000Lcqis5qwuThxY8BLFXLohpj8qLJrbyyrNz",
    "T00000LaJNvRRvQ9gH2btt1YphmixhN7BFL2bkXk",
    "T00000LPWeXeAUzkHjEEE7grtChi6WcuH9dFtJM9",
    "T00000LfamVYcNXFGXqcuzPmDqs9BGD2KxEhT9yW",
    "T00000LWwfuNo89Q2otKayzi8GMzogkKSv15ndfk",
    "T00000LKg5aTDkXvqKMqtvMV44DdkgDx4BYf1eVb",
    "T00000LKMnarz6kUEBz8USCyLkBpZz4aYbrtScMF",
    "T00000LiEe5Y9ZgZeaytsgCPsVvJ3HpffK259u8e",
    "T00000LboDEsiBWN4k6ccMeDLT4WKtb1beG1FRGp",
    "T00000LdM1j893tTKhVuEQ2Ack8dw8WiLJtbb3S8",
    "T00000LS4ujRys1pHUHXetzvW7BauprXsKmfEMWR",
    "T00000Ld85cHh21G9Rh7VDPid2nvEQCraLuo9Fw3",
    "T00000LNdt2uYXSQGRiUbmTACQJhwXBQkCcvh9pT",
    "T00000LMVPdJmDdQjLwHpCMQt9JvUSYqx3KaMXcg",
    "T00000Lbjg8S3uapDKDJ44uY5cYBVAZ4PuqKVfok",
    "T00000LVekE2DMk7tmmSVeQv13ubeeydW3aX9L4B",
    "T00000LMhBuqXgaPD538iCKEicAKyrnq8G92zMFp",
    "T00000LNTg2RWpwrsjECSrgHL9JVEnSzjbdeibm5",
    "T00000LSnPmCKsxuJRLBuXjTqdruwMAxDqaPJXZ5",
    "T00000LgzdzP6vKFaFVVVScyZVasR4K5CNvPPjHc",
    "T00000LSNgbv5K5CLRD46nYSrDJpYaigRMMw68TK",
    "T00000LRM4gSxhxvf1EEDphBPU2GNvvAjgnP1hng",
    "T00000LPTtuTYgahbTzYWWr3j3ZdMpjdEeYkUKtZ",
    "T00000LMdMmkZAPhiayj4fLUS6hLEuL1HHVZVKCX",
    "T00000LS1Mzxp5kYvrqqWJqW97H2n61iBUhq53r8",
    "T00000LNrweafKbJsqFgPm8MsndTN9PYyRrtBddt",
    "T00000Lgqs4kZZWJj7hReow93s3U6KWSmHXEWAdn",
    "T00000LPfbz8Ubjokgwi8tojGECzrFx4tSb82mgg",
    "T00000LSZR1eWeLzZaxUK4x66k6HjGh7jF6CvTWW",
    "T00000LcaZVNssLVtifkzMrT9cpnV33981qSzmK2",
    "T00000Lg54D8r5pciuNEwadDcb4qvnhW3RbQRQYb",
    "T00000LabqtZeDRSHqrjFbxAGwzHoMht3YBwMg2K",
    "T00000LSw1pAXBCHY7YaboLRdQd7j1KNFkuxB1Wd",
    "T00000LT29byUEjPJU1DHrZUXz343zZxWiE1KcF1",
    "T00000LL4gpjntbiqsuiBiztornAdo2z3DdjprMo",
    "T00000LQPkFLhwcKfUih3CpK3dRkLj49f8H4rQzp",
    "T00000LbLA7bq7UjP2wpahrsxC7PiSVC4LLSvgcj",
    "T00000LgocfbwLdPP8TkqhjezpBHyJ4oUo5t1duW",
    "T00000LWCj5Xe7S1QKKY3hLtftUbtdeQ3UBPRtgc",
    "T00000LNSYfCVf3AS8MbqQHN9RktsHEaSA3bJVFY",
    "T00000LeA76AMbfgSCowhrrdRik6bp7kVrtrzDsz",
    "T00000LP1vTyFsJVNkxubCWhgFu2nnLPgiUum9Xg",
    "T00000LKfeHP6aPZpzmQZ1oNEeaH5DCB7vv9G4Tx",
    "T00000LgMMgdni9RreodRUHHzhSpKDMDxGvP2kYY",
    "T00000LY9Qeg3Vq4KAYqtMMiiooFvUKhKW8kMbwb",
    "T00000LPCEofeLyHJN8nuxdsBB7NM4nQQ4YmVjFo",
    "T00000LaJ9yTjs5iCQZY73HFZ993YYthtGWiUCuM",
    "T00000LWpyUZzSmkJuEqkGakY254FNopJgKc7PHH",
    "T00000LfaTA2tVabfhUiTDe9HATKTpKuU3z6iXZX",
    "T00000LMyWDzjXf8xhdX3iAD6dQhJzzgqg19HDYM",
    "T00000LWqTT9VUvabdXcpLvq41mBPeXAQBW59yvS",
    "T00000Ldvv8y8DkSqTpcynCsLD3BbWQt4WPABrRn",
    "T00000LeEkGstf8h277DFHoAhGksDTd2k6P23L7q",
    "T00000LPJDJ1aNtQBPdj7uJtxK4qhcAoXRbLrcxe",
    "T00000La73wV7oMHhSLP8SfLEpWmJmoVFdAwGxNS",
    "T00000LQD7916SK3GGdQrZxQvXh19CLjDnE1iaSS",
    "T00000LdH3Ju67X1eS3perXhC4FUb8gcmt78bwDe",
    "T00000LUiKQsJnaX2aZ45hV9iFPtD6SCpvKqVCJn",
    "T00000LPxP83Zg8EWccJwoQHsJAQ9NM1Z4HPj9gQ",
    "T00000LMDK3keteM4H5wjUFuNpt18mk2RX2LiXGj",
    "T00000LYZWVWLLMVybLUaX2yTJTPYpTsQwigMpby",
    "T00000LertfDwHqUvhpj3wqfBmQBhpN9PHYepLEt",
    "T00000LKz5rg6BFFLBWQ6Rd4Hbmm2srCaTSVxPHi",
    "T00000LMMMmUqoTAdtSdRC5pLheGKi68jyCZmD3o",
    "T00000LbYZyrKSsrnYxnd283NfzpUTDft5yJPyJm",
    "T00000LKgBspGmZoQbtvSvbkYtDcYek3vEL22UVE",
    "T00000LaoDoptDA42fZizVyKsy3meGxiEX1yJ5ua",
    "T00000LTJnqmnxtXmjZrkMcEsgzwqqGNXbbWr1WM",
    "T00000LS7i1vWoBVd5JdMuDosqtwAnxGgKgi2WfL",
    "T00000LQ1c3y84xpgMfSaKEjdkqE9xtAagoDyMAG",
    "T00000Layn9tQeaWerSYt6Uxv6Q62PmPtAC9WcEc",
    "T00000LVFzSZtjLXGJ6HkazCj1J7iAznUQtP2aan",
    "T00000LcRKFXFKS1xvGEwXMcNmMaTffLRTmC5Lse",
    "T00000LWyn1HQ5eTT3tj7ubu2dUopg26dB3RQy5C",
    "T00000LaS4gthtEq5PgadEdhVENyDdtqdfapJZ3W",
    "T00000LKvUEk3wCuTNevHTDCz62wAzore4H4aj6a",
    "T00000LLyz23Vsn5NYfsrsKwaGxWGKgLEgHvmaHE",
    "T00000LhRYd4WYNsGKvHb45ArgevKDmngJLDeJ2q",
    "T00000LiAATNw8UHGqZnmztTzPLn5QtEd3tmHzcQ",
    "T00000LghcFjJsUUSV5eyX7HLgaxv355JhFTva8w",
    "T00000LUCrTJBJ3gjntSCs4JgwyAeaPV3JxERon5",
    "T00000LVXdhjeVSxV47162xzLw3Qx1GXVZiQnY93",
    "T00000LS6nYP9YvMCS4Qg7gw5xmDTvybsfKcN4ZT",
    "T00000LZgakH7ypioAvjDJSdpRdJoARcWYPiqSox",
    "T00000Lf42svjFY3CG6UTRJzMAQ7vZQTBGP1d1iu",
    "T00000LLwH2rP7V7XTbncEoamYrp1XT7SwXjv6qz",
    "T00000LaWmQZTXJoSTBwJtGKe51dECCRGVQrF7VV",
    "T00000LY3WFsYcVzXMs6AWy1tR8P4f3Y1nL41G62",
    "T00000LXkCvZZTvSTnfUPvC7J1jCw6eyfpN2t2uZ",
    "T00000Li52qKH22psJPiiUo9t7xBeWFoqpkZSJGF",
    "T00000LZcP7bAYhycAqHWF5J7B4cvPVvYnHz1XcH",
    "T00000LToF15updfLRwhLJ5FNTVb337NERHeCtth",
    "T00000LbKtsQgzRSLjYHaFHU3LNuihct9dzoZWkZ",
    "T00000LbxPRGDHYXZ82moqXjbTBgf35Ud2YL7yfw",
    "T00000LMv8siMFsHbk5wXftyxbQGnogz8dDaFiXq",
    "T00000LUEvUGQsFTikTXjv4RcMC4D9Qfus3ZnWuX",
    "T00000LaRiY22fmKPQJCsANmhTVy11GYE49vHtQm",
    "T00000LRwZYo2tqKupopAeUER4i6yaXtCSwpzLk1",
    "T00000LTZSBfYvTKKnBncKsAbk2suYFNqeyf5wKn",
    "T00000LcmXaxTRgn3uv2nvWQ8BobZiNrqCjwhJ6a",
    "T00000Le7twQ3gS9FD1msXomn7gDSNUZXWZE2uAT",
    "T00000LV2jVxXWCuUpW2xCXSBBPDJFBLyVmbHGX4",
    "T00000LLRQr6pwaMMmnwEUYqqE1fAfUzRfuhkXpi",
    "T00000LXd6gYPuhfqaQYjTx8YnFKD5TzuWajSQyb",
    "T00000LRPR9LGPXCBJ1KWNH3BkqA8ioMujxnkoNu",
    "T00000Lh7FZgAr5S1oYpoULGkdizVgaZvgPm4U3g",
    "T00000LNECedpMmrnwce8Xs4moUrE5SFm1fPm7C6",
    "T00000LcdTZDcp7Gso7qbLqUKxEZ8nkGKgVG4GG4",
    "T00000LheyNBQcQ9JUY32cJrjJ1AoSJhuGEQ4E9F",
    "T00000LMgm37gPU1yYe2wjd7D1U3fyrybSfVzoUe",
    "T00000LRrm95jexiCoebG5HDCKnrS4opQqPVoRXc",
    "T00000LTBUcoMKy1K9gPTk72qitRuZgvy79XpwvW",
    "T00000LSAmUG42LhoWYezmZGnVUq7VQtKi59DwPd",
    "T00000LPw4TZR2tWqZqLcwFFWt2KBN5EBn2xNNSJ",
    "T00000LKL1nC2K8aZRd2kryoBXLacUf3xnxia2Ri",
    "T00000LPoRKzVRSYRKrZN6Au6PdFZb8aAp17msZ3",
    "T00000LNatiC59GMJaqLxkJNjhEztMK5h4nAhHbY",
    "T00000LftBHndG3t5ccLy7nKA6gD2AY1fGBeaQih",
    "T00000LXixxGzo2YFJgcUEtSK5RqtD9jViTgnAaa",
    "T00000LNc314VG5QNUBAxaz1PAvzRGv1Xq1djTUg",
    "T00000LaToP9yivui1cCLZDQ2KrTguDaeKr4YtkW",
    "T00000LbyfM9cr7FwTeUsSkCbdjZpzWHhqK3CqRD",
    "T00000LhERQMkhR9kPjX8Ziv1BPg5q7WkTR3mU5C",
    "T00000LTFAsTVpjGDychtQhzi2AVQ41bPgnAvCJn",
    "T00000LhKRmiZRwBYhmZK2dk3yM2WEA39eTGKrBG",
    "T00000LbsgFf53bxjt33fV7g9xjXaXQcNqQgT8dq",
    "T00000LakoJ4xKV6JZihUDGJzLfkzf9q14fEfCof",
    "T00000LdHahzLpS74apuEhrkUaLVzTXJyorBL3U3",
    "T00000LZB9g1JhZMU9d97yESTpky8pKnz4YpZmnv",
    "T00000LMK8hjgzVjncqqEUht9rR8Shccf6CPuRAG",
    "T00000Lhck4JR2dXTyCtYmR423Bcekczmiyba4KP",
    "T00000Lc9UAie2MCrCRtQuMAKicQ4KMWkfUXLfMy",
    "T00000LYaRNrZ5aomTj98oiD2eX82QPpp2gFiXHG",
    "T00000Lc1QiQ4SPU4bzRir9dGnNvramwSVq6SQHq",
    "T00000LYectmhJRGQSeqprj7BejDBnU2F4Hk5xvw",
    "T00000LhTGYq1VSgiGnEGMNwNjyPGyWXApGXCset",
    "T00000LcANCDmU1vcZiihzQDrv1pii8SCeh73pxx",
    "T00000LR6zqGpPRCxtHpkhTsVCLXCgR2741i8t5N",
    "T00000LYuCKYiPtgruEnQTTyqEoCNiCGMxsV8mv4",
    "T00000LZpQwp9DTReXCgSdHGSwgbNGmia8s7uXZk",
    "T00000LSan35jSJAcL1DkC2XTDuEKa8rHYzFDJLa",
    "T00000LV1DeX4rKEoz6iXjNRhQHwp26VFVzieBeZ",
    "T00000LTjcQBn2F59waD6ZFsABzv8hQkw3Zympje",
    "T00000Lh8DN8qeRpE7BxQUr6UVNFU393oo58UUJF",
    "T00000LPPU8wZtVe4TkRtEYPtQ2Yti5KAueJRuvz",
    "T00000LcFLcWJu4TSrDNVdct8fu4Uqc7vKnE6JGj",
    "T00000LM7BF5nroTKJausb2WifAEGmJ6mJ7bTyAK",
    "T00000LUZ8Qrdhwxz4RxRzBkhc6dtGJYzMmRiYLW",
    "T00000LNdPiAsTAKuWRhwDoh1A49uJgbqaP6jAWg",
    "T00000LPdGpAXU8Xh9UXbScoZuBQeTZbuSUA8exB",
    "T00000LeTVhHtasJxpMJHnS5Lm6DqwcM4mj4Nj8a",
    "T00000LdRqo8hdxMrsgZMN6EUdGUXKjhLatEVYvH",
    "T00000LKTFZiMTdSxeyZPtaG3oC7zycfXiZMeb3p",
    "T00000LhBVJsPkEWx5iJBdQM23vbYZVWfGGBzD3U",
    "T00000LPqrr6wh9NRjPJV4y7vgrz9cRSWgE8a649",
    "T00000LVKVukbx48srsHVsbSfQHnWLjFomm7sxj8",
    "T00000LbjjcPbRWwKNT86a1EJ97Tf1zZCGohbZY6",
    "T00000LLVGFHWxzDmvbVaWFFd4nDEWbVn3wWX4Be",
    "T00000LP891JJsHd2chQnqzseTMsK7FpYLcnn3pe",
    "T00000Lg5GfMYfCYsnGFJePaxB7fJLqoUVyrHmWY",
    "T00000LSe6yLen9H51e3zCt8bGe7UrdsJbb35oPL",
    "T00000LdEa8wg4FFMeTHYucUJqdkxFdCCijRBs2T",
    "T00000LZaoP4XzbagEVaoxo1kYAHTYn57gBZd3j9",
    "T00000LZVPANkvfWYS4XXEboVPMho5FY3rhmy8gA",
    "T00000LQLSvfUPeSDajs82bKwXRek645shHKHeKH",
    "T00000LPw7DCiwm3FM8zc9VzDbCjcGQQFQZRmy67",
    "T00000LKsMdgr1hQ4z2TCucucVGfMUiHHc66U5Mw",
    "T00000LPrJJ45GuT6tAp2hc3fdZFf4fLeB9YJ6qt",
    "T00000LarPP9YR5Y9qFPTs1p5mgvDpKdo3aQT2CG",
    "T00000LaQjgjBAoDULPSmPUnnN9oddPrEyZnhp8e",
    "T00000Lfwv8PYg14pckDkPSp1bBKpgbEjHBYfPVZ",
    "T00000Ld22GLXmBd4AgZjLhSYnuftn7ChX4N7tHr",
    "T00000LZkPY6SwvYPvawUPA2efws5RDmrYLziL4g",
    "T00000LQ5DUqVKmk1SwtZ7AC7E6RUXdYPzUTGDJQ",
    "T00000LT1k8Z7uLWbLoC4hsZRBDapptqN5m8npmq",
    "T00000LSa7nbnBZaChSE8476hAWxXJKk6Y7iUVRz",
    "T00000LcLCA51S5Guu52U4dSH2VKW7yYqtnwhban",
    "T00000LWMQiLGh7FZkUbsD1tAGeio7k64XQNQ9Un",
    "T00000LKK9Uvgn9MTstZHJuRSPnZ6y9wFFTJMiNk",
    "T00000LN5qXreptvLbvc3H5hogsdb5vySec2Jj8T",
    "T00000LRR4kSSmWiandHfZTh5hgDwy5Y1jJdmtpN",
    "T00000LZ4FAF7vt3fegW1RMjorM3rCG6B2JaYDxt",
    "T00000LKYFfbFBiTgcnPBJE1ssjHxJG8b56v9B38",
    "T00000LQauYrRb1o8pcLoUDUqK46gXf4DmwRCKcx",
    "T00000LczuCfEzm6Ajxj6DrGSDcaCpMTZDFoJCLj",
    "T00000LS4cQe9zPuWi2Vc2VzyvQHbS4ChUs8RP4Q",
    "T00000LUSkoL7WczTXrudnT2hRqT3uHibka6kXD2",
    "T00000LPuaNmo2uDaB6xYZdd3pMELYoENYUhwVT8",
    "T00000LUocZRS8867BbuR1cqeCg8KW7c73CQBvB4",
    "T00000LPr9S5jyBWgHtFxeMBZtD5D6HhgLZemW5e",
    "T00000LdhrRZoxeUAS3oJ4RTX69n6azkvCaEyjYp",
    "T00000LadDDekMyreNS9v2nT86S3kHYi7UcUTtcL",
    "T00000LhfvkvoRTRVEtWkhhXjVs5R9qRax38Npqt",
    "T00000LKjEgQi1c1CqRcB6FssitV4XUHSYdwSnAg",
    "T00000LY22gdHdaXgctBr9Nxhpatc3z9E41EzpMQ",
    "T00000LMVJDSZLEUstNUGk2GvVF79gNwV2b5U4s3",
    "T00000LLmJTrPWh6GGtVPvDp8mEVKYRPTzNMsPP1",
    "T00000LbSb2PMzhHWNgrbEyrW9cVehUHKshtgYEM",
    "T00000LcoT8WFVCzgkckcGN7oGs3QqWJPtCuf2Ei",
    "T00000LZvUvgWY48SboaHsM3jCxvKhZT6cLbDcXr",
    "T00000LWrDX8ReFWpGUj2nRirUvtv5v5c7Vp7Ld4",
    "T00000LLpk47uy7Zs12gZkjonjjDobi37Gswq65u",
    "T00000LdSewR155KqV6a5zvLQUHMXYt1L1q4jAdf",
    "T00000LRh1CwW1jgY1VqvoKLPFRqyWdq6CibPVHJ",
    "T00000LQjqiQvsGobVdYE3kgvzsiLZNdbSUCdyAn",
    "T00000LR7YUsDCDTsQMhsP2m4QA9FtNyNPNsQjhw",
    "T00000LRQ9ADQw7Na66F889N85gq2Btjy3x4inMS",
    "T00000LRUbuDUuLNM91uqeU5kQRqZfcMMNVp1JnV",
    "T00000LRSSQBMXBFDUQAVMwaEZMDuH2AYLavgw3m",
    "T00000LfGbpvEVrkcu2VwMLCrdJqFj29mFVZ8FdF",
    "T00000LULr22gYGqQBjHSTbend1QRXTqrWCgFd25",
    "T00000LQkMsrBn4wL9d3rNddT7MT6wX51EWuLDZz",
    "T00000LLJBxh2eiCaKQA9f4HFL1JnYEgPxUrDjj6",
    "T00000LhYB5xpdVdyAEmbmo6Wvz28Z1u21F7ZPad",
    "T00000LPhtbwGGKGGSmZuLpHoESo98KkvaEQPG1V",
    "T00000LWYsuu1FPBUmnTZwC7WGcxupJrssQd9kX5",
    "T00000LP4Zg4KDgxpo54sbz2Ny3SZpfTi3yokt2t",
    "T00000LNwdRsde2yRaFRXM1d1ARbKN1PHQvPUmxT",
    "T00000LccK1Dg5e5hLg9CRtk4r1wQ4fKxEiD86kr",
    "T00000LhaREk5svy8V52WyQPQrsb1SzjJQVZo7Ud",
    "T00000LNU4Qnu3T9G6Byr8SoPPc5AtUX5xEQ3Ngz",
    "T00000LfhjpB8KSAGhB1cRREEiKg7TkuxdTVNt6y",
    "T00000Lfj8KBECsVoVf9Yq2LmAG5D2Q6G9rPrgq5",
    "T00000LXyDMtc6ivUrVQzpkvDTvsbErug7evkMLh",
    "T00000LVXjiJWDQ283zMD3pNHAmtUWetPLL7cu2g",
    "T00000LNPGStrG9MyFSGgEbVN4svWuRf8aDjB2HS",
    "T00000LfYTsSTeqzRHjUZ3xufu8DW74JWmGuSzZd",
    "T00000LUhz2avHkbWTBJfg6UzwDzFpdcLj4N9VWs",
    "T00000LgzrZNdhNTi22EkEiYYKU773jkUX24DJFM",
    "T00000LciDiTfMX4A8F1Ur9MU4ve24UFodsLmogZ",
    "T00000LUuvBRPLsy9BhMSeoXwWEttXwd1dZkJiTh",
    "T00000LcFCWNV12iTTt374E8NbQpSBQ35ZpWiCgT",
    "T00000LM5AK1wcP2qbLgRrcS9u11QHogVvS8PD23",
    "T00000LiHqTun6ybWQFLtWr6cVsH8MRRoTHHHwzj",
    "T00000LTSyvCKwZNPXfL94Q3GXRDDtUjmUKokcWW",
    "T00000LPjWhTHeqN9BY7Uii3D7qRxxkcZHNzv6SK",
    "T00000LYhRBrVaCsX8nCQesdL2t6Z8stvJ71XW1E",
    "T00000LaC3knvhxNfZRjjts4DxCb9HmDBEzqxs68",
    "T00000LKj1GvBRBv1hUwzCHdsw2aWBJJgSVu7C1e",
    "T00000LY9hLTPsrW76Whoxbig17jGi5JJnxvcqVM",
    "T00000LboXBPF4r5pX8VxX27vBweTmn2bjjBxG76",
    "T00000LVhutQ9jQ2fLZNtEqZPDokeeJFyV7vNoBB",
    "T00000LbvKBLUKXR2VqQbFxEqJk6dMxrcNTFwUJ4",
    "T00000LVGgZfyp5AHAmCA8UokBAzgjdHxtvfBy3a",
    "T00000LQajcR3GbppshLR1b51HQ2TotPwayUJQpk",
    "T00000Li5G7E5MDXV8prwdLkCGXZEt49vRg89NK6",
    "T00000Ld1t9BmccH9KevdXmmvLwphEZBWrMGXfnp",
    "T00000LTuomNffL8zdVqGB4nAcYBswWdNL8zo1ow",
    "T00000LZNbzphxSoN6Cersipq4RYjaT1zcwa38vc",
    "T00000LgjkxFBncemtyoHsdSXXJy6v3qd1xW8nV8",
    "T00000LVbQNdwmYt6xgKRTc6YBGaBiY1xFeE8yNN",
    "T00000LPzuJnNweTbXQGVg5XMVvYZmrET6z3FcM2",
    "T00000LW1dz8Vmg1rZ5UtEjjBxSiQA8e3Q2kDcYf",
    "T00000LLDMKEjbu9MXAcNh9zKTjAzh4fGo84pTi6",
    "T00000Le9vBx2nk73MK7uWMeJjZzjrS2ENSvkWWr",
    "T00000LdSr73AtchpZkmkSbDPC4RXXNig3tSv1tN",
    "T00000LYc1HXr57Jv6Aw1TFQ8t3jPWBPifWNy7DJ",
    "T00000LSEDCg4MmyuzUQzxPQ3tZY57Jc3scdRvpr",
    "T00000LNVDs1YAAgYGvLm3muS4Zww73hiTYWNND2",
    "T00000LiLfvejG283hZeDbqavNzEAuPrWZoQK2ct",
    "T00000LRoACTh99NL6R8AwXjxtv1LyM6tdsS16wr",
    "T00000Lc7BMf6HU4ubwwvkopQMpo2TUCQSuyzrHk",
    "T00000LP3hGM68dQCSfU98LV3RKrrV9prx749kaT",
    "T00000LhXYGJ3bKbhULe9DEdzCXhRpMDJhCykVFU",
    "T00000LW2MEcGYYtoBrdH8K8iJAv6sfwfEL5EHYh",
    "T00000LUKz21BydZXDQdwvyoMzb9P8jZujSvMsqo",
    "T00000Ld7wH7iLzUXpPwPLSXgy3H49RcbzeTkAT5",
    "T00000LZE8Z9Xre7hivdTfVWNbcEe7qhGmK5sNpe",
    "T00000LKibYQG7qmQ1nzjfgz37ECRnAuQyEpnruG",
    "T00000Lfgi8LXrKjgMt9AGvKH1aFLae5hywcB16c",
    "T00000LUcKSJAW1oBhPCZk5BpjpydyNknsqXZJFS",
    "T00000LbEXUq53t6Q8XWtJpib3wk9nJPHycskpvD",
    "T00000Ld7G81CA19g2NXBg9UjrCkShGarggYjhFT",
    "T00000LTWYpCQBDsbtwie4JUrhhpe9XBHVauq3dB",
    "T00000LVfkR1xSWYGSuD34G9vd5s4MqrNXxoUmDt",
    "T00000LeEbtLLdU8tmVH7xNE3iNo9V5Bbye8vcSn",
    "T00000LUgE9UjSfh7YATNEz2SFAF1DQiiVK6xjdd",
    "T00000LfdbtKz8ohaf3ji5D1FB8L5y8pf18EzD4D",
    "T00000LgDZy4jhcrSKRYUFUMJ68mnJtLLvQAtf6A",
    "T00000LX2WsY3Fjg7X5CwuirHE5QHSrhtrK5kxom",
    "T00000LRuovE16X6Rscpp2fqTUJPjguKuouJf6en",
    "T00000LNtB4crCpPJMkVCe3P9kSSH7RqpxGnWFnW",
    "T00000LZ1hxqJmvwjGPW4FL72oFCqSJzDcoz78Ho",
    "T00000LbevdeKFmcDR2z4rHzpygG1HkwFRKFkQGz",
    "T00000LSGYe1WJDSzMuRz6sVzc8WXqqiiRVXnSd2",
    "T00000LPxHuupH8Dk8me1vFrwCbssxdVy5ssZjvg",
    "T00000LRRCZGTz2XJXoZroT7B62Eirqu44yDhpv4",
    "T00000LP5y6oEitgxi6Rpf4AND9AKSkWmjto5beX",
    "T00000LdyYUdnRtPvrusimKg6FxQdKwkcHMyPuve",
    "T00000Lb1uEqThUXmvgn8mwXNQ8RvRm8NAgXwRXX",
    "T00000LKvvt6cMnpT45J1Vess2QLaLc9d45cxLUc",
    "T00000Lch5nH1Pqpp8EUEUNxH6S1a2wiqe8Ef51h",
    "T00000LUjX1ev48hyTDwbd2VNNLTZxwcfN5P8bVV",
    "T00000LSyT8joQtEa5vJhavfCtTTAjixBzLRespH",
    "T00000LhH4GkpwJRRjikk76DwmhJPYuRd2ojB1qT",
    "T00000LeiEwBRW7j4ahVzLA4tNk4LdrkNmztynhm",
    "T00000LZHL6Rwoq1bJwaX1nRJtqEuQXayq5Dv2q2",
    "T00000LLKAqz5fQFhw7YTSZPE7bKK3EYHaB6MLZz",
    "T00000LdAx4GkZ9wyYTBq6VWhD7k418kU7HoXGDw",
    "T00000LWd2EajPditG4385othsKDXB829PUoHcKq",
    "T00000LPQ6G5132rAG4RcWv1PbJx2WfNEzdxDkZQ",
    "T00000LNEizD2DKkztyLwwN3XYCHoopYU5ijvMEP",
    "T00000Lgu7PHa1f97JVatcrvUrHrC6YELVB3a1Bd",
    "T00000Lhfnfoyty9j8rvc1RN4iyFbCS479HaMccR",
    "T00000LUemdZP5QQ9dBvmwwXdfbefEFm3kR8PJPc",
    "T00000LbgmAgR5LHpRmmJtAmYqfWHchJz3HBestn",
    "T00000LcbnV5bRVs2rQuEzLXzQUWXEv2x64DwyHf",
    "T00000La7wLUAH4dzd5eDLmHcJkhLSSvyVFCbQ99",
    "T00000LWuXr2xPhmWco5gWmiekSm5TcgbWDSYmaP",
    "T00000Lcoff1fjL13wuZbxMC4Yp3aqZ77DNMy76x",
    "T00000Lf5HHx31PzXVZFtxt6iZhh9EL3HwN2dfhn",
    "T00000LQ3Ym7Ky3B8JVdFCbAGbBtMbBFBwE6cTND",
    "T00000LZMiX6qaHdahy9DYmhFgwKDSWLE1xpRDuD",
    "T00000LZKzMeDRkShbLwutRNLGdgfh6WQr6kZrVW",
    "T00000LfRJ4JiF1iFR3BbhkdYcdmZcSxnXN9tmN7",
    "T00000LLGHzfZ6eQSmLXaEFvr83nmgHVf64Saccj",
    "T00000LNeNgSYK8VhCTtWzrJnrZ9g41amVVc7p99",
    "T00000LN343QPCDzi1AkQCMNxxCSRgT1BBx5o2pR",
    "T00000LbTvzG4srr8qszVj6gUJ2KHBtGy75nGpxE",
    "T00000Lg5uRWfnRtvRWktJUKyYvaqYF19c5mxDGy",
    "T00000LUTNPjGG95TdufpJg3iiK4eRrbvCPuxHa2",
    "T00000LQnf4PqtCwXDiSPJCLsL2uGCVtd31VYBBv",
    "T00000LRL8MeLPSbWgxXjXMCccVHEhD2gqMDHh4G",
    "T00000LPSvpHJH7oEsvSUYhgCDhjBAJAc6G2mEtv",
    "T00000LMQyjpcrTVeiHpV6yfBCm2ZQ4YKy4ePMqe",
    "T00000LWTTPLdidaPV7fXeqWCokmRswnxLEYzngX",
    "T00000LbSaoqBsPFGjudw8NBgUffZ5TmP6SNxYJ9",
    "T00000LUS9pnAmHg792ZSw4a5g4VDvhcHwNSskze",
    "T00000LMPAkPQAkcBfyHrDQP3WR1meEsTzSYhcht",
    "T00000LbLRKzj5TKo9zBj3jQnYs3NyZ8pcru2pZp",
    "T00000LMccX2msnYqx9V2Be6LChZCBUb9Juft5FK",
    "T00000LUJYBzKUefW4HGd1RGejrruetAVfb9MNwA",
    "T00000LbBhgHF5arRaBr4zpvdTSce6QB4SJPhzqh",
    "T00000LMYMTRfXix2tMfnr4DvUMBKmcAWgcM2wKo",
    "T00000LPqx32d1rVNqzLUbdUmHkji49XnzK9qQQh",
    "T00000LSDe8wpux6AcT5oxGUDFw7mLNadX7XMp7c",
    "T00000LZhoN6RH5D3eCbkyC2i6aR4jy7v8ZaDExT",
    "T00000LL5noJQ8TsPLEfTGPfxSt2VQvZozvhbZvB",
    "T00000LXeoKJdhKZWMmjT5KTBvtXjACwWJkvLATw",
    "T00000LNffrYGrx5XRZ187X28syY9Tairj4j12Ny",
    "T00000LhAnJNKjAhe5sNFKn5XqpLF6QbMSnBUHfZ",
    "T00000LRrnYKbbbjAx95UfYxuJveNtsMZ4rKL1Ve",
    "T00000LTfgKymo1gx2dJRJswbtFx9FdgRb8waBym",
    "T00000LVSWC1oPh9Sb4Un2FDNRwsMiNKH8dn3khd",
    "T00000LW1rgNSZ2WQuNikRUUrayKRsrsfZAKzMYV",
    "T00000Li25c7iZ3wQTfEoxez9F4CQYf6GBBvzXnT",
    "T00000Ld7Sj67vGJCL9T5gjweBa84SebndPyxLaW",
    "T00000LWE4nJYCwukJUCPm7ssB1oWpsZUJBwRVKQ",
    "T00000LZkKRYY29718SYm4yuQePUyRYTzrd2RYSN",
    "T00000LWyhJgkmY6gU9vxtBrpGK8exgbnvbMvjsT",
    "T00000LRkLahvRHSM9r9hWH3ZR1kTKFTcdLLmF4p",
    "T00000LZVa7AQiaxJDhLno1nefmPoMv1VkYhMHu5",
    "T00000LajvnRuGfm6i2Gm2JWSLGApjPbQxHPSewQ",
    "T00000LTFkGR4LmvgReayh2egU1FxdfQPeYTaouj",
    "T00000LeoBVBTHYrR9NSq58dvDCVMqkt5zcHhf61",
    "T00000Lf3vN7qx2vivCNEGtZSDcYbkXv3w6HDUhC",
    "T00000LepdjbnNve9pDepQMqKTXRz5dpZeLLiCDL",
    "T00000LLuhEfitSJBRyhD4qnErxUNbY34UNCpkKR",
    "T00000LNsm9dYT5yK3Vj5vKWeJi2KFne49Fn8o1u",
    "T00000LPV8mrxoZdYiAhbmPf41jQRZTSYDHRcG8H",
    "T00000LYoAbcfh76T17F5z6FmESfeKa5go6xNFdC",
    "T00000LbSf44RhtWiFG22GWnq9a69ZANcijPMusc",
    "T00000LWnzhDSvED8LtGp5VmJwy9myy3EAkrSL5N",
    "T00000Laag2dtCwVGJAW4JzQYUP4gDRNyxkahq4U",
    "T00000LSUuhvv2VoPwMQrQDF7jGa9zDcQcHtijMZ",
    "T00000LYDXVdnX29mXSgrRzZ9rEsC8JPszmCtiTS",
    "T00000LMajJmfKbp3ap7ofbJn3PtRHNeWzfLrro7",
    "T00000LdaukWYv4cwB9XPfUvcBWXaTzm8kb6wRG5",
    "T00000LSa5gemTG1xvPpshAzdQNB4g2fLhnVN54G",
    "T00000LKw2y4k19PctdphdfmkaCcEPtjdZcJWEhC",
    "T00000LM74G1r5noNks9LBPkzo4hEMxqQZi5jyoo",
    "T00000LLnAu2mBCBzqrErfURqJ4AswPJYbsdpdgw",
    "T00000LMyKLkaqJ1yrw91gMnezzTjtgvy99i4yPM",
    "T00000LfXJSrS1uVywpZ5Mb8KUvBYgJ2qUA4uyrb",
    "T00000LPEKheuGh3PWRWZZEJDd2rCV9ZJJfP5Pzb",
    "T00000Lg1CQUMwsokwab7aPz2Zz7nNEBFkXEgoWp",
    "T00000LQXDuH5KRNBBAeJpCjLomthgGoVgwH8s5Q",
    "T00000LezKTC2Pe7az4zCTWksLa78GQyNiB2ARBX",
    "T00000LhhwThxRV6uraGZZExx9o1m1eb86tGbdqt",
    "T00000LSY9D9XAS9PCMRmkxovxM4ngJeqGGKf4cD",
    "T00000LRk1G5h7nTA1Aa6wrBkK9QjHvoeW9gnVMh",
    "T00000LLjgqxgsCx4MPA6ds6T2LvCm2SxC1VrDM8",
    "T00000Lda8Rbb1eGhb8gDekdMqLduiixfH2Henfq",
    "T00000LL8H1s99gdixxxikHTv4dTfTj4HS3UUy4D",
    "T00000LSHKeKC1K4NPr9whtUtwPX5PLJMy8x9DvK",
    "T00000LL97meupgtL3GtrNjoN1tPPRjp4vDfVT6q",
    "T00000LVuQHJe9dmcsm1yxHiokD8UtyyqPmxY6w8",
    "T00000LQgXRPZ7KeMnmc1Nwyt3CPv2VLvK2QrmHb",
    "T00000LUttp9sU8jN2kR19BgpqNcFMrLeKE99Bjn",
    "T00000LUjv1DxfziQNtuJLRv76Fs5J9hryHkViEP",
    "T00000LfGXq5QAv2asxUjoKzxTx8vW78pJWZauoC",
    "T00000LcUD1MZ9ADeLECEETwCZKYkHNt7ny4VZs9",
    "T00000LbxcH1p6fxp6wy1QfUpbwaXrNh8CQkS9K2",
    "T00000LKX21gKJXjaphd7HHBpSs4FfB8aQ65bdFe",
    "T00000LNL9R9FzftBZuteCPDBSxn5D5wX3JV75Vj",
    "T00000LUDYTuvotBhv9yfA7uc1SvvA4HdxCo99qX",
    "T00000LNNUFsJoRYovKS4h8rauGAqfsD1kfbQjL7",
    "T00000LagjfBjeNXSw8E4EgKM8qq6WQPJ7Um1Pov",
    "T00000LfYtN29ehDKwkw5ENN2JH1N4Ym4qoL7sde",
    "T00000Lf78Kwh4uv44CjWBqvw6kfPs96bYKjjJsf",
    "T00000LNL72ChvsBnEUgXgn38NfpMQpQJtZKYrgF",
    "T00000LLttWobcLGSv5uBDyabq4Pbeqokkzk7UfC",
    "T00000LaUfSo6iSVxS7Gf26j8F8DEECgMbZY4MfJ",
    "T00000LNp9HfLgApKXrFegxHPRn2PwsWgBkPZeTF",
    "T00000Lbc68LzssYB22DYNLRRbcwRy5Nu672AanR",
    "T00000LUMK8YwY3Vp57ZAug1WeHHp3KSjDQjJWje",
    "T00000LR67o31RT6iDsodarHtqMQHrcAe5bqDAK7",
    "T00000LUMsSauiY2FEdC59Ve1GvaQ7mwvsUmZifk",
    "T00000LLBiUddneK4KirXwgVpiGivtAgsRVSVcju",
    "T00000LfR4o45CDURaD8MsqrgKrHp4JhYaFgYgPV",
    "T00000LYdArHYMnHw6QqVKrEYog7dXg3R9fHbdSW",
    "T00000LfArx1MJmy441XdSVrEECwRPyD4qBHSTfe",
    "T00000LeriiwApZtFV4TnPKMMgDAYBM3rJ82V6Nj",
    "T00000LfysNajn3Gt2BTPTkVnKsHu1ixYrJiNYhs",
    "T00000LfDkpWqhNm8THDbWpVjvMQKxdnEU4s4Ati",
    "T00000Ldzvr3DfhZe28kgoExdLCv3TBPi5Yz58FW",
    "T00000LeToc1FNQoKjxn3nnc29wEhuVfmYmouexE",
    "T00000LQEn5Sycte7QUJ9EFhST2JcPfWpmGX6RAz",
    "T00000LXdnTmu55HaJucmWc8dAwhzciRnXG2TTvY",
    "T00000LTzrofE5pWK71XBLxGAXNgR7tnn3nWbRkN",
    "T00000LUkvNYiPWGo8mbwuMEjMw1UJ9bmkvPSsn4",
    "T00000LbCEmqbrjprLkkU6WShpH2QtfRbGrG5MLa",
    "T00000LabZe8JGYDXQkAfnZbBfaXCUpRPHFKLCJX",
    "T00000Lh9reQap1erAY3ZbCBmWssF77QActDFSgo",
    "T00000LXCZYVCrpJ9zvZpXj2onkWJgh1xsXNibsJ",
    "T00000LQtoii2mZ4pCwodAXD3CnQJ41PGbKWo8Ux",
    "T00000Li2B5BTQm62ovhRxpJgu79ad5BqoS7FBBj",
    "T00000LM7oSPMsvZpNbumNiy9kRtoTKv3BBVLukd",
    "T00000LMpBvoHgH42iVHJUrjYCAnraN6nb4vZDRj",
    "T00000LLUfz8TYaQgdshuhKfR7P9tk5b4L4fJQhG",
    "T00000LTrSLpj9x3FYosZRWyEArose6D83UXZT5f",
    "T00000Ld7AkkerXhQMKpLmP3en7iDqB8YDp2jWKa",
    "T00000LZFz9ZefGDDdY8YABxQVi27K3D31bq4v5c",
    "T00000LbXWAWnLKwTcKNBfkd1mYnALZAJr3Jz5ga",
    "T00000LZkdAq2QG97iTEW7wELLGdNT9fEh1ogg3M",
    "T00000LffeQVbChSGjSaroJzjMYVTqAEDbqQQZS8",
    "T00000LSqb8DKVwWXSYb1YnxLDK4523xRbjZhSWk",
    "T00000LgJ5PzrVGBsLtdp2QYLj4APRrUNo9Mi3gN",
    "T00000LTY6wCac1zbMP4Ej1WnCFCo69Qhi6Djr5r",
    "T00000LMyWKDNo8iFXp1XYGxe7gDX8zWiZvjWmZe",
    "T00000LZNThTNVxa28mXVKf9BveSQ7LrNiH25a1h",
    "T00000LPUkoL1p3FziikJYELrrNfmYTXZrC1msxV",
    "T00000LWdXwTNTaCeNFGGnmm7CxXGDAfmeDiJWVQ",
    "T00000LYPMaiFxT3PjpZoSL1uoXUsZDgDBNpBrAD",
    "T00000LZyzVH6VrwUUp6dP4dMaTxtzxLxcUx4uhB",
    "T00000LWJhedgeUPwmPAusgFeKJc6ycnEQfcLHvX",
    "T00000LKqCv4LgDwctDeg3RNtLudQeRqJ9rUVHYd",
    "T00000LeWWYDBKsNGDjsbnbo5YZMfLniWSdUBQRy",
    "T00000LQ6p2WcFbYYc6i5rHPJ79b7HKQByvBz5uT",
    "T00000LM8xazDhsAQmT3dKTQ3rTorAwsmBxXQude",
    "T00000Ld7ZNtCkz83kbn9qQpf2quyJxyHKjLyjzQ",
    "T00000LTJT9bhTvpxhqh6LnMUbPM58fyjcAfDJSz",
    "T00000LZDdF1v9aerUtxFGUtHLyoahGag53h5p1z",
    "T00000LebcRP8PUpgBkXYFW46frXkTmimRZkkUQH",
    "T00000LdsqiSP7GCpM9bVb2u7R7GMRhM1Cqv7P5f",
    "T00000LiMYtie9zfHJXdAMRvfbkN9aQ2bBjUGtKw",
    "T00000LhtNFXAcmBapVGK5LcFNcuLYuREFduFoUx",
    "T00000LUVvVGWhBfN5iNcJUtr5UUzaY34N7gt5xC",
    "T00000LUPLAr8dND9sSXcy7Jn2CrpaXS9jTL2uuo",
    "T00000LWvqmyHBqBqM8ko3wktDLxQLtL6v9CwPMP",
    "T00000LQEoYi6NeAWo1FLhwUy5miTVsXmQs2HsxL",
    "T00000LdR6S7Cgt1HmbVtAKDPXQ2TKcmcbeBLS3X",
    "T00000LY7RbvpQcMBP21srsgoe8gzWvkkdAxwFAJ",
    "T00000LUCoN21vXwK9xsUoajfiZgNr6WGYUzt5jU",
    "T00000LVTiMZtJ6t3B7sfU2f4b17EhDwkPM5weC3",
    "T00000LexM16Fm2nuHdXaLCQqef22qd12x4fhvPc",
    "T00000Lb3oeToLSRyH78D158HgiaQcRyfCt7e6cn",
    "T00000LTazfJWzei3xyprerwY5cG3HZ7tjcoj6Yk",
    "T00000LiUjCfyjw3Ga6eHbUdyrUiuqGykgjEjgTg",
    "T00000LavRKRdSVwPwUm1yjKdQJ22JCTu2y6w7dY",
    "T00000LgwUDqxY21xm8jCjeJVuZftNJKojPeHtfu",
    "T00000LfnTWNmivVxCfFYD9pSpAZ1oyDma14h5PB",
    "T00000LaZ8LYUYW15MevnNZ6NtUF5b1ZdwA6UhGy",
    "T00000LUwyES7ucLX7vvGMokEKByPhSNotqmeWGb",
    "T00000LMFg21Gwom5VU45rRiyBwN6f88YBdzgE3P",
    "T00000LUT5EKyf1u6UfXci1b2cidDvYFX7gyfZA3",
    "T00000LV4KCkMqnYpqDzLXCAwp1SYFs7F48LriZY",
    "T00000LLFumjNtR4LBxMkgNpgqgyaGEuGD8RBXzb",
    "T00000LNxi7cv54WGacUXrbLU1pdLq5CDXt7CtR2",
    "T00000LRQsKTtPg2d9bnM6YAxokCzS29ZecUwFrv",
    "T00000LPiWgTm76oTA2daK1BTnse14S4CxRXehRW",
    "T00000Lbhi2QijP8jAj7dy4ePiMY1JFusJii5y8Z",
    "T00000LVPBxJtgJekV5vkEB5dn87LnkdXm6biUg8",
    "T00000LKHK9wDEtuo7t1Jau7mM93icFxPdk9WLkZ",
    "T00000LUExMWQcMiRCiBJXbTTnTGNbs2vXZsAzdB",
    "T00000LgBcQMc82gFLbxoaWynXxN6mNunWsDExZX",
    "T00000LLdE8bccNb85emvwLn14a3bg13YYueC43f",
    "T00000LdKwmuPpHgYMNmhUGsxxWym8sp4JjsEXeY",
    "T00000LZwLLS6GSC42EkPi8AxmaZwhDFEf74gkge",
    "T00000LcPMPKh6jdzhkwAKyhPJoBAmRLLNHQrPBU",
    "T00000LMCtTg4CXxr1KQgpqn5ZvUFnBBns9DgR2G",
    "T00000LTzk1ZyxyyL9ZbrT9RWPy9dArjWGoVaac4",
    "T00000LLhUorvESbYz8LRn2VKmyjrinQxt5hh6dp",
    "T00000LUdx1n1mXQCqGdBLQQeSDsa5TyTiSyoN9n",
    "T00000LWyopkBPiG4wdDkZrcHGTmPCLyWL1RVeJ9",
    "T00000LNhmGyexbZFigxLpXMMJDHmDAgRQEALBQG",
    "T00000LgPMnwtep6UziFpVUyizAkLMzBpWGXWTHQ",
    "T00000LPN7mS8gffBv9caw82ipDRv9nFwdprEvqn",
    "T00000LgrucSMMobNuTvCtMYh4YH2xccoKScuD7X",
    "T00000LaPpDYh1U6xJusKmDBNik4p339tePbRtvW",
    "T00000LLRPikBYKACxM8JcWnCbjEt7zegMGTMrdh",
    "T00000LWNKiHoQumGCsDoqoGztLqzerHAkPh6PEF",
    "T00000LP3NxQEbHJAWw2iWrjQTBiDhQedErergf6",
    "T00000LdxNX3heNcLRBHCn1VkQa1kGc1DML836M1",
    "T00000Lhdg79Vkrh7uzBuRuERaZmWnhBe9pvHpyD",
    "T00000LLrMJ978Zz6AV5wDZvR31uLwoVbSP9rEbw",
    "T00000LXo5y2AFLu19HV4jvwyQWajDZEmLMNi7Qs",
    "T00000LZMHuHoF3m43Svho8WqJAe9j7j4YmMwYaw",
};


class xtop_test_table_contract : public testing::Test {
public:
    xtop_test_table_contract() : m_table_contract(common::xnetwork_id_t{255}), node_serv{common::xaccount_address_t{"mocked_nodesvr"}, "null"} {
        auto hash_plugin = new xtop_hash_t();
        // top::config::config_register.get_instance().set(config::xmin_free_gas_asset_onchain_goverance_parameter_t::name, std::to_string(ASSET_TOP(100)));
        // top::config::config_register.get_instance().set(config::xfree_gas_onchain_goverance_parameter_t::name, std::to_string(25000));
        // top::config::config_register.get_instance().set(config::xmax_validator_stake_onchain_goverance_parameter_t::name, std::to_string(5000));
        top::config::config_register.get_instance().set(config::xchain_name_configuration_t::name, std::string{top::config::chain_name_testnet});
        top::config::config_register.get_instance().set(config::xroot_hash_configuration_t::name, std::string{});
        // data::xrootblock_para_t para;
        // data::xrootblock_t::init(para);
        // auto node_id = common::xnode_id_t{"T00000LgGPqEpiK6XLCKRj9gVPN8Ej1aMbyAb3Hu"};
        // auto sign_key = "ONhWC2LJtgi9vLUyoa48MF3tiXxqWf7jmT9KtOg/Lwo=";
        // m_bus = top::make_object_ptr<mbus::xmessage_bus_t>(true, 1000);
        // m_store = top::store::xstore_factory::create_store_with_memdb();
        // base::xvchain_t::instance().set_xdbstore(m_store.get());
        // m_blockstore.attach(store::get_vblockstore());
        // m_nodesvr_ptr = make_object_ptr<election::xvnode_house_t>(node_id, sign_key, m_blockstore, make_observer(m_bus.get()));
        // contract::xcontract_manager_t::instance().init(make_observer(m_store), xobject_ptr_t<store::xsyncvstore_t>{});
        // contract::xcontract_manager_t::set_nodesrv_ptr(m_nodesvr_ptr);
        nodeservice_add_group(1, m_group_xip1, test_account, 256);
        nodeservice_add_group(1, m_group_xip2, test_account, 512);
    }
    void nodeservice_add_group(uint64_t elect_blk_height, common::xip2_t const & group_xip, std::vector<std::string> const & nodes, uint32_t num) {
        node_serv.add_group(group_xip.network_id(), group_xip.zone_id(), group_xip.cluster_id(), group_xip.group_id(), (uint16_t)group_xip.size(), elect_blk_height);
        xmocked_vnode_group_t * node_group = dynamic_cast<xmocked_vnode_group_t *>(node_serv.get_group(group_xip).get());
        node_group->reset_nodes();
        for (std::size_t i = 0; i < num; ++i) {
            node_group->add_node(common::xaccount_address_t{nodes[i]});
        }

        assert(node_group->get_nodes().size() == num);
    }

    void tableblock_statistics_handle(const xvip2_t leader_xip, const uint32_t txs_count, const uint32_t blk_count, std::vector<base::xvoter> const & voter_info, xstatistics_data_t & data){
        // height
        // uint64_t block_height = get_network_height_from_xip2(leader_xip);
        uint64_t block_height = 10;
        auto it_height = data.detail.find(block_height);
        if (it_height == data.detail.end()) {
            xelection_related_statistics_data_t election_related_data;
            std::pair<std::map<uint64_t, xelection_related_statistics_data_t>::iterator, bool> ret = data.detail.insert(std::make_pair(block_height, election_related_data));
            it_height = ret.first;
        }
        // gid
        uint8_t group_idx = uint8_t(get_group_id_from_xip2(leader_xip));
        common::xgroup_id_t group_id = common::xgroup_id_t{group_idx};
        common::xcluster_id_t cluster_id = common::xcluster_id_t{0};
        common::xzone_id_t zone_id = common::xzone_id_t{0};
        common::xnetwork_id_t network_id = common::xnetwork_id_t{0};
        common::xgroup_address_t group_addr(network_id, zone_id, cluster_id, group_id);
        auto it_group = it_height->second.group_statistics_data.find(group_addr);
        if (it_group == it_height->second.group_statistics_data.end()) {
            xgroup_related_statistics_data_t group_related_data;
            auto ret = it_height->second.group_statistics_data.insert(std::make_pair(group_addr, group_related_data));
            it_group = ret.first;
        }
        // nid
        uint16_t slot_idx = uint16_t(get_node_id_from_xip2(leader_xip));
        // common::xslot_id_t slot_id = common::xslot_id_t{slot_idx};
        if(it_group->second.account_statistics_data.size() < size_t(slot_idx+1)){
            it_group->second.account_statistics_data.resize(slot_idx+1);
        }
        // workload
        it_group->second.account_statistics_data[slot_idx].block_data.block_count += blk_count;
        it_group->second.account_statistics_data[slot_idx].block_data.transaction_count += txs_count;
        // vote
        uint32_t vote_count = 0;
        for (auto const & item : voter_info) {
            if (item.is_voted) {
                it_group->second.account_statistics_data[slot_idx].vote_data.vote_count++;
                vote_count++;
            }
        }
        // printf("[tableblock_statistics] xip: [%" PRIu64 ", %" PRIu64 "], block_height: %" PRIu64 ", group_id: %u, slot_id: %u, "
        //     "work add block_count: %u, block_count: %u, add txs_count %u, transaction_count: %u, add vote count: %u, vote_count: %u\n",
        //         leader_xip.high_addr,
        //         leader_xip.low_addr,
        //         block_height,
        //         group_idx,
        //         slot_idx,
        //         1,
        //         it_group->second.account_statistics_data[slot_idx].block_data.block_count,
        //         txs_count, 
        //         it_group->second.account_statistics_data[slot_idx].block_data.transaction_count,
        //         vote_count,
        //         it_group->second.account_statistics_data[slot_idx].vote_data.vote_count);
    }

    std::map<common::xgroup_address_t, xgroup_workload_t> test_get_workload_from_data(xstatistics_data_t const & stat_data) {
        std::map<common::xgroup_address_t, xgroup_workload_t> group_workload;
        // auto node_service = contract::xcontract_manager_t::instance().get_node_service();
        auto workload_per_tableblock = 2;
        auto workload_per_tx = 1;
        for (auto const & static_item: stat_data.detail) {
            auto elect_statistic = static_item.second;
            for (auto const & group_item: elect_statistic.group_statistics_data) {
                common::xgroup_address_t const & group_addr = group_item.first;
                xgroup_related_statistics_data_t const & group_account_data = group_item.second;
                xvip2_t const &group_xvip2 = top::common::xip2_t{
                    group_addr.network_id(),
                    group_addr.zone_id(),
                    group_addr.cluster_id(),
                    group_addr.group_id(),
                    (uint16_t)group_account_data.account_statistics_data.size(),
                    static_item.first};
                // printf("[xzec_workload_contract_v2::accumulate_workload] group xvip2: %" PRIu64 ", %" PRIu64 "\n",
                //        group_xvip2.high_addr,
                //        group_xvip2.low_addr);
                for (size_t slotid = 0; slotid < group_account_data.account_statistics_data.size(); ++slotid) {
                    auto account_str = group_addr.to_string() + "_" +std::to_string(slotid);
                    uint32_t block_count = group_account_data.account_statistics_data[slotid].block_data.block_count;
                    uint32_t tx_count = group_account_data.account_statistics_data[slotid].block_data.transaction_count;
                    uint32_t workload = block_count * workload_per_tableblock + tx_count * workload_per_tx;
                    if (workload > 0) {
                        auto it2 = group_workload.find(group_addr);
                        if (it2 == group_workload.end()) {
                            xgroup_workload_t group_workload_info;
                            auto ret = group_workload.insert(std::make_pair(group_addr, group_workload_info));
                            XCONTRACT_ENSURE(ret.second, "insert workload failed");
                            it2 = ret.first;
                        }

                        it2->second.m_leader_count[account_str] += workload;
                    }
                }
            }
        }
        return group_workload;
    }

    int rand_num(int min, int max) {
        char tmp;
        std::string buffer;
        std::random_device rd;
        std::default_random_engine random(rd());
        int temp = max - min + 1;
        return (random() % temp) + min;
    }

    static void SetUpTestCase() {}
    static void TearDownTestCase() {}

    xstatistics_data_t data;
    xobject_ptr_t<mbus::xmessage_bus_face_t> m_bus;
    xobject_ptr_t<store::xstore_face_t> m_store;
    xobject_ptr_t<base::xvblockstore_t> m_blockstore;
    xobject_ptr_t<base::xvnodesrv_t> m_nodesvr_ptr;
    xcontract::xtable_statistic_info_collection_contract m_table_contract;

    common::xip2_t m_group_xip1{common::xnetwork_id_t{255}, common::xconsensus_zone_id, common::xdefault_cluster_id, common::xgroup_id_t{1}, 256, 1};
    common::xip2_t m_group_xip2{common::xnetwork_id_t{255}, common::xconsensus_zone_id, common::xdefault_cluster_id, common::xgroup_id_t{64}, 512, 1};
    common::xgroup_address_t m_group_addr1{m_group_xip1.xip()};
    common::xgroup_address_t m_group_addr2{m_group_xip2.xip()};
    std::string m_table{std::string{sys_contract_sharding_statistic_info_addr} + "@" + std::to_string(1)};
    xmocked_vnodesvr_t node_serv;
};
using xtest_table_contract_t = xtop_test_table_contract;

TEST_F(xtest_table_contract_t, test_node_service) {
    xstatistics_data_t data;
    xelection_related_statistics_data_t elect_data;
    auto vnode_group = node_serv.get_group(m_group_xip2);
    assert(vnode_group != nullptr);

    xgroup_related_statistics_data_t group_data;
    // set vote data
    for (std::size_t i = 0; i < vnode_group->get_size(); ++i) {
        xaccount_related_statistics_data_t account_data;
        account_data.block_data.block_count = i;
        account_data.block_data.transaction_count = i;
        group_data.account_statistics_data.push_back(account_data);
    }
    elect_data.group_statistics_data[m_group_addr2] = group_data;
    data.detail[1] = elect_data;
    for (std::size_t i = 0; i < 512; ++i) {
        EXPECT_EQ(data.detail[1].group_statistics_data[m_group_addr2].account_statistics_data[i].block_data.block_count, i);
        EXPECT_EQ(data.detail[1].group_statistics_data[m_group_addr2].account_statistics_data[i].block_data.transaction_count, i);
    }
}

TEST_F(xtest_table_contract_t, test_get_workload_from_data) {
    xvip2_t leader_xip_test[8];
    leader_xip_test[0] = {0x2080c04, 10}; // group id 3, node id 4, time 10
    leader_xip_test[1] = {0xa181c08, 20}; // group id 7, node id 8, time 10
    leader_xip_test[2] = {0xa181808, 20}; // group id 6, node id 8, time 20
    leader_xip_test[3] = {0x2080c00, 10}; // group id 3, node id 0, time 10
    leader_xip_test[4] = {0x2080c01, 10}; // group id 3, node id 1, time 10
    leader_xip_test[5] = {0x2080c02, 10}; // group id 3, node id 2, time 10
    leader_xip_test[6] = {0x2080c06, 10}; // group id 3, node id 6, time 10
    leader_xip_test[7] = {0x2080c00, 10}; // group id 3, node id 0, time 10

    uint32_t tx_count_test[8] = {100, 110, 120, 130, 140, 150, 160, 170};
    uint32_t blk_count_test[8] = {10, 11, 12, 13, 14, 15, 16, 17};
    
    std::vector<std::vector<base::xvoter>> voter_info_test;
    std::vector<base::xvoter> temp;
    for(int i = 0; i < 8; i++){
        base::xvoter vote;
        vote.is_voted = 0;
        temp.push_back(vote);
        vote.is_voted = 1;
        temp.push_back(vote);
        voter_info_test.push_back(temp);
    }
    for(size_t i = 0; i < 8; i++){
        tableblock_statistics_handle(leader_xip_test[i], tx_count_test[i], blk_count_test[i], voter_info_test[i], data);
    }
    auto const stat_data = data;
    auto group_workload = test_get_workload_from_data(stat_data);
    EXPECT_EQ(group_workload.size(), 3);
    common::xgroup_address_t group_addr7(common::xnetwork_id_t{0}, common::xzone_id_t{0}, common::xcluster_id_t{0}, common::xgroup_id_t{7});
    common::xgroup_address_t group_addr6(common::xnetwork_id_t{0}, common::xzone_id_t{0}, common::xcluster_id_t{0}, common::xgroup_id_t{6});
    common::xgroup_address_t group_addr3(common::xnetwork_id_t{0}, common::xzone_id_t{0}, common::xcluster_id_t{0}, common::xgroup_id_t{3});

    EXPECT_EQ(group_workload[group_addr3].m_leader_count.size(), 5);
    EXPECT_EQ(group_workload[group_addr3].m_leader_count[group_addr3.to_string() + "_" + std::to_string(0)], (130+170)+2*30);
    EXPECT_EQ(group_workload[group_addr3].m_leader_count[group_addr3.to_string() + "_" + std::to_string(1)], 140+2*14);
    EXPECT_EQ(group_workload[group_addr3].m_leader_count[group_addr3.to_string() + "_" + std::to_string(2)], 150+2*15);
    EXPECT_EQ(group_workload[group_addr3].m_leader_count[group_addr3.to_string() + "_" + std::to_string(3)], 0);
    EXPECT_EQ(group_workload[group_addr3].m_leader_count[group_addr3.to_string() + "_" + std::to_string(4)], 100+2*10);
    EXPECT_EQ(group_workload[group_addr3].m_leader_count[group_addr3.to_string() + "_" + std::to_string(5)], 0);
    EXPECT_EQ(group_workload[group_addr3].m_leader_count[group_addr3.to_string() + "_" + std::to_string(6)], 160+2*16);

    EXPECT_EQ(group_workload[group_addr7].m_leader_count.size(), 1);
    for(size_t slotid = 0; slotid < 9; ++slotid) {
        if(slotid == 8) {
            EXPECT_EQ(group_workload[group_addr7].m_leader_count[group_addr7.to_string() + "_" + std::to_string(slotid)], 110+2*11);
        } else {
            EXPECT_EQ(group_workload[group_addr7].m_leader_count[group_addr7.to_string() + "_" + std::to_string(slotid)], 0);
        }
    }
    EXPECT_EQ(group_workload[group_addr6].m_leader_count.size(), 1);  
    for(size_t slotid = 0; slotid < 9; ++slotid) {
        if(slotid == 8) {
            EXPECT_EQ(group_workload[group_addr6].m_leader_count[group_addr6.to_string() + "_" + std::to_string(slotid)], 120+2*12);
        } else {
            EXPECT_EQ(group_workload[group_addr6].m_leader_count[group_addr6.to_string() + "_" + std::to_string(slotid)], 0);
        }
    }  
}

#if 0

TEST_F(xtest_table_contract_t, test_get_workload_from_data_time) {
    std::vector<xvip2_t> leader_xip_test;

    for (auto i = 0; i < 256; i++) {
        common::xip2_t ip{common::xnetwork_id_t{255}, common::xconsensus_zone_id, common::xdefault_cluster_id, common::xgroup_id_t{1}, common::xslot_id_t{i}};
        leader_xip_test.emplace_back(ip);
    }

    for (auto i = 0; i < 512; i++) {
        common::xip2_t ip{common::xnetwork_id_t{255}, common::xconsensus_zone_id, common::xdefault_cluster_id, common::xgroup_id_t{64}, common::xslot_id_t{i}};
        leader_xip_test.emplace_back(ip);
    }

    std::vector<std::vector<base::xvoter>> voter_info_test;
    std::vector<base::xvoter> temp;
    for(int i = 0; i < 8; i++){
        base::xvoter vote;
        vote.is_voted = 0;
        temp.push_back(vote);
        vote.is_voted = 1;
        temp.push_back(vote);
        voter_info_test.push_back(temp);
    }
    for(size_t i = 0; i < 256; i++){
        tableblock_statistics_handle(leader_xip_test[i], 5, 5, voter_info_test[0], data);
    }
    auto const stat_data = data;
    uint64_t time_total = 0;
    uint64_t num_in_second = 0;
    uint64_t last_timestamp = 0;
    uint32_t second = 1;
    uint64_t num_total = 0;
    uint64_t max_rounds = 0;
    uint64_t min_rounds = 0;
    uint64_t rounds = 100;
    for (;;) {
        uint64_t time_s = xtime_utl::time_now_ms();
        auto group_workload = test_get_workload_from_data(stat_data);
        uint64_t time_e = xtime_utl::time_now_ms();
        time_total += time_e - time_s;
        num_in_second++;
        if (time_total / 1000 >= second) {
            std::cout << "second " << second << "(" << time_total - last_timestamp << "ms)" << ", total rounds: " << num_in_second << std::endl;
            num_total += num_in_second;
            if (num_in_second > max_rounds) {
                max_rounds = num_in_second;
            } else if (num_in_second < min_rounds) {
                min_rounds = num_in_second;
            }
            if (second >= rounds) {
                break;
            } else {
                num_in_second = 0;
                second++;
            }
        }
    }
    std::cout << "avr rounds per second: " << num_total/rounds << std::endl;
    std::cout << "max rounds per second: " << max_rounds << std::endl;
    std::cout << "min rounds per second: " << min_rounds << std::endl;
}
#endif
NS_END3
