#pragma once

#include <string>
#include <functional>

namespace YimMenu
{
    enum class LicensePermissions : uint8_t
    {
        FREE = 0,
        BASIC = 1,
        REGULAR = 2,
        ULTIMATE = 3,
    };

    struct AuthResult
    {
        bool success = false;
        std::string message;
        std::string translated_message;
        LicensePermissions privilege = LicensePermissions::FREE;
        std::string perm_sig;
        std::string root_name;
        uint8_t unlocks = 0;
        int64_t direct_packet_magic = 0;
        int64_t tunables_version = 0;
    };

    class Authentication
    {
    public:
        // Configuración
        static void SetAPIEndpoint(const std::string& endpoint);
        static void SetVersion(const std::string& version);
        
        // Gestión de claves
        static bool LoadActivationKey();
        static bool SaveActivationKey(const std::string& key);
        static bool ClearActivationKey();
        static std::string GetActivationKey();
        
        // Autenticación
        static AuthResult Authenticate(const std::string& activation_key);
        static AuthResult SendHeartbeat(uint64_t session);
        
        // Estado
        static bool IsAuthenticated();
        static bool HasValidSession();
        static LicensePermissions GetPrivilege();
        static std::string GetRootName();
        static uint8_t GetUnlocks();
        
        // Utilidades
        static std::string GenerateHWID();
        static std::string GenerateIdentity();
        
    private:
        static std::string s_APIEndpoint;
        static std::string s_Version;
        static std::string s_ActivationKey;
        static AuthResult s_LastResult;
        static uint64_t s_CurrentSession;
        static int64_t s_PermExpiry;
        
        static std::string ReadFile(const std::string& path);
        static bool WriteFile(const std::string& path, const std::string& content);
        static std::string HTTPPost(const std::string& url, const std::string& data);
        static bool VerifySignature(const std::string& perm_sig, const std::string& data);
    };
}
