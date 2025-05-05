#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include "./m256.h"
#include "keyUtils.h"
#include "K12AndKeyUtil.h"
#define _A 0
#define _B 1
#define _C 2
#define _D 3
#define _E 4
#define _F 5
#define _G 6
#define _H 7
#define _I 8
#define _J 9
#define _K 10
#define _L 11
#define _M 12
#define _N 13
#define _O 14
#define _P 15
#define _Q 16
#define _R 17
#define _S 18
#define _T 19
#define _U 20
#define _V 21
#define _W 22
#define _X 23
#define _Y 24
#define _Z 25
#define ID(_00, _01, _02, _03, _04, _05, _06, _07, _08, _09, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, _32, _33, _34, _35, _36, _37, _38, _39, _40, _41, _42, _43, _44, _45, _46, _47, _48, _49, _50, _51, _52, _53, _54, _55) _mm256_set_epi64x(((((((((((((((uint64_t)_55) * 26 + _54) * 26 + _53) * 26 + _52) * 26 + _51) * 26 + _50) * 26 + _49) * 26 + _48) * 26 + _47) * 26 + _46) * 26 + _45) * 26 + _44) * 26 + _43) * 26 + _42, ((((((((((((((uint64_t)_41) * 26 + _40) * 26 + _39) * 26 + _38) * 26 + _37) * 26 + _36) * 26 + _35) * 26 + _34) * 26 + _33) * 26 + _32) * 26 + _31) * 26 + _30) * 26 + _29) * 26 + _28, ((((((((((((((uint64_t)_27) * 26 + _26) * 26 + _25) * 26 + _24) * 26 + _23) * 26 + _22) * 26 + _21) * 26 + _20) * 26 + _19) * 26 + _18) * 26 + _17) * 26 + _16) * 26 + _15) * 26 + _14, ((((((((((((((uint64_t)_13) * 26 + _12) * 26 + _11) * 26 + _10) * 26 + _09) * 26 + _08) * 26 + _07) * 26 + _06) * 26 + _05) * 26 + _04) * 26 + _03) * 26 + _02) * 26 + _01) * 26 + _00)

typedef m256i id;
constexpr uint32_t QVAULT_MAX_NUMBER_OF_PROPOSAL = 65536;

template <typename T, uint64_t L>
struct array
{
private:
    static_assert(L && !(L & (L - 1)),
        "The capacity of the array must be 2^N."
        );

    T _values[L];

public:
    // Return number of elements in array
    static inline constexpr uint64_t capacity()
    {
        return L;
    }

    // Get element of array
    inline const T& get(uint64_t index) const
    {
        return _values[index & (L - 1)];
    }

    // Set element of array
    inline void set(uint64_t index, const T& value)
    {
        _values[index & (L - 1)] = value;
    }

    // Set content of array by copying memory (size must match)
    template <typename AT>
    inline void setMem(const AT& value)
    {
        static_assert(sizeof(_values) == sizeof(value), "This function can only be used if the overall size of both objects match.");
        // This if is resolved at compile time
        if (sizeof(_values) == 32)
        {
            // assignment uses __m256i intrinsic CPU functions which should be very fast
            *((id*)_values) = *((id*)&value);
        }
        else
        {
            // generic copying
            copyMemory(*this, value);
        }
    }

    // Set all elements to passed value
    inline void setAll(const T& value)
    {
        for (uint64_t i = 0; i < L; ++i)
            _values[i] = value;
    }

    // Set elements in range to passed value
    inline void setRange(uint64_t indexBegin, uint64_t indexEnd, const T& value)
    {
        for (uint64_t i = indexBegin; i < indexEnd; ++i)
            _values[i & (L - 1)] = value;
    }

    // Returns true if all elements of the range equal value (and range is valid).
    inline bool rangeEquals(uint64_t indexBegin, uint64_t indexEnd, const T& value) const
    {
        if (indexEnd > L || indexBegin > indexEnd)
            return false;
        for (uint64_t i = indexBegin; i < indexEnd; ++i)
        {
            if (!(_values[i] == value))
                return false;
        }
        return true;
    }
};

struct stakingInfo
{
    id stakerAddress;
    uint32_t amount;
};

stakingInfo staker[1048576];
stakingInfo votingPower[1048576];

struct GPInfo                   // General proposal
{
    id proposer;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

GPInfo GP[QVAULT_MAX_NUMBER_OF_PROPOSAL];

struct QCPInfo                   // Quorum change proposal
{
    id proposer;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint32_t newQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

QCPInfo QCP[QVAULT_MAX_NUMBER_OF_PROPOSAL];

struct IPOPInfo         // IPO participation
{
    id proposer;
    uint64_t totalWeight;
    uint64_t assignedFund;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t ipoContractIndex;
    uint32_t currentQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum. 3 is the insufficient invest funds.
};

IPOPInfo IPOP[QVAULT_MAX_NUMBER_OF_PROPOSAL];

struct QEarnPInfo       // Qearn participation proposal
{
    id proposer;
    uint64_t amountOfInvestPerEpoch;
    uint64_t assignedFundPerEpoch;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint8_t numberOfEpoch;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum. 3 is the insufficient funds.
};

QEarnPInfo QEarnP[QVAULT_MAX_NUMBER_OF_PROPOSAL];

struct FundPInfo            // Fundraising proposal
{
    id proposer;
    uint64_t pricePerOneQcap;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t amountOfQcap;
    uint32_t restSaleAmount;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

FundPInfo FundP[QVAULT_MAX_NUMBER_OF_PROPOSAL];

struct MKTPInfo                 //  Marketplace proposal
{
    id proposer;
    uint64_t amountOfQubic;
    uint64_t shareName;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t amountOfQcap;
    uint32_t currentQuorumPercent;
    uint32_t proposedEpoch;
    uint32_t shareIndex;
    uint32_t amountOfShare;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum. 3 is the insufficient funds. 4 is the insufficient Qcap.
};

MKTPInfo MKTP[QVAULT_MAX_NUMBER_OF_PROPOSAL];

struct AlloPInfo
{
    id proposer;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t currentQuorumPercent;
    uint32_t reinvested;
    uint32_t distributed;
    uint32_t team;
    uint32_t burnQcap;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

AlloPInfo AlloP[QVAULT_MAX_NUMBER_OF_PROPOSAL];

struct MSPInfo
{
    id proposer;
    uint32_t currentTotalVotingPower;
    uint32_t numberOfYes;
    uint32_t numberOfNo;
    uint32_t proposedEpoch;
    uint32_t muslimShareIndex;
    uint32_t currentQuorumPercent;
    uint8_t result;  // 0 is the passed proposal, 1 is the rejected proposal. 2 is the insufficient quorum.
};

MSPInfo MSP[1024];

id QCAP_ISSUER;
id reinvestingAddress;
id adminAddress;
id muslim[1048576];
uint64_t proposalCreateFund, reinvestingFund, totalNotMSRevenue, totalMuslimRevenue, fundForBurn, totalHistoryRevenue, rasiedFundByQcap, lastRoundPriceOfQcap, revenueByQearn;
uint64_t revenueInQcapPerEpoch[65536];
uint64_t revenueForOneQcapPerEpoch[65536];
uint64_t revenueForOneMuslimPerEpoch[65536];
uint64_t revenueForOneQvaultPerEpoch[65536];
uint64_t revenueForReinvestPerEpoch[65536];
uint64_t revenuePerShare[1024];
uint32_t muslimShares[64];
uint32_t burntQcapAmPerEpoch[65536];
uint32_t totalVotingPower, totalStakedQcapAmount, qcapSoldAmount;
uint32_t shareholderDividend, QCAPHolderPermille, reinvestingPermille, devPermille, burnPermille, qcapBurnPermille, totalQcapBurntAmount;
uint32_t numberOfStaker, numberOfVotingPower;
uint32_t numberOfGP;
uint32_t numberOfQCP;
uint32_t numberOfIPOP;
uint32_t numberOfQEarnP;
uint32_t numberOfFundP;
uint32_t numberOfMKTP;
uint32_t numberOfAlloP;
uint32_t numberOfMSP;
uint32_t transferRightsFee;
uint32_t numberOfMuslimShare;
int32_t numberOfMuslim;
uint32_t quorumPercent;

// Function to write new state to a file
void writeNewState(const std::string& filename) {
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile) {
        throw std::runtime_error("Failed to open the new state file.");
    }

    QCAP_ISSUER = ID(_Q, _C, _A, _P, _W, _M, _Y, _R, _S, _H, _L, _B, _J, _H, _S, _T, _T, _Z, _Q, _V, _C, _I, _B, _A, _R, _V, _O, _A, _S, _K, _D, _E, _N, _A, _S, _A, _K, _N, _O, _B, _R, _G, _P, _F, _W, _W, _K, _R, _C, _U, _V, _U, _A, _X, _Y, _E);
    adminAddress = ID(_H, _E, _C, _G, _U, _G, _H, _C, _J, _K, _Q, _O, _S, _D, _T, _M, _E, _H, _Q, _Y, _W, _D, _D, _T, _L, _F, _D, _A, _S, _Z, _K, _M, _G, _J, _L, _S, _R, _C, _S, _T, _H, _H, _A, _P, _P, _E, _D, _L, _G, _B, _L, _X, _J, _M, _N, _D);
    qcapSoldAmount = 1652235;
    transferRightsFee = 1000000;
    quorumPercent = 670;
    devPermille = 20;
    qcapBurnPermille = 0;
    burnPermille = 0;
    QCAPHolderPermille = 500;
    reinvestingPermille = 450;
    shareholderDividend = 30;

    outfile.write(reinterpret_cast<const char*>(&staker), sizeof(staker));
    outfile.write(reinterpret_cast<const char*>(&votingPower), sizeof(votingPower));
    outfile.write(reinterpret_cast<const char*>(&GP), sizeof(GP));
    outfile.write(reinterpret_cast<const char*>(&QCP), sizeof(QCP));
    outfile.write(reinterpret_cast<const char*>(&IPOP), sizeof(IPOP));
    outfile.write(reinterpret_cast<const char*>(&QEarnP), sizeof(QEarnP));
    outfile.write(reinterpret_cast<const char*>(&FundP), sizeof(FundP));
    outfile.write(reinterpret_cast<const char*>(&MKTP), sizeof(MKTP));
    outfile.write(reinterpret_cast<const char*>(&AlloP), sizeof(AlloP));
    outfile.write(reinterpret_cast<const char*>(&MSP), sizeof(MSP));
    outfile.write(reinterpret_cast<const char*>(&QCAP_ISSUER), sizeof(QCAP_ISSUER));
    outfile.write(reinterpret_cast<const char*>(&reinvestingAddress), sizeof(reinvestingAddress));
    outfile.write(reinterpret_cast<const char*>(&adminAddress), sizeof(adminAddress));
    outfile.write(reinterpret_cast<const char*>(&muslim), sizeof(muslim));
    outfile.write(reinterpret_cast<const char*>(&proposalCreateFund), sizeof(proposalCreateFund));
    outfile.write(reinterpret_cast<const char*>(&reinvestingFund), sizeof(reinvestingFund));
    outfile.write(reinterpret_cast<const char*>(&totalNotMSRevenue), sizeof(totalNotMSRevenue));
    outfile.write(reinterpret_cast<const char*>(&totalMuslimRevenue), sizeof(totalMuslimRevenue));
    outfile.write(reinterpret_cast<const char*>(&fundForBurn), sizeof(fundForBurn));
    outfile.write(reinterpret_cast<const char*>(&totalHistoryRevenue), sizeof(totalHistoryRevenue));
    outfile.write(reinterpret_cast<const char*>(&rasiedFundByQcap), sizeof(rasiedFundByQcap));
    outfile.write(reinterpret_cast<const char*>(&lastRoundPriceOfQcap), sizeof(lastRoundPriceOfQcap));
    outfile.write(reinterpret_cast<const char*>(&revenueByQearn), sizeof(revenueByQearn));
    outfile.write(reinterpret_cast<const char*>(&revenueInQcapPerEpoch), sizeof(revenueInQcapPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenueForOneQcapPerEpoch), sizeof(revenueForOneQcapPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenueForOneMuslimPerEpoch), sizeof(revenueForOneMuslimPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenueForOneQvaultPerEpoch), sizeof(revenueForOneQvaultPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenueForReinvestPerEpoch), sizeof(revenueForReinvestPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&revenuePerShare), sizeof(revenuePerShare));
    outfile.write(reinterpret_cast<const char*>(&muslimShares), sizeof(muslimShares));
    outfile.write(reinterpret_cast<const char*>(&burntQcapAmPerEpoch), sizeof(burntQcapAmPerEpoch));
    outfile.write(reinterpret_cast<const char*>(&totalVotingPower), sizeof(totalVotingPower));
    outfile.write(reinterpret_cast<const char*>(&totalStakedQcapAmount), sizeof(totalStakedQcapAmount));
    outfile.write(reinterpret_cast<const char*>(&qcapSoldAmount), sizeof(qcapSoldAmount));
    outfile.write(reinterpret_cast<const char*>(&shareholderDividend), sizeof(shareholderDividend));
    outfile.write(reinterpret_cast<const char*>(&QCAPHolderPermille), sizeof(QCAPHolderPermille));
    outfile.write(reinterpret_cast<const char*>(&reinvestingPermille), sizeof(reinvestingPermille));
    outfile.write(reinterpret_cast<const char*>(&devPermille), sizeof(devPermille));
    outfile.write(reinterpret_cast<const char*>(&burnPermille), sizeof(burnPermille));
    outfile.write(reinterpret_cast<const char*>(&totalQcapBurntAmount), sizeof(totalQcapBurntAmount));
    outfile.write(reinterpret_cast<const char*>(&numberOfStaker), sizeof(numberOfStaker));
    outfile.write(reinterpret_cast<const char*>(&numberOfVotingPower), sizeof(numberOfVotingPower));
    outfile.write(reinterpret_cast<const char*>(&numberOfGP), sizeof(numberOfGP));
    outfile.write(reinterpret_cast<const char*>(&numberOfQCP), sizeof(numberOfQCP));
    outfile.write(reinterpret_cast<const char*>(&numberOfIPOP), sizeof(numberOfIPOP));
    outfile.write(reinterpret_cast<const char*>(&numberOfQEarnP), sizeof(numberOfQEarnP));
    outfile.write(reinterpret_cast<const char*>(&numberOfFundP), sizeof(numberOfFundP));
    outfile.write(reinterpret_cast<const char*>(&numberOfMKTP), sizeof(numberOfMKTP));
    outfile.write(reinterpret_cast<const char*>(&numberOfAlloP), sizeof(numberOfAlloP));
    outfile.write(reinterpret_cast<const char*>(&numberOfMSP), sizeof(numberOfMSP));
    outfile.write(reinterpret_cast<const char*>(&transferRightsFee), sizeof(transferRightsFee));
    outfile.write(reinterpret_cast<const char*>(&numberOfMuslimShare), sizeof(numberOfMuslimShare));
    outfile.write(reinterpret_cast<const char*>(&numberOfMuslim), sizeof(numberOfMuslim));
    outfile.write(reinterpret_cast<const char*>(&quorumPercent), sizeof(quorumPercent));

    if (!outfile) {
        throw std::runtime_error("Failed to write id to the file.");
    }
    outfile.close();
}

int main() {
    try {
        // File paths
        const std::string newStateFile = "contract0010.161";

        // Write the new state to a file
        writeNewState(newStateFile);

        std::cout << "Migration completed successfully. New state saved to: " << newStateFile << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}