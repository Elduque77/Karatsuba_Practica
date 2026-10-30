# Práctica 2: Multiplicación Rápida de Enteros - Algoritmo de Karatsuba
**Asignatura:** Análisis y Diseño de Algoritmos  
**Tema:** Divide y Vencerás - Recurrencias y Teorema Maestro  
**Lenguaje:** C++ (C++17)  
**Integrantes:** Juan Fernando Duque y Sebastian Henao Mejia 

---

## Descripción del Proyecto

Este proyecto implementa y compara experimentalmente dos algoritmos para la multiplicación de enteros de precisión arbitraria representados como cadenas de caracteres (`std::string`) en base 10:

1. **Multiplicación Clásica (Θ(n²)):** Método tradicional basado en multiplicaciones de dígitos individuales.
2. **Algoritmo de Karatsuba (Θ(nˡᵒᵍ₂ ³) ≈ Θ(n¹.⁵⁸⁵)):** Algoritmo basado en la técnica de *Divide y Vencerás* que reduce la multiplicación de dos números de $n$ dígitos a 3 subproblemas de tamaño $n/2$.

### Decisiones de Diseño Implementadas
* **Representación interna:** Cadenas de caracteres (`std::string`) en base 10 con el dígito más significativo en la posición `0`.
* **Relleno a potencia de 2:** Para garantizar particiones exactas en la recursión, los operandos se rellenan con ceros a la izquierda hasta igualar la potencia de 2 más cercana.
* **Orden de resta seguro:** Se aplica $(P_3 - P_1) - P_2$ para asegurar resultados no negativos en las etapas intermedias sin requerir aritmética con signo.
* **Normalización:** Eliminación automática de ceros a la izquierda en los resultados auxiliares.

---

## Estructura del Repositorio

```
.
├── karatsuba.cpp        # Código fuente principal en C++
├── README.md            # Instrucciones de uso y documentación
└── tiempos_umbral_1.csv # Datos de la experimentación generados automáticamente
```

---

## Requisitos de Compilación

Se requiere un compilador de C++ con soporte para **C++17** (como `g++` o `clang++`).

### Comando de Compilación (Sugerido con optimización `-O3`)
```bash
g++ -std=c++17 -O3 karatsuba.cpp -o karatsuba
```

---

## Instrucciones de Ejecución

El programa cuenta con un ejecutable multipropósito que soporta distintas modalidades a través de la línea de comandos:

### 1. Ejecución Completa (Validación + Experimento por defecto)
Ejecuta las pruebas de validación automatizadas y posteriormente lanza el experimento de medición de tiempos guardando los datos en `tiempos_umbral_1.csv`:
```bash
./karatsuba
```

### 2. Modo Validación Únicamente (`test`)
Efectúa la verificación contra la multiplicación clásica cubriendo los casos de borde obligatorios (1 dígito, ceros a la izquierda, ceros intercalados, longitudes impares y productos de más de 2000 dígitos):
```bash
./karatsuba test
```

### 3. Modo Experimento / Benchmark (`bench [umbral] [maxDigitos]`)
Permite correr las pruebas de tiempo configurando el umbral del caso base y el número máximo de dígitos (por defecto `maxDigitos = 4096`):
```bash
./karatsuba bench 1 4096
```

### 4. Multiplicación Directa de dos Números (`A B`)
Compara ambos algoritmos multiplicando dos números ingresados como argumentos:
```bash
./karatsuba 123456789 987654321
```

---

## Medición de Tiempos y Salida

El módulo de medición utiliza `std::chrono::steady_clock` y promedia repeticiones hasta acumular al menos 200 ms por cada tamaño $n$ para superar el límite de resolución del reloj. Los resultados se consumen mediante una variable sumidero para impedir que el compilador elimine los cálculos por optimización.

La ejecución del benchmarking genera automáticamente un archivo `.csv` con la siguiente estructura:
```csv
digitos,clasica_us,karatsuba_us,razon_clasica_sobre_karatsuba
2,0.15,0.85,0.176
4,0.45,2.10,0.214
...
```

### Análisis Gráfico del Rendimiento
<img width="847" height="567" alt="image" src="https://github.com/user-attachments/assets/07b8ec68-ec55-4708-a642-c96a10bbdf96" />
link del excel con la tabla y la gráfica
https://eafit-my.sharepoint.com/:x:/g/personal/jfduqueg1_eafit_edu_co/cQpv2v_s4YZET5rePQkY-25XEgUCOuGKN7EQ6h0GB3C2uBUhew



---

## Declaración de Uso Ético de IA

En cumplimiento con las políticas de uso ético de la asignatura, el grupo declara haber utilizado herramientas de Inteligencia Artificial (Gemini) y (Claude Code) exclusivamente como apoyo en:
1. El diseño del flujo de trabajo en Git/GitHub (manejo de ramas y merges).
2. La aclaración y estructuración de la deducción teórica de las recurrencias (Teorema Maestro y Árbol de Recurrencia).
3. La revisión conceptual de la lógica para evitar desbordamientos de pila y la optimización del protocolo de medición de tiempos.

Toda la lógica de código e implementación explicada en este repositorio ha sido comprendida, verificada y probada por los integrantes del grupo.
