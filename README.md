# CONTROL DE VELOCIDAD DE UN MOTOR

En esta entrada, exploraremos una implementación básica y fácil de entender del control de velocidad de un motor usando Arduino. Aunque es un ejemplo simple, sirve como un excelente punto de partida hacia implementaciones más avanzadas y profesionales. Para este proyecto, he utilizado un conocido motorreductor junto con un encoder infrarrojo, elementos comunes en la construcción de maquetas de vehículos a pequeña escala.

<h2>Esquema de conexiones</h2>
<p align="center">
  <img src="https://garikoitz.info/blog/wp-content/uploads/2024/01/Motor_encoder_v1_bb-1024x803.png" width="450" alt="conexiones">
</p>

<h2>Esquema de control</h2>
<p align="center">
  <img src="https://garikoitz.info/blog/wp-content/uploads/2024/01/Esquema_control_motorcc.png" width="120" alt="conexiones">
</p>

En el esquema de control hay dos componentes principales identificados:

<b>Encoder (SC-1)</b>: Este dispositivo se utiliza para capturar la velocidad de rotación del eje del motor y convertirla en señales eléctricas. Cada vez que el eje gira, el encoder genera pulsos que se pueden contar para determinar la velocidad de rotación.

<b>Motor de corriente continua</b>: Representado en la parte inferior de la imagen. El motor es la parte del sistema que realiza el trabajo mecánico y cuya velocidad se desea controlar. La señal de control proveniente del controlador PID modula la energía suministrada al motor para mantener la velocidad deseada.

El motor se pone en marcha y su velocidad de rotación es capturada por el encoder. Esta velocidad es enviada al controlador PID, que compara la velocidad real (variable de proceso o PV) con la velocidad deseada (punto de consigna o SP). Si hay una diferencia entre la velocidad real y la deseada, el controlador PID calcula una señal de control (OP) adecuada para ajustar la potencia del motor y corregir el error. Esta señal de control ajusta la velocidad del motor, ya sea aumentándola o disminuyéndola para que coincida con el. Este es un proceso continuo y dinámico donde el controlador PID constantemente ajusta la señal de control en respuesta a las variaciones en la velocidad del motor para mantener la velocidad estable. En resumen, este esquema representa un sistema de control en lazo cerrado para mantener una velocidad constante en un motor de corriente continua, utilizando para ello un controlador PID que procesa la información del encoder y ajusta la señal de potencia al motor.

Tenéis más información en la entrada del blog: https://garikoitz.info/blog/2024/01/sintonizar-pid-con-arduino-control-de-velocidad-de-un-motor/
