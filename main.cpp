#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <cassert>

using namespace std;

// --- VARIABLES GLOBALES ---
mutex cerrojo_mapa; // mutex
unordered_map<string, int> conteo_global_ips;

// --- GENERACIÓN DE DATOS ---
// Genera un log sintético con tráfico normal y un ataque DDoS inyectado
void generar_log_sintetico(const string& nombre_archivo) {
    ofstream archivo(nombre_archivo);
    string ip_atacante = "192.168.1.100";
    
    for (int i = 0; i < 100000; i++) {
        // Inyectamos el ataque: El 30% del tráfico vendrá de la IP atacante
        if (i % 3 == 0) {
            archivo << "[2026-05-21 10:00:00] " << ip_atacante << " 443\n";
        } else {
            // Tráfico normal (IPs aleatorias simuladas)
            archivo << "[2026-05-21 10:00:00] 10.0.0." << (i % 255) << " 80\n";
        }
    }
    archivo.close();
    cout << "[-] Archivo de log sintético generado con éxito.\n";
}

// --- 2. SOLUCIÓN SECUENCIAL (Para comparar tiempos) ---
string detectar_anomalia_secuencial(const vector<string>& lineas_log) {
    unordered_map<string, int> conteo_local;
    for (const string& linea : lineas_log) {
        // Extracción rudimentaria de la IP
        size_t inicio_ip = linea.find("] ") + 2;
        size_t fin_ip = linea.find(" ", inicio_ip);
        if (inicio_ip != string::npos && fin_ip != string::npos) {
            string ip = linea.substr(inicio_ip, fin_ip - inicio_ip);
            conteo_local[ip]++;
        }
    }

    string ip_sospechosa = "";
    int max_conexiones = 0;
    for (auto const& par : conteo_local) {
        if (par.second > max_conexiones) {
            max_conexiones = par.second;
            ip_sospechosa = par.first;
        }
    }
    return ip_sospechosa;
}

// --- 3. SOLUCIÓN PARALELA ---
// Función que ejecutará cada hilo de forma independiente
void procesar_bloque_log(const vector<string>& lineas_log, int inicio, int fin) {
    unordered_map<string, int> conteo_local;
    
    // Procesamiento Aislado
    for (int i = inicio; i < fin; i++) {
        size_t inicio_ip = lineas_log[i].find("] ") + 2;
        size_t fin_ip = lineas_log[i].find(" ", inicio_ip);
        if (inicio_ip != string::npos && fin_ip != string::npos) {
            string ip = lineas_log[i].substr(inicio_ip, fin_ip - inicio_ip);
            conteo_local[ip]++;
        }
    }

    // Fusión de Resultados
    lock_guard<mutex> candado(cerrojo_mapa);
    for (auto const& par : conteo_local) {
        conteo_global_ips[par.first] += par.second;
    }
}

string detectar_anomalia_paralelo(const vector<string>& lineas_log) {
    conteo_global_ips.clear();
    int num_hilos = thread::hardware_concurrency(); // Obtiene los núcleos disponibles
    if (num_hilos == 0) num_hilos = 4; // Respaldo de seguridad
    
    vector<thread> hilos;
    int tamano_bloque = lineas_log.size() / num_hilos;

    // Repartición del trabajo
    for (int i = 0; i < num_hilos; i++) {
        int inicio = i * tamano_bloque;
        int fin = (i == num_hilos - 1) ? lineas_log.size() : inicio + tamano_bloque;
        hilos.push_back(thread(procesar_bloque_log, ref(lineas_log), inicio, fin));
    }

    // Esperar a que todos terminen
    for (auto& hilo : hilos) {
        hilo.join();
    }

    // Identificar al atacante desde el mapa global
    string ip_sospechosa = "";
    int max_conexiones = 0;
    for (auto const& par : conteo_global_ips) {
        if (par.second > max_conexiones) {
            max_conexiones = par.second;
            ip_sospechosa = par.first;
        }
    }
    return ip_sospechosa;
}

// --- PRUEBAS AUTOMATIZADAS ---
void ejecutar_pruebas(const vector<string>& lineas_log) {
    cout << "\n[!] Ejecutando pruebas automatizadas de integridad...\n";
    string resultado = detectar_anomalia_paralelo(lineas_log);
    
    // Si la aserción falla, el programa crashea indicando error. 
    // Si pasa, no hace nada y continúa.
    assert(resultado == "192.168.1.100" && "Fallo en la prueba: IP atacante incorrecta");
    
    cout << "[+] Prueba STC0104 Superada: El motor detectó correctamente al atacante (" << resultado << ").\n";
}

// --- FUNCIÓN PRINCIPAL ---
int main() {
    string nombre_archivo = "network_log.txt";
    generar_log_sintetico(nombre_archivo);

    // Cargar archivo a memoria
    vector<string> lineas_log;
    ifstream archivo(nombre_archivo);
    string linea;
    while (getline(archivo, linea)) {
        lineas_log.push_back(linea);
    }
    archivo.close();

    cout << "[-] Log cargado en memoria. Iniciando análisis...\n\n";

    // Medición Secuencial
    auto inicio_seq = chrono::high_resolution_clock::now();
    string ip_seq = detectar_anomalia_secuencial(lineas_log);
    auto fin_seq = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> tiempo_seq = fin_seq - inicio_seq;

    // Medición Paralela
    auto inicio_par = chrono::high_resolution_clock::now();
    string ip_par = detectar_anomalia_paralelo(lineas_log);
    auto fin_par = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> tiempo_par = fin_par - inicio_par;

    // Resultados
    cout << "=== REPORTE DE RENDIMIENTO ===\n";
    cout << "Atacante detectado: " << ip_par << "\n";
    cout << "Tiempo Secuencial : " << tiempo_seq.count() << " ms\n";
    cout << "Tiempo Paralelo   : " << tiempo_par.count() << " ms\n";
    
    // Pruebas
    ejecutar_pruebas(lineas_log);

    return 0;
}
