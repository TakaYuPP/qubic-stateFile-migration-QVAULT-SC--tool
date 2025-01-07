#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include "./m256.h"
#include "keyUtils.h"
#include "K12AndKeyUtil.h"

constexpr uint32_t QVAULT_MAX_NUMBER_OF_BANNED_ADDRESSES = 16;
constexpr uint32_t QVAULT_MAX_EPOCHS = 4096;
typedef m256i id;

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

struct OldState {
    id QCAP_ISSUER;
    id authAddress1, authAddress2, authAddress3, newAuthAddress1, newAuthAddress2, newAuthAddress3;
    id reinvestingAddress, newReinvestingAddress1, newReinvestingAddress2, newReinvestingAddress3;
    id adminAddress, newAdminAddress1, newAdminAddress2, newAdminAddress3;
    id bannedAddress1, bannedAddress2, bannedAddress3;
    id unbannedAddress1, unbannedAddress2, unbannedAddress3;
    array<id, QVAULT_MAX_NUMBER_OF_BANNED_ADDRESSES> bannedAddress;
    uint32_t numberOfBannedAddress;
    uint32_t shareholderDividend, QCAPHolderPermille, reinvestingPermille, devPermille, burnPermille;
    uint32_t newQCAPHolderPermille1, newReinvestingPermille1, newDevPermille1;
    uint32_t newQCAPHolderPermille2, newReinvestingPermille2, newDevPermille2;
    uint32_t newQCAPHolderPermille3, newReinvestingPermille3, newDevPermille3;
};

struct StatsInfo
{
    uint64_t totalRevenue;
    uint64_t revenueOfQcapHolders;
    uint64_t revenueOfOneQcap;
    uint64_t revenueOfQvaultHolders;
    uint64_t revenueOfOneQvault;
    uint64_t revenueOfReinvesting;
    uint64_t revenueOfDevTeam;
};

id QCAP_ISSUER;
id authAddress1, authAddress2, authAddress3, newAuthAddress1, newAuthAddress2, newAuthAddress3;
id reinvestingAddress, newReinvestingAddress1, newReinvestingAddress2, newReinvestingAddress3;
id adminAddress, newAdminAddress1, newAdminAddress2, newAdminAddress3;
id bannedAddress1, bannedAddress2, bannedAddress3;
id unbannedAddress1, unbannedAddress2, unbannedAddress3;
array<id, QVAULT_MAX_NUMBER_OF_BANNED_ADDRESSES> bannedAddress;
uint32_t numberOfBannedAddress;
uint32_t shareholderDividend, QCAPHolderPermille, reinvestingPermille, devPermille, burnPermille;
uint32_t newQCAPHolderPermille1, newReinvestingPermille1, newDevPermille1;
uint32_t newQCAPHolderPermille2, newReinvestingPermille2, newDevPermille2;
uint32_t newQCAPHolderPermille3, newReinvestingPermille3, newDevPermille3;

array<StatsInfo, QVAULT_MAX_EPOCHS> _allEpochStats;

// Function to read old state from a file
OldState readOldState(const std::string& filename) {
    std::ifstream infile(filename, std::ios::binary);
    if (!infile) {
        throw std::runtime_error("Failed to open the old state file.");
    }

    OldState oldState;
    infile.read(reinterpret_cast<char*>(&oldState.QCAP_ISSUER), sizeof(oldState.QCAP_ISSUER));
    infile.read(reinterpret_cast<char*>(&oldState.authAddress1), sizeof(oldState.authAddress1));
    infile.read(reinterpret_cast<char*>(&oldState.authAddress2), sizeof(oldState.authAddress2));
    infile.read(reinterpret_cast<char*>(&oldState.authAddress3), sizeof(oldState.authAddress3));
    infile.read(reinterpret_cast<char*>(&oldState.newAuthAddress1), sizeof(oldState.newAuthAddress1));
    infile.read(reinterpret_cast<char*>(&oldState.newAuthAddress2), sizeof(oldState.newAuthAddress2));
    infile.read(reinterpret_cast<char*>(&oldState.newAuthAddress3), sizeof(oldState.newAuthAddress3));
    infile.read(reinterpret_cast<char*>(&oldState.reinvestingAddress), sizeof(oldState.reinvestingAddress));
    infile.read(reinterpret_cast<char*>(&oldState.newReinvestingAddress1), sizeof(oldState.newReinvestingAddress1));
    infile.read(reinterpret_cast<char*>(&oldState.newReinvestingAddress2), sizeof(oldState.newReinvestingAddress2));
    infile.read(reinterpret_cast<char*>(&oldState.newReinvestingAddress3), sizeof(oldState.newReinvestingAddress3));
    infile.read(reinterpret_cast<char*>(&oldState.adminAddress), sizeof(oldState.adminAddress));
    infile.read(reinterpret_cast<char*>(&oldState.newAdminAddress1), sizeof(oldState.newAdminAddress1));
    infile.read(reinterpret_cast<char*>(&oldState.newAdminAddress2), sizeof(oldState.newAdminAddress2));
    infile.read(reinterpret_cast<char*>(&oldState.newAdminAddress3), sizeof(oldState.newAdminAddress3));
    infile.read(reinterpret_cast<char*>(&oldState.bannedAddress1), sizeof(oldState.bannedAddress1));
    infile.read(reinterpret_cast<char*>(&oldState.bannedAddress2), sizeof(oldState.bannedAddress2));
    infile.read(reinterpret_cast<char*>(&oldState.bannedAddress3), sizeof(oldState.bannedAddress3));
    infile.read(reinterpret_cast<char*>(&oldState.unbannedAddress1), sizeof(oldState.unbannedAddress1));
    infile.read(reinterpret_cast<char*>(&oldState.unbannedAddress2), sizeof(oldState.unbannedAddress2));
    infile.read(reinterpret_cast<char*>(&oldState.unbannedAddress3), sizeof(oldState.unbannedAddress3));
    infile.read(reinterpret_cast<char*>(&oldState.bannedAddress), sizeof(oldState.bannedAddress));

    infile.read(reinterpret_cast<char*>(&oldState.numberOfBannedAddress), sizeof(oldState.numberOfBannedAddress));
    infile.read(reinterpret_cast<char*>(&oldState.shareholderDividend), sizeof(oldState.shareholderDividend));
    infile.read(reinterpret_cast<char*>(&oldState.QCAPHolderPermille), sizeof(oldState.QCAPHolderPermille));
    infile.read(reinterpret_cast<char*>(&oldState.reinvestingPermille), sizeof(oldState.reinvestingPermille));
    infile.read(reinterpret_cast<char*>(&oldState.devPermille), sizeof(oldState.devPermille));
    infile.read(reinterpret_cast<char*>(&oldState.burnPermille), sizeof(oldState.burnPermille));
    infile.read(reinterpret_cast<char*>(&oldState.newQCAPHolderPermille1), sizeof(oldState.newQCAPHolderPermille1));
    infile.read(reinterpret_cast<char*>(&oldState.newReinvestingPermille1), sizeof(oldState.newReinvestingPermille1));
    infile.read(reinterpret_cast<char*>(&oldState.newDevPermille1), sizeof(oldState.newDevPermille1));
    infile.read(reinterpret_cast<char*>(&oldState.newQCAPHolderPermille2), sizeof(oldState.newQCAPHolderPermille2));
    infile.read(reinterpret_cast<char*>(&oldState.newReinvestingPermille2), sizeof(oldState.newReinvestingPermille2));
    infile.read(reinterpret_cast<char*>(&oldState.newDevPermille2), sizeof(oldState.newDevPermille2));
    infile.read(reinterpret_cast<char*>(&oldState.newQCAPHolderPermille3), sizeof(oldState.newQCAPHolderPermille3));
    infile.read(reinterpret_cast<char*>(&oldState.newReinvestingPermille3), sizeof(oldState.newReinvestingPermille3));
    infile.read(reinterpret_cast<char*>(&oldState.newDevPermille3), sizeof(oldState.newDevPermille3));
    infile.close();

    return oldState;
}

// Function to write new state to a file
void writeNewState(const std::string& filename) {
    std::ofstream outfile(filename);
    if (!outfile) {
        throw std::runtime_error("Failed to open the new state file.");
    }

    outfile.write(reinterpret_cast<const char*>(&QCAP_ISSUER), sizeof(QCAP_ISSUER));
    outfile.write(reinterpret_cast<const char*>(&authAddress1), sizeof(authAddress1));
    outfile.write(reinterpret_cast<const char*>(&authAddress2), sizeof(authAddress2));
    outfile.write(reinterpret_cast<const char*>(&authAddress3), sizeof(authAddress3));
    outfile.write(reinterpret_cast<const char*>(&newAuthAddress1), sizeof(newAuthAddress1));
    outfile.write(reinterpret_cast<const char*>(&newAuthAddress2), sizeof(newAuthAddress2));
    outfile.write(reinterpret_cast<const char*>(&newAuthAddress3), sizeof(newAuthAddress3));
    outfile.write(reinterpret_cast<const char*>(&reinvestingAddress), sizeof(reinvestingAddress));
    outfile.write(reinterpret_cast<const char*>(&newReinvestingAddress1), sizeof(newReinvestingAddress1));
    outfile.write(reinterpret_cast<const char*>(&newReinvestingAddress2), sizeof(newReinvestingAddress2));
    outfile.write(reinterpret_cast<const char*>(&newReinvestingAddress3), sizeof(newReinvestingAddress3));
    outfile.write(reinterpret_cast<const char*>(&adminAddress), sizeof(adminAddress));
    outfile.write(reinterpret_cast<const char*>(&newAdminAddress1), sizeof(newAdminAddress1));
    outfile.write(reinterpret_cast<const char*>(&newAdminAddress2), sizeof(newAdminAddress2));
    outfile.write(reinterpret_cast<const char*>(&newAdminAddress3), sizeof(newAdminAddress3));
    outfile.write(reinterpret_cast<const char*>(&bannedAddress1), sizeof(bannedAddress1));
    outfile.write(reinterpret_cast<const char*>(&bannedAddress2), sizeof(bannedAddress2));
    outfile.write(reinterpret_cast<const char*>(&bannedAddress3), sizeof(bannedAddress3));
    outfile.write(reinterpret_cast<const char*>(&unbannedAddress1), sizeof(unbannedAddress1));
    outfile.write(reinterpret_cast<const char*>(&unbannedAddress2), sizeof(unbannedAddress2));
    outfile.write(reinterpret_cast<const char*>(&unbannedAddress3), sizeof(unbannedAddress3));
    for(uint32_t i = 0 ; i < QVAULT_MAX_NUMBER_OF_BANNED_ADDRESSES; i++)
    {
        outfile.write(reinterpret_cast<const char*>(&bannedAddress.get(i)), sizeof(bannedAddress.get(i)));
    }
    outfile.write(reinterpret_cast<const char*>(&numberOfBannedAddress), sizeof(numberOfBannedAddress));
    outfile.write(reinterpret_cast<const char*>(&shareholderDividend), sizeof(shareholderDividend));
    outfile.write(reinterpret_cast<const char*>(&QCAPHolderPermille), sizeof(QCAPHolderPermille));
    outfile.write(reinterpret_cast<const char*>(&reinvestingPermille), sizeof(reinvestingPermille));
    outfile.write(reinterpret_cast<const char*>(&devPermille), sizeof(devPermille));
    outfile.write(reinterpret_cast<const char*>(&burnPermille), sizeof(burnPermille));
    outfile.write(reinterpret_cast<const char*>(&newQCAPHolderPermille1), sizeof(newQCAPHolderPermille1));
    outfile.write(reinterpret_cast<const char*>(&newReinvestingPermille1), sizeof(newReinvestingPermille1));
    outfile.write(reinterpret_cast<const char*>(&newDevPermille1), sizeof(newDevPermille1));
    outfile.write(reinterpret_cast<const char*>(&newQCAPHolderPermille2), sizeof(newQCAPHolderPermille2));
    outfile.write(reinterpret_cast<const char*>(&newReinvestingPermille2), sizeof(newReinvestingPermille2));
    outfile.write(reinterpret_cast<const char*>(&newDevPermille2), sizeof(newDevPermille2));
    outfile.write(reinterpret_cast<const char*>(&newQCAPHolderPermille3), sizeof(newQCAPHolderPermille3));
    outfile.write(reinterpret_cast<const char*>(&newReinvestingPermille3), sizeof(newReinvestingPermille3));
    outfile.write(reinterpret_cast<const char*>(&newDevPermille3), sizeof(newDevPermille3));

    StatsInfo zeroValue;
    zeroValue.revenueOfDevTeam = 0;
    zeroValue.revenueOfOneQcap = 0;
    zeroValue.revenueOfOneQvault = 0;
    zeroValue.revenueOfQcapHolders = 0;
    zeroValue.revenueOfQvaultHolders = 0;
    zeroValue.revenueOfReinvesting = 0;
    zeroValue.totalRevenue = 0;

    for(uint32_t i = 0 ; i < QVAULT_MAX_EPOCHS; i++)
    {
        _allEpochStats.set(i, zeroValue);
        outfile.write(reinterpret_cast<const char*>(&_allEpochStats.get(i).revenueOfDevTeam), sizeof(_allEpochStats.get(i).revenueOfDevTeam));
        outfile.write(reinterpret_cast<const char*>(&_allEpochStats.get(i).revenueOfOneQcap), sizeof(_allEpochStats.get(i).revenueOfOneQcap));
        outfile.write(reinterpret_cast<const char*>(&_allEpochStats.get(i).revenueOfOneQvault), sizeof(_allEpochStats.get(i).revenueOfOneQvault));
        outfile.write(reinterpret_cast<const char*>(&_allEpochStats.get(i).revenueOfQcapHolders), sizeof(_allEpochStats.get(i).revenueOfQcapHolders));
        outfile.write(reinterpret_cast<const char*>(&_allEpochStats.get(i).revenueOfQvaultHolders), sizeof(_allEpochStats.get(i).revenueOfQvaultHolders));
        outfile.write(reinterpret_cast<const char*>(&_allEpochStats.get(i).revenueOfReinvesting), sizeof(_allEpochStats.get(i).revenueOfReinvesting));
        outfile.write(reinterpret_cast<const char*>(&_allEpochStats.get(i).totalRevenue), sizeof(_allEpochStats.get(i).totalRevenue));
    }
    if (!outfile) {
        throw std::runtime_error("Failed to write id to the file.");
    }
    outfile.close();
}

int main() {
    try {
        // File paths
        const std::string oldStateFile = "contract0010.142";
        const std::string newStateFile = "new_contract0010.142";

        // Read the old state
        OldState oldState = readOldState(oldStateFile);

        // Migrate to the new state

        QCAP_ISSUER = oldState.QCAP_ISSUER;
        authAddress1 = oldState.authAddress1;
        authAddress2 = oldState.authAddress2;
        authAddress3 = oldState.authAddress3;
        newAuthAddress1 = oldState.newAuthAddress1;
        newAuthAddress2 = oldState.newAuthAddress2;
        newAuthAddress3 = oldState.newAuthAddress3;
        reinvestingAddress = oldState.reinvestingAddress;
        newReinvestingAddress1 = oldState.newReinvestingAddress1;
        newReinvestingAddress2 = oldState.newReinvestingAddress2;
        newReinvestingAddress3 = oldState.newReinvestingAddress3;
        adminAddress = oldState.adminAddress;
        newAdminAddress1 = oldState.newAdminAddress1;
        newAdminAddress2 = oldState.newAdminAddress2;
        newAdminAddress3 = oldState.newAdminAddress3;
        bannedAddress1 = oldState.bannedAddress1;
        bannedAddress2 = oldState.bannedAddress2;
        bannedAddress3 = oldState.bannedAddress3;
        unbannedAddress1 = oldState.unbannedAddress1;
        unbannedAddress2 = oldState.unbannedAddress2;
        unbannedAddress3 = oldState.unbannedAddress3;
        for(uint32_t i = 0 ; i < QVAULT_MAX_NUMBER_OF_BANNED_ADDRESSES; i++)
        {
            bannedAddress.set(i, oldState.bannedAddress.get(i));
        }
        numberOfBannedAddress = oldState.numberOfBannedAddress;
        shareholderDividend = oldState.shareholderDividend;
        QCAPHolderPermille = oldState.QCAPHolderPermille;
        reinvestingPermille = oldState.reinvestingPermille;
        devPermille = oldState.devPermille;
        burnPermille = oldState.burnPermille;
        newQCAPHolderPermille1 = oldState.newQCAPHolderPermille1;
        newReinvestingPermille1 = oldState.newReinvestingPermille1;
        newDevPermille1 = oldState.newDevPermille1;
        newQCAPHolderPermille2 = oldState.newQCAPHolderPermille2;
        newReinvestingPermille2 = oldState.newReinvestingPermille2;
        newDevPermille2 = oldState.newDevPermille2;
        newQCAPHolderPermille3 = oldState.newQCAPHolderPermille3;
        newReinvestingPermille3 = oldState.newReinvestingPermille3;
        newDevPermille3 = oldState.newDevPermille3;

        // Write the new state to a file
        writeNewState(newStateFile);

        std::cout << "Migration completed successfully. New state saved to: " << newStateFile << std::endl;

        // Print the values of old state file
        char QCAP_ISSUER_P[128] = {0};
        char authAddress1_P[128] = {0};
        char authAddress2_P[128] = {0};
        char authAddress3_P[128] = {0};
        char newAuthAddress1_P[128] = {0};
        char newAuthAddress2_P[128] = {0};
        char newAuthAddress3_P[128] = {0};
        char reinvestingAddress_P[128] = {0};
        char newReinvestingAddress1_P[128] = {0};
        char newReinvestingAddress2_P[128] = {0};
        char newReinvestingAddress3_P[128] = {0};
        char adminAddress_P[128] = {0};
        char newAdminAddress1_P[128] = {0};
        char newAdminAddress2_P[128] = {0};
        char newAdminAddress3_P[128] = {0};
        char bannedAddress1_P[128] = {0};
        char bannedAddress2_P[128] = {0};
        char bannedAddress3_P[128] = {0};
        char unbannedAddress1_P[128] = {0};
        char unbannedAddress2_P[128] = {0};
        char unbannedAddress3_P[128] = {0};
        char bannedAddress_P[QVAULT_MAX_NUMBER_OF_BANNED_ADDRESSES][128] = {0};
        
        getIdentityFromPublicKey(QCAP_ISSUER.m256i_u8, QCAP_ISSUER_P, false);
        getIdentityFromPublicKey(authAddress1.m256i_u8, authAddress1_P, false);
        getIdentityFromPublicKey(authAddress2.m256i_u8, authAddress2_P, false);
        getIdentityFromPublicKey(authAddress3.m256i_u8, authAddress3_P, false);
        getIdentityFromPublicKey(newAuthAddress1.m256i_u8, newAuthAddress1_P, false);
        getIdentityFromPublicKey(newAuthAddress2.m256i_u8, newAuthAddress2_P, false);
        getIdentityFromPublicKey(newAuthAddress3.m256i_u8, newAuthAddress3_P, false);
        getIdentityFromPublicKey(reinvestingAddress.m256i_u8, reinvestingAddress_P, false);
        getIdentityFromPublicKey(newReinvestingAddress1.m256i_u8, newReinvestingAddress1_P, false);
        getIdentityFromPublicKey(newReinvestingAddress2.m256i_u8, newReinvestingAddress2_P, false);
        getIdentityFromPublicKey(newReinvestingAddress3.m256i_u8, newReinvestingAddress3_P, false);
        getIdentityFromPublicKey(adminAddress.m256i_u8, adminAddress_P, false);
        getIdentityFromPublicKey(newAdminAddress1.m256i_u8, newAdminAddress1_P, false);
        getIdentityFromPublicKey(newAdminAddress2.m256i_u8, newAdminAddress2_P, false);
        getIdentityFromPublicKey(newAdminAddress3.m256i_u8, newAdminAddress3_P, false);
        getIdentityFromPublicKey(bannedAddress1.m256i_u8, bannedAddress1_P, false);
        getIdentityFromPublicKey(bannedAddress2.m256i_u8, bannedAddress2_P, false);
        getIdentityFromPublicKey(bannedAddress3.m256i_u8, bannedAddress3_P, false);
        getIdentityFromPublicKey(unbannedAddress1.m256i_u8, unbannedAddress1_P, false);
        getIdentityFromPublicKey(unbannedAddress2.m256i_u8, unbannedAddress2_P, false);
        getIdentityFromPublicKey(unbannedAddress3.m256i_u8, unbannedAddress3_P, false);
        for(uint32_t i = 0 ; i < QVAULT_MAX_NUMBER_OF_BANNED_ADDRESSES; i++)
        {
            getIdentityFromPublicKey(bannedAddress.get(i).m256i_u8, bannedAddress_P[i], false);
        }

        printf("QCAP_ISSUER: %s\n", QCAP_ISSUER_P);
        printf("authAddress1: %s\n", authAddress1_P);
        printf("authAddress2: %s\n", authAddress2_P);
        printf("authAddress3: %s\n", authAddress3_P);
        printf("newAuthAddress1: %s\n", newAuthAddress1_P);
        printf("newAuthAddress2: %s\n", newAuthAddress2_P);
        printf("newAuthAddress3: %s\n", newAuthAddress3_P);
        printf("reinvestingAddress: %s\n", reinvestingAddress_P);
        printf("newReinvestingAddress1: %s\n", newReinvestingAddress1_P);
        printf("newReinvestingAddress2: %s\n", newReinvestingAddress2_P);
        printf("newReinvestingAddress3: %s\n", newReinvestingAddress3_P);
        printf("adminAddress: %s\n", adminAddress_P);
        printf("newAdminAddress1: %s\n", newAdminAddress1_P);
        printf("newAdminAddress2: %s\n", newAdminAddress2_P);
        printf("newAdminAddress3: %s\n", newAdminAddress3_P);
        printf("newBannedAddress1: %s\n", bannedAddress1_P);
        printf("newBannedAddress2: %s\n", bannedAddress2_P);
        printf("newBannedAddress3: %s\n", bannedAddress3_P);
        printf("unbannedAddress1: %s\n", unbannedAddress1_P);
        printf("unbannedAddress2: %s\n", unbannedAddress2_P);
        printf("unbannedAddress3: %s\n", unbannedAddress3_P);
        
        for(uint32_t i = 0 ; i < QVAULT_MAX_NUMBER_OF_BANNED_ADDRESSES; i++)
        {
            printf("BannedAddress%d: %s\n", i + 1, bannedAddress_P[i]);
        }

        printf("numberOfBannedAddress: %llu\n", numberOfBannedAddress);
        printf("shareholderDividend: %llu\n", shareholderDividend);
        printf("QCAPHolderPermille: %llu\n", QCAPHolderPermille);
        printf("reinvestingPermille: %llu\n", reinvestingPermille);
        printf("devPermille: %llu\n", devPermille);
        printf("burnPermille: %llu\n", burnPermille);
        printf("newQCAPHolderPermille1: %llu\n", newQCAPHolderPermille1);
        printf("newReinvestingPermille1: %llu\n", newReinvestingPermille1);
        printf("newDevPermille1: %llu\n", newDevPermille1);
        printf("newQCAPHolderPermille2: %llu\n", newQCAPHolderPermille2);
        printf("newReinvestingPermille2: %llu\n", newReinvestingPermille2);
        printf("newDevPermille2: %llu\n", newDevPermille2);
        printf("newQCAPHolderPermille3: %llu\n", newQCAPHolderPermille3);
        printf("newReinvestingPermille3: %llu\n", newReinvestingPermille3);
        printf("newDevPermille3: %llu\n", newDevPermille3);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}