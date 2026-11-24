#include <iostream> //permite mostrar y leer datos en la consola (cout y cin).
#include <string> //permite trabajar con nombres de procesos.
#include <fstream> //permite guardar y recuperar informacion en archivos.
#include <limits> //permite limpiar correctamente la entrada del teclado.
using namespace std;

// LISTA ENLAZADA: almacena todos los procesos registrados.
// Cada nodo guarda los datos y la direccion del siguiente nodo.
struct Proceso {
    int id;
    string nombre;
    int prioridad;
    Proceso* siguiente;
};

// COLA DE PRIORIDAD: atiende primero la prioridad mas alta.
// Se mantiene ordenada: el primer nodo tiene la prioridad numerica mayor.
struct NodoCola {
    int id;
    string nombre;
    int prioridad;
    NodoCola* siguiente;
};

// PILA: administra los bloques de memoria asignados.
// Sigue LIFO: el ultimo bloque agregado es el primero que se libera.
struct Bloque {
    int idProceso;
    int tamanio;
    Bloque* siguiente;
};

Proceso* lista = NULL;
NodoCola* frente = NULL;
Bloque* cima = NULL;

// Lee enteros y evita que una entrada incorrecta detenga el programa
// Lee un entero y evita que una entrada no numerica deje bloqueado el menu.
int leerEntero(string mensaje) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return valor;
        }
        cout << "Entrada no valida. Ingrese un numero.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Recorre la lista desde el primer nodo hasta encontrar el ID solicitado.
Proceso* buscarProceso(int id) {
    Proceso* actual = lista;
    while (actual != NULL) {
        if (actual->id == id) return actual;
        actual = actual->siguiente;
    }
    return NULL;
}

// Crea un nodo nuevo y lo agrega a la lista enlazada de procesos.
void registrarProceso() {
    int id = leerEntero("ID del proceso: ");
    if (buscarProceso(id) != NULL) {
        cout << "Ya existe un proceso con ese ID.\n";
        return;
    }

    string nombre;
    cout << "Nombre del proceso: ";
    getline(cin, nombre);
    int prioridad = leerEntero("Prioridad (numero mayor = mas prioridad): ");

    Proceso* nuevo = new Proceso;
    nuevo->id = id;
    nuevo->nombre = nombre;
    nuevo->prioridad = prioridad;
    nuevo->siguiente = lista;
    lista = nuevo;

    cout << "Proceso registrado correctamente.\n";
}

void mostrarProcesos() {
    if (lista == NULL) {
        cout << "No hay procesos registrados.\n";
        return;
    }

    cout << "\n--- PROCESOS REGISTRADOS ---\n";
    Proceso* actual = lista;
    while (actual != NULL) {
        cout << "ID: " << actual->id
             << " | Nombre: " << actual->nombre
             << " | Prioridad: " << actual->prioridad << "\n";
        actual = actual->siguiente;
    }
}

void buscarProcesoMenu() {
    int id = leerEntero("Ingrese el ID que desea buscar: ");
    Proceso* encontrado = buscarProceso(id);
    if (encontrado == NULL) {
        cout << "Proceso no encontrado.\n";
    } else {
        cout << "Encontrado: " << encontrado->nombre
             << " | Prioridad: " << encontrado->prioridad << "\n";
    }
}

// Modifica la prioridad de un proceso.
// Si ya estaba en la cola, actualiza tambien su posicion segun la nueva prioridad.
void modificarPrioridad() {
    int id = leerEntero("ID del proceso: ");
    Proceso* p = buscarProceso(id);
    if (p == NULL) {
        cout << "Proceso no encontrado.\n";
        return;
    }

    p->prioridad = leerEntero("Nueva prioridad: ");

    // Buscar y retirar el nodo anterior de la cola, si existe.
    NodoCola* actual = frente;
    NodoCola* anterior = NULL;
    bool estabaEnCola = false;

    while (actual != NULL) {
        if (actual->id == id) {
            NodoCola* borrar = actual;
            actual = actual->siguiente;
            if (anterior == NULL) frente = actual;
            else anterior->siguiente = actual;
            delete borrar;
            estabaEnCola = true;
            break;
        }
        anterior = actual;
        actual = actual->siguiente;
    }

    // Volver a insertar el proceso en el lugar correcto de la cola.
    if (estabaEnCola) {
        NodoCola* nuevo = new NodoCola;
        nuevo->id = p->id;
        nuevo->nombre = p->nombre;
        nuevo->prioridad = p->prioridad;
        nuevo->siguiente = NULL;

        if (frente == NULL || nuevo->prioridad > frente->prioridad) {
            nuevo->siguiente = frente;
            frente = nuevo;
        } else {
            NodoCola* pos = frente;
            while (pos->siguiente != NULL &&
                   pos->siguiente->prioridad >= nuevo->prioridad) {
                pos = pos->siguiente;
            }
            nuevo->siguiente = pos->siguiente;
            pos->siguiente = nuevo;
        }
    }

    cout << "Prioridad actualizada correctamente.\n";
}

void eliminarProceso() {
    int id = leerEntero("ID del proceso que desea eliminar: ");
    Proceso* actual = lista;
    Proceso* anterior = NULL;

    while (actual != NULL && actual->id != id) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == NULL) {
        cout << "Proceso no encontrado.\n";
        return;
    }

    if (anterior == NULL) lista = actual->siguiente;
    else anterior->siguiente = actual->siguiente;

    delete actual;

    // Retirar tambien el proceso de la cola.
    NodoCola* q = frente;
    NodoCola* anteriorQ = NULL;
    while (q != NULL) {
        if (q->id == id) {
            NodoCola* borrar = q;
            q = q->siguiente;
            if (anteriorQ == NULL) frente = q;
            else anteriorQ->siguiente = q;
            delete borrar;
        } else {
            anteriorQ = q;
            q = q->siguiente;
        }
    }

    // Liberar todos los bloques de memoria de ese proceso.
    Bloque* b = cima;
    Bloque* anteriorB = NULL;
    while (b != NULL) {
        if (b->idProceso == id) {
            Bloque* borrar = b;
            b = b->siguiente;
            if (anteriorB == NULL) cima = b;
            else anteriorB->siguiente = b;
            delete borrar;
        } else {
            anteriorB = b;
            b = b->siguiente;
        }
    }

    cout << "Proceso eliminado de la lista, de la cola y de la pila de memoria.\n";
}

// Inserta ordenadamente: prioridad mayor al frente
// Copia los datos del proceso a un nodo de cola y lo inserta por prioridad.
void encolarProceso() {
    int id = leerEntero("ID del proceso que desea encolar: ");
    Proceso* p = buscarProceso(id);
    if (p == NULL) {
        cout << "Primero debe registrar el proceso.\n";
        return;
    }

    NodoCola* revisar = frente;
    while (revisar != NULL) {
        if (revisar->id == id) {
            cout << "Ese proceso ya esta en la cola.\n";
            return;
        }
        revisar = revisar->siguiente;
    }

    NodoCola* nuevo = new NodoCola;
    nuevo->id = p->id;
    nuevo->nombre = p->nombre;
    nuevo->prioridad = p->prioridad;
    nuevo->siguiente = NULL;

    if (frente == NULL || nuevo->prioridad > frente->prioridad) {
        nuevo->siguiente = frente;
        frente = nuevo;
    } else {
        NodoCola* actual = frente;
        while (actual->siguiente != NULL &&
               actual->siguiente->prioridad >= nuevo->prioridad) {
            actual = actual->siguiente;
        }
        nuevo->siguiente = actual->siguiente;
        actual->siguiente = nuevo;
    }

    cout << "Proceso agregado a la cola de prioridad.\n";
}

void mostrarCola() {
    if (frente == NULL) {
        cout << "La cola esta vacia.\n";
        return;
    }

    cout << "\n--- COLA DE PRIORIDAD (mayor prioridad primero) ---\n";
    NodoCola* actual = frente;
    while (actual != NULL) {
        cout << "ID: " << actual->id << " | " << actual->nombre
             << " | Prioridad: " << actual->prioridad << "\n";
        actual = actual->siguiente;
    }
}

void buscarEnCola() {
    int id = leerEntero("ID que desea buscar en la cola: ");
    NodoCola* actual = frente;
    while (actual != NULL && actual->id != id) actual = actual->siguiente;

    if (actual == NULL) cout << "El proceso no esta en la cola.\n";
    else cout << "Proceso encontrado en la cola: " << actual->nombre
              << " | Prioridad: " << actual->prioridad << "\n";
}

void quitarDeCola() {
    int id = leerEntero("ID que desea retirar de la cola: ");
    NodoCola* actual = frente;
    NodoCola* anterior = NULL;

    while (actual != NULL && actual->id != id) {
        anterior = actual;
        actual = actual->siguiente;
    }

    if (actual == NULL) {
        cout << "Proceso no encontrado en la cola.\n";
        return;
    }

    if (anterior == NULL) frente = actual->siguiente;
    else anterior->siguiente = actual->siguiente;

    delete actual;
    cout << "Proceso retirado de la cola.\n";
}

// Retira de la cola el proceso que esta primero (mayor prioridad).
void ejecutarProceso() {
    if (frente == NULL) {
        cout << "No hay procesos pendientes para ejecutar.\n";
        return;
    }

    NodoCola* ejecutado = frente;
    frente = frente->siguiente;
    cout << "Ejecutando proceso: " << ejecutado->nombre
         << " (ID " << ejecutado->id << ", prioridad "
         << ejecutado->prioridad << ")\n";
    delete ejecutado;
}

// PILA: PUSH
// PUSH: agrega un nuevo bloque en la cima de la pila.
void asignarMemoria() {
    int id = leerEntero("ID del proceso: ");
    Proceso* p = buscarProceso(id);
    if (p == NULL) {
        cout << "El proceso no esta registrado.\n";
        return;
    }

    int tamanio = leerEntero("Cantidad de memoria a asignar (unidades): ");
    if (tamanio <= 0) {
        cout << "La cantidad debe ser mayor que cero.\n";
        return;
    }

    Bloque* nuevo = new Bloque;
    nuevo->idProceso = id;
    nuevo->tamanio = tamanio;
    nuevo->siguiente = cima;
    cima = nuevo;
    cout << "Memoria asignada. Bloque agregado a la pila (PUSH).\n";
}

// PILA: POP
// POP: retira el bloque que esta en la cima de la pila.
void liberarMemoria() {
    if (cima == NULL) {
        cout << "La pila de memoria esta vacia.\n";
        return;
    }

    Bloque* eliminado = cima;
    cima = cima->siguiente;
    cout << "Memoria liberada: proceso " << eliminado->idProceso
         << ", " << eliminado->tamanio << " unidades (POP).\n";
    delete eliminado;
}

void mostrarMemoria() {
    if (cima == NULL) {
        cout << "No hay bloques de memoria asignados.\n";
        return;
    }

    cout << "\n--- PILA DE MEMORIA (cima a base) ---\n";
    Bloque* actual = cima;
    while (actual != NULL) {
        cout << "Proceso: " << actual->idProceso
             << " | Memoria: " << actual->tamanio << " unidades\n";
        actual = actual->siguiente;
    }
}

void buscarEnPila() {
    int id = leerEntero("ID del proceso que desea buscar en la pila: ");
    Bloque* actual = cima;
    int encontrado = 0;
    while (actual != NULL) {
        if (actual->idProceso == id) {
            cout << "Bloque encontrado: " << actual->tamanio << " unidades.\n";
            encontrado = 1;
        }
        actual = actual->siguiente;
    }
    if (!encontrado) cout << "No hay bloques de ese proceso en la pila.\n";
}

// Guarda las tres estructuras en un archivo de texto
// Escribe en procesos.txt el contenido de las tres estructuras.
void guardarDatos() {
    ofstream archivo("procesos.txt");
    if (!archivo) {
        cout << "No se pudo abrir el archivo para guardar.\n";
        return;
    }

    int cantidad = 0;
    Proceso* p = lista;
    while (p != NULL) { cantidad++; p = p->siguiente; }
    archivo << cantidad << "\n";
    p = lista;
    while (p != NULL) {
        archivo << p->id << "\n" << p->prioridad << "\n" << p->nombre << "\n";
        p = p->siguiente;
    }

    cantidad = 0;
    NodoCola* q = frente;
    while (q != NULL) { cantidad++; q = q->siguiente; }
    archivo << cantidad << "\n";
    q = frente;
    while (q != NULL) {
        archivo << q->id << "\n" << q->prioridad << "\n" << q->nombre << "\n";
        q = q->siguiente;
    }

    cantidad = 0;
    Bloque* b = cima;
    while (b != NULL) { cantidad++; b = b->siguiente; }
    archivo << cantidad << "\n";
    b = cima;
    while (b != NULL) {
        archivo << b->idProceso << "\n" << b->tamanio << "\n";
        b = b->siguiente;
    }

    archivo.close();
    cout << "Datos guardados en el archivo procesos.txt.\n";
}

void limpiarEstructuras() {
    while (lista != NULL) {
        Proceso* aux = lista;
        lista = lista->siguiente;
        delete aux;
    }
    while (frente != NULL) {
        NodoCola* aux = frente;
        frente = frente->siguiente;
        delete aux;
    }
    while (cima != NULL) {
        Bloque* aux = cima;
        cima = cima->siguiente;
        delete aux;
    }
}

// Recupera las tres estructuras desde el archivo
// Lee procesos.txt y reconstruye los nodos de lista, cola y pila.
void cargarDatos() {
    ifstream archivo("procesos.txt");
    if (!archivo) {
        cout << "No existe procesos.txt. Primero guarde los datos.\n";
        return;
    }

    limpiarEstructuras();
    int cantidad, id, prioridad, tamanio;
    string nombre;

    if (!(archivo >> cantidad)) {
        cout << "El archivo esta vacio o tiene un formato incorrecto.\n";
        return;
    }
    archivo.ignore(numeric_limits<streamsize>::max(), '\n');

    for (int i = 0; i < cantidad; i++) {
        archivo >> id >> prioridad;
        archivo.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(archivo, nombre);
        Proceso* nuevo = new Proceso;
        nuevo->id = id;
        nuevo->prioridad = prioridad;
        nuevo->nombre = nombre;
        nuevo->siguiente = lista;
        lista = nuevo;
    }

    archivo >> cantidad;
    for (int i = 0; i < cantidad; i++) {
        archivo >> id >> prioridad;
        archivo.ignore(numeric_limits<streamsize>::max(), '\n');
        getline(archivo, nombre);

        NodoCola* nuevo = new NodoCola;
        nuevo->id = id;
        nuevo->prioridad = prioridad;
        nuevo->nombre = nombre;
        nuevo->siguiente = NULL;

        if (frente == NULL || prioridad > frente->prioridad) {
            nuevo->siguiente = frente;
            frente = nuevo;
        } else {
            NodoCola* actual = frente;
            while (actual->siguiente != NULL &&
                   actual->siguiente->prioridad >= prioridad) {
                actual = actual->siguiente;
            }
            nuevo->siguiente = actual->siguiente;
            actual->siguiente = nuevo;
        }
    }

    archivo >> cantidad;
    for (int i = 0; i < cantidad; i++) {
        archivo >> id >> tamanio;
        Bloque* nuevo = new Bloque;
        nuevo->idProceso = id;
        nuevo->tamanio = tamanio;
        nuevo->siguiente = cima;
        cima = nuevo;
    }

    archivo.close();
    cout << "Datos cargados correctamente.\n";
}

// SUBMENU 1: administra la lista enlazada de procesos.
void menuProcesos() {
    int opcion;
    do {
        cout << "\n========== GESTOR DE PROCESOS ==========\n";
        cout << "1. Registrar proceso\n";
        cout << "2. Mostrar todos los procesos\n";
        cout << "3. Buscar proceso por ID\n";
        cout << "4. Eliminar proceso\n";
        cout << "5. Modificar prioridad\n";
        cout << "0. Volver al menu principal\n";
        opcion = leerEntero("Seleccione una opcion: ");

        switch (opcion) {
            case 1: registrarProceso(); break;
            case 2: mostrarProcesos(); break;
            case 3: buscarProcesoMenu(); break;
            case 4: eliminarProceso(); break;
            case 5: modificarPrioridad(); break;
            case 0: cout << "Volviendo al menu principal...\n"; break;
            default: cout << "Opcion incorrecta.\n";
        }
    } while (opcion != 0);
}

// SUBMENU 2: organiza y ejecuta procesos mediante una cola de prioridad.
void menuPlanificador() {
    int opcion;
    do {
        cout << "\n========== PLANIFICADOR DE CPU ==========\n";
        cout << "1. Encolar proceso por ID\n";
        cout << "2. Mostrar cola de prioridad\n";
        cout << "3. Buscar proceso en la cola\n";
        cout << "4. Retirar proceso de la cola\n";
        cout << "5. Ejecutar siguiente proceso\n";
        cout << "0. Volver al menu principal\n";
        opcion = leerEntero("Seleccione una opcion: ");

        switch (opcion) {
            case 1: encolarProceso(); break;
            case 2: mostrarCola(); break;
            case 3: buscarEnCola(); break;
            case 4: quitarDeCola(); break;
            case 5: ejecutarProceso(); break;
            case 0: cout << "Volviendo al menu principal...\n"; break;
            default: cout << "Opcion incorrecta.\n";
        }
    } while (opcion != 0);
}

// SUBMENU 3: simula la asignacion y liberacion de memoria usando una pila.
void menuMemoria() {
    int opcion;
    do {
        cout << "\n========== GESTOR DE MEMORIA ==========\n";
        cout << "1. Asignar memoria (PUSH)\n";
        cout << "2. Liberar memoria (POP)\n";
        cout << "3. Mostrar estado de memoria\n";
        cout << "4. Buscar bloque por ID de proceso\n";
        cout << "0. Volver al menu principal\n";
        opcion = leerEntero("Seleccione una opcion: ");

        switch (opcion) {
            case 1: asignarMemoria(); break;
            case 2: liberarMemoria(); break;
            case 3: mostrarMemoria(); break;
            case 4: buscarEnPila(); break;
            case 0: cout << "Volviendo al menu principal...\n"; break;
            default: cout << "Opcion incorrecta.\n";
        }
    } while (opcion != 0);
}

// SUBMENU 4: permite guardar y recuperar los datos usando procesos.txt.
void menuArchivos() {
    int opcion;
    do {
        cout << "\n========== ARCHIVOS / PERSISTENCIA ==========\n";
        cout << "1. Guardar datos\n";
        cout << "2. Cargar datos\n";
        cout << "0. Volver al menu principal\n";
        opcion = leerEntero("Seleccione una opcion: ");

        switch (opcion) {
            case 1: guardarDatos(); break;
            case 2: cargarDatos(); break;
            case 0: cout << "Volviendo al menu principal...\n"; break;
            default: cout << "Opcion incorrecta.\n";
        }
    } while (opcion != 0);
}

// Funcion principal: muestra el menu general y abre el modulo seleccionado.
int main() {
    int opcion;
    do {
        cout << "\n===============================================\n";
        cout << "       SISTEMA DE GESTION DE PROCESOS\n";
        cout << "===============================================\n";
        cout << "1. Gestor de Procesos\n";
        cout << "2. Planificador de CPU\n";
        cout << "3. Gestor de Memoria (Pila)\n";
        cout << "4. Guardar\n";
        cout << "0. Salir\n";
        opcion = leerEntero("Seleccione una opcion: ");

        switch (opcion) {
            case 1: menuProcesos(); break;
            case 2: menuPlanificador(); break;
            case 3: menuMemoria(); break;
            case 4: menuArchivos(); break;
            case 0: cout << "Saliendo del sistema...\n"; break;
            default: cout << "Opcion incorrecta. Intente nuevamente.\n";
        }
    } while (opcion != 0);

    // Antes de cerrar, se libera cada nodo reservado dinamicamente con new.
    limpiarEstructuras();
    return 0;
}