#include "datos/Texto.h"
#include <sstream>
#include <set>
#include <cctype>

// ---------------------------------------------------------------------------
// Plegado de tildes (UTF-8 -> ASCII)
// ---------------------------------------------------------------------------
// El CSV esta en UTF-8: los caracteres fuera de ASCII ocupan de 2 a 4 bytes.
// Las letras latinas con tilde que aparecen en el dataset (e, a, u, o con
// macron como en "Itō", c, s...) estan entre U+00C0 y U+017F, y en UTF-8
// siempre se codifican con 2 bytes cuyo primer byte es 0xC3, 0xC4 o 0xC5.
// Esta tabla da su version ASCII en minuscula; se indexa con
// (primerByte - 0xC3) * 64 + (segundoByte - 0x80).
// " " = simbolo que actua como separador (el signo de multiplicar y dividir).
// Generada con la base de datos Unicode (descomposicion NFKD sin diacriticos).
static const char* const PLEGADO_LATINO[192] = {
    "a", "a", "a", "a", "a", "a", "ae", "c",  // U+00C0 ÀÁÂÃÄÅÆÇ
    "e", "e", "e", "e", "i", "i", "i", "i",  // U+00C8 ÈÉÊËÌÍÎÏ
    "d", "n", "o", "o", "o", "o", "o", " ",  // U+00D0 ÐÑÒÓÔÕÖ×
    "o", "u", "u", "u", "u", "y", "th", "ss",  // U+00D8 ØÙÚÛÜÝÞß
    "a", "a", "a", "a", "a", "a", "ae", "c",  // U+00E0 àáâãäåæç
    "e", "e", "e", "e", "i", "i", "i", "i",  // U+00E8 èéêëìíîï
    "d", "n", "o", "o", "o", "o", "o", " ",  // U+00F0 ðñòóôõö÷
    "o", "u", "u", "u", "u", "y", "th", "y",  // U+00F8 øùúûüýþÿ
    "a", "a", "a", "a", "a", "a", "c", "c",  // U+0100 ĀāĂăĄąĆć
    "c", "c", "c", "c", "c", "c", "d", "d",  // U+0108 ĈĉĊċČčĎď
    "d", "d", "e", "e", "e", "e", "e", "e",  // U+0110 ĐđĒēĔĕĖė
    "e", "e", "e", "e", "g", "g", "g", "g",  // U+0118 ĘęĚěĜĝĞğ
    "g", "g", "g", "g", "h", "h", "h", "h",  // U+0120 ĠġĢģĤĥĦħ
    "i", "i", "i", "i", "i", "i", "i", "i",  // U+0128 ĨĩĪīĬĭĮį
    "i", "i", "ij", "ij", "j", "j", "k", "k",  // U+0130 İıĲĳĴĵĶķ
    "k", "l", "l", "l", "l", "l", "l", "l",  // U+0138 ĸĹĺĻļĽľĿ
    "l", "l", "l", "n", "n", "n", "n", "n",  // U+0140 ŀŁłŃńŅņŇ
    "n", "n", "n", "n", "o", "o", "o", "o",  // U+0148 ňŉŊŋŌōŎŏ
    "o", "o", "oe", "oe", "r", "r", "r", "r",  // U+0150 ŐőŒœŔŕŖŗ
    "r", "r", "s", "s", "s", "s", "s", "s",  // U+0158 ŘřŚśŜŝŞş
    "s", "s", "t", "t", "t", "t", "t", "t",  // U+0160 ŠšŢţŤťŦŧ
    "u", "u", "u", "u", "u", "u", "u", "u",  // U+0168 ŨũŪūŬŭŮů
    "u", "u", "u", "u", "w", "w", "y", "y",  // U+0170 ŰűŲųŴŵŶŷ
    "y", "z", "z", "z", "z", "z", "z", "s",  // U+0178 ŸŹźŻżŽžſ
};

// Cantidad de bytes del caracter UTF-8 segun su primer byte.
static size_t largoUTF8(unsigned char primerByte) {
    if (primerByte < 0x80) return 1;             // 0xxxxxxx: ASCII
    if (primerByte >= 0xC2 && primerByte <= 0xDF) return 2;
    if (primerByte >= 0xE0 && primerByte <= 0xEF) return 3;
    if (primerByte >= 0xF0 && primerByte <= 0xF4) return 4;
    return 1;                                     // byte invalido suelto
}

// Traduce el caracter de 'largo' bytes que empieza en texto[i]:
// - letra latina con tilde      -> su version ASCII ("e" para "é")
// - puntuacion / simbolo Unicode -> " " (separador)
// - cualquier otro (malayalam, japones...) -> se conserva intacto, para que
//   la palabra no se parta.
static std::string plegarCaracter(const std::string& texto, size_t i, size_t largo) {
    unsigned char b0 = static_cast<unsigned char>(texto[i]);
    if (largo == 2) {
        unsigned char b1 = static_cast<unsigned char>(texto[i + 1]);
        if (b0 == 0xC2) return " ";               // U+0080-U+00BF: espacio duro, simbolos, comillas angulares
        if (b0 >= 0xC3 && b0 <= 0xC5) {
            return PLEGADO_LATINO[(b0 - 0xC3) * 64 + (b1 - 0x80)];
        }
    }
    if (largo == 3 && b0 == 0xE2) return " ";     // U+2000-U+2FFF: apostrofo curvo, rayas, comillas, puntos suspensivos, simbolos de moneda
    return texto.substr(i, largo);
}

// ---------------------------------------------------------------------------

std::string normalizarTexto(const std::string& texto) {
    std::string resultado;
    resultado.reserve(texto.size());
    size_t i = 0;
    while (i < texto.size()) {
        unsigned char c = static_cast<unsigned char>(texto[i]);
        size_t largo = largoUTF8(c);
        if (largo == 1) {
            if (std::isalnum(c)) {
                resultado += static_cast<char>(std::tolower(c));
            } else {
                resultado += ' '; // puntuacion / simbolos -> separador
            }
        } else if (i + largo > texto.size()) {
            resultado += ' ';     // caracter incompleto al final del texto
        } else {
            resultado += plegarCaracter(texto, i, largo);
        }
        i += largo;
    }
    return resultado;
}

std::vector<std::string> tokenizar(const std::string& textoNormalizado) {
    std::vector<std::string> tokens;
    std::istringstream stream(textoNormalizado);
    std::string palabra;
    while (stream >> palabra) {
        tokens.push_back(palabra);
    }
    return tokens;
}

// Normaliza un tag para usarlo como clave: recorta espacios, pasa a
// minusculas y pliega tildes ("François" -> "francois"), pero NO toca
// la puntuacion ASCII (a diferencia de normalizarTexto) porque aca se busca
// coincidencia EXACTA del tag completo ("steven spielberg"), y nombres
// propios como "S. Fleming" necesitan conservar sus puntos.
std::string normalizarTag(const std::string& texto) {
    size_t inicio = texto.find_first_not_of(' ');
    size_t fin = texto.find_last_not_of(' ');
    if (inicio == std::string::npos) return "";

    std::string resultado;
    size_t i = inicio;
    while (i <= fin) {
        unsigned char c = static_cast<unsigned char>(texto[i]);
        size_t largo = largoUTF8(c);
        if (largo == 1) {
            resultado += static_cast<char>(std::tolower(c));
        } else if (i + largo > fin + 1) {
            break;                // caracter incompleto: se descarta
        } else {
            std::string plegado = plegarCaracter(texto, i, largo);
            resultado += plegado;
        }
        i += largo;
    }
    return resultado;
}

// Una marca de cita de Wikipedia es un corchete cuyo contenido es:
// un numero ("12"), una sola letra ("a"), una nota ("Note 1", "N 2") o una
// advertencia editorial ("citation needed", "clarification needed").
// Corchetes con palabras reales ("[her]", "[his]") NO son marcas.
static bool esMarcaDeCita(const std::string& contenido) {
    if (contenido.empty()) return false;

    bool todoDigitos = true;
    for (char c : contenido) {
        if (!std::isdigit(static_cast<unsigned char>(c))) todoDigitos = false;
    }
    if (todoDigitos) return true;

    if (contenido.size() == 1 && std::isalpha(static_cast<unsigned char>(contenido[0]))) return true;

    const std::string prefijos[] = {"Note ", "note ", "N "};
    for (const std::string& prefijo : prefijos) {
        if (contenido.compare(0, prefijo.size(), prefijo) == 0 && contenido.size() > prefijo.size()
            && std::isdigit(static_cast<unsigned char>(contenido[prefijo.size()]))) {
            return true;
        }
    }

    const std::string sufijo = "needed";
    return contenido.size() >= sufijo.size()
        && contenido.compare(contenido.size() - sufijo.size(), sufijo.size(), sufijo) == 0;
}

std::string quitarMarcasDeCita(const std::string& texto) {
    const size_t LARGO_MAXIMO_MARCA = 30;
    std::string resultado;
    resultado.reserve(texto.size());
    size_t i = 0;
    while (i < texto.size()) {
        if (texto[i] == '[') {
            size_t cierre = texto.find(']', i + 1);
            if (cierre != std::string::npos && cierre - i - 1 <= LARGO_MAXIMO_MARCA
                && esMarcaDeCita(texto.substr(i + 1, cierre - i - 1))) {
                i = cierre + 1; // salta la marca completa
                continue;
            }
        }
        resultado += texto[i];
        i++;
    }
    return resultado;
}

// Conectores y pronombres frecuentes en ingles. Un set (S5) da busqueda en
// O(log n), igual que el ejemplo "contador de palabras sin conectores".
static const std::set<std::string> STOPWORDS = {
    "the", "a", "an", "and", "or", "of", "to", "in", "on", "at", "by", "for",
    "with", "his", "her", "he", "she", "is", "was", "are", "were", "be",
    "been", "it", "its", "as", "that", "this", "from", "they", "their",
    "them", "him", "who", "which", "but", "not", "has", "had", "have", "into",
    "after", "when", "then", "there", "while", "where", "what", "out", "up",
    "one", "all", "so", "than", "also", "s"
};

bool esStopword(const std::string& palabra) {
    return STOPWORDS.find(palabra) != STOPWORDS.end();
}
