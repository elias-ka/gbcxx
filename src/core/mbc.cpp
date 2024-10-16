#include "core/mbc.hpp"

namespace cb
{
    MbcVariant load_cartridge(const std::vector<u8>& cartrom)
    {
        {
            switch (cartrom[0x0147])
            {
            case 0x00:
            {
                LOG_INFO("Using rom only MBC");
                return MbcRomOnly{cartrom};
            }
            case 0x01:
            case 0x02:
            case 0x03:
            {
                LOG_INFO("Using MBC1");
                return Mbc1{cartrom};
            }
            }
            DIE("Unimplemented cartridge type {:#02x}", cartrom[0x0147]);
            return std::monostate{};
        }
    }
} // namespace cb
