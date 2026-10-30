#include <iostream>
#include <string>
#include <algorithm>
#include <chrono>
#include <vector>
#include <cassert>
#include <random>
#include <fstream>
#include <iomanip>

using namespace std;

// --- FUNCIONES AUXILIARES ---

// Elimina los ceros innecesarios a la izquierda (excepto si el número es solo "0")
string normalizar(string s) {
    size_t primerNoCero = s.find_first_not_of('0');
    if (primerNoCero == string::npos) {
        return "0";
    }
    return s.substr(primerNoCero);
}

// Rellenar con ceros a la izquierda hasta alcanzar una longitud dada
string rellenarIzquierda(string s, size_t longitud) {
    if (s.length() >= longitud) return s;
    return string(longitud - s.length(), '0') + s;
}

// 1. sumar(A, B): Admite A y B de longitudes distintas; el resultado puede tener un dígito más.
string sumar(string A, string B) {
    string resultado = "";
    int i = A.length() - 1;
    int j = B.length() - 1;
    int acarreo = 0;

    while (i >= 0 || j >= 0 || acarreo > 0) {
        int suma = acarreo;
        if (i >= 0) {
            suma += A[i] - '0';
            i--;
        }
        if (j >= 0) {
            suma += B[j] - '0';
            j--;
        }
        acarreo = suma / 10;
        resultado += to_string(suma % 10);
    }

    reverse(resultado.begin(), resultado.end());
    return normalizar(resultado);
}

// 2. restar(A, B): Precondición A >= B. El resultado se normaliza eliminando ceros a la izquierda.
string restar(string A, string B) {
    // Aseguramos que B tenga la misma longitud que A rellenando con ceros a la izquierda para facilitar la resta dígito a dígito
    B = rellenarIzquierda(B, A.length());
    string resultado = "";
    int prestamo = 0;

    for (int i = A.length() - 1; i >= 0; i--) {
        int sub = (A[i] - '0') - (B[i] - '0') - prestamo;
        if (sub < 0) {
            sub += 10;
            prestamo = 1;
        } else {
            prestamo = 0;
        }
        resultado += to_string(sub);
    }

    reverse(resultado.begin(), resultado.end());
    return normalizar(resultado);
}

// 3. desplazar(A, k): Añade k ceros a la derecha (equivalente a A * B^k)
string desplazar(string A, int k) {
    if (A == "0") return "0";
    return A + string(k, '0');
}

// 4. partir(A, m): Separa A en su mitad alta y su mitad baja respecto a la posición m
void partir(string A, size_t m, string &alta, string &baja) {
    size_t len = A.length();
    if (len <= m) {
        alta = "0";
        baja = A;
    } else {
        alta = A.substr(0, len - m);
        baja = A.substr(len - m);
    }
    alta = normalizar(alta);
    baja = normalizar(baja);
}

// --- ALGORITMOS DE MULTIPLICACIÓN ---

// Multiplicación clásica O(n^2) como referencia de correctitud
string multiplicacionClasica(string A, string B) {
    int n = A.length();
    int m = B.length();
    vector<int> res(n + m, 0);

    for (int i = n - 1; i >= 0; i--) {
        for (int j = m - 1; j >= 0; j--) {
            int mul = (A[i] - '0') * (B[j] - '0');
            int suma = mul + res[i + j + 1];
            res[i + j + 1] = suma % 10;
            res[i + j] += suma / 10;
        }
    }

    string resultado = "";
    for (int num : res) {
        resultado += to_string(num);
    }
    return normalizar(resultado);
}

// ============================================================================
//  ALGORITMO DE KARATSUBA
// ============================================================================
//
//  Representación: string en base 10, dígito MÁS significativo en la posición 0
//  (la misma que usan las funciones auxiliares de arriba).
//
//  Idea: con x = x1·10^m + x0  e  y = y1·10^m + y0
//      P1 = x1·y1,   P2 = x0·y0,   P3 = (x1 + x0)·(y1 + y0)
//      x1·y0 + x0·y1 = P3 - P1 - P2
//      x·y = P1·10^(2m) + (P3 - P1 - P2)·10^m + P2
//  Tres productos recursivos en lugar de cuatro  =>  T(n) = 3T(n/2) + Θ(n).

// Tamaño del caso base. Con UMBRAL = 1 se cumple literalmente la sección 3.4
// (números de 1 dígito). Se puede subir desde la línea de comandos para
// estudiar el efecto de las constantes ocultas (ver modo "bench").
size_t UMBRAL = 1;

// Menor potencia de 2 que es >= n
size_t siguientePotenciaDe2(size_t n) {
    size_t p = 1;
    while (p < n) p <<= 1;
    return p;
}

// Núcleo recursivo. Admite operandos de longitud variable porque las sumas
// x1 + x0, y1 + y0 pueden producir un dígito extra (m + 1 dígitos), y
// partir() normaliza las mitades (pueden quedar más cortas).
string karatsubaRec(const string &X, const string &Y) {
    size_t n = max(X.length(), Y.length());

    // Caso base: multiplicación directa
    if (n <= UMBRAL) {
        if (n == 1) return to_string((X[0] - '0') * (Y[0] - '0'));
        return multiplicacionClasica(X, Y);
    }

    // Igualar longitudes y partir respecto de m = n/2
    string x = rellenarIzquierda(X, n);
    string y = rellenarIzquierda(Y, n);
    size_t m = n / 2;

    string x1, x0, y1, y0;
    partir(x, m, x1, x0);
    partir(y, m, y1, y0);

    // Los tres productos recursivos
    string P1 = karatsubaRec(x1, y1);
    string P2 = karatsubaRec(x0, y0);
    string P3 = karatsubaRec(sumar(x1, x0), sumar(y1, y0));

    // Orden de resta seguro (sección 3.3): primero P3 - P1 y luego - P2.
    // P3 - P1 = P2 + x1·y0 + x0·y1 >= P2, así nunca hay negativos intermedios.
    string central = restar(restar(P3, P1), P2);

    // Recomposición: P1·10^(2m) + central·10^m + P2
    return sumar(sumar(desplazar(P1, 2 * m), desplazar(central, m)), P2);
}

// Interfaz pública: normaliza, rellena a la misma potencia de 2 y llama al núcleo
string karatsuba(string A, string B) {
    A = normalizar(A);
    B = normalizar(B);
    if (A == "0" || B == "0") return "0";

    size_t n = siguientePotenciaDe2(max(A.length(), B.length()));
    A = rellenarIzquierda(A, n);
    B = rellenarIzquierda(B, n);

    return normalizar(karatsubaRec(A, B));
}

// ============================================================================
//  VALIDACIÓN
// ============================================================================

mt19937_64 rng(20261101);

// Número aleatorio de exactamente n dígitos (primer dígito distinto de 0)
string numeroAleatorio(size_t n) {
    uniform_int_distribution<int> d(0, 9), d1(1, 9);
    string s(n, '0');
    s[0] = '0' + d1(rng);
    for (size_t i = 1; i < n; i++) s[i] = '0' + d(rng);
    return s;
}

int pruebasOK = 0, pruebasFallidas = 0;

void verificar(const string &nombre, const string &obtenido, const string &esperado) {
    if (obtenido == esperado) {
        pruebasOK++;
    } else {
        pruebasFallidas++;
        cout << "  [FALLA] " << nombre << "\n"
             << "     esperado: " << esperado.substr(0, 60) << (esperado.size() > 60 ? "..." : "") << "\n"
             << "     obtenido: " << obtenido.substr(0, 60) << (obtenido.size() > 60 ? "..." : "") << "\n";
    }
}

// Compara Karatsuba contra la clásica (y opcionalmente contra un valor conocido)
void caso(const string &nombre, const string &A, const string &B, const string &esperado = "") {
    string clasica = multiplicacionClasica(A, B);
    if (!esperado.empty()) verificar(nombre + " (clasica vs valor conocido)", clasica, esperado);
    verificar(nombre, karatsuba(A, B), clasica);
}

void ejecutarPruebas() {
    cout << "=== VALIDACION (UMBRAL = " << UMBRAL << ") ===\n";

    // Funciones auxiliares
    verificar("sumar longitudes distintas", sumar("999", "1"), "1000");
    verificar("sumar con cero", sumar("0", "456"), "456");
    verificar("restar normaliza", restar("1000", "999"), "1");
    verificar("restar iguales", restar("12345", "12345"), "0");
    verificar("desplazar", desplazar("123", 3), "123000");
    verificar("desplazar cero", desplazar("0", 5), "0");
    {
        string a, b;
        partir("12345678", 4, a, b);
        verificar("partir alta", a, "1234");
        verificar("partir baja", b, "5678");
        partir("12000034", 4, a, b);
        verificar("partir baja con ceros", b, "34");
    }

    // 1. Números de 1 dígito (las 100 combinaciones)
    for (int a = 0; a <= 9; a++)
        for (int b = 0; b <= 9; b++)
            caso("1 digito " + to_string(a) + "x" + to_string(b),
                 to_string(a), to_string(b), to_string(a * b));

    // 2. Muchos ceros a la izquierda
    caso("ceros izq 1", "0000000123", "045", "5535");
    caso("ceros izq 2", "00000000000000000009", "0000009", "81");
    caso("ceros izq 3 (todo ceros)", "0000", "000000", "0");
    caso("ceros izq 4", "000", "98765", "0");

    // 3. Ceros intercalados
    caso("ceros intercalados 1", "1000000001", "100010001", "100010001100010001");
    caso("ceros intercalados 2", "10203040506070809", "9080706050403020");
    caso("potencias de 10", "1" + string(100, '0'), "1" + string(57, '0'), "1" + string(157, '0'));

    // 4. Longitudes impares antes del relleno
    caso("impar 3x3", "123", "456", "56088");
    caso("impar 5x3", "12345", "678", "8369910");
    caso("impar 7x5", "1234567", "89012", "109891277804");
    caso("impar 17x9", numeroAleatorio(17), numeroAleatorio(9));

    // Casos que fuerzan acarreo en x1 + x0 (resultado de m + 1 dígitos)
    caso("todos nueves 8x8", "99999999", "99999999", "9999999800000001");
    caso("todos nueves 129x77", string(129, '9'), string(77, '9'));
    caso("conocido 1234x5678", "1234", "5678", "7006652");

    // 5. Productos grandes
    caso("grande 500x500", numeroAleatorio(500), numeroAleatorio(500));
    caso("grande 1000x777", numeroAleatorio(1000), numeroAleatorio(777));
    caso("grande 2048x2048", numeroAleatorio(2048), numeroAleatorio(2048));

    // Pruebas aleatorias de longitudes variadas
    uniform_int_distribution<int> len(1, 300);
    for (int t = 0; t < 300; t++) {
        size_t la = len(rng), lb = len(rng);
        caso("aleatorio " + to_string(la) + "x" + to_string(lb), numeroAleatorio(la), numeroAleatorio(lb));
    }

    cout << "Pruebas correctas: " << pruebasOK << "   fallidas: " << pruebasFallidas << "\n\n";
}

// ============================================================================
//  EXPERIMENTACIÓN (tiempos)
// ============================================================================

// Tiempo promedio (en microsegundos) de una llamada a f(A, B).
// Repite hasta acumular al menos minTotal (muy por encima de la resolución de
// steady_clock) y al menos minReps veces. El resultado se "consume" sumando su
// longitud a 'sumidero' para que el compilador no elimine el cálculo.
template <typename F>
double medir(F f, const string &A, const string &B, size_t &sumidero) {
    using reloj = chrono::steady_clock;
    const auto minTotal = chrono::milliseconds(200);
    const int minReps = 3;

    int reps = 0;
    auto inicio = reloj::now();
    auto fin = inicio;
    do {
        string r = f(A, B);
        sumidero += r.length();
        reps++;
        fin = reloj::now();
    } while (reps < minReps || fin - inicio < minTotal);

    return chrono::duration<double, micro>(fin - inicio).count() / reps;
}

void ejecutarExperimento(size_t maxDigitos) {
    cout << "=== EXPERIMENTACION (UMBRAL = " << UMBRAL << ") ===\n";
    string nombreCsv = "tiempos_umbral_" + to_string(UMBRAL) + ".csv";
    ofstream csv(nombreCsv);
    csv << "digitos,clasica_us,karatsuba_us,razon_clasica_sobre_karatsuba\n";

    cout << left << setw(10) << "n" << setw(18) << "clasica (us)"
         << setw(18) << "karatsuba (us)" << "clasica/karatsuba\n";

    size_t sumidero = 0;
    long long cruce = -1;
    for (size_t n = 2; n <= maxDigitos; n *= 2) {
        string A = numeroAleatorio(n), B = numeroAleatorio(n);

        // Verificación: ambos deben coincidir (también consume el resultado)
        if (karatsuba(A, B) != multiplicacionClasica(A, B)) {
            cout << "ERROR: resultados distintos para n = " << n << "\n";
            return;
        }

        double tc = medir(multiplicacionClasica, A, B, sumidero);
        double tk = medir(karatsuba, A, B, sumidero);
        double razon = tc / tk;

        cout << left << setw(10) << n << setw(18) << fixed << setprecision(2) << tc
             << setw(18) << tk << setprecision(3) << razon << "\n";
        csv << n << "," << tc << "," << tk << "," << razon << "\n";

        if (tk < tc && cruce == -1) cruce = (long long)n;
        if (tk >= tc) cruce = -1;  // el cruce debe mantenerse en los tamaños siguientes
    }

    if (cruce != -1)
        cout << "\nPunto de cruce: Karatsuba supera a la clasica desde n = " << cruce << " digitos.\n";
    else
        cout << "\nNo aparece punto de cruce en el rango medido (hasta " << maxDigitos << " digitos).\n";
    cout << "Datos guardados en " << nombreCsv << "   (sumidero = " << sumidero << ")\n\n";
}

// ============================================================================
//  MAIN
// ============================================================================
//  Uso:
//    ./karatsuba                 -> pruebas + experimento (UMBRAL = 1)
//    ./karatsuba test            -> solo validación
//    ./karatsuba bench [U] [N]   -> solo experimento con caso base de U dígitos
//                                   hasta N dígitos (por defecto U = 1, N = 4096)
//    ./karatsuba A B             -> multiplica dos números dados
int main(int argc, char *argv[]) {
    string modo = argc > 1 ? argv[1] : "todo";

    if (modo == "test") {
        ejecutarPruebas();
        return pruebasFallidas == 0 ? 0 : 1;
    }
    if (modo == "bench") {
        if (argc > 2) UMBRAL = max<size_t>(1, stoul(argv[2]));
        size_t maxN = argc > 3 ? stoul(argv[3]) : 4096;
        ejecutarExperimento(maxN);
        return 0;
    }
    if (modo == "todo") {
        ejecutarPruebas();
        if (pruebasFallidas == 0) ejecutarExperimento(4096);
        return pruebasFallidas == 0 ? 0 : 1;
    }

    // Dos números como argumentos
    if (argc == 3) {
        cout << "Clasica:   " << multiplicacionClasica(argv[1], argv[2]) << "\n";
        cout << "Karatsuba: " << karatsuba(argv[1], argv[2]) << "\n";
        return 0;
    }
    cout << "Uso: " << argv[0] << " [test | bench [umbral] [maxDigitos] | A B]\n";
    return 1;
}