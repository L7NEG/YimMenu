#include "Auth.hpp"

#include <windows.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <algorithm>

// Para HTTP requests - usar libcurl o WinHTTP
#include <winhttp.h>
#pragma comment(lib, "winhttp.lib")

// Para hashing
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

namespace YimMenu
{
    // ============================================
    // VARIABLES ESTÁTICAS
    // ============================================
    std::string Authentication::s_APIEndpoint = "http://localhost/yimmenu_auth/api/heartbeat.php";
    std::string Authentication::s_Version = "1.0.0";
    std::string Authentication::s_ActivationKey = "";
    AuthResult Authentication::s_LastResult = {};
    uint64_t Authentication::s_CurrentSession = 0;
    int64_t Authentication::s_PermExpiry = 0;

    // ============================================
    // CONFIGURACIÓN
    // ============================================
    void Authentication::SetAPIEndpoint(const std::string& endpoint)
    {
        s_APIEndpoint = endpoint;
    }

    void Authentication::SetVersion(const std::string& version)
    {
        s_Version = version;
    }

    // ============================================
    // GESTIÓN DE CLAVES
    // ============================================
    std::string Authentication::ReadFile(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        if (!file.is_open())
            return "";
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool Authentication::WriteFile(const std::string& path, const std::string& content)
    {
        std::ofstream file(path, std::ios::binary);
        if (!file.is_open())
            return false;
        
        file << content;
        return true;
    }

    bool Authentication::LoadActivationKey()
    {
        // Ruta donde se guarda la key (ajustar según YimMenu)
        std::string path = "yimmenu_auth.key";
        s_ActivationKey = ReadFile(path);
        
        // Eliminar caracteres whitespace
        s_ActivationKey.erase(std::remove_if(s_ActivationKey.begin(), s_ActivationKey.end(), ::isspace), s_ActivationKey.end());
        
        return !s_ActivationKey.empty();
    }

    bool Authentication::SaveActivationKey(const std::string& key)
    {
        std::string path = "yimmenu_auth.key";
        s_ActivationKey = key;
        return WriteFile(path, key);
    }

    bool Authentication::ClearActivationKey()
    {
        std::string path = "yimmenu_auth.key";
        s_ActivationKey = "";
        
        try {
            std::remove(path.c_str());
            return true;
        } catch(...) {
            return false;
        }
    }

    std::string Authentication::GetActivationKey()
    {
        return s_ActivationKey;
    }

    // ============================================
    // GENERACIÓN DE HWID E IDENTITY
    // ============================================
    std::string Authentication::GenerateHWID()
    {
        // Generar HWID basado en información del sistema
        // En producción, usar información real del hardware
        
        std::string hwid = "";
        
        // Obtener volumen serial del disco C:
        DWORD serial_low = 0, serial_high = 0;
        if (GetVolumeInformationA("C:\\", nullptr, 0, &serial_low, nullptr, nullptr, nullptr, 0))
        {
            hwid += std::to_string(serial_low);
        }
        
        // Rellenar hasta 16 caracteres
        while (hwid.length() < 16)
        {
            hwid += (char)('0' + (rand() % 10));
        }
        
        return hwid.substr(0, 16);
    }

    std::string Authentication::GenerateIdentity()
    {
        // Generar identity aleatorio de 12 caracteres
        static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::string identity = "";
        
        for (int i = 0; i < 12; ++i)
        {
            identity += alphanum[rand() % (sizeof(alphanum) - 1)];
        }
        
        return identity;
    }

    // ============================================
    // HTTP POST
    // ============================================
    std::string Authentication::HTTPPost(const std::string& url, const std::string& data)
    {
        std::string result = "";
        
        try {
            // Parsear URL
            std::string host = url;
            std::string path = "/";
            
            size_t protocol_end = url.find("://");
            if (protocol_end != std::string::npos)
                host = url.substr(protocol_end + 3);
            
            size_t path_start = host.find("/");
            if (path_start != std::string::npos)
            {
                path = host.substr(path_start);
                host = host.substr(0, path_start);
            }
            
            // WinHTTP
            HINTERNET hSession = WinHttpOpen(L"YimMenu/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, nullptr, nullptr, 0);
            if (!hSession)
                return "";
            
            HINTERNET hConnect = WinHttpConnect(hSession, std::wstring(host.begin(), host.end()).c_str(), INTERNET_DEFAULT_HTTP_PORT, 0);
            if (!hConnect)
            {
                WinHttpCloseHandle(hSession);
                return "";
            }
            
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", std::wstring(path.begin(), path.end()).c_str(),
                                                    nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);
            if (!hRequest)
            {
                WinHttpCloseHandle(hConnect);
                WinHttpCloseHandle(hSession);
                return "";
            }
            
            // Enviar request
            std::string headers = "Content-Type: application/json\r\n";
            BOOL bResult = WinHttpSendRequest(hRequest,
                                              WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                              (LPVOID)data.c_str(), data.length(),
                                              data.length(), 0);
            
            if (bResult)
            {
                bResult = WinHttpReceiveResponse(hRequest, nullptr);
                
                if (bResult)
                {
                    // Leer respuesta
                    DWORD dwSize = 0;
                    std::string response = "";
                    
                    do
                    {
                        dwSize = 0;
                        WinHttpQueryDataAvailable(hRequest, &dwSize);
                        
                        if (dwSize > 0)
                        {
                            char* buffer = new char[dwSize + 1];
                            DWORD dwDownloaded = 0;
                            
                            if (WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded))
                            {
                                buffer[dwDownloaded] = '\0';
                                response += buffer;
                            }
                            
                            delete[] buffer;
                        }
                    } while (dwSize > 0);
                    
                    result = response;
                }
            }
            
            WinHttpCloseHandle(hRequest);
            WinHttpCloseHandle(hConnect);
            WinHttpCloseHandle(hSession);
            
        } catch(...) {
            return "";
        }
        
        return result;
    }

    // ============================================
    // AUTENTICACIÓN
    // ============================================
    AuthResult Authentication::Authenticate(const std::string& activation_key)
    {
        AuthResult result = {};
        
        // Generar datos para el heartbeat
        std::string hwid = GenerateHWID();
        std::string identity = GenerateIdentity();
        int64_t expiry = time(nullptr) + (24 * 60 * 60);  // 24 horas
        uint64_t session = (uint64_t)rand() | ((uint64_t)rand() << 17);
        
        s_CurrentSession = session;
        
        // Construir JSON
        std::stringstream json;
        json << "{";
        json << "\"v\":\"" << s_Version << "\",";
        json << "\"a\":\"" << activation_key << "\",";
        json << "\"x\":" << expiry << ",";
        json << "\"i\":\"" << identity << "\",";
        json << "\"h\":\"" << hwid << "\",";
        json << "\"l\":\"es\",";
        json << "\"s\":\"" << session << "\"";
        json << "}";
        
        // Enviar request
        std::string response = HTTPPost(s_APIEndpoint, json.str());
        
        if (response.empty())
        {
            result.message = "Error de conexión con el servidor";
            return result;
        }
        
        // Parsear respuesta (JSON simple)
        // En producción, usar una librería como nlohmann/json
        
        // Buscar campos en la respuesta
        auto findString = [&response](const std::string& key) -> std::string {
            std::string search = "\"" + key + "\":\"";
            size_t pos = response.find(search);
            if (pos == std::string::npos)
                return "";
            
            pos += search.length();
            size_t end = response.find("\"", pos);
            if (end == std::string::npos)
                return "";
            
            return response.substr(pos, end - pos);
        };
        
        auto findInt = [&response](const std::string& key) -> int64_t {
            std::string search = "\"" + key + "\":";
            size_t pos = response.find(search);
            if (pos == std::string::npos)
                return 0;
            
            pos += search.length();
            return std::stoll(response.substr(pos));
        };
        
        // Verificar errores
        std::string t = findString("t");
        std::string m = findString("m");
        
        if (!t.empty())
        {
            result.translated_message = t;
            result.message = m;
            
            if (t == "INVALID_KEY" || t == "SUSPENDED" || t == "SHARE")
            {
                return result;
            }
        }
        
        // Extraer datos de autenticación
        std::string s = findString("s");
        std::string r = findString("r");
        
        if (s.empty())
        {
            result.message = "Respuesta inválida del servidor";
            return result;
        }
        
        // Extraer privilegio del signature
        uint8_t privilege = 0;
        if (!s.empty())
        {
            privilege = s[0] - '0';
        }
        
        result.success = true;
        result.privilege = (LicensePermissions)privilege;
        result.perm_sig = s;
        result.root_name = r;
        result.unlocks = (uint8_t)findInt("u");
        result.direct_packet_magic = findInt("d");
        result.tunables_version = findInt("a");
        result.message = m;
        
        s_LastResult = result;
        s_PermExpiry = expiry;
        s_ActivationKey = activation_key;
        
        return result;
    }

    AuthResult Authentication::SendHeartbeat(uint64_t session)
    {
        // Similar a Authenticate pero con sesión existente
        return Authenticate(s_ActivationKey);
    }

    // ============================================
    // ESTADO
    // ============================================
    bool Authentication::IsAuthenticated()
    {
        return s_LastResult.success && s_LastResult.privilege != LicensePermissions::FREE;
    }

    bool Authentication::HasValidSession()
    {
        return IsAuthenticated() && time(nullptr) < s_PermExpiry;
    }

    LicensePermissions Authentication::GetPrivilege()
    {
        return s_LastResult.privilege;
    }

    std::string Authentication::GetRootName()
    {
        return s_LastResult.root_name;
    }

    uint8_t Authentication::GetUnlocks()
    {
        return s_LastResult.unlocks;
    }
}
