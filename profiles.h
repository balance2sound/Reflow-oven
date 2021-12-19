char *nombre_perfil[] = {"PROFILE 1", "PROFILE 2", "PROFILE 3", "LOC CUSTOM"};
//{perfil{etapa{temperatura, segundos}}}
int perfiles[][4][2] = {{{150, 60}, {200, 120}, {250, 40}, {150, 0}}, {{120, 10}, {250, 10}, {250, 10}, {200, 0}}, {{250, 80}, {300, 70}, {300, 120}, {50, 0}}};
int numero_perfiles = 2;
char *texto_tipo_pcb[] = {"One side top", "Double side", "One side top BGA", "Double side BGA"};
const int numero_tipo_pcb = 3; //numbers -1 (zero)
char *version_programa = "V: 1.0", *numero_serie = "R00001";

char *abecedario[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
char *user = "Balance 2 Sound"; //20 character max
char *description = "2021"; //CAN BE USED AS A CODE OR REFERENCE NUMBER //30 character max

//pines
