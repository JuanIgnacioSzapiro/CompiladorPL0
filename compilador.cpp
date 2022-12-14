#include <iostream>
#include <fstream>
#include <vector>
#include<string.h>
#include <cctype>
#include <cstdlib>
#include <string>
#include <conio.h>
using std::cout; using std::cerr;
using std::endl; using std::string;
using std::ifstream; using std::vector;
struct leido{
    string tipoDato;
    string datoLeido;
};
struct datosTablaSemantica{
    string nombre;
    string tipo;
    int valor;//para variables debe tener un numero unico que represente su ubicacion
};
const int MAXTABLA=128;
const int POSCOMIENZOTEXT=512;
const int SIZEOFIMAGE = 240;
const int BASEOFDATA = 208;
const string PR_NULL = "NULL";
const string PR_EOF = "EOF";
const string PR_BEGIN = "BEGIN";
const string PR_CALL = "CALL";
const string PR_DO = "DO";
const string PR_CONST = "CONST";
const string PR_END= "END";
const string PR_IF = "IF";
const string PR_ODD = "ODD";
const string PR_READLN = "READLN";
const string PR_PROCEDURE = "PROCEDURE";
const string PR_THEN = "THEN";
const string PR_VAR = "VAR";
const string PR_WHILE = "WHILE";
const string PR_WRITE = "WRITE";
const string PR_WRITELN = "WRITELN";
const string PR_CADENA = "Cadena";
const string PR_NUMERO = "Número";
const string PR_IDENTIFICADOR = "Identificador";
typedef leido informacion;
typedef datosTablaSemantica tabla[MAXTABLA];
string devolverTipoCadena(string);
leido analizadorLexico(FILE *, int &);
bool esEOF(char c);
bool esPalabraReservada(string);
bool terminacionCadena(char, int &);
bool esMultichar(char);
bool esNoLegible(char, int &);
char casteoIntChar(int);
string cadenaToMayusculas(string);
leido fabricarLeido(string, string);
void analizadorSintactico(FILE *);
void programa(FILE *, informacion &, int &);
void bloque(FILE *, informacion &, tabla , int, unsigned char [], int &, int &, int &);
void proposicion(FILE *, informacion &, tabla , int, unsigned char [], int &, int, int &);
void condicion(FILE *, informacion &, tabla , int, unsigned char [], int &, int, int &, int &);
void expresion(FILE *, informacion &, tabla , int, unsigned char [], int &, int, int &);
void termino(FILE *, informacion &, tabla , int, unsigned char [], int &, int, int &);
void factor(FILE *, informacion &, tabla , int, unsigned char [], int &, int, int &);
void cargarTabla(FILE *, datosTablaSemantica, tabla , int, int, int);
int buscadorSemantico(datosTablaSemantica, tabla , int, int);
void inicializarDatosTabalaSemantica(datosTablaSemantica &);
void generarParteLongitudFija(unsigned char[]);
void cargar32Impura(unsigned char [], int &, int);
void cargar32Pura(unsigned char [], int, int);
void fixUp(unsigned char [], int, int);
void movEdi(unsigned char [], int &, int);
void movEaxEdi(unsigned char [], int &, int);
void movEdiEax(unsigned char [], int &, int);
void movEax(unsigned char [], int &, int);
void xchgEaxEbx(unsigned char [], int &);
void pushEax(unsigned char [], int &);
void popEax(unsigned char [], int &);
void popEbx(unsigned char [], int &);
void addEaxEbx(unsigned char [], int &);
void subEaxEbx(unsigned char [], int &);
void imulEbx(unsigned char [], int &);
void idivEbx(unsigned char [], int &);
void cdq(unsigned char [], int &);
void negEax(unsigned char [], int &);
void testAl(unsigned char [], int &);
void cmpEbxEax(unsigned char [], int &);
void je_jz(unsigned char [], int &);
void jne_jnz(unsigned char [], int &);
void jg(unsigned char [], int &);
void jge(unsigned char [], int &);
void jl(unsigned char [], int &);
void jle(unsigned char [], int &);
void jpo(unsigned char [], int &);
void jmp(unsigned char [], int &, int);
void call(unsigned char [], int &, int);
void ret(unsigned char [], int &);
void errores(FILE *, int, string, informacion, int);
/**
 * @brief 
 * 
 * @return int 
 */
int main(){
    string filename("BIEN-09.PL0");
    FILE* archivo = fopen(filename.c_str(), "r");
    if (archivo == nullptr){
        cout<<"CERROR EN APERTURA DE ARCHIVO"<<endl;
        return "\nERROR EN APERTURA DE ARCHIVO: ",EXIT_FAILURE;
    }
    cout<<"COMIENZO DEL PROGRAMA"<<endl;
    analizadorSintactico(archivo);
    return EXIT_SUCCESS;
}
/**
 * @brief 
 * 
 * @param c 
 * @return true 
 * @return false 
 */
bool esEOF(char c){
    return c=='\0'||c==-1;
}
/**
 * @brief 
 * 
 * @param arch 
 * @param lineaError 
 * @return leido 
 */
leido analizadorLexico(FILE *arch, int &lineaError){
    char charTok;
    string cadena;
    bool punt=false, espacio=false, eof=false;
    cadena="";
    do{
        charTok=casteoIntChar(fgetc(arch));
    }while(esNoLegible(charTok, lineaError));
    if(ispunct(charTok)){
        if(charTok=='\''){
            cadena+=charTok;
            do{
                charTok=casteoIntChar(fgetc(arch));
                if(!terminacionCadena(charTok, lineaError)){
                    cadena+=charTok;
                }
            }while(!terminacionCadena(charTok, lineaError));
            if(charTok=='\''){
                cadena+=charTok;
            }
            else{
                fseek(arch, -1, SEEK_CUR);
                return fabricarLeido(cadena, PR_NULL);
            }
        }
        else if(esMultichar(charTok)){
            switch(charTok){
                case ':':{
                    if(casteoIntChar(fgetc(arch))=='='){
                        return fabricarLeido(":=", "Símbolo multichar ':='");
                    }
                    else{
                        fseek(arch, -1, SEEK_CUR);
                        return fabricarLeido(cadena, PR_NULL);
                    }                
                    break;
                }
                case '<':{
                    if(casteoIntChar(fgetc(arch))=='='){
                        return fabricarLeido("<=", "Símbolo multichar '<='");
                    }
                    else {
                        fseek(arch, -1, SEEK_CUR);
                        if(casteoIntChar(fgetc(arch))=='>'){
                            return fabricarLeido("<>", "Símbolo multichar '<>'");
                        }
                        else{
                            fseek(arch, -1, SEEK_CUR);
                            return fabricarLeido("<", "Símbolo unichar '<'");                        
                        }
                    }                
                    break;
                }
                case '>':{
                    if(casteoIntChar(fgetc(arch))=='='){
                        return fabricarLeido(">=", "Símbolo multichar '>='");
                    }
                    else{
                        fseek(arch, -1, SEEK_CUR);
                        return fabricarLeido(">", "Símbolo unichar '>'");                    
                    }
                    break;
                }
            }
        }
        else{
            cadena+=charTok;
            return fabricarLeido(cadena, "Símbolo unichar '"+cadena+"'");
        }        
    }
    else if(isalpha(charTok)){
        cadena+=charTok;
        while(isalnum(casteoIntChar(fgetc(arch)))){
            fseek(arch, -1, SEEK_CUR);
            charTok=casteoIntChar(fgetc(arch));
            cadena+=charTok;
        }
        fseek(arch, -1, SEEK_CUR);
    }
    else if(isdigit(charTok)){
        cadena+=charTok;
        while(isdigit(casteoIntChar(fgetc(arch)))){
            fseek(arch, -1, SEEK_CUR);
            charTok=casteoIntChar(fgetc(arch));
            cadena+=charTok;
        }
        fseek(arch, -1, SEEK_CUR);
    }
    else if(charTok==EOF||charTok==-1||charTok=='\0'||charTok=='\000'){
        return fabricarLeido(cadena, PR_EOF);        
    }
    else{
        return fabricarLeido(cadena, PR_NULL);
    }
    return fabricarLeido(cadena, devolverTipoCadena(cadena));
}
/**
 * @brief 
 * 
 * @param c 
 * @return true 
 * @return false 
 */
bool esMultichar(char c){
    return c==':'||c=='<'||c=='>';
}
/**
 * @brief 
 * 
 * @param cad 
 * @return string 
 */
string devolverTipoCadena(string cad){
    if(esPalabraReservada(cad)){
        return cadenaToMayusculas(cad);
    }
    else if(cad[0]=='\"'||cad[0]=='\''){
        return PR_CADENA;
    }
    else if(isdigit(cad[0])){
        return PR_NUMERO;
    }
    else{
        return PR_IDENTIFICADOR;
    }
}
/**
 * @brief 
 * 
 * @param cad 
 * @return true 
 * @return false 
 */
bool esPalabraReservada(string cad){
    int contador=0;
    string palabrasReservadas[]={PR_BEGIN, PR_CALL, PR_DO, PR_CONST, PR_END, PR_IF, PR_ODD, PR_READLN, PR_PROCEDURE, PR_THEN, PR_VAR, PR_WHILE, PR_WRITE, PR_WRITELN};
    string cadUpper;
    do{
        cadUpper+=toupper(cad[contador]);
        contador++;
    }while(contador<cad.length());
    contador=0;
    do{
        if(cadUpper.compare(palabrasReservadas[contador])==0){
            return true;
        }
        contador++;
    }while(contador<(sizeof(palabrasReservadas)/sizeof(string)));
    return false;
}
/**
 * @brief 
 * 
 * @param c 
 * @param lineaError 
 * @return true 
 * @return false 
 */
bool terminacionCadena(char c, int &lineaError){
    if(c=='\n'){
        lineaError++;
    }
    return c=='\''||c==EOF||c=='\n';
}
/**
 * @brief 
 * 
 * @param c 
 * @param lineaError 
 * @return true 
 * @return false 
 */
bool esNoLegible(char c, int &lineaError){
    if(c=='\n'){
        lineaError++;
    }
    return c=='\n'||c==' '||c=='\t';
}
/**
 * @brief 
 * 
 * @param c 
 * @return char 
 */
char casteoIntChar(int c){
    return char(c);
}
/**
 * @brief 
 * 
 * @param cadena 
 * @return string 
 */
string cadenaToMayusculas(string cadena){
    int contador=0;
    string cadenaCopia="";
    do{
        cadenaCopia+=toupper(cadena[contador]);
        contador++;
    }while(contador<cadena.length());
    return cadenaCopia;
}
/**
 * @brief 
 * 
 * @param cadena 
 * @param tipoCadena 
 * @return leido 
 */
leido fabricarLeido(string cadena, string tipoCadena){
    informacion datos;
    datos.datoLeido=cadena;
    datos.tipoDato=tipoCadena;
    return datos;
}
/**
 * @brief 
 * 
 * @param archivo 
 */
void analizadorSintactico(FILE *archivo){
    informacion datos;
    int lineaError=1;
    programa(archivo, datos, lineaError);
    if(datos.tipoDato==PR_EOF){
        errores(archivo, 0, "", datos, lineaError);
    }
    else{
        errores(archivo, 2, "", datos, lineaError);
    }
}
/**
 * @brief 
 * 
 * @param archivo 
 * @param datos 
 * @param lineaError 
 */
void programa(FILE *archivo, informacion &datos, int &lineaError){
    unsigned char memoria[8192];
    tabla tablaSemantica={"","",0};
    datos=analizadorLexico(archivo, lineaError);
    generarParteLongitudFija(memoria);
    int contadorMemoria=1792, contadorVariables=0;
    movEdi(memoria, contadorMemoria, 0);
    bloque(archivo, datos, tablaSemantica, 0, memoria, contadorMemoria, contadorVariables, lineaError);
    if(datos.datoLeido=="."){
        datos=analizadorLexico(archivo, lineaError);
    }
    else{
        errores(archivo, 1, "", datos, lineaError);
    }
    int indiceNegativoFinalizacionPrograma=1416-(contadorMemoria+5);
    jmp(memoria, contadorMemoria, indiceNegativoFinalizacionPrograma);
    fixUp(memoria, 1793, contadorMemoria-0x200+0x401000);
    for(int i=0; i<contadorVariables; i++){ 
        memoria[contadorMemoria++]=0x00;
        memoria[contadorMemoria++]=0x00;
        memoria[contadorMemoria++]=0x00;
        memoria[contadorMemoria++]=0x00;
    }
    int virtualSize=contadorMemoria-POSCOMIENZOTEXT; 
    fixUp(memoria, 416, virtualSize);
    int fileAligment=POSCOMIENZOTEXT;
    while((contadorMemoria%fileAligment)!=0){
        memoria[contadorMemoria++]=0x00;
    }
    int sizeOfCodeSection=contadorMemoria-POSCOMIENZOTEXT;
    fixUp(memoria, 188, sizeOfCodeSection);
    int sizeOfRawData=contadorMemoria-POSCOMIENZOTEXT;
    fixUp(memoria, 424, sizeOfRawData);
    int sectionAlignment=4096;
    fixUp(memoria, SIZEOFIMAGE, (2+sizeOfCodeSection/sectionAlignment)*sectionAlignment);
    fixUp(memoria, BASEOFDATA,(2+sizeOfRawData/sectionAlignment)*sectionAlignment);
    FILE * archivoExe;
    archivoExe=fopen("Resultado.exe", "w+b");
    fwrite(memoria, sizeof(unsigned char), contadorMemoria, archivoExe);
    fclose(archivoExe);
}
/**
 * @brief 
 * 
 * @param archivo 
 * @param datos 
 * @param tablaSemantica 
 * @param base 
 * @param memoria 
 * @param contadorMemoria 
 * @param contadorVariable 
 * @param lineaError 
 */
void bloque(FILE *archivo, informacion &datos, tabla tablaSemantica, int base, unsigned char memoria[], int &contadorMemoria, int &contadorVariable, int &lineaError){
    jmp(memoria, contadorMemoria, 0);
    int posMemSaltoTo1raProp=contadorMemoria;
    int desplazamiento=0;
    datosTablaSemantica datosSemanticos;
    inicializarDatosTabalaSemantica(datosSemanticos);
    int valorBuscado;
    if(datos.tipoDato==PR_CONST){
        datosSemanticos.tipo=datos.tipoDato;
        datos=analizadorLexico(archivo, lineaError);
        if(datos.tipoDato==PR_IDENTIFICADOR){
            datosSemanticos.nombre=datos.datoLeido;
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, PR_IDENTIFICADOR, datos, lineaError);
        }
        if(datos.datoLeido=="="){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar '='", datos, lineaError);
        }
        if(datos.tipoDato==PR_NUMERO){
            datosSemanticos.valor=std::stoi(datos.datoLeido.c_str());
            valorBuscado=buscadorSemantico(datosSemanticos, tablaSemantica, base, base+desplazamiento-1);
            if(valorBuscado==-1){
                cargarTabla(archivo, datosSemanticos, tablaSemantica, base, desplazamiento++, lineaError);
            }
            else{
                datos.datoLeido=datosSemanticos.nombre;
                datos.tipoDato=datosSemanticos.tipo;
                errores(archivo, 4, "", datos, lineaError);
            }
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, PR_NUMERO, datos, lineaError);
        }
        while(datos.datoLeido==","){
            datos=analizadorLexico(archivo, lineaError);
            if(datos.tipoDato==PR_IDENTIFICADOR){
                datosSemanticos.nombre=datos.datoLeido;
                datos=analizadorLexico(archivo, lineaError);
            }
            else{
                errores(archivo, 3, PR_IDENTIFICADOR, datos, lineaError);
            }
            if(datos.datoLeido=="="){
                datos=analizadorLexico(archivo, lineaError);
            }
            else{
                errores(archivo, 3, "Símbolo unichar '='", datos, lineaError);
            }
            if(datos.tipoDato==PR_NUMERO){
                datosSemanticos.valor=std::stoi(datos.datoLeido.c_str());
                valorBuscado=buscadorSemantico(datosSemanticos, tablaSemantica, base, base+desplazamiento-1);
                if(valorBuscado==-1){
                    cargarTabla(archivo, datosSemanticos, tablaSemantica, base, desplazamiento++, lineaError);
                }
                else{
                    datos.datoLeido=datosSemanticos.nombre;
                    datos.tipoDato=datosSemanticos.tipo;
                    errores(archivo, 4, "", datos, lineaError);
                }
                datos=analizadorLexico(archivo, lineaError);                
            }
            else{
                errores(archivo, 3, PR_NUMERO, datos, lineaError);
            }
        }
        if(datos.datoLeido==";"){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar ';'", datos, lineaError);
        }
    }
    if(datos.tipoDato==PR_VAR){
        datosSemanticos.tipo=datos.tipoDato;
        datos=analizadorLexico(archivo, lineaError);
        if(datos.tipoDato==PR_IDENTIFICADOR){
            datosSemanticos.nombre=datos.datoLeido;
            if(buscadorSemantico(datosSemanticos, tablaSemantica, base, base+desplazamiento-1)==-1){
                datosSemanticos.valor=contadorVariable*4;
                cargarTabla(archivo, datosSemanticos, tablaSemantica, base, desplazamiento++, lineaError);
                contadorVariable++;
            }
            else{
                datos.datoLeido=datosSemanticos.nombre;
                datos.tipoDato=datosSemanticos.tipo;
                errores(archivo, 4, "", datos, lineaError);
            }
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, PR_IDENTIFICADOR, datos, lineaError);
        }
        while(datos.datoLeido==","){
            datos=analizadorLexico(archivo, lineaError);
            if(datos.tipoDato==PR_IDENTIFICADOR){
                datosSemanticos.nombre=datos.datoLeido;
                if(buscadorSemantico(datosSemanticos, tablaSemantica, base, base+desplazamiento-1)==-1){
                    datosSemanticos.valor=contadorVariable*4;
                    cargarTabla(archivo, datosSemanticos, tablaSemantica, base, desplazamiento++, lineaError);
                    contadorVariable++;
                }
                else{
                    datos.datoLeido=datosSemanticos.nombre;
                    datos.tipoDato=datosSemanticos.tipo;
                    errores(archivo, 4, "", datos, lineaError);
                }
                datos=analizadorLexico(archivo, lineaError);
            }
            else{
                errores(archivo, 3, PR_IDENTIFICADOR, datos, lineaError);
            }
        }
        if(datos.datoLeido==";"){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar ';' o símbolo unichar ','", datos, lineaError);
        }
    }
    while(datos.tipoDato==PR_PROCEDURE){
        datosSemanticos.valor=contadorMemoria;
        datosSemanticos.tipo=datos.tipoDato;
        datos=analizadorLexico(archivo, lineaError);
        if(datos.tipoDato==PR_IDENTIFICADOR){
            datosSemanticos.nombre=datos.datoLeido;
            if(buscadorSemantico(datosSemanticos, tablaSemantica, base, base+desplazamiento-1)==-1){
                cargarTabla(archivo, datosSemanticos, tablaSemantica, base, desplazamiento++, lineaError);
            }
            else{
                datos.datoLeido=datosSemanticos.nombre;
                datos.tipoDato=datosSemanticos.tipo;
                errores(archivo, 4, "", datos, lineaError);
            }
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, PR_IDENTIFICADOR, datos, lineaError);
        }
        if(datos.datoLeido==";"){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar ';'", datos, lineaError);
        }
        bloque(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        ret(memoria, contadorMemoria);
        if(datos.datoLeido==";"){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar ';'", datos, lineaError);
        }
    }
    fixUp(memoria, posMemSaltoTo1raProp-4, contadorMemoria-posMemSaltoTo1raProp);
    proposicion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
}
/**
 * @brief 
 * 
 * @param archivo 
 * @param datos 
 * @param tablaSemantica 
 * @param base 
 * @param memoria 
 * @param contadorMemoria 
 * @param contadorVariable 
 * @param lineaError 
 */
void proposicion(FILE *archivo, informacion &datos, tabla tablaSemantica, int base, unsigned char memoria[], int &contadorMemoria, int contadorVariable, int &lineaError){
    int desplazamiento=0;
    datosTablaSemantica datosSemanticos;
    int valorBusqueda;
    int posicionParaFixUp=0;
    if(datos.tipoDato==PR_IDENTIFICADOR){
        datosSemanticos.nombre=datos.datoLeido;
        valorBusqueda=buscadorSemantico(datosSemanticos, tablaSemantica, 0, base+desplazamiento-1);
        if(valorBusqueda==-1){
            errores(archivo, 5, "", datos, lineaError);
        }
        if(tablaSemantica[valorBusqueda].tipo!=PR_VAR){
            errores(archivo, 3, PR_VAR, datos, lineaError);
        }
        datos=analizadorLexico(archivo, lineaError);
        if(datos.datoLeido==":="){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo multichar ':='", datos, lineaError);
        }
        expresion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        popEax(memoria, contadorMemoria);
        movEdiEax(memoria, contadorMemoria, tablaSemantica[valorBusqueda].valor);
    }
    else if(datos.tipoDato==PR_CALL){
        datos=analizadorLexico(archivo, lineaError);
        if(datos.tipoDato==PR_IDENTIFICADOR){
            datosSemanticos.nombre=datos.datoLeido;
            valorBusqueda=buscadorSemantico(datosSemanticos, tablaSemantica, 0, base+desplazamiento-1);
            if(valorBusqueda==-1){
                errores(archivo, 5, "", datos, lineaError);
            }
            if(tablaSemantica[valorBusqueda].tipo!=PR_PROCEDURE){
                errores(archivo, 3, PR_PROCEDURE, datos, lineaError);
            }
            call(memoria, contadorMemoria,tablaSemantica[valorBusqueda].valor-(contadorMemoria+5));
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, PR_IDENTIFICADOR, datos, lineaError);
        }
    }
    else if(datos.tipoDato==PR_BEGIN){
        datos=analizadorLexico(archivo, lineaError);
        proposicion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        while(datos.datoLeido==";"){
            datos=analizadorLexico(archivo, lineaError);
            proposicion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        }
        if(datos.tipoDato==PR_END){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "END o un símbolo unichar ';'", datos, lineaError);
        }
    }
    else if(datos.tipoDato==PR_IF){
        datos=analizadorLexico(archivo, lineaError);
        condicion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError, posicionParaFixUp);
        if(datos.tipoDato==PR_THEN){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, PR_THEN, datos, lineaError);
        }
        proposicion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        fixUp(memoria, posicionParaFixUp-4, contadorMemoria-posicionParaFixUp);
    }
    else if(datos.tipoDato==PR_WHILE){
        int posADondeSaltar=contadorMemoria;
        datos=analizadorLexico(archivo, lineaError);      
        condicion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError, posicionParaFixUp);
        if(datos.tipoDato==PR_DO){            
            datos=analizadorLexico(archivo, lineaError);
            proposicion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
            jmp(memoria, contadorMemoria, posADondeSaltar-contadorMemoria-5);
            fixUp(memoria, posicionParaFixUp-4, contadorMemoria-posicionParaFixUp);
        }
        else{
            errores(archivo, 3, PR_DO, datos, lineaError);
        }
    }
    else if(datos.tipoDato==PR_READLN){
        datos=analizadorLexico(archivo, lineaError);
        if(datos.datoLeido=="("){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar '('", datos, lineaError);
        }
        if(datos.tipoDato==PR_IDENTIFICADOR){
            datosSemanticos.nombre=datos.datoLeido;
            valorBusqueda=buscadorSemantico(datosSemanticos, tablaSemantica, 0, base+desplazamiento-1);
            if(tablaSemantica[valorBusqueda].tipo!=PR_CONST&&tablaSemantica[valorBusqueda].tipo!=PR_VAR){
                errores(archivo, 3, "CONST o VAR", datos, lineaError);
            }
            if(valorBusqueda==-1){
                errores(archivo, 5, "", datos, lineaError);
            }
            call(memoria, contadorMemoria, 1424- (contadorMemoria+5));
            movEdiEax(memoria, contadorMemoria, tablaSemantica[valorBusqueda].valor);
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, PR_IDENTIFICADOR, datos, lineaError);
        }
        while(datos.datoLeido==","){
            datos=analizadorLexico(archivo, lineaError);
            if(datos.tipoDato==PR_IDENTIFICADOR){
                datosSemanticos.nombre=datos.datoLeido;
                valorBusqueda=buscadorSemantico(datosSemanticos, tablaSemantica, 0, base+desplazamiento-1);
                if(valorBusqueda==-1){
                    errores(archivo, 5, "", datos, lineaError);
                }
                if(tablaSemantica[valorBusqueda].tipo!=PR_CONST&&tablaSemantica[valorBusqueda].tipo!=PR_VAR){
                    errores(archivo, 3, "CONST o VAR", datos, lineaError);
                }
                if(valorBusqueda==-1){
                    errores(archivo, 5, "", datos, lineaError);
                }
                call(memoria, contadorMemoria, 1424- (contadorMemoria+5));
                movEdiEax(memoria, contadorMemoria, tablaSemantica[valorBusqueda].valor);
                datos=analizadorLexico(archivo, lineaError);
            }
            else{
                errores(archivo, 3, PR_IDENTIFICADOR, datos, lineaError);
            }
        }
        if(datos.datoLeido==")"){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar ')'", datos, lineaError);
        }
    }
    else if(datos.tipoDato==PR_WRITELN){
        datos=analizadorLexico(archivo, lineaError);
        if(datos.datoLeido=="("){
            datos=analizadorLexico(archivo, lineaError);
            if(datos.tipoDato==PR_CADENA){
                int largoCadena=datos.datoLeido.length()-1;
                int posAbsCadena=contadorMemoria+15-0x200+0x401000;
                movEax(memoria, contadorMemoria, posAbsCadena);
                int distanciaNegativa= 992-(contadorMemoria+5);
                call(memoria, contadorMemoria, distanciaNegativa);
                jmp(memoria, contadorMemoria, largoCadena);
                for(int i=1; i<datos.datoLeido.length()-1; i++){
                    memoria[contadorMemoria++]=datos.datoLeido.at(i);
                }
                memoria[contadorMemoria++]=0x00;
                datos=analizadorLexico(archivo, lineaError);
            }
            else{
                expresion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);                
                popEax(memoria, contadorMemoria);
                int distanciaNegativa= 1056-(contadorMemoria+5);
                call(memoria, contadorMemoria,distanciaNegativa);
            }
            while(datos.datoLeido==","){
                datos=analizadorLexico(archivo, lineaError);
                if(datos.tipoDato==PR_CADENA){
                    int largoCadena=datos.datoLeido.length()-1;
                    int posAbsCadena=contadorMemoria+15-0x200+0x401000;
                    movEax(memoria, contadorMemoria, posAbsCadena);
                    int distanciaNegativa= 992-(contadorMemoria+5);
                    call(memoria, contadorMemoria,distanciaNegativa);
                    jmp(memoria, contadorMemoria,largoCadena);
                    for(int i=1; i<datos.datoLeido.length()-1; i++){
                        memoria[contadorMemoria++]=datos.datoLeido.at(i);
                    }
                    memoria[contadorMemoria++]=0x00;
                    datos=analizadorLexico(archivo, lineaError);
                }
                else{
                    expresion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);                
                    popEax(memoria, contadorMemoria);
                    int distanciaNegativa= 1056-(contadorMemoria+5);
                    call(memoria, contadorMemoria,distanciaNegativa);
                }
            }
            if(datos.datoLeido==")"){
                datos=analizadorLexico(archivo, lineaError);
            }
            else{
                errores(archivo, 3, "Símbolo unichar ')'", datos, lineaError);
            }
        }
        int posSaltoDeLinea=1040-(contadorMemoria+5); 
        call(memoria, contadorMemoria, posSaltoDeLinea);
    }
    else if(datos.tipoDato==PR_WRITE){
        datos=analizadorLexico(archivo, lineaError);
        if(datos.datoLeido=="("){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar '('", datos, lineaError);
        }
        if(datos.tipoDato==PR_CADENA){
            int largoCadena=datos.datoLeido.length()-1;
            int posAbsCadena=contadorMemoria+15-0x200+0x401000;
            movEax(memoria, contadorMemoria, posAbsCadena);
            int distanciaNegativa= 992-(contadorMemoria+5);
            call(memoria, contadorMemoria,distanciaNegativa);
            jmp(memoria, contadorMemoria,largoCadena);
            for(int i=1; i<datos.datoLeido.length()-1; i++){
                memoria[contadorMemoria++]=datos.datoLeido.at(i);
            }
            memoria[contadorMemoria++]=0x00;
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            expresion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);                
            popEax(memoria, contadorMemoria);
            int distanciaNegativa= 1056-(contadorMemoria+5);
            call(memoria, contadorMemoria,distanciaNegativa);
        }
        while(datos.datoLeido==","){
            datos=analizadorLexico(archivo, lineaError);
            if(datos.tipoDato==PR_CADENA){
                int largoCadena=datos.datoLeido.length()-1;
                int posAbsCadena=contadorMemoria+15-0x200+0x401000;
                movEax(memoria, contadorMemoria, posAbsCadena);
                int distanciaNegativa= 992-(contadorMemoria+5);
                call(memoria, contadorMemoria,distanciaNegativa);
                jmp(memoria, contadorMemoria, largoCadena);
                for(int i=1; i<datos.datoLeido.length()-1; i++){
                    memoria[contadorMemoria++]=datos.datoLeido.at(i);
                }
                memoria[contadorMemoria++]=0x00;
                datos=analizadorLexico(archivo, lineaError);
            }
            else{
                expresion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);                
                popEax(memoria, contadorMemoria);
                int distanciaNegativa= 1056-(contadorMemoria+5);
                call(memoria, contadorMemoria,distanciaNegativa);
            }
        }
        if(datos.datoLeido==")"){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar ')'", datos, lineaError);
        }
    }
}
/**
 * @brief 
 * 
 * @param archivo 
 * @param datos 
 * @param tablaSemantica 
 * @param base 
 * @param memoria 
 * @param contadorMemoria 
 * @param contadorVariable 
 * @param lineaError 
 * @param posicionParaFixUp 
 */
void condicion(FILE *archivo, informacion &datos, tabla tablaSemantica, int base, unsigned char memoria[], int &contadorMemoria, int contadorVariable, int &lineaError, int &posicionParaFixUp){
    int desplazamiento=0;
    if(datos.tipoDato==PR_ODD){
        datos=analizadorLexico(archivo, lineaError);
        expresion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        popEax(memoria, contadorMemoria);
        testAl(memoria, contadorMemoria);
        memoria[contadorMemoria++]=0x01;
        jpo(memoria, contadorMemoria);
        memoria[contadorMemoria++]=0x05;
        jmp(memoria, contadorMemoria, 0);
        posicionParaFixUp=contadorMemoria;
    }
    else{
        expresion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        unsigned char queFue;
        if(datos.datoLeido=="="){
            queFue=0x74;
            datos=analizadorLexico(archivo, lineaError);
        }
        else if(datos.datoLeido=="<>"){
            queFue=0x75;
            datos=analizadorLexico(archivo, lineaError);
        }
        else if(datos.datoLeido=="<"){
            queFue=0x7C;
            datos=analizadorLexico(archivo, lineaError);
        }
        else if(datos.datoLeido=="<="){
            queFue=0x7E;
            datos=analizadorLexico(archivo, lineaError);
        }
        else if(datos.datoLeido==">"){
            queFue=0x7F;
            datos=analizadorLexico(archivo, lineaError);
        }
        else if(datos.datoLeido==">="){
            queFue=0x7D;
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolos unichar '=', Símbolos unichar '<', Símbolos unichar '>', símbolos multichar '<>', símbolos multichar '<=' o símbolos multichar '>='", datos, lineaError);
        }
        expresion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        popEax(memoria, contadorMemoria);
        popEbx(memoria, contadorMemoria);
        cmpEbxEax(memoria, contadorMemoria);
        memoria[contadorMemoria++]=queFue;
        memoria[contadorMemoria++]=0x05;
        jmp(memoria, contadorMemoria, 0);
        posicionParaFixUp=contadorMemoria;
    }
}
/**
 * @brief 
 * 
 * @param archivo 
 * @param datos 
 * @param tablaSemantica 
 * @param base 
 * @param memoria 
 * @param contadorMemoria 
 * @param contadorVariable 
 * @param lineaError 
 */
void expresion(FILE *archivo, informacion &datos, tabla tablaSemantica, int base, unsigned char memoria[], int &contadorMemoria, int contadorVariable, int &lineaError){
    int desplazamiento=0;
    char queFue=datos.datoLeido[0];
    if(datos.datoLeido=="+"){
        datos=analizadorLexico(archivo, lineaError);
    }
    if(datos.datoLeido=="-"){
        datos=analizadorLexico(archivo, lineaError);
    }
    termino(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
    if(queFue=='-'){
        popEax(memoria, contadorMemoria);
        negEax(memoria, contadorMemoria);
        pushEax(memoria, contadorMemoria);
    }
    while(datos.datoLeido=="+"||datos.datoLeido=="-"){
        queFue=datos.datoLeido[0];
        datos=analizadorLexico(archivo, lineaError);
        termino(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        if(queFue=='-'){
            popEax(memoria, contadorMemoria);
            popEbx(memoria, contadorMemoria);
            xchgEaxEbx(memoria, contadorMemoria);
            subEaxEbx(memoria, contadorMemoria);
            pushEax(memoria, contadorMemoria);
        }
        else{//=='+'
            popEax(memoria, contadorMemoria);
            popEbx(memoria, contadorMemoria);
            addEaxEbx(memoria, contadorMemoria);
            pushEax(memoria, contadorMemoria);
        }
    }
}
/**
 * @brief 
 * 
 * @param archivo 
 * @param datos 
 * @param tablaSemantica 
 * @param base 
 * @param memoria 
 * @param contadorMemoria 
 * @param contadorVariable 
 * @param lineaError 
 */
void termino(FILE *archivo, informacion &datos, tabla tablaSemantica, int base, unsigned char memoria[], int &contadorMemoria, int contadorVariable, int &lineaError){
    int desplazamiento=0;
    datosTablaSemantica datosSemanticos;
    factor(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
    while(datos.datoLeido=="*"||datos.datoLeido=="/"){
        char queFue=datos.datoLeido[0];
        if(datos.datoLeido=="*"){
        }
        else{ // =="/"
        }
        datos=analizadorLexico(archivo, lineaError);
        factor(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        if(queFue=='*'){
            popEax(memoria, contadorMemoria);
            popEbx(memoria, contadorMemoria);
            imulEbx(memoria, contadorMemoria);
            pushEax(memoria, contadorMemoria);
        }//=="/"
        else{
            popEax(memoria, contadorMemoria);
            popEbx(memoria, contadorMemoria);
            xchgEaxEbx(memoria, contadorMemoria);
            cdq(memoria, contadorMemoria);
            idivEbx(memoria, contadorMemoria);
            pushEax(memoria, contadorMemoria);
        }
    }
}
/**
 * @brief 
 * 
 * @param archivo 
 * @param datos 
 * @param tablaSemantica 
 * @param base 
 * @param memoria 
 * @param contadorMemoria 
 * @param contadorVariable 
 * @param lineaError 
 */
void factor(FILE *archivo, informacion &datos, tabla tablaSemantica, int base, unsigned char memoria[], int &contadorMemoria, int contadorVariable, int &lineaError){
    int desplazamiento=0;
    datosTablaSemantica datosSemanticos;
    datosSemanticos.nombre=datos.datoLeido;
    if(datos.tipoDato==PR_IDENTIFICADOR){
        int valorBusqueda=buscadorSemantico(datosSemanticos, tablaSemantica, 0, base+desplazamiento-1);
        if(valorBusqueda==-1){
            errores(archivo, 5, "", datos, lineaError);
        } 
        if(tablaSemantica[valorBusqueda].tipo!=PR_CONST&&tablaSemantica[valorBusqueda].tipo!=PR_VAR){
            errores(archivo, 3, "CONST o VAR", datos, lineaError);
        }
        else if(tablaSemantica[valorBusqueda].tipo==PR_CONST){
            movEax(memoria, contadorMemoria, tablaSemantica[valorBusqueda].valor);
            pushEax(memoria, contadorMemoria);
        }
        else{//=='VAR'
            movEaxEdi(memoria, contadorMemoria, tablaSemantica[valorBusqueda].valor);
            pushEax(memoria, contadorMemoria);
        }
        datos=analizadorLexico(archivo, lineaError);
    }
    else if(datos.tipoDato==PR_NUMERO){
        movEax(memoria, contadorMemoria, stoi(datos.datoLeido));
        pushEax(memoria, contadorMemoria);
        datos=analizadorLexico(archivo, lineaError);
    }
    else if(datos.datoLeido=="("){
        datos=analizadorLexico(archivo, lineaError);
        expresion(archivo, datos, tablaSemantica, base+desplazamiento, memoria, contadorMemoria, contadorVariable, lineaError);
        if(datos.datoLeido==")"){
            datos=analizadorLexico(archivo, lineaError);
        }
        else{
            errores(archivo, 3, "Símbolo unichar ')'", datos, lineaError);
        }
    }
    else{
        errores(archivo, 3, "Identificador, número, símbolo unichar '(' o cadena", datos, lineaError);
    }
}
/**
 * @brief 
 * 
 * @param archivo 
 * @param datos 
 * @param tablaSemantica 
 * @param base 
 * @param desplazamiento 
 * @param lineaError 
 */
void cargarTabla(FILE *archivo, datosTablaSemantica datos, tabla tablaSemantica, int base, int desplazamiento, int lineaError){
    if((base+desplazamiento)>=MAXTABLA){
        informacion x;
        errores(archivo, 6, "", x, lineaError);
    }
    tablaSemantica[base+desplazamiento].nombre=datos.nombre;
    tablaSemantica[base+desplazamiento].tipo=datos.tipo;
    tablaSemantica[base+desplazamiento].valor=datos.valor;
}
/**
 * @brief 
 * 
 * @param datos 
 * @param tablaSemantica 
 * @param base 
 * @param baseDesplazada 
 * @return int 
 */
int buscadorSemantico(datosTablaSemantica datos, tabla tablaSemantica, int base, int baseDesplazada){
    int contador;
    for(contador=baseDesplazada; contador>=base; contador--){
        if(tablaSemantica[contador].nombre==datos.nombre){
            return contador;
        }
    }
    return -1;
}
/**
 * @brief 
 * 
 * @param datosSemanticos 
 */
void inicializarDatosTabalaSemantica(datosTablaSemantica &datosSemanticos){
    datosSemanticos.nombre="";
    datosSemanticos.tipo="";
    datosSemanticos.valor=0;
}
/**
 * @brief 
 * 
 * @param memoria 
 */
void generarParteLongitudFija(unsigned char memoria[]){
    int contador=448;
    /* MS-DOS COMPATIBLE HEADER */
    memoria[0] = 0x4D; // 'M' (Magic number)
    memoria[1] = 0x5A; // 'Z'
    memoria[2] = 0x60; // Bytes on last block
    memoria[3] = 0x01; // (1 bl. = 512 bytes)
    memoria[4] = 0x01; // Number of blocks
    memoria[5] = 0x00; // in the EXE file
    memoria[6] = 0x00; // Number of re-
    memoria[7] = 0x00; // location entries
    memoria[8] = 0x04; // Size of header
    memoria[9] = 0x00; // (x 16 bytes)
    memoria[10] = 0x00; // Minimum extra
    memoria[11] = 0x00; // paragraphs needed
    memoria[12] = 0xFF; // Maximum extra
    memoria[13] = 0xFF; // paragraphs needed
    memoria[14] = 0x00; // Initial (relative)
    memoria[15] = 0x00; // SS value
    memoria[16] = 0x60; // Initial SP value
    memoria[17] = 0x01;
    memoria[18] = 0x00; // Checksum
    memoria[19] = 0x00;
    memoria[20] = 0x00; // Initial IP value
    memoria[21] = 0x00;
    memoria[22] = 0x00; // Initial (relative)
    memoria[23] = 0x00; // CS value
    memoria[24] = 0x40; // Offset of the 1st
    memoria[25] = 0x00; // relocation item
    memoria[26] = 0x00; // Overlay number.
    memoria[27] = 0x00; // (0 = main program)
    memoria[28] = 0x00; // Reserved word
    memoria[29] = 0x00;
    memoria[30] = 0x00; // Reserved word
    memoria[31] = 0x00;
    memoria[32] = 0x00; // Reserved word
    memoria[33] = 0x00;
    memoria[34] = 0x00; // Reserved word
    memoria[35] = 0x00;
    memoria[36] = 0x00; // OEM identifier
    memoria[37] = 0x00;
    memoria[38] = 0x00; // OEM information
    memoria[39] = 0x00;
    memoria[40] = 0x00; // Reserved word
    memoria[41] = 0x00;
    memoria[42] = 0x00; // Reserved word
    memoria[43] = 0x00;
    memoria[44] = 0x00; // Reserved word
    memoria[45] = 0x00;
    memoria[46] = 0x00; // Reserved word
    memoria[47] = 0x00;
    memoria[48] = 0x00; // Reserved word
    memoria[49] = 0x00;
    memoria[50] = 0x00; // Reserved word
    memoria[51] = 0x00;
    memoria[52] = 0x00; // Reserved word
    memoria[53] = 0x00;
    memoria[54] = 0x00; // Reserved word
    memoria[55] = 0x00;
    memoria[56] = 0x00; // Reserved word
    memoria[57] = 0x00;
    memoria[58] = 0x00; // Reserved word
    memoria[59] = 0x00;
    memoria[60] = 0xA0; // Start of the COFF
    memoria[61] = 0x00; // header
    memoria[62] = 0x00;
    memoria[63] = 0x00;
    memoria[64] = 0x0E; // PUSH CS
    memoria[65] = 0x1F; // POP DS
    memoria[66] = 0xBA; // MOV DX,000E
    memoria[67] = 0x0E;
    memoria[68] = 0x00;
    memoria[69] = 0xB4; // MOV AH,09
    memoria[70] = 0x09;
    memoria[71] = 0xCD; // INT 21
    memoria[72] = 0x21;
    memoria[73] = 0xB8; // MOV AX,4C01
    memoria[74] = 0x01;
    memoria[75] = 0x4C;
    memoria[76] = 0xCD; // INT 21
    memoria[77] = 0x21;
    memoria[78] = 0x54; // 'T'
    memoria[79] = 0x68; // 'h'
    memoria[80] = 0x69; // 'i'
    memoria[81] = 0x73; // 's'
    memoria[82] = 0x20; // ' '
    memoria[83] = 0x70; // 'p'
    memoria[84] = 0x72; // 'r'
    memoria[85] = 0x6F; // 'o'
    memoria[86] = 0x67; // 'g'
    memoria[87] = 0x72; // 'r'
    memoria[88] = 0x61; // 'a'
    memoria[89] = 0x6D; // 'm'
    memoria[90] = 0x20; // ' '
    memoria[91] = 0x69; // 'i'
    memoria[92] = 0x73; // 's'
    memoria[93] = 0x20; // ' '
    memoria[94] = 0x61; // 'a'
    memoria[95] = 0x20; // ' '
    memoria[96] = 0x57; // 'W'
    memoria[97] = 0x69; // 'i'
    memoria[98] = 0x6E; // 'n'
    memoria[99] = 0x33; // '3'
    memoria[100] = 0x32; // '2'
    memoria[101] = 0x20; // ' '
    memoria[102] = 0x63; // 'c'
    memoria[103] = 0x6F; // 'o'
    memoria[104] = 0x6E; // 'n'
    memoria[105] = 0x73; // 's'
    memoria[106] = 0x6F; // 'o'
    memoria[107] = 0x6C; // 'l'
    memoria[108] = 0x65; // 'e'
    memoria[109] = 0x20; // ' '
    memoria[110] = 0x61; // 'a'
    memoria[111] = 0x70; // 'p'
    memoria[112] = 0x70; // 'p'
    memoria[113] = 0x6C; // 'l'
    memoria[114] = 0x69; // 'i'
    memoria[115] = 0x63; // 'c'
    memoria[116] = 0x61; // 'a'
    memoria[117] = 0x74; // 't'
    memoria[118] = 0x69; // 'i'
    memoria[119] = 0x6F; // 'o'
    memoria[120] = 0x6E; // 'n'
    memoria[121] = 0x2E; // '.'
    memoria[122] = 0x20; // ' '
    memoria[123] = 0x49; // 'I'
    memoria[124] = 0x74; // 't'
    memoria[125] = 0x20; // ' '
    memoria[126] = 0x63; // 'c'
    memoria[127] = 0x61; // 'a'
    memoria[128] = 0x6E; // 'n'
    memoria[129] = 0x6E; // 'n'
    memoria[130] = 0x6F; // 'o'
    memoria[131] = 0x74; // 't'
    memoria[132] = 0x20; // ' '
    memoria[133] = 0x62; // 'b'
    memoria[134] = 0x65; // 'e'
    memoria[135] = 0x20; // ' '
    memoria[136] = 0x72; // 'r'
    memoria[137] = 0x75; // 'u'
    memoria[138] = 0x6E; // 'n'
    memoria[139] = 0x20; // ' '
    memoria[140] = 0x75; // 'u'
    memoria[141] = 0x6E; // 'n'
    memoria[142] = 0x64; // 'd'
    memoria[143] = 0x65; // 'e'
    memoria[144] = 0x72; // 'r'
    memoria[145] = 0x20; // ' '
    memoria[146] = 0x4D; // 'M'
    memoria[147] = 0x53; // 'S'
    memoria[148] = 0x2D; // '-'
    memoria[149] = 0x44; // 'D'
    memoria[150] = 0x4F; // 'O'
    memoria[151] = 0x53; // 'S'
    memoria[152] = 0x2E; // '.'
    memoria[153] = 0x0D; // Carriage return
    memoria[154] = 0x0A; // Line feed
    memoria[155] = 0x24; // String end ('$')
    memoria[156] = 0x00;
    memoria[157] = 0x00;
    memoria[158] = 0x00;
    memoria[159] = 0x00;
    /* COFF HEADER - 8 Standard fields */
    memoria[160] = 0x50; // 'P'
    memoria[161] = 0x45; // 'E'
    memoria[162] = 0x00; // '\0'
    memoria[163] = 0x00; // '\0'
    memoria[164] = 0x4C; // Machine:
    memoria[165] = 0x01; // >= Intel 386
    memoria[166] = 0x01; // Number of
    memoria[167] = 0x00; // sections
    memoria[168] = 0x00; // Time/Date stamp
    memoria[169] = 0x00;
    memoria[170] = 0x53;
    memoria[171] = 0x4C;
    memoria[172] = 0x00; // Pointer to symbol
    memoria[173] = 0x00; // table
    memoria[174] = 0x00; // (deprecated)
    memoria[175] = 0x00;
    memoria[176] = 0x00; // Number of symbols
    memoria[177] = 0x00; // (deprecated)
    memoria[178] = 0x00;
    memoria[179] = 0x00;
    memoria[180] = 0xE0; // Size of optional
    memoria[181] = 0x00; // header
    memoria[182] = 0x02; // Characteristics:
    memoria[183] = 0x01; // 32BIT_MACHINE EXE
    /* OPTIONAL HEADER - 8 Standard fields */
    /* (For image files, it is required) */
    memoria[184] = 0x0B; // Magic number
    memoria[185] = 0x01; // (010B = PE32)
    memoria[186] = 0x01;// Maj.Linker Version
    memoria[187] = 0x00;// Min.Linker Version
    memoria[188] = 0x00; // Size of code
    memoria[189] = 0x06; // (text) section
    memoria[190] = 0x00;
    memoria[191] = 0x00;
    memoria[192] = 0x00; // Size of
    memoria[193] = 0x00; // initialized data
    memoria[194] = 0x00; // section
    memoria[195] = 0x00;
    memoria[196] = 0x00; // Size of
    memoria[197] = 0x00; // uninitialized
    memoria[198] = 0x00; // data section
    memoria[199] = 0x00;
    memoria[200] = 0x00; // Starting address
    memoria[201] = 0x15; // relative to the
    memoria[202] = 0x00; // image base
    memoria[203] = 0x00;
    memoria[204] = 0x00; // Base of code
    memoria[205] = 0x10;
    memoria[206] = 0x00;
    memoria[207] = 0x00;
    /* OPT.HEADER - 1 PE32 specific field */
    memoria[208] = 0x00; // Base of data
    memoria[209] = 0x20;
    memoria[210] = 0x00;
    memoria[211] = 0x00;
    /* OPT.HEADER - 21 Win-Specific Fields */
    memoria[212] = 0x00; // Image base
    memoria[213] = 0x00; // (Preferred
    memoria[214] = 0x40; // address of image
    memoria[215] = 0x00; // when loaded)
    memoria[216] = 0x00; // Section alignment
    memoria[217] = 0x10;
    memoria[218] = 0x00;
    memoria[219] = 0x00;
    memoria[220] = 0x00; // File alignment
    memoria[221] = 0x02; // (Default is 512)
    memoria[222] = 0x00;
    memoria[223] = 0x00;
    memoria[224] = 0x04; // Major OS version
    memoria[225] = 0x00;
    memoria[226] = 0x00; // Minor OS version
    memoria[227] = 0x00;
    memoria[228] = 0x00;// Maj. image version
    memoria[229] = 0x00;
    memoria[230] = 0x00;// Min. image version
    memoria[231] = 0x00;
    memoria[232] = 0x04;// Maj.subsystem ver.
    memoria[233] = 0x00;
    memoria[234] = 0x00;// Min.subsystem ver.
    memoria[235] = 0x00;
    memoria[236] = 0x00; // Win32 version
    memoria[237] = 0x00; // (Reserved, must
    memoria[238] = 0x00; // be zero)
    memoria[239] = 0x00;
    memoria[240] = 0x00;// Size of image
    memoria[241] = 0x20;// (It must be a
    memoria[242] = 0x00;// multiple of the
    memoria[243] = 0x00;// section alignment)
    memoria[244] = 0x00; // Size of headers
    memoria[245] = 0x02; // (rounded up to a
    memoria[246] = 0x00; // multiple of the
    memoria[247] = 0x00; // file alignment)
    memoria[248] = 0x00; // Checksum
    memoria[249] = 0x00;
    memoria[250] = 0x00;
    memoria[251] = 0x00;
    memoria[252] = 0x03; // Windows subsystem
    memoria[253] = 0x00; // (03 = console)
    memoria[254] = 0x00; // DLL charac-
    memoria[255] = 0x00; // teristics
    memoria[256] = 0x00; // Size of stack
    memoria[257] = 0x00; // reserve
    memoria[258] = 0x10;
    memoria[259] = 0x00;
    memoria[260] = 0x00; // Size of stack
    memoria[261] = 0x10; // commit
    memoria[262] = 0x00;
    memoria[263] = 0x00;
    memoria[264] = 0x00; // Size of heap
    memoria[265] = 0x00; // reserve
    memoria[266] = 0x10;
    memoria[267] = 0x00;
    memoria[268] = 0x00; // Size of heap
    memoria[269] = 0x10; // commit
    memoria[270] = 0x00;
    memoria[271] = 0x00;
    memoria[272] = 0x00; // Loader flags
    memoria[273] = 0x00; // (Reserved, must
    memoria[274] = 0x00; // be zero)
    memoria[275] = 0x00;
    memoria[276] = 0x10; // Number of
    memoria[277] = 0x00; // relative virtual
    memoria[278] = 0x00; // addresses (RVAs)
    memoria[279] = 0x00;
    /* OPT. HEADER - 16 Data Directories */
    memoria[280] = 0x00; // Export Table
    memoria[281] = 0x00;
    memoria[282] = 0x00;
    memoria[283] = 0x00;
    memoria[284] = 0x00;
    memoria[285] = 0x00;
    memoria[286] = 0x00;
    memoria[287] = 0x00;
    memoria[288] = 0x1C; // Import Table
    memoria[289] = 0x10;
    memoria[290] = 0x00;
    memoria[291] = 0x00;
    memoria[292] = 0x28;
    memoria[293] = 0x00;
    memoria[294] = 0x00;
    memoria[295] = 0x00;
    memoria[296] = 0x00; // Resource Table
    memoria[297] = 0x00;
    memoria[298] = 0x00;
    memoria[299] = 0x00;
    memoria[300] = 0x00;
    memoria[301] = 0x00;
    memoria[302] = 0x00;
    memoria[303] = 0x00;
    memoria[304] = 0x00; // Exception Table
    memoria[305] = 0x00;
    memoria[306] = 0x00;
    memoria[307] = 0x00;
    memoria[308] = 0x00;
    memoria[309] = 0x00;
    memoria[310] = 0x00;
    memoria[311] = 0x00;
    memoria[312] = 0x00; // Certificate Table
    memoria[313] = 0x00;
    memoria[314] = 0x00;
    memoria[315] = 0x00;
    memoria[316] = 0x00;
    memoria[317] = 0x00;
    memoria[318] = 0x00;
    memoria[319] = 0x00;
    memoria[320] = 0x00; // Base Relocation
    memoria[321] = 0x00; // Table
    memoria[322] = 0x00;
    memoria[323] = 0x00;
    memoria[324] = 0x00;
    memoria[325] = 0x00;
    memoria[326] = 0x00;
    memoria[327] = 0x00;
    memoria[328] = 0x00; // Debug
    memoria[329] = 0x00;
    memoria[330] = 0x00;
    memoria[331] = 0x00;
    memoria[332] = 0x00;
    memoria[333] = 0x00;
    memoria[334] = 0x00;
    memoria[335] = 0x00;
    memoria[336] = 0x00; // Architecture
    memoria[337] = 0x00;
    memoria[338] = 0x00;
    memoria[339] = 0x00;
    memoria[340] = 0x00;
    memoria[341] = 0x00;
    memoria[342] = 0x00;
    memoria[343] = 0x00;
    memoria[344] = 0x00; // Global Ptr
    memoria[345] = 0x00;
    memoria[346] = 0x00;
    memoria[347] = 0x00;
    memoria[348] = 0x00;
    memoria[349] = 0x00;
    memoria[350] = 0x00;
    memoria[351] = 0x00;
    memoria[352] = 0x00; // TLS Table
    memoria[353] = 0x00;
    memoria[354] = 0x00;
    memoria[355] = 0x00;
    memoria[356] = 0x00;
    memoria[357] = 0x00;
    memoria[358] = 0x00;
    memoria[359] = 0x00;
    memoria[360] = 0x00; // Load Config Table
    memoria[361] = 0x00;
    memoria[362] = 0x00;
    memoria[363] = 0x00;
    memoria[364] = 0x00;
    memoria[365] = 0x00;
    memoria[366] = 0x00;
    memoria[367] = 0x00;
    memoria[368] = 0x00; // Bound Import
    memoria[369] = 0x00;
    memoria[370] = 0x00;
    memoria[371] = 0x00;
    memoria[372] = 0x00;
    memoria[373] = 0x00;
    memoria[374] = 0x00;
    memoria[375] = 0x00;
    memoria[376] = 0x00; // IAT
    memoria[377] = 0x10;
    memoria[378] = 0x00;
    memoria[379] = 0x00;
    memoria[380] = 0x1C;
    memoria[381] = 0x00;
    memoria[382] = 0x00;
    memoria[383] = 0x00;
    memoria[384] = 0x00; // Delay Import
    memoria[385] = 0x00; // Descriptor
    memoria[386] = 0x00;
    memoria[387] = 0x00;
    memoria[388] = 0x00;
    memoria[389] = 0x00;
    memoria[390] = 0x00;
    memoria[391] = 0x00;
    memoria[392] = 0x00; // CLR Runtime
    memoria[393] = 0x00; // Header
    memoria[394] = 0x00;
    memoria[395] = 0x00;
    memoria[396] = 0x00;
    memoria[397] = 0x00;
    memoria[398] = 0x00;
    memoria[399] = 0x00;
    memoria[400] = 0x00; // Reserved, must be
    memoria[401] = 0x00; // zero
    memoria[402] = 0x00;
    memoria[403] = 0x00;
    memoria[404] = 0x00;
    memoria[405] = 0x00;
    memoria[406] = 0x00;
    memoria[407] = 0x00;
    /* SECTIONS TABLE (40 bytes per entry) */
    /* FIRST ENTRY: TEXT HEADER */
    memoria[408] = 0x2E; // '.' (Name)
    memoria[409] = 0x74; // 't'
    memoria[410] = 0x65; // 'e'
    memoria[411] = 0x78; // 'x'
    memoria[412] = 0x74; // 't'
    memoria[413] = 0x00;
    memoria[414] = 0x00;
    memoria[415] = 0x00;
    memoria[416] = 0x24; // Virtual size
    memoria[417] = 0x05;
    memoria[418] = 0x00;
    memoria[419] = 0x00;
    memoria[420] = 0x00; // Virtual address
    memoria[421] = 0x10;
    memoria[422] = 0x00;
    memoria[423] = 0x00;
    memoria[424] = 0x00; // Size of raw data
    memoria[425] = 0x06;
    memoria[426] = 0x00;
    memoria[427] = 0x00;
    memoria[428] = 0x00; // Pointer to
    memoria[429] = 0x02; // raw data
    memoria[430] = 0x00;
    memoria[431] = 0x00;
    memoria[432] = 0x00; // Pointer to
    memoria[433] = 0x00; // relocations
    memoria[434] = 0x00;
    memoria[435] = 0x00;
    memoria[436] = 0x00; // Pointer to
    memoria[437] = 0x00; // line numbers
    memoria[438] = 0x00;
    memoria[439] = 0x00;
    memoria[440] = 0x00; // Number of
    memoria[441] = 0x00; // relocations
    memoria[442] = 0x00; // Number of
    memoria[443] = 0x00; // line numbers
    memoria[444] = 0x20; // Characteristics
    memoria[445] = 0x00; // (Readable,
    memoria[446] = 0x00; // Writeable &
    memoria[447] = 0xE0; // Executable)
    while(contador<=511){
        memoria[contador]=0x00;
        contador++;
    }
    memoria[512]=0x6E;
    memoria[513]=0x10;
    memoria[514]=0x0;
    memoria[515]=0x0;
    memoria[516]=0x7C;
    memoria[517]=0x10;
    memoria[518]=0x0;
    memoria[519]=0x0;
    memoria[520]=0x8C;
    memoria[521]=0x10;
    memoria[522]=0x0;
    memoria[523]=0x0;
    memoria[524]=0x98;
    memoria[525]=0x10;
    memoria[526]=0x0;
    memoria[527]=0x0;
    memoria[528]=0xA4;
    memoria[529]=0x10;
    memoria[530]=0x0;
    memoria[531]=0x0;
    memoria[532]=0xB6;
    memoria[533]=0x10;
    memoria[534]=0x0;
    memoria[535]=0x0;
    memoria[536]=0x0;
    memoria[537]=0x0;
    memoria[538]=0x0;
    memoria[539]=0x0;
    memoria[540]=0x52;
    memoria[541]=0x10;
    memoria[542]=0x0;
    memoria[543]=0x0;
    memoria[544]=0x0;
    memoria[545]=0x0;
    memoria[546]=0x0;
    memoria[547]=0x0;
    memoria[548]=0x0;
    memoria[549]=0x0;
    memoria[550]=0x0;
    memoria[551]=0x0;
    memoria[552]=0x44;
    memoria[553]=0x10;
    memoria[554]=0x0;
    memoria[555]=0x0;
    memoria[556]=0x0;
    memoria[557]=0x10;
    memoria[558]=0x0;
    memoria[559]=0x0;
    memoria[560]=0x0;
    memoria[561]=0x0;
    memoria[562]=0x0;
    memoria[563]=0x0;
    memoria[564]=0x0;
    memoria[565]=0x0;
    memoria[566]=0x0;
    memoria[567]=0x0;
    memoria[568]=0x0;
    memoria[569]=0x0;
    memoria[570]=0x0;
    memoria[571]=0x0;
    memoria[572]=0x0;
    memoria[573]=0x0;
    memoria[574]=0x0;
    memoria[575]=0x0;
    memoria[576]=0x0;
    memoria[577]=0x0;
    memoria[578]=0x0;
    memoria[579]=0x0;
    memoria[580]=0x4B;
    memoria[581]=0x45;
    memoria[582]=0x52;
    memoria[583]=0x4E;
    memoria[584]=0x45;
    memoria[585]=0x4C;
    memoria[586]=0x33;
    memoria[587]=0x32;
    memoria[588]=0x2E;
    memoria[589]=0x64;
    memoria[590]=0x6C;
    memoria[591]=0x6C;
    memoria[592]=0x0;
    memoria[593]=0x0;
    memoria[594]=0x6E;
    memoria[595]=0x10;
    memoria[596]=0x0;
    memoria[597]=0x0;
    memoria[598]=0x7C;
    memoria[599]=0x10;
    memoria[600]=0x0;
    memoria[601]=0x0;
    memoria[602]=0x8C;
    memoria[603]=0x10;
    memoria[604]=0x0;
    memoria[605]=0x0;
    memoria[606]=0x98;
    memoria[607]=0x10;
    memoria[608]=0x0;
    memoria[609]=0x0;
    memoria[610]=0xA4;
    memoria[611]=0x10;
    memoria[612]=0x0;
    memoria[613]=0x0;
    memoria[614]=0xB6;
    memoria[615]=0x10;
    memoria[616]=0x0;
    memoria[617]=0x0;
    memoria[618]=0x0;
    memoria[619]=0x0;
    memoria[620]=0x0;
    memoria[621]=0x0;
    memoria[622]=0x0;
    memoria[623]=0x0;
    memoria[624]=0x45;
    memoria[625]=0x78;
    memoria[626]=0x69;
    memoria[627]=0x74;
    memoria[628]=0x50;
    memoria[629]=0x72;
    memoria[630]=0x6F;
    memoria[631]=0x63;
    memoria[632]=0x65;
    memoria[633]=0x73;
    memoria[634]=0x73;
    memoria[635]=0x0;
    memoria[636]=0x0;
    memoria[637]=0x0;
    memoria[638]=0x47;
    memoria[639]=0x65;
    memoria[640]=0x74;
    memoria[641]=0x53;
    memoria[642]=0x74;
    memoria[643]=0x64;
    memoria[644]=0x48;
    memoria[645]=0x61;
    memoria[646]=0x6E;
    memoria[647]=0x64;
    memoria[648]=0x6C;
    memoria[649]=0x65;
    memoria[650]=0x0;
    memoria[651]=0x0;
    memoria[652]=0x0;
    memoria[653]=0x0;
    memoria[654]=0x52;
    memoria[655]=0x65;
    memoria[656]=0x61;
    memoria[657]=0x64;
    memoria[658]=0x46;
    memoria[659]=0x69;
    memoria[660]=0x6C;
    memoria[661]=0x65;
    memoria[662]=0x0;
    memoria[663]=0x0;
    memoria[664]=0x0;
    memoria[665]=0x0;
    memoria[666]=0x57;
    memoria[667]=0x72;
    memoria[668]=0x69;
    memoria[669]=0x74;
    memoria[670]=0x65;
    memoria[671]=0x46;
    memoria[672]=0x69;
    memoria[673]=0x6C;
    memoria[674]=0x65;
    memoria[675]=0x0;
    memoria[676]=0x0;
    memoria[677]=0x0;
    memoria[678]=0x47;
    memoria[679]=0x65;
    memoria[680]=0x74;
    memoria[681]=0x43;
    memoria[682]=0x6F;
    memoria[683]=0x6E;
    memoria[684]=0x73;
    memoria[685]=0x6F;
    memoria[686]=0x6C;
    memoria[687]=0x65;
    memoria[688]=0x4D;
    memoria[689]=0x6F;
    memoria[690]=0x64;
    memoria[691]=0x65;
    memoria[692]=0x0;
    memoria[693]=0x0;
    memoria[694]=0x0;
    memoria[695]=0x0;
    memoria[696]=0x53;
    memoria[697]=0x65;
    memoria[698]=0x74;
    memoria[699]=0x43;
    memoria[700]=0x6F;
    memoria[701]=0x6E;
    memoria[702]=0x73;
    memoria[703]=0x6F;
    memoria[704]=0x6C;
    memoria[705]=0x65;
    memoria[706]=0x4D;
    memoria[707]=0x6F;
    memoria[708]=0x64;
    memoria[709]=0x65;
    memoria[710]=0x0;
    memoria[711]=0x0;
    memoria[712]=0x0;
    memoria[713]=0x0;
    memoria[714]=0x0;
    memoria[715]=0x0;
    memoria[716]=0x0;
    memoria[717]=0x0;
    memoria[718]=0x0;
    memoria[719]=0x0;
    memoria[720]=0x50;
    memoria[721]=0xA2;
    memoria[722]=0x1C;
    memoria[723]=0x11;
    memoria[724]=0x40;
    memoria[725]=0x0;
    memoria[726]=0x31;
    memoria[727]=0xC0;
    memoria[728]=0x3;
    memoria[729]=0x5;
    memoria[730]=0x2C;
    memoria[731]=0x11;
    memoria[732]=0x40;
    memoria[733]=0x0;
    memoria[734]=0x75;
    memoria[735]=0x0D;
    memoria[736]=0x6A;
    memoria[737]=0xF5;
    memoria[738]=0xFF;
    memoria[739]=0x15;
    memoria[740]=0x4;
    memoria[741]=0x10;
    memoria[742]=0x40;
    memoria[743]=0x0;
    memoria[744]=0xA3;
    memoria[745]=0x2C;
    memoria[746]=0x11;
    memoria[747]=0x40;
    memoria[748]=0x0;
    memoria[749]=0x6A;
    memoria[750]=0x0;
    memoria[751]=0x68;
    memoria[752]=0x30;
    memoria[753]=0x11;
    memoria[754]=0x40;
    memoria[755]=0x0;
    memoria[756]=0x6A;
    memoria[757]=0x1;
    memoria[758]=0x68;
    memoria[759]=0x1C;
    memoria[760]=0x11;
    memoria[761]=0x40;
    memoria[762]=0x0;
    memoria[763]=0x50;
    memoria[764]=0xFF;
    memoria[765]=0x15;
    memoria[766]=0x0C;
    memoria[767]=0x10;
    memoria[768]=0x40;
    memoria[769]=0x0;
    memoria[770]=0x9;
    memoria[771]=0xC0;
    memoria[772]=0x75;
    memoria[773]=0x8;
    memoria[774]=0x6A;
    memoria[775]=0x0;
    memoria[776]=0xFF;
    memoria[777]=0x15;
    memoria[778]=0x0;
    memoria[779]=0x10;
    memoria[780]=0x40;
    memoria[781]=0x0;
    memoria[782]=0x81;
    memoria[783]=0x3D;
    memoria[784]=0x30;
    memoria[785]=0x11;
    memoria[786]=0x40;
    memoria[787]=0x0;
    memoria[788]=0x1;
    memoria[789]=0x0;
    memoria[790]=0x0;
    memoria[791]=0x0;
    memoria[792]=0x75;
    memoria[793]=0xEC;
    memoria[794]=0x58;
    memoria[795]=0xC3;
    memoria[796]=0x0;
    memoria[797]=0x57;
    memoria[798]=0x72;
    memoria[799]=0x69;
    memoria[800]=0x74;
    memoria[801]=0x65;
    memoria[802]=0x20;
    memoria[803]=0x65;
    memoria[804]=0x72;
    memoria[805]=0x72;
    memoria[806]=0x6F;
    memoria[807]=0x72;
    memoria[808]=0x0;
    memoria[809]=0x0;
    memoria[810]=0x0;
    memoria[811]=0x0;
    memoria[812]=0x0;
    memoria[813]=0x0;
    memoria[814]=0x0;
    memoria[815]=0x0;
    memoria[816]=0x0;
    memoria[817]=0x0;
    memoria[818]=0x0;
    memoria[819]=0x0;
    memoria[820]=0x0;
    memoria[821]=0x0;
    memoria[822]=0x0;
    memoria[823]=0x0;
    memoria[824]=0x0;
    memoria[825]=0x0;
    memoria[826]=0x0;
    memoria[827]=0x0;
    memoria[828]=0x0;
    memoria[829]=0x0;
    memoria[830]=0x0;
    memoria[831]=0x0;
    memoria[832]=0x60;
    memoria[833]=0x31;
    memoria[834]=0xC0;
    memoria[835]=0x3;
    memoria[836]=0x5;
    memoria[837]=0xCC;
    memoria[838]=0x11;
    memoria[839]=0x40;
    memoria[840]=0x0;
    memoria[841]=0x75;
    memoria[842]=0x37;
    memoria[843]=0x6A;
    memoria[844]=0xF6;
    memoria[845]=0xFF;
    memoria[846]=0x15;
    memoria[847]=0x4;
    memoria[848]=0x10;
    memoria[849]=0x40;
    memoria[850]=0x0;
    memoria[851]=0xA3;
    memoria[852]=0xCC;
    memoria[853]=0x11;
    memoria[854]=0x40;
    memoria[855]=0x0;
    memoria[856]=0x68;
    memoria[857]=0xD0;
    memoria[858]=0x11;
    memoria[859]=0x40;
    memoria[860]=0x0;
    memoria[861]=0x50;
    memoria[862]=0xFF;
    memoria[863]=0x15;
    memoria[864]=0x10;
    memoria[865]=0x10;
    memoria[866]=0x40;
    memoria[867]=0x0;
    memoria[868]=0x80;
    memoria[869]=0x25;
    memoria[870]=0xD0;
    memoria[871]=0x11;
    memoria[872]=0x40;
    memoria[873]=0x0;
    memoria[874]=0xF9;
    memoria[875]=0xFF;
    memoria[876]=0x35;
    memoria[877]=0xD0;
    memoria[878]=0x11;
    memoria[879]=0x40;
    memoria[880]=0x0;
    memoria[881]=0xFF;
    memoria[882]=0x35;
    memoria[883]=0xCC;
    memoria[884]=0x11;
    memoria[885]=0x40;
    memoria[886]=0x0;
    memoria[887]=0xFF;
    memoria[888]=0x15;
    memoria[889]=0x14;
    memoria[890]=0x10;
    memoria[891]=0x40;
    memoria[892]=0x0;
    memoria[893]=0xA1;
    memoria[894]=0xCC;
    memoria[895]=0x11;
    memoria[896]=0x40;
    memoria[897]=0x0;
    memoria[898]=0x6A;
    memoria[899]=0x0;
    memoria[900]=0x68;
    memoria[901]=0xD4;
    memoria[902]=0x11;
    memoria[903]=0x40;
    memoria[904]=0x0;
    memoria[905]=0x6A;
    memoria[906]=0x1;
    memoria[907]=0x68;
    memoria[908]=0xBE;
    memoria[909]=0x11;
    memoria[910]=0x40;
    memoria[911]=0x0;
    memoria[912]=0x50;
    memoria[913]=0xFF;
    memoria[914]=0x15;
    memoria[915]=0x8;
    memoria[916]=0x10;
    memoria[917]=0x40;
    memoria[918]=0x0;
    memoria[919]=0x9;
    memoria[920]=0xC0;
    memoria[921]=0x61;
    memoria[922]=0x90;
    memoria[923]=0x75;
    memoria[924]=0x8;
    memoria[925]=0x6A;
    memoria[926]=0x0;
    memoria[927]=0xFF;
    memoria[928]=0x15;
    memoria[929]=0x0;
    memoria[930]=0x10;
    memoria[931]=0x40;
    memoria[932]=0x0;
    memoria[933]=0x0F;
    memoria[934]=0xB6;
    memoria[935]=0x5;
    memoria[936]=0xBE;
    memoria[937]=0x11;
    memoria[938]=0x40;
    memoria[939]=0x0;
    memoria[940]=0x81;
    memoria[941]=0x3D;
    memoria[942]=0xD4;
    memoria[943]=0x11;
    memoria[944]=0x40;
    memoria[945]=0x0;
    memoria[946]=0x1;
    memoria[947]=0x0;
    memoria[948]=0x0;
    memoria[949]=0x0;
    memoria[950]=0x74;
    memoria[951]=0x5;
    memoria[952]=0xB8;
    memoria[953]=0xFF;
    memoria[954]=0xFF;
    memoria[955]=0xFF;
    memoria[956]=0xFF;
    memoria[957]=0xC3;
    memoria[958]=0x0;
    memoria[959]=0x52;
    memoria[960]=0x65;
    memoria[961]=0x61;
    memoria[962]=0x64;
    memoria[963]=0x20;
    memoria[964]=0x65;
    memoria[965]=0x72;
    memoria[966]=0x72;
    memoria[967]=0x6F;
    memoria[968]=0x72;
    memoria[969]=0x0;
    memoria[970]=0x0;
    memoria[971]=0x0;
    memoria[972]=0x0;
    memoria[973]=0x0;
    memoria[974]=0x0;
    memoria[975]=0x0;
    memoria[976]=0x0;
    memoria[977]=0x0;
    memoria[978]=0x0;
    memoria[979]=0x0;
    memoria[980]=0x0;
    memoria[981]=0x0;
    memoria[982]=0x0;
    memoria[983]=0x0;
    memoria[984]=0x0;
    memoria[985]=0x0;
    memoria[986]=0x0;
    memoria[987]=0x0;
    memoria[988]=0x0;
    memoria[989]=0x0;
    memoria[990]=0x0;
    memoria[991]=0x0;
    memoria[992]=0x60;//esto es para el WRITE string
    memoria[993]=0x89;
    memoria[994]=0xC6;
    memoria[995]=0x30;
    memoria[996]=0xC0;
    memoria[997]=0x2;
    memoria[998]=0x6;
    memoria[999]=0x74;
    memoria[1000]=0x8;
    memoria[1001]=0x46;
    memoria[1002]=0xE8;
    memoria[1003]=0xE1;
    memoria[1004]=0xFE;
    memoria[1005]=0xFF;
    memoria[1006]=0xFF;
    memoria[1007]=0xEB;
    memoria[1008]=0xF2;
    memoria[1009]=0x61;
    memoria[1010]=0x90;
    memoria[1011]=0xC3;
    memoria[1012]=0x0;
    memoria[1013]=0x0;
    memoria[1014]=0x0;
    memoria[1015]=0x0;
    memoria[1016]=0x0;
    memoria[1017]=0x0;
    memoria[1018]=0x0;
    memoria[1019]=0x0;
    memoria[1020]=0x0;
    memoria[1021]=0x0;
    memoria[1022]=0x0;
    memoria[1023]=0x0;
    memoria[1024]=0x4;
    memoria[1025]=0x30;
    memoria[1026]=0xE8;
    memoria[1027]=0xC9;
    memoria[1028]=0xFE;
    memoria[1029]=0xFF;
    memoria[1030]=0xFF;
    memoria[1031]=0xC3;
    memoria[1032]=0x0;
    memoria[1033]=0x0;
    memoria[1034]=0x0;
    memoria[1035]=0x0;
    memoria[1036]=0x0;
    memoria[1037]=0x0;
    memoria[1038]=0x0;
    memoria[1039]=0x0;
    memoria[1040]=0xB0;//salto de línea
    memoria[1041]=0x0D;
    memoria[1042]=0xE8;
    memoria[1043]=0xB9;
    memoria[1044]=0xFE;
    memoria[1045]=0xFF;
    memoria[1046]=0xFF;
    memoria[1047]=0xB0;
    memoria[1048]=0x0A;
    memoria[1049]=0xE8;
    memoria[1050]=0xB2;
    memoria[1051]=0xFE;
    memoria[1052]=0xFF;
    memoria[1053]=0xFF;
    memoria[1054]=0xC3;
    memoria[1055]=0x0;
    memoria[1056]=0x3D;//salto de línea
    memoria[1057]=0x0;
    memoria[1058]=0x0;
    memoria[1059]=0x0;
    memoria[1060]=0x80;
    memoria[1061]=0x75;
    memoria[1062]=0x4E;
    memoria[1063]=0xB0;
    memoria[1064]=0x2D;
    memoria[1065]=0xE8;
    memoria[1066]=0xA2;
    memoria[1067]=0xFE;
    memoria[1068]=0xFF;
    memoria[1069]=0xFF;
    memoria[1070]=0xB0;
    memoria[1071]=0x2;
    memoria[1072]=0xE8;
    memoria[1073]=0xCB;
    memoria[1074]=0xFF;
    memoria[1075]=0xFF;
    memoria[1076]=0xFF;
    memoria[1077]=0xB0;
    memoria[1078]=0x1;
    memoria[1079]=0xE8;
    memoria[1080]=0xC4;
    memoria[1081]=0xFF;
    memoria[1082]=0xFF;
    memoria[1083]=0xFF;
    memoria[1084]=0xB0;
    memoria[1085]=0x4;
    memoria[1086]=0xE8;
    memoria[1087]=0xBD;
    memoria[1088]=0xFF;
    memoria[1089]=0xFF;
    memoria[1090]=0xFF;
    memoria[1091]=0xB0;
    memoria[1092]=0x7;
    memoria[1093]=0xE8;
    memoria[1094]=0xB6;
    memoria[1095]=0xFF;
    memoria[1096]=0xFF;
    memoria[1097]=0xFF;
    memoria[1098]=0xB0;
    memoria[1099]=0x4;
    memoria[1100]=0xE8;
    memoria[1101]=0xAF;
    memoria[1102]=0xFF;
    memoria[1103]=0xFF;
    memoria[1104]=0xFF;
    memoria[1105]=0xB0;
    memoria[1106]=0x8;
    memoria[1107]=0xE8;
    memoria[1108]=0xA8;
    memoria[1109]=0xFF;
    memoria[1110]=0xFF;
    memoria[1111]=0xFF;
    memoria[1112]=0xB0;
    memoria[1113]=0x3;
    memoria[1114]=0xE8;
    memoria[1115]=0xA1;
    memoria[1116]=0xFF;
    memoria[1117]=0xFF;
    memoria[1118]=0xFF;
    memoria[1119]=0xB0;
    memoria[1120]=0x6;
    memoria[1121]=0xE8;
    memoria[1122]=0x9A;
    memoria[1123]=0xFF;
    memoria[1124]=0xFF;
    memoria[1125]=0xFF;
    memoria[1126]=0xB0;
    memoria[1127]=0x4;
    memoria[1128]=0xE8;
    memoria[1129]=0x93;
    memoria[1130]=0xFF;
    memoria[1131]=0xFF;
    memoria[1132]=0xFF;
    memoria[1133]=0xB0;
    memoria[1134]=0x8;
    memoria[1135]=0xE8;
    memoria[1136]=0x8C;
    memoria[1137]=0xFF;
    memoria[1138]=0xFF;
    memoria[1139]=0xFF;
    memoria[1140]=0xC3;
    memoria[1141]=0x3D;
    memoria[1142]=0x0;
    memoria[1143]=0x0;
    memoria[1144]=0x0;
    memoria[1145]=0x0;
    memoria[1146]=0x7D;
    memoria[1147]=0x0B;
    memoria[1148]=0x50;
    memoria[1149]=0xB0;
    memoria[1150]=0x2D;
    memoria[1151]=0xE8;
    memoria[1152]=0x4C;
    memoria[1153]=0xFE;
    memoria[1154]=0xFF;
    memoria[1155]=0xFF;
    memoria[1156]=0x58;
    memoria[1157]=0xF7;
    memoria[1158]=0xD8;
    memoria[1159]=0x3D;
    memoria[1160]=0x0A;
    memoria[1161]=0x0;
    memoria[1162]=0x0;
    memoria[1163]=0x0;
    memoria[1164]=0x0F;
    memoria[1165]=0x8C;
    memoria[1166]=0xEF;
    memoria[1167]=0x0;
    memoria[1168]=0x0;
    memoria[1169]=0x0;
    memoria[1170]=0x3D;
    memoria[1171]=0x64;
    memoria[1172]=0x0;
    memoria[1173]=0x0;
    memoria[1174]=0x0;
    memoria[1175]=0x0F;
    memoria[1176]=0x8C;
    memoria[1177]=0xD1;
    memoria[1178]=0x0;
    memoria[1179]=0x0;
    memoria[1180]=0x0;
    memoria[1181]=0x3D;
    memoria[1182]=0xE8;
    memoria[1183]=0x3;
    memoria[1184]=0x0;
    memoria[1185]=0x0;
    memoria[1186]=0x0F;
    memoria[1187]=0x8C;
    memoria[1188]=0xB3;
    memoria[1189]=0x0;
    memoria[1190]=0x0;
    memoria[1191]=0x0;
    memoria[1192]=0x3D;
    memoria[1193]=0x10;
    memoria[1194]=0x27;
    memoria[1195]=0x0;
    memoria[1196]=0x0;
    memoria[1197]=0x0F;
    memoria[1198]=0x8C;
    memoria[1199]=0x95;
    memoria[1200]=0x0;
    memoria[1201]=0x0;
    memoria[1202]=0x0;
    memoria[1203]=0x3D;
    memoria[1204]=0xA0;
    memoria[1205]=0x86;
    memoria[1206]=0x1;
    memoria[1207]=0x0;
    memoria[1208]=0x7C;
    memoria[1209]=0x7B;
    memoria[1210]=0x3D;
    memoria[1211]=0x40;
    memoria[1212]=0x42;
    memoria[1213]=0x0F;
    memoria[1214]=0x0;
    memoria[1215]=0x7C;
    memoria[1216]=0x61;
    memoria[1217]=0x3D;
    memoria[1218]=0x80;
    memoria[1219]=0x96;
    memoria[1220]=0x98;
    memoria[1221]=0x0;
    memoria[1222]=0x7C;
    memoria[1223]=0x47;
    memoria[1224]=0x3D;
    memoria[1225]=0x0;
    memoria[1226]=0xE1;
    memoria[1227]=0xF5;
    memoria[1228]=0x5;
    memoria[1229]=0x7C;
    memoria[1230]=0x2D;
    memoria[1231]=0x3D;
    memoria[1232]=0x0;
    memoria[1233]=0xCA;
    memoria[1234]=0x9A;
    memoria[1235]=0x3B;
    memoria[1236]=0x7C;
    memoria[1237]=0x13;
    memoria[1238]=0xBA;
    memoria[1239]=0x0;
    memoria[1240]=0x0;
    memoria[1241]=0x0;
    memoria[1242]=0x0;
    memoria[1243]=0xBB;
    memoria[1244]=0x0;
    memoria[1245]=0xCA;
    memoria[1246]=0x9A;
    memoria[1247]=0x3B;
    memoria[1248]=0xF7;
    memoria[1249]=0xFB;
    memoria[1250]=0x52;
    memoria[1251]=0xE8;
    memoria[1252]=0x18;
    memoria[1253]=0xFF;
    memoria[1254]=0xFF;
    memoria[1255]=0xFF;
    memoria[1256]=0x58;
    memoria[1257]=0xBA;
    memoria[1258]=0x0;
    memoria[1259]=0x0;
    memoria[1260]=0x0;
    memoria[1261]=0x0;
    memoria[1262]=0xBB;
    memoria[1263]=0x0;
    memoria[1264]=0xE1;
    memoria[1265]=0xF5;
    memoria[1266]=0x5;
    memoria[1267]=0xF7;
    memoria[1268]=0xFB;
    memoria[1269]=0x52;
    memoria[1270]=0xE8;
    memoria[1271]=0x5;
    memoria[1272]=0xFF;
    memoria[1273]=0xFF;
    memoria[1274]=0xFF;
    memoria[1275]=0x58;
    memoria[1276]=0xBA;
    memoria[1277]=0x0;
    memoria[1278]=0x0;
    memoria[1279]=0x0;
    memoria[1280]=0x0;
    memoria[1281]=0xBB;
    memoria[1282]=0x80;
    memoria[1283]=0x96;
    memoria[1284]=0x98;
    memoria[1285]=0x0;
    memoria[1286]=0xF7;
    memoria[1287]=0xFB;
    memoria[1288]=0x52;
    memoria[1289]=0xE8;
    memoria[1290]=0xF2;
    memoria[1291]=0xFE;
    memoria[1292]=0xFF;
    memoria[1293]=0xFF;
    memoria[1294]=0x58;
    memoria[1295]=0xBA;
    memoria[1296]=0x0;
    memoria[1297]=0x0;
    memoria[1298]=0x0;
    memoria[1299]=0x0;
    memoria[1300]=0xBB;
    memoria[1301]=0x40;
    memoria[1302]=0x42;
    memoria[1303]=0x0F;
    memoria[1304]=0x0;
    memoria[1305]=0xF7;
    memoria[1306]=0xFB;
    memoria[1307]=0x52;
    memoria[1308]=0xE8;
    memoria[1309]=0xDF;
    memoria[1310]=0xFE;
    memoria[1311]=0xFF;
    memoria[1312]=0xFF;
    memoria[1313]=0x58;
    memoria[1314]=0xBA;
    memoria[1315]=0x0;
    memoria[1316]=0x0;
    memoria[1317]=0x0;
    memoria[1318]=0x0;
    memoria[1319]=0xBB;
    memoria[1320]=0xA0;
    memoria[1321]=0x86;
    memoria[1322]=0x1;
    memoria[1323]=0x0;
    memoria[1324]=0xF7;
    memoria[1325]=0xFB;
    memoria[1326]=0x52;
    memoria[1327]=0xE8;
    memoria[1328]=0xCC;
    memoria[1329]=0xFE;
    memoria[1330]=0xFF;
    memoria[1331]=0xFF;
    memoria[1332]=0x58;
    memoria[1333]=0xBA;
    memoria[1334]=0x0;
    memoria[1335]=0x0;
    memoria[1336]=0x0;
    memoria[1337]=0x0;
    memoria[1338]=0xBB;
    memoria[1339]=0x10;
    memoria[1340]=0x27;
    memoria[1341]=0x0;
    memoria[1342]=0x0;
    memoria[1343]=0xF7;
    memoria[1344]=0xFB;
    memoria[1345]=0x52;
    memoria[1346]=0xE8;
    memoria[1347]=0xB9;
    memoria[1348]=0xFE;
    memoria[1349]=0xFF;
    memoria[1350]=0xFF;
    memoria[1351]=0x58;
    memoria[1352]=0xBA;
    memoria[1353]=0x0;
    memoria[1354]=0x0;
    memoria[1355]=0x0;
    memoria[1356]=0x0;
    memoria[1357]=0xBB;
    memoria[1358]=0xE8;
    memoria[1359]=0x3;
    memoria[1360]=0x0;
    memoria[1361]=0x0;
    memoria[1362]=0xF7;
    memoria[1363]=0xFB;
    memoria[1364]=0x52;
    memoria[1365]=0xE8;
    memoria[1366]=0xA6;
    memoria[1367]=0xFE;
    memoria[1368]=0xFF;
    memoria[1369]=0xFF;
    memoria[1370]=0x58;
    memoria[1371]=0xBA;
    memoria[1372]=0x0;
    memoria[1373]=0x0;
    memoria[1374]=0x0;
    memoria[1375]=0x0;
    memoria[1376]=0xBB;
    memoria[1377]=0x64;
    memoria[1378]=0x0;
    memoria[1379]=0x0;
    memoria[1380]=0x0;
    memoria[1381]=0xF7;
    memoria[1382]=0xFB;
    memoria[1383]=0x52;
    memoria[1384]=0xE8;
    memoria[1385]=0x93;
    memoria[1386]=0xFE;
    memoria[1387]=0xFF;
    memoria[1388]=0xFF;
    memoria[1389]=0x58;
    memoria[1390]=0xBA;
    memoria[1391]=0x0;
    memoria[1392]=0x0;
    memoria[1393]=0x0;
    memoria[1394]=0x0;
    memoria[1395]=0xBB;
    memoria[1396]=0x0A;
    memoria[1397]=0x0;
    memoria[1398]=0x0;
    memoria[1399]=0x0;
    memoria[1400]=0xF7;
    memoria[1401]=0xFB;
    memoria[1402]=0x52;
    memoria[1403]=0xE8;
    memoria[1404]=0x80;
    memoria[1405]=0xFE;
    memoria[1406]=0xFF;
    memoria[1407]=0xFF;
    memoria[1408]=0x58;
    memoria[1409]=0xE8;
    memoria[1410]=0x7A;
    memoria[1411]=0xFE;
    memoria[1412]=0xFF;
    memoria[1413]=0xFF;
    memoria[1414]=0xC3;
    memoria[1415]=0x0;
    memoria[1416]=0xFF;//finaliza el programa
    memoria[1417]=0x15;
    memoria[1418]=0x0;
    memoria[1419]=0x10;
    memoria[1420]=0x40;
    memoria[1421]=0x0;
    memoria[1422]=0x0;
    memoria[1423]=0x0;
    memoria[1424]=0xB9;//salto de línea
    memoria[1425]=0x0;
    memoria[1426]=0x0;
    memoria[1427]=0x0;
    memoria[1428]=0x0;
    memoria[1429]=0xB3;
    memoria[1430]=0x3;
    memoria[1431]=0x51;
    memoria[1432]=0x53;
    memoria[1433]=0xE8;
    memoria[1434]=0xA2;
    memoria[1435]=0xFD;
    memoria[1436]=0xFF;
    memoria[1437]=0xFF;
    memoria[1438]=0x5B;
    memoria[1439]=0x59;
    memoria[1440]=0x3C;
    memoria[1441]=0x0D;
    memoria[1442]=0x0F;
    memoria[1443]=0x84;
    memoria[1444]=0x34;
    memoria[1445]=0x1;
    memoria[1446]=0x0;
    memoria[1447]=0x0;
    memoria[1448]=0x3C;
    memoria[1449]=0x8;
    memoria[1450]=0x0F;
    memoria[1451]=0x84;
    memoria[1452]=0x94;
    memoria[1453]=0x0;
    memoria[1454]=0x0;
    memoria[1455]=0x0;
    memoria[1456]=0x3C;
    memoria[1457]=0x2D;
    memoria[1458]=0x0F;
    memoria[1459]=0x84;
    memoria[1460]=0x9;
    memoria[1461]=0x1;
    memoria[1462]=0x0;
    memoria[1463]=0x0;
    memoria[1464]=0x3C;
    memoria[1465]=0x30;
    memoria[1466]=0x7C;
    memoria[1467]=0xDB;
    memoria[1468]=0x3C;
    memoria[1469]=0x39;
    memoria[1470]=0x7F;
    memoria[1471]=0xD7;
    memoria[1472]=0x2C;
    memoria[1473]=0x30;
    memoria[1474]=0x80;
    memoria[1475]=0xFB;
    memoria[1476]=0x0;
    memoria[1477]=0x74;
    memoria[1478]=0xD0;
    memoria[1479]=0x80;
    memoria[1480]=0xFB;
    memoria[1481]=0x2;
    memoria[1482]=0x75;
    memoria[1483]=0x0C;
    memoria[1484]=0x81;
    memoria[1485]=0xF9;
    memoria[1486]=0x0;
    memoria[1487]=0x0;
    memoria[1488]=0x0;
    memoria[1489]=0x0;
    memoria[1490]=0x75;
    memoria[1491]=0x4;
    memoria[1492]=0x3C;
    memoria[1493]=0x0;
    memoria[1494]=0x74;
    memoria[1495]=0xBF;
    memoria[1496]=0x80;
    memoria[1497]=0xFB;
    memoria[1498]=0x3;
    memoria[1499]=0x75;
    memoria[1500]=0x0A;
    memoria[1501]=0x3C;
    memoria[1502]=0x0;
    memoria[1503]=0x75;
    memoria[1504]=0x4;
    memoria[1505]=0xB3;
    memoria[1506]=0x0;
    memoria[1507]=0xEB;
    memoria[1508]=0x2;
    memoria[1509]=0xB3;
    memoria[1510]=0x1;
    memoria[1511]=0x81;
    memoria[1512]=0xF9;
    memoria[1513]=0xCC;
    memoria[1514]=0xCC;
    memoria[1515]=0xCC;
    memoria[1516]=0x0C;
    memoria[1517]=0x7F;
    memoria[1518]=0xA8;
    memoria[1519]=0x81;
    memoria[1520]=0xF9;
    memoria[1521]=0x34;
    memoria[1522]=0x33;
    memoria[1523]=0x33;
    memoria[1524]=0xF3;
    memoria[1525]=0x7C;
    memoria[1526]=0xA0;
    memoria[1527]=0x88;
    memoria[1528]=0xC7;
    memoria[1529]=0xB8;
    memoria[1530]=0x0A;
    memoria[1531]=0x0;
    memoria[1532]=0x0;
    memoria[1533]=0x0;
    memoria[1534]=0xF7;
    memoria[1535]=0xE9;
    memoria[1536]=0x3D;
    memoria[1537]=0x8;
    memoria[1538]=0x0;
    memoria[1539]=0x0;
    memoria[1540]=0x80;
    memoria[1541]=0x74;
    memoria[1542]=0x11;
    memoria[1543]=0x3D;
    memoria[1544]=0xF8;
    memoria[1545]=0xFF;
    memoria[1546]=0xFF;
    memoria[1547]=0x7F;
    memoria[1548]=0x75;
    memoria[1549]=0x13;
    memoria[1550]=0x80;
    memoria[1551]=0xFF;
    memoria[1552]=0x7;
    memoria[1553]=0x7E;
    memoria[1554]=0x0E;
    memoria[1555]=0xE9;
    memoria[1556]=0x7F;
    memoria[1557]=0xFF;
    memoria[1558]=0xFF;
    memoria[1559]=0xFF;
    memoria[1560]=0x80;
    memoria[1561]=0xFF;
    memoria[1562]=0x8;
    memoria[1563]=0x0F;
    memoria[1564]=0x8F;
    memoria[1565]=0x76;
    memoria[1566]=0xFF;
    memoria[1567]=0xFF;
    memoria[1568]=0xFF;
    memoria[1569]=0xB9;
    memoria[1570]=0x0;
    memoria[1571]=0x0;
    memoria[1572]=0x0;
    memoria[1573]=0x0;
    memoria[1574]=0x88;
    memoria[1575]=0xF9;
    memoria[1576]=0x80;
    memoria[1577]=0xFB;
    memoria[1578]=0x2;
    memoria[1579]=0x74;
    memoria[1580]=0x4;
    memoria[1581]=0x1;
    memoria[1582]=0xC1;
    memoria[1583]=0xEB;
    memoria[1584]=0x3;
    memoria[1585]=0x29;
    memoria[1586]=0xC8;
    memoria[1587]=0x91;
    memoria[1588]=0x88;
    memoria[1589]=0xF8;
    memoria[1590]=0x51;
    memoria[1591]=0x53;
    memoria[1592]=0xE8;
    memoria[1593]=0xC3;
    memoria[1594]=0xFD;
    memoria[1595]=0xFF;
    memoria[1596]=0xFF;
    memoria[1597]=0x5B;
    memoria[1598]=0x59;
    memoria[1599]=0xE9;
    memoria[1600]=0x53;
    memoria[1601]=0xFF;
    memoria[1602]=0xFF;
    memoria[1603]=0xFF;
    memoria[1604]=0x80;
    memoria[1605]=0xFB;
    memoria[1606]=0x3;
    memoria[1607]=0x0F;
    memoria[1608]=0x84;
    memoria[1609]=0x4A;
    memoria[1610]=0xFF;
    memoria[1611]=0xFF;
    memoria[1612]=0xFF;
    memoria[1613]=0x51;
    memoria[1614]=0x53;
    memoria[1615]=0xB0;
    memoria[1616]=0x8;
    memoria[1617]=0xE8;
    memoria[1618]=0x7A;
    memoria[1619]=0xFC;
    memoria[1620]=0xFF;
    memoria[1621]=0xFF;
    memoria[1622]=0xB0;
    memoria[1623]=0x20;
    memoria[1624]=0xE8;
    memoria[1625]=0x73;
    memoria[1626]=0xFC;
    memoria[1627]=0xFF;
    memoria[1628]=0xFF;
    memoria[1629]=0xB0;
    memoria[1630]=0x8;
    memoria[1631]=0xE8;
    memoria[1632]=0x6C;
    memoria[1633]=0xFC;
    memoria[1634]=0xFF;
    memoria[1635]=0xFF;
    memoria[1636]=0x5B;
    memoria[1637]=0x59;
    memoria[1638]=0x80;
    memoria[1639]=0xFB;
    memoria[1640]=0x0;
    memoria[1641]=0x75;
    memoria[1642]=0x7;
    memoria[1643]=0xB3;
    memoria[1644]=0x3;
    memoria[1645]=0xE9;
    memoria[1646]=0x25;
    memoria[1647]=0xFF;
    memoria[1648]=0xFF;
    memoria[1649]=0xFF;
    memoria[1650]=0x80;
    memoria[1651]=0xFB;
    memoria[1652]=0x2;
    memoria[1653]=0x75;
    memoria[1654]=0x0F;
    memoria[1655]=0x81;
    memoria[1656]=0xF9;
    memoria[1657]=0x0;
    memoria[1658]=0x0;
    memoria[1659]=0x0;
    memoria[1660]=0x0;
    memoria[1661]=0x75;
    memoria[1662]=0x7;
    memoria[1663]=0xB3;
    memoria[1664]=0x3;
    memoria[1665]=0xE9;
    memoria[1666]=0x11;
    memoria[1667]=0xFF;
    memoria[1668]=0xFF;
    memoria[1669]=0xFF;
    memoria[1670]=0x89;
    memoria[1671]=0xC8;
    memoria[1672]=0xB9;
    memoria[1673]=0x0A;
    memoria[1674]=0x0;
    memoria[1675]=0x0;
    memoria[1676]=0x0;
    memoria[1677]=0xBA;
    memoria[1678]=0x0;
    memoria[1679]=0x0;
    memoria[1680]=0x0;
    memoria[1681]=0x0;
    memoria[1682]=0x3D;
    memoria[1683]=0x0;
    memoria[1684]=0x0;
    memoria[1685]=0x0;
    memoria[1686]=0x0;
    memoria[1687]=0x7D;
    memoria[1688]=0x8;
    memoria[1689]=0xF7;
    memoria[1690]=0xD8;
    memoria[1691]=0xF7;
    memoria[1692]=0xF9;
    memoria[1693]=0xF7;
    memoria[1694]=0xD8;
    memoria[1695]=0xEB;
    memoria[1696]=0x2;
    memoria[1697]=0xF7;
    memoria[1698]=0xF9;
    memoria[1699]=0x89;
    memoria[1700]=0xC1;
    memoria[1701]=0x81;
    memoria[1702]=0xF9;
    memoria[1703]=0x0;
    memoria[1704]=0x0;
    memoria[1705]=0x0;
    memoria[1706]=0x0;
    memoria[1707]=0x0F;
    memoria[1708]=0x85;
    memoria[1709]=0xE6;
    memoria[1710]=0xFE;
    memoria[1711]=0xFF;
    memoria[1712]=0xFF;
    memoria[1713]=0x80;
    memoria[1714]=0xFB;
    memoria[1715]=0x2;
    memoria[1716]=0x0F;
    memoria[1717]=0x84;
    memoria[1718]=0xDD;
    memoria[1719]=0xFE;
    memoria[1720]=0xFF;
    memoria[1721]=0xFF;
    memoria[1722]=0xB3;
    memoria[1723]=0x3;
    memoria[1724]=0xE9;
    memoria[1725]=0xD6;
    memoria[1726]=0xFE;
    memoria[1727]=0xFF;
    memoria[1728]=0xFF;
    memoria[1729]=0x80;
    memoria[1730]=0xFB;
    memoria[1731]=0x3;
    memoria[1732]=0x0F;
    memoria[1733]=0x85;
    memoria[1734]=0xCD;
    memoria[1735]=0xFE;
    memoria[1736]=0xFF;
    memoria[1737]=0xFF;
    memoria[1738]=0xB0;
    memoria[1739]=0x2D;
    memoria[1740]=0x51;
    memoria[1741]=0x53;
    memoria[1742]=0xE8;
    memoria[1743]=0xFD;
    memoria[1744]=0xFB;
    memoria[1745]=0xFF;
    memoria[1746]=0xFF;
    memoria[1747]=0x5B;
    memoria[1748]=0x59;
    memoria[1749]=0xB3;
    memoria[1750]=0x2;
    memoria[1751]=0xE9;
    memoria[1752]=0xBB;
    memoria[1753]=0xFE;
    memoria[1754]=0xFF;
    memoria[1755]=0xFF;
    memoria[1756]=0x80;
    memoria[1757]=0xFB;
    memoria[1758]=0x3;
    memoria[1759]=0x0F;
    memoria[1760]=0x84;
    memoria[1761]=0xB2;
    memoria[1762]=0xFE;
    memoria[1763]=0xFF;
    memoria[1764]=0xFF;
    memoria[1765]=0x80;
    memoria[1766]=0xFB;
    memoria[1767]=0x2;
    memoria[1768]=0x75;
    memoria[1769]=0x0C;
    memoria[1770]=0x81;
    memoria[1771]=0xF9;
    memoria[1772]=0x0;
    memoria[1773]=0x0;
    memoria[1774]=0x0;
    memoria[1775]=0x0;
    memoria[1776]=0x0F;
    memoria[1777]=0x84;
    memoria[1778]=0xA1;
    memoria[1779]=0xFE;
    memoria[1780]=0xFF;
    memoria[1781]=0xFF;
    memoria[1782]=0x51;
    memoria[1783]=0xE8;
    memoria[1784]=0x14;
    memoria[1785]=0xFD;
    memoria[1786]=0xFF;
    memoria[1787]=0xFF;
    memoria[1788]=0x59;
    memoria[1789]=0x89;
    memoria[1790]=0xC8;
    memoria[1791]=0xC3;
}
/**
 * @brief 
 * 
 * @param vector 
 * @param contador 
 * @param bits 
 */
void cargar32Impura(unsigned char vector[], int &contador, int bits){
    unsigned char b1, b2, b3, b4;
    b1=(unsigned char)((bits>>24)&255);
    b2=(unsigned char)((bits>>16)&255);
    b3=(unsigned char)((bits>>8)&255);
    b4=(unsigned char)(bits&255);
    vector[contador++]=b4;
    vector[contador++]=b3;
    vector[contador++]=b2;
    vector[contador++]=b1;
}
/**
 * @brief 
 * 
 * @param vector 
 * @param contador 
 * @param bits 
 */
void cargar32Pura(unsigned char vector[], int contador, int bits){
    unsigned char b1, b2, b3, b4;
    b1=(unsigned char)((bits>>24)&255);
    b2=(unsigned char)((bits>>16)&255);
    b3=(unsigned char)((bits>>8)&255);
    b4=(unsigned char)(bits&255);
    vector[contador++]=b4;
    vector[contador++]=b3;
    vector[contador++]=b2;
    vector[contador++]=b1;
}
/**
 * @brief 
 * 
 * @param memoria 
 * @param posMemo 
 * @param salto hacia adelante (+) o hacia atras (-)
 */
void fixUp(unsigned char memoria[], int posMemo, int salto){
    cargar32Pura(memoria, posMemo, salto);
}
/**
 * @brief 
 * copia el primero en el segundo
 * @param memoria 
 * @param contadorMemoria 
 * @param datosSemanticos
 */
void movEdi(unsigned char memoria[], int &contadorMemoria, int distancia){
    memoria[contadorMemoria++]=0xBF;
    cargar32Impura(memoria, contadorMemoria, distancia);
}
/**
 * @brief 
 * copia el primero en el segundo
 * @param memoria 
 * @param contadorMemoria 
 * @param datosSemanticos
 */
void movEaxEdi(unsigned char memoria[], int &contadorMemoria, int distancia){
    memoria[contadorMemoria++]=0x8B;
    memoria[contadorMemoria++]=0x87;
    cargar32Impura(memoria, contadorMemoria, distancia);
}
/**
 * @brief 
 * copia el primero en el segundo
 * @param memoria 
 * @param contadorMemoria 
 * @param datosSemanticos
 */
void movEdiEax(unsigned char memoria[], int &contadorMemoria, int distancia){
    memoria[contadorMemoria++]=0x89;
    memoria[contadorMemoria++]=0x87;
    cargar32Impura(memoria, contadorMemoria, distancia);
}
/**
 * @brief 
 * copia el primero en el segundo
 * @param memoria 
 * @param contadorMemoria 
 * @param datosSemanticos
 */
void movEax(unsigned char memoria[], int &contadorMemoria, int distancia){
    memoria[contadorMemoria++]=0xB8;
    cargar32Impura(memoria, contadorMemoria, distancia);
}
/**
 * @brief 
 * intercambia los valores de los operandos
 * @param memoria 
 * @param contadorMemoria 
 */
void xchgEaxEbx(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x93;
}
/**
 * @brief 
 * MANDA EL VALOR DEL OPERANDO A LA PILA
 * @param memoria 
 * @param contadorMemoria 
 */
void pushEax(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x50;
}
/**
 * @brief 
 * EXTRAE EL VALOR DE LA PILA Y LO COLOCA EN EL OPERANDO
 * @param memoria 
 * @param contadorMemoria 
 */
void popEax(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x58;
}
/**
 * @brief 
 * SUMA AMBOS OPERANDOS Y COLOCA EL RESULTADO EN EL PRIMERO
 * @param memoria 
 * @param contadorMemoria 
 */
void popEbx(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x5B;
}/**
 * @brief 
 * 
 * @param memoria 
 * @param contadorMemoria 
 */
void addEaxEbx(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x01;
    memoria[contadorMemoria++]=0xD8;
}
/**
 * @brief 
 * LE RESTA EL SEGUNDO OPERANDO AL PRIMERO Y COLOCA EL RESULTADO EN EL PRIMER OPERANDO
 * @param memoria 
 * @param contadorMemoria 
 */
void subEaxEbx(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x29;
    memoria[contadorMemoria++]=0xD8;
}
/**
 * @brief 
 * COLOCA EN EDx:EAX EL PRODUCTO DE EAX POR EBX
 * @param memoria 
 * @param contadorMemoria 
 */
void imulEbx(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0xF7;
    memoria[contadorMemoria++]=0xEB;
}
/**
 * @brief 
 * DIVIDE EDx:EAX POR EL OPERANDO Y COLOCA EL COCIENTE EN EAX Y EL RESTO EN EDX
 * @param memoria 
 * @param contadorMemoria 
 */
void idivEbx(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0xF7;
    memoria[contadorMemoria++]=0xFB;
}
/**
 * @brief 
 * LLENA TODOS LOS BITS DE EDX CON EL VALOR DEL BIT DEL SIGNO DE EAX
 * @param memoria 
 * @param contadorMemoria 
 */
void cdq(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x99;
}
/**
 * @brief 
 * CAMBIA EL SIGNO DE EAX
 * @param memoria 
 * @param contadorMemoria 
 */
void negEax(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0xF7;
    memoria[contadorMemoria++]=0xD8;
}
/**
 * 
 * @brief 
 * CALCULA EL “Y” ENTRE LOS OPERANDOS Y MODIFICA VARIAS BANDERAS, ENTRE ELLAS PF (PARITY FLAG)
 * @param memoria 
 * @param contadorMemoria
 * @param valor 
 */
void testAl(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0xA8;
}
/**
 * @brief 
 * COMPARA EL PRIMER OPERANDO CON EL SEGUNDO PARA QUE, SEGÚN EL RESULTADO DE LA COMPARACIÓN, PUEDAN HACERSE SALTOS CONDICIONALES A CONTINUACIÓN
 * @param memoria 
 * @param contadorMemoria
 */
void cmpEbxEax(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x39;
    memoria[contadorMemoria++]=0xC3;
}
/**
 * @brief 
 * JE (JUMP IF =)
 * @param memoria 
 * @param contadorMemoria 
 */
void je_jz(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x74;
}
/**
 * @brief 
 * JNE (JUMP IF NOT =)
 * @param memoria 
 * @param contadorMemoria 
 */
void jne_jnz(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x75;
}
/**
 * @brief 
 * JG (JUMP IF >)
 * @param memoria 
 * @param contadorMemoria 
 */
void jg(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x7F;
}
/**
 * @brief 
 * JGE (JUMP IF > OR =)
 * @param memoria 
 * @param contadorMemoria 
 */
void jge(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x7D;
}
/**
 * @brief 
 * JL (JUMP IF <)
 * @param memoria 
 * @param contadorMemoria 
 */
void jl(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x7C;
}
/**
 * @brief 
 * JLE (JUMP IF < OR =)
 * @param memoria 
 * @param contadorMemoria 
 */
void jle(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x7E;
}
/**
 * @brief 
 * JPO (JUMP IF PARITY ODD)
 * @param memoria 
 * @param contadorMemoria 
 */
void jpo(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0x7B;
}
/**
 * @brief 
 * SALTA A LA DIRECCIÓN UBICADA abcdefgh BYTES ANTES O DESPUÉS DE LA DIRECCIÓN ACTUAL
 * @param memoria 
 * @param contadorMemoria 
 */
void jmp(unsigned char memoria[], int &contadorMemoria, int desplazamiento){
    memoria[contadorMemoria++]=0xE9;
    cargar32Impura(memoria, contadorMemoria, desplazamiento);
}
/**
 * @brief 
 * INVOCA LA SUBRUTINA UBICADA abcdefgh BYTES ANTES O DESPUÉS DE LA DIRECCIÓN ACTUAL
 * @param memoria 
 * @param contadorMemoria 
 */
void call(unsigned char memoria[], int &contadorMemoria, int desplazamiento){
    memoria[contadorMemoria++]=0xE8;
    cargar32Impura(memoria, contadorMemoria, desplazamiento);
}
/**
 * @brief 
 * RETORNA AL PUNTO DESDE DONDE SE LLAMÓ UNA SUBRUTINA
 * @param memoria 
 * @param contadorMemoria 
 */
void ret(unsigned char memoria[], int &contadorMemoria){
    memoria[contadorMemoria++]=0xC3;
}
/**
 * @brief 
 * 
 * @param archivo 
 * @param codigo 
 * @param datosEsperado 
 * @param datoRecibido 
 * @param lineaError 
 */
void errores(FILE *archivo, int codigo, string datosEsperado, informacion datoRecibido, int lineaError){
    if(codigo==0){
        cout<<"\nTerminación correcta del compilador"<<endl;
        fclose(archivo);
    }
    else{
        switch(codigo){
            case 1:
                cout<<"En la línea: "<<lineaError<<". Falta el punto de terminación. Dato recibido: "<<datoRecibido.datoLeido<<endl;
                break;
                
            case 2:
                cout<<"En la línea: "<<lineaError<<". Hay contenido luego del punto. Dato recibido: "<<datoRecibido.datoLeido<<endl;
                break;
            case 3:
                cout<<"En la línea: "<<lineaError<<". Se esperaba: "<<datosEsperado<<". Dato recibido: "<<datoRecibido.datoLeido<<endl;
                break;
            case 4:
                cout<<"En la línea: "<<lineaError<<". Ya fue declarado ese "<< datoRecibido.tipoDato<<" \""<<datoRecibido.datoLeido+"\""<<endl;
                break;
            case 5:
                cout<<"En la línea: "<<lineaError<<". \""<<datoRecibido.datoLeido<<"\" no fue declarado"<<endl;
                break;
            case 6:
                cout<<"En la línea: "<<lineaError<<". Se ha excedido la cantidad de identificadores posbiles"<<endl;
            break;
        }
        fclose(archivo);
        exit(1);
    }    
}