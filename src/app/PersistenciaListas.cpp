#include "app/Plataforma.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace {

namespace fs = std::filesystem;
constexpr const char* FORMATO = "LISTAS_V1";

// Lee un ID entero, no negativo y sin texto extra.
bool leerId(const std::string& texto, int& id) {
    std::istringstream entrada(texto);
    char resto;
    return (entrada >> id) && id >= 0 && !(entrada >> resto);
}

} // namespace

bool Plataforma::cargarListas(const std::string& rutaArchivo) {
    std::error_code ec;
    if (!fs::exists(rutaArchivo, ec)) {
        return !ec; // Primera ejecucion: archivo inexistente, listas vacias.
    }
    if (ec) return false;

    std::ifstream archivo(rutaArchivo);
    if (!archivo) return false;

    std::string linea;
    if (!std::getline(archivo, linea) || linea != FORMATO) return false;

    std::vector<int> nuevosLikes;
    std::vector<int> nuevosVerMasTarde;

    while (std::getline(archivo, linea)) {
        std::istringstream lector(linea);
        std::string tipo, valor, extra;
        if (!(lector >> tipo >> valor) || (lector >> extra)) return false;

        int id = -1;
        if (!leerId(valor, id) || static_cast<size_t>(id) >= peliculas.size()) {
            return false;
        }
        std::vector<int>* lista = nullptr;
        if (tipo == "LIKE") lista = &nuevosLikes;
        else if (tipo == "VER_MAS_TARDE") lista = &nuevosVerMasTarde;
        else return false;

        if (std::find(lista->begin(), lista->end(), id) == lista->end()) {
            lista->push_back(id);
        }
    }
    if (archivo.bad()) return false;

    // Solo sustituimos las listas cuando se valido todo el archivo.
    likes = std::move(nuevosLikes);
    verMasTarde = std::move(nuevosVerMasTarde);
    return true;
}

bool Plataforma::guardarListas(const std::string& rutaArchivo) const {
    const fs::path destino(rutaArchivo);
    const fs::path temporal = destino.string() + ".tmp";
    std::error_code ec;

    if (!destino.parent_path().empty()) {
        fs::create_directories(destino.parent_path(), ec);
        if (ec) return false;
    }

    {
        std::ofstream archivo(temporal, std::ios::trunc);
        if (!archivo) return false;
        archivo << FORMATO << '\n';
        for (int id : likes) archivo << "LIKE " << id << '\n';
        for (int id : verMasTarde) archivo << "VER_MAS_TARDE " << id << '\n';
        archivo.close();
        if (!archivo) {
            fs::remove(temporal, ec);
            return false;
        }
    }

    // La sustitucion se realiza despues de escribir el archivo completo.
    // Si falla, se mantiene intacta la version anterior.
#ifdef _WIN32
    // En Windows, std::filesystem::rename no siempre reemplaza un destino
    // existente. MoveFileExW permite actualizar el archivo ya guardado.
    if (!MoveFileExW(temporal.wstring().c_str(), destino.wstring().c_str(),
                     MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        fs::remove(temporal, ec);
        return false;
    }
#else
    fs::rename(temporal, destino, ec);
    if (ec) {
        std::error_code ignorado;
        fs::remove(temporal, ignorado);
        return false;
    }
#endif
    return true;
}
