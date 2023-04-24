char *nombre_perfil[] = {"PROFILE 1", "PROFILE 2", "PROFILE 3", "LOC CUSTOM","DEBUG1","PRUEBA PID"};
//{perfil{etapa{temperatura, segundos}}}//     perfil 1                           |  perfil 2                                                |  perfil 3                                              |   perfil 4                                          |  perfil 5                                       |   perfil 6          
int perfiles[][5][2] = {{{150, 50}, {150, 10}, {300, 100}, {300, 10},{50, 100}}, {{150, 80}, {250, 80}, {300, 100}, {300, 60},{50, 20}}, {{180, 80}, {240, 70}, {300, 120}, {300, 20},{150, 100}},{{200, 50}, {250, 70}, {300, 30}, {300, 50},{150, 10}},{{120, 5}, {250, 2}, {250, 2}, {200, 0},{150, 0}},{{120, 5}, {250, 2}, {250, 2}, {200, 0},{150, 0}}};
int numero_perfiles = (sizeof (perfiles) / sizeof (perfiles[0]))-1;
char *texto_tipo_pcb[] = {"One side top", "Double side", "One side top BGA", "Double side BGA"};
const int numero_tipo_pcb = (sizeof(texto_tipo_pcb) / sizeof(texto_tipo_pcb[0]))-1; //numbers -1 (zero)
char *version_programa = "V: 1.0", *numero_serie = "R00001";

//char *abecedario[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
char *user = "DIY Reflow"; //20 character max
char *description = "GitHub:Balance2Sound"; //CAN BE USED AS A CODE OR REFERENCE NUMBER //30 character max
