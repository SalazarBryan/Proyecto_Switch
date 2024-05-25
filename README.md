PROYECTO ELÉCTRICO.



input_port_counts: Es un arreglo de enteros (unsigned int) que indica cuántas tramas hay disponibles en cada puerto de entrada.

input_port_streams: Es un arreglo ethernet_frame que representa las tramas de entrada para cada puerto. MAX_PORTS indica el número máximo de puertos y MAX_FRAMES_PER_PORT el número máximo de tramas por puerto.

output_port_counts: Es un arreglo como input_port_counts, que lleva la cuenta de cuántas tramas están listas para ser enviadas.



Lectura y Almacenamiento de Tramas de Entrada:

Un bucle for recorre todos los puertos (MAX_PORTS).

input_port_counts[i] verifica si hay alguna trama disponible en el puerto i.

buffer_count[i] < MAX_FRAMES_PER_PORT verifica si hay espacio en el buffer temporal port_buffers para almacenar más tramas.
Si ambas condiciones se cumplen, una trama se lee del puerto de entrada i (input_port_streams[i][--input_port_counts[i]]) y se almacena en el buffer temporal correspondiente (port_buffers[i][buffer_count[i]++]).


(((struct Port {
    ethernet_frame frames[MAX_FRAMES_PER_PORT];
    unsigned int count;
};

Port input_ports[MAX_PORTS];
Port output_ports[MAX_PORTS];))) -- ejemplo como declarar puertos

input_ports: Array de estructuras Port, donde cada Port contiene un array de ethernet_frame para almacenar tramas entrantes y un contador count para el número de tramas disponibles.