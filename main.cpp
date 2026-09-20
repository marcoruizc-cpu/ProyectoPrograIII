#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fstream>

using namespace std;

// ==========================================
// 1. ESTRUCTURAS DE DATOS PRINCIPALES
// ==========================================

struct Movie {
    int id;
    int year;
    string title;
    string director;
    string cast;
    string genre;
    string plot;
    int likes = 0; // Para el algoritmo de ranking e importancia
};

// Nodo del Árbol Trie (Almacena caracteres en sus ramas)
struct TrieNode {
    unordered_map<char, TrieNode*> children;
    vector<int> movieIds; // IDs de películas asociadas a la palabra/prefijo
    bool isEndOfWord = false;
};

// Auxiliar para limpiar puntuación y normalizar texto
string cleanText(const string& str) {
    string cleaned = "";
    for (char c : str) {
        if (isalnum(c) || c == ' ') {
            cleaned += tolower(c);
        }
    }
    return cleaned;
}

// Separar una cadena de texto en palabras individuales (Tokens)
vector<string> tokenize(const string& str) {
    string cleaned = cleanText(str);
    stringstream ss(cleaned);
    string word;
    vector<string> tokens;
    while (ss >> word) {
        tokens.push_back(word);
    }
    return tokens;
}

// ==========================================
// 2. IMPLEMENTACIÓN DEL ÁRBOL TRIE
// ==========================================

class MovieTrie {
private:
    TrieNode* root;

    // Liberación recursiva de memoria del árbol
    void freeNode(TrieNode* node) {
        if (!node) return;
        for (auto pair : node->children) {
            freeNode(pair.second);
        }
        delete node;
    }

public:
    MovieTrie() {
        root = new TrieNode();
    }

    ~MovieTrie() {
        freeNode(root);
    }

    // Insertar una palabra o prefijo en el Árbol
    void insert(const string& word, int movieId) {
        TrieNode* current = root;
        for (char ch : word) {
            ch = tolower(ch);
            if (current->children.find(ch) == current->children.end()) {
                current->children[ch] = new TrieNode();
            }
            current = current->children[ch];

            // Evitar duplicados consecutivos del mismo ID en el mismo nodo
            if (current->movieIds.empty() || current->movieIds.back() != movieId) {
                current->movieIds.push_back(movieId);
            }
        }
        current->isEndOfWord = true;
    }

    // Buscar una palabra/sub-palabra y retornar los IDs coincidentes
    vector<int> search(const string& query) {
        TrieNode* current = root;
        for (char ch : query) {
            ch = tolower(ch);
            if (current->children.find(ch) == current->children.end()) {
                return {}; // No hay coincidencias en este camino del árbol
            }
            current = current->children[ch];
        }
        return current->movieIds;
    }
};

// ==========================================
// 3. PLATAFORMA DE STREAMING
// ==========================================

class StreamingPlatform {
private:
    vector<Movie> movies;
    MovieTrie mainTrie; // Árbol Trie global para la indexación rápida de términos

    // Listas personalizadas del usuario
    unordered_set<int> watchLaterList;
    unordered_set<int> likedMoviesList;

public:
    // Indexar una película en la estructura de datos y en el Trie
    void addMovie(const Movie& movie) {
        movies.push_back(movie);
        int movieId = movie.id;

        // Indexar palabras del título
        for (const string& w : tokenize(movie.title)) mainTrie.insert(w, movieId);

        // Indexar palabras de la sinopsis
        for (const string& w : tokenize(movie.plot)) mainTrie.insert(w, movieId);

        // Indexar Tags (Director y Género)
        for (const string& w : tokenize(movie.director)) mainTrie.insert(w, movieId);
        for (const string& w : tokenize(movie.genre)) mainTrie.insert(w, movieId);
    }

    // Método para abrir y cargar la base de datos CSV procesada
    bool loadCSV(const string& filename) {
        ifstream file(filename);
        if (!file.is_open()) {
            cerr << "\n[!] Error: No se pudo abrir el archivo " << filename << endl;
            return false;
        }

        string line;
        // Omitir la fila de cabecera
        getline(file, line);

        int idCounter = 0;
        while (getline(file, line)) {
            if (line.empty()) continue;

            stringstream ss(line);
            string yearStr, title, director, cast, genre, plot;

            // Lectura por delimitador '|'
            getline(ss, yearStr, '|');
            getline(ss, title, '|');
            getline(ss, director, '|');
            getline(ss, cast, '|');
            getline(ss, genre, '|');
            getline(ss, plot, '|');

            Movie m;
            m.id = idCounter++;
            m.title = title;
            m.director = director;
            m.cast = cast;
            m.genre = genre;
            m.plot = plot;

            try {
                m.year = yearStr.empty() ? 0 : stoi(yearStr);
            } catch (...) {
                m.year = 0;
            }

            addMovie(m);
        }

        file.close();
        cout << "\n[✓] Base de datos cargada correctamente. Total de peliculas indexadas: " << movies.size() << endl;
        return true;
    }

    // Algoritmo para ranking e importancia de películas
    vector<int> rankResults(const vector<string>& queryTokens, const unordered_set<int>& candidateIds, const string& rawQuery) {
        vector<pair<int, int>> scoredMovies; // {Puntuación, MovieID}
        string cleanRawQuery = cleanText(rawQuery);

        for (int id : candidateIds) {
            const Movie& m = movies[id];
            int score = 0;

            string cleanTitle = cleanText(m.title);
            string cleanPlot = cleanText(m.plot);

            // Coincidencia de la frase exacta en el título (máxima puntuación)
            if (cleanTitle.find(cleanRawQuery) != string::npos) score += 50;

            // Coincidencia individual por tokens
            for (const string& token : queryTokens) {
                if (cleanTitle.find(token) != string::npos) score += 10;
                if (cleanPlot.find(token) != string::npos) score += 3;
                if (cleanText(m.genre).find(token) != string::npos) score += 5;
                if (cleanText(m.director).find(token) != string::npos) score += 5;
            }

            // Bonificación por cantidad de Likes
            score += m.likes * 2;

            scoredMovies.push_back({score, id});
        }

        // Ordenamiento descendente por puntuación
        sort(scoredMovies.begin(), scoredMovies.end(), [](const pair<int, int>& a, const pair<int, int>& b) {
            return a.first > b.first;
        });

        vector<int> sortedIds;
        for (const auto& item : scoredMovies) {
            sortedIds.push_back(item.second);
        }
        return sortedIds;
    }

    // Búsqueda interactiva con paginación de 5 en 5 resultados
    void searchMovies(const string& rawQuery) {
        vector<string> tokens = tokenize(rawQuery);
        if (tokens.empty()) {
            cout << "\n[!] Ingrese un termino de busqueda valido.\n";
            return;
        }

        unordered_set<int> candidateSet;

        // Recuperar registros del Árbol Trie para cada palabra
        for (const string& token : tokens) {
            vector<int> matches = mainTrie.search(token);
            for (int id : matches) {
                candidateSet.insert(id);
            }
        }

        if (candidateSet.empty()) {
            cout << "\n==========================================";
            cout << "\nNo se encontraron coincidencias para: \"" << rawQuery << "\"\n";
            cout << "==========================================\n";
            return;
        }

        // Aplicar ordenamiento de importancia
        vector<int> rankedIds = rankResults(tokens, candidateSet, rawQuery);

        int totalResults = rankedIds.size();
        int offset = 0;
        char choice;

        do {
            cout << "\n==========================================";
            cout << "\n RESULTADOS DE BUSQUEDA (" << offset + 1 << "-" << min(offset + 5, totalResults) << " de " << totalResults << ")";
            cout << "\n==========================================\n";

            int limit = min(offset + 5, totalResults);
            for (int i = offset; i < limit; ++i) {
                int id = rankedIds[i];
                cout << "[" << i + 1 << "] " << movies[id].title << " (" << movies[id].year << ")"
                     << " | Genero: " << movies[id].genre
                     << " | Director: " << movies[id].director
                     << " | Likes: " << movies[id].likes << "\n";
            }

            cout << "------------------------------------------\n";
            if (limit < totalResults) {
                cout << "[V] Ver siguientes 5 coincidencias\n";
            }
            cout << "[S] Seleccionar una pelicula por numero (#)\n";
            cout << "[X] Volver al menu principal\n";
            cout << "Opcion: ";
            cin >> choice;
            choice = toupper(choice);

            if (choice == 'V' && limit < totalResults) {
                offset += 5;
            } else if (choice == 'S') {
                int selIndex;
                cout << "Ingrese el numero de la pelicula: ";
                cin >> selIndex;
                if (selIndex >= 1 && selIndex <= totalResults) {
                    displayMovieDetail(rankedIds[selIndex - 1]);
                } else {
                    cout << "\n[!] Seleccion invalida.\n";
                }
            }

        } while (choice == 'V' && offset < totalResults);
    }

    // Mostrar detalle de la película
    void displayMovieDetail(int movieId) {
        Movie& m = movies[movieId];
        cout << "\n==========================================";
        cout << "\n TITULO: " << m.title << " (" << m.year << ")";
        cout << "\n Director: " << m.director;
        cout << "\n Genero: " << m.genre;
        cout << "\n Elenco: " << m.cast;
        cout << "\n------------------------------------------";
        cout << "\n SINOPSIS:\n " << m.plot;
        cout << "\n==========================================\n";

        cout << "[1] Dar Like (+1)\n";
        cout << "[2] Anadir a 'Ver Mas Tarde'\n";
        cout << "[0] Volver\n";
        cout << "Opcion: ";
        int opt;
        cin >> opt;

        if (opt == 1) {
            m.likes++;
            likedMoviesList.insert(m.id);
            cout << "\n[✓] Le has dado Like a: " << m.title << "\n";
        } else if (opt == 2) {
            watchLaterList.insert(m.id);
            cout << "\n[✓] Agregada a tu lista de 'Ver Mas Tarde'.\n";
        }
    }

    // Pantalla de Inicio (Ver más tarde & Recomendaciones)
    void showHomeScreen() {
        cout << "\n==========================================";
        cout << "\n      PLATAFORMA DE STREAMING (INICIO)   ";
        cout << "\n==========================================\n";

        // 1. Mostrar 'Ver Más Tarde'
        cout << ">> TU LISTA 'VER MAS TARDE':\n";
        if (watchLaterList.empty()) {
            cout << "   (No tienes peliculas en esta lista)\n";
        } else {
            for (int id : watchLaterList) {
                cout << "   - " << movies[id].title << " (" << movies[id].year << ")\n";
            }
        }

        // 2. Recomendaciones personalizadas
        cout << "\n>> RECOMENDADAS PARA TI (Segun tus Likes):\n";
        if (likedMoviesList.empty()) {
            cout << "   (Dale 'Like' a alguna pelicula para generar sugerencias)\n";
        } else {
            unordered_set<string> likedGenres;
            for (int id : likedMoviesList) {
                for (const string& g : tokenize(movies[id].genre)) {
                    likedGenres.insert(g);
                }
            }

            int count = 0;
            for (const Movie& m : movies) {
                if (likedMoviesList.count(m.id)) continue;

                bool match = false;
                for (const string& g : tokenize(m.genre)) {
                    if (likedGenres.count(g)) {
                        match = true;
                        break;
                    }
                }

                if (match) {
                    cout << "   - " << m.title << " | Genero: " << m.genre << "\n";
                    count++;
                    if (count >= 5) break;
                }
            }
            if (count == 0) cout << "   (No se encontraron sugerencias adicionales)\n";
        }
        cout << "==========================================\n";
    }
};

// ==========================================
// 4. MAIN INTERACTIVO
// ==========================================

int main() {
    StreamingPlatform platform;

    // Cargar la base de datos CSV pre-procesada
    cout << "Cargando base de datos de peliculas desde 'movies_clean.csv'..." << endl;
    if (!platform.loadCSV("movies_clean.csv")) {
        cout << "\n[!] Asegurese de ejecutar primero el script de Python para generar 'movies_clean.csv'.\n";
        return 1;
    }

    int option;
    string query;

    do {
        cout << "\n--- MENU PRINCIPAL ---\n";
        cout << "1. Ir a Pantalla de Inicio (Ver Mas Tarde & Recomendaciones)\n";
        cout << "2. Buscar Pelicula (palabra, frase o Tag)\n";
        cout << "3. Salir\n";
        cout << "Seleccione una opcion: ";
        cin >> option;
        cin.ignore();

        switch (option) {
            case 1:
                platform.showHomeScreen();
                break;
            case 2:
                cout << "\nIngrese su termino de busqueda (ej. 'barco', 'barco fantasma', 'nolan'): ";
                getline(cin, query);
                platform.searchMovies(query);
                break;
            case 3:
                cout << "\n¡Saliendo del programa!\n";
                break;
            default:
                cout << "\n[!] Opcion no valida.\n";
        }
    } while (option != 3);

    return 0;
}