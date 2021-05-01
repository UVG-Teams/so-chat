/*
    Autores:
        Francisco Rosal
        Gian Luca Rivera
*/

#include <string>
#include <iostream>

using namespace std;

int main() {
    int choice;
    do {
        cout << "\n1. Chat con todos los usuarios" << endl 
             << "2. Chat privado" << endl
             << "3. Cambiar estado" << endl
             << "4. Usuarios conectados" << endl
             << "5. Información de un usuario" << endl
             << "6. Ayuda" << endl
             << "7. Salir\n" << endl;
        cout << "Ingresa una opción: " << endl;
        cin >> choice;

        switch(choice){
            case 1:
                cout << "Opcion 1\n" << endl;
                break;
            case 2:
                cout << "Opcion 2\n" << endl;
                break;
            case 3:
                cout << "Opcion 3\n" << endl;
                break;
            case 4:
                cout << "Opcion 4\n" << endl;
                break;
            case 5:
                cout << "Opcion 5\n" << endl;
                break;
            case 6:
                cout << "Opcion 6\n" << endl;
                break;
            case 7:
                cout << "Adios :)" << endl;
                return 0;
        }
    }while(choice != 7);
}