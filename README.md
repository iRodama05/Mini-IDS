# Mini-IDS: Motor Paralelo de Detección Proactiva de Amenazas
## Análisis del problema
### **Infraestructuras de Alta Densidad:**
En las redes modernas, cada interacción deja un rastro. Ya sea en el nodo central de distribución de una organización o en la arquitectura en la nube de un servicio en línea, los enrutadores y firewalls generan bitácoras de tráfico (logs) de forma incesante. Estos registros son la primera línea de visibilidad para cualquier sistema de defensa proactiva.

Sin embargo, cuando ocurre un evento anómalo, como un escaneo masivo de puertos para buscar vulnerabilidades o un ataque de denegación de servicio (DDoS), el volumen de estas bitácoras explota, generando millones de registros por minuto. La capacidad de reaccionar ante una amenaza depende directamente de la velocidad con la que se pueda leer, procesar y entender esta avalancha de información.

### **El Cuello de Botella Secuencial:**
El análisis tradicional de bitácoras de red se enfrenta a un límite físico ineludible. Si un programa lee un archivo de registro línea por línea desde el principio hasta el final (procesamiento secuencial), el tiempo necesario para encontrar un patrón de ataque crece linealmente con el tamaño del archivo.

Para un archivo de unos pocos megabytes, esto toma fracciones de segundo. Para gigabytes de tráfico de red en tiempo real, el motor de detección se vuelve un cuello de botella. Mientras el sistema sigue leyendo las primeras líneas del registro, el atacante ya ha comprometido la red.

En términos simples: Es el equivalente a intentar encontrar una palabra específica en una enciclopedia de mil páginas leyendo cada página tú solo, una por una.

### **Deconstrucción Lógica y Procesamiento Simultáneo:**
Este proyecto aborda el cuello de botella aplicando una metodología de deconstrucción lógica sobre los datos mediante el uso estricto del paradigma de programación en paralelo utilizando C++.

En lugar de leer el flujo de red de forma lineal, el motor:

1. **Calcula la masa de datos:** Analiza el tamaño total de la bitácora de red.

2. **Deconstruye la carga:** Fragmenta el archivo masivo en bloques lógicos independientes.

3. **Distribuye el esfuerzo:** Asigna cada bloque a un hilo de procesamiento distinto (thread) dentro del procesador multi-núcleo.

4. **Sintetiza la amenaza:** Cada hilo reporta sus hallazgos de forma concurrente, fusionando los resultados en un único reporte de anomalías en una fracción del tiempo original.

## Arquitectura del Sistema

Para lograr una defensa proactiva eficiente, el motor evita el análisis lineal tradicional y emplea una arquitectura de procesamiento paralelo de datos. La solución se fundamenta en la deconstrucción lógica del flujo de información, dividiendo el problema masivo en sub-tareas computacionales autónomas que se ejecutan simultáneamente.

### 1. Ingesta y Modelado de Datos
El sistema recibe bitácoras de tráfico de red en texto plano (archivos `.log`). Para mantener la eficiencia espacial y temporal, cada línea se modela internamente extrayendo únicamente los datos clave para la detección de anomalías:

- `Timestamp` (Marca de tiempo)

- `Source_IP` (Dirección IP de origen)

- `Target_Port` (Puerto de destino)

2. Cómo Funciona el Trabajo en Paralelo (División y Conquista)
El programa en C++ está diseñado para aprovechar todos los núcleos de tu computadora al mismo tiempo, haciendo que trabajen en equipo. Se divide en tres pasos muy sencillos:
