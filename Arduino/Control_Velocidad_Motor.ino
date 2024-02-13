/***************************************************************
* Control de velocidad de Motor CC mediante encoder infrarrojo
* Desarrollado por Garikoitz Martínez [garikoitz.info] [01/2024]
* https://garikoitz.info/blog/?p=2131
***************************************************************/
/***************************************************************
* Librerías
***************************************************************/
#include <PID_v1.h>
/***************************************************************
* Variables
***************************************************************/
int IN2 = 2; // Encoder en pin 2
int IN3 = 5; // Input3 conectada al pin 5
int IN4 = 4; // Input4 conectada al pin 4
int ENB = 3; // ENB conectada al pin 3 de Arduino
const long baud = 9600; // serial baud rate
int Ts = 100; //Sample time in ms
unsigned long previousMillis = 0;
static volatile unsigned long lastTimeDebounce = 0; //Tiempo del rebote
//Motores
float M1_PV = 0.0;
float M1_SP = 0.0;
int M1_OP = 0;
int M1SPMIN = 100;   //en AUTO
int M1SPMAX = 350; //con este motor max 400 RPMs a 7V
int M1OPMIN = 0;
int M1OPMAX = 100; //5% = 100pwm -> se empieza a mover a partir de 100pwm cuando vienes de motor parado
//Encoder
double RPMs = 0.0;
double RPMsFilter = RPMs;
double alpha = 0.1; //Filtro EMA 0.2-0.4 con Ts=1000ms y 0.1-0.2 con Ts=100ms
volatile int frecuencia = 0;//numero de pulsos leidos por Arduino
int nRanuras = 20; //numero de ranuras del disco
// global variables
const char sp = ' ';           
const char nl = '\n';
boolean newData = false; 
char Buffer[64]; 
int buffer_index = 0; 
String cmd;  
float val;
/***************************************************************
* Sintonías calculadas (Lazo abierto --> Kc=1.125 T0=.4s Tp=.3s)
***************************************************************/
//double Kc=0.67, Ki=1.76, Kd=0.00;   //CC-25
//double Kc=0.40, Ki=0.87, Kd=0.00;   //CC-10
//double Kc=0.08, Ki=0.27, Kd=0.00;   //LAMBDA Tf=3s
//---------------------------------------------------------------
double SetpointM1, InputM1, OutputM1;
double KcM1=0.67, KiM1=1.76, KdM1=0.0; 
PID PIDM1(&InputM1, &OutputM1, &SetpointM1, KcM1, KiM1, KdM1, P_ON_E, DIRECT);    //PI-D
//PID PIDT1(&InputT1, &OutputT1, &SetpointT1, KcT1, KiT1, KdT1, P_ON_M, DIRECT);  //I-PD
/***************************************************************
* SETUP
***************************************************************/
void setup()
{
  pinMode (ENB, OUTPUT);
  pinMode (IN3, OUTPUT);
  pinMode (IN4, OUTPUT);
  //Preparamos la salida para que el motor gire en un sentido
  digitalWrite (IN3, LOW);
  digitalWrite (IN4, HIGH);
  while (!Serial) {
    // wait for serial port to connect.
  }
  Serial.begin(baud);
  //Serial.flush();
  attachInterrupt(digitalPinToInterrupt(IN2),interrupcion0,RISING); //Lectura de los pulsos Encoder
  //
  previousMillis=0;
  PIDM1.SetOutputLimits(0, 255);
  PIDM1.SetMode(MANUAL);
  if (Ts < 100){
    PIDM1.SetSampleTime(Ts);
  }
}
/***************************************************************
* BUCLE PRINCIPAL
***************************************************************/
void loop()
{
  if (millis() - previousMillis > Ts)
  {
    //Lectura del encoder
    RPMs = (frecuencia * 60000.0)/((millis()-previousMillis)*nRanuras);
    RPMsFilter = alpha*RPMs+(1.0-alpha)*RPMsFilter; //Filtro EMA
    M1_PV = RPMsFilter;
    previousMillis = millis();
    frecuencia = 0;
    //Funciones Lectura/Escritura Serial
    LeoCMD();
    ProcesoCMD();
    EjecutoCMD();
    //PID (Parámetros vía serial cmds)
    if (PIDM1.GetMode() == 1){//AUTO
        InputM1 = M1_PV;
        SetpointM1 = M1_SP;
        PIDM1.Compute();
        M1_OP = map(OutputM1, 0, 255, 0, 100); //PWM -> %
        analogWrite(ENB,OutputM1);
     }else if (PIDM1.GetMode() == 0) {//MANUAL
		/*>>INICIALIZACIÓN<< -> para evitar problemas al pasar de MANUAL a AUTO
		(1) --> PV Tracking -> igualamos SP a PV
		(2) --> Calculamos la salida de control para que 'inicialice' correctamente el PID sin provocar saltos bruscos al pasar de Manual a Auto.
		*/
		M1_SP = M1_PV; //(1)
        OutputM1 = map(M1_OP, 0, 100, 0, 255); //(2)
     }
     //Para Arduino COM Plotter
     Serial.print("#");        //Char inicio
     Serial.print(M1_SP,0);    //
     Serial.write(" ");        //separador
     Serial.print(M1_PV,0);    //
     Serial.write(" ");        //separador
     Serial.print(M1_OP);      //
     Serial.println();
    //
  } //millis
} //loop
/***************************************************************
* FUNCIONES
***************************************************************/
void interrupcion0()
{
  if(digitalRead(IN2) && (micros()-lastTimeDebounce >= 500))
  {
    lastTimeDebounce = micros();
    frecuencia++;  
  }  
}
//No hace falta tocar
void LeoCMD() {
  while (Serial && (Serial.available() > 0) && (newData == false)) {
    int byte = Serial.read();
    if ((byte != '\r') && (byte != nl) && (buffer_index < 64)) {
      Buffer[buffer_index] = byte;
      buffer_index++;
    }
    else {
      newData = true;
    }
  }   
}
//No hace falta tocar
void ProcesoCMD(void) {
  if (newData) {
    String read_ = String(Buffer);
    // separate command from associated data
    int idx = read_.indexOf(sp);
    cmd = read_.substring(0, idx);
    cmd.trim();
    cmd.toUpperCase();
    // extract data. toFloat() returns 0 on error
    String data = read_.substring(idx + 1);
    data.trim();
    val = data.toFloat();
    // reset parameter for next command
    memset(Buffer, 0, sizeof(Buffer));
    buffer_index = 0;
    newData = false;
  }
}
//Aquí personaliza tus comandos
void EjecutoCMD(void) {
  if (cmd == "M1A") {
    PIDM1.SetMode(AUTOMATIC);
  }
  else if (cmd == "M1M") {
    PIDM1.SetMode(MANUAL);
  }
  else if (cmd == "M1SP") {
    if (PIDM1.GetMode() == 1){
      if (val > M1SPMAX){
        val = M1SPMAX;
      }
    if (val < M1SPMIN){
        val = M1SPMIN;
      }
    M1_SP = val;
    }
  }
  else if (cmd == "M1OP") {
    if (PIDM1.GetMode() == 0){
      if (val > M1OPMAX){
        val = M1OPMAX;
      }
      if (val < M1OPMIN){
        val = M1OPMIN;
      }
      M1_OP = val;
      //map(value, fromLow, fromHigh, toLow, toHigh)
      val = map(val, 0, 100, 0, 255);
      analogWrite(ENB,val);
    }
  }
  else if (cmd == "M1KC") {
    PIDM1.SetTunings(val,KiM1,KdM1);
  }
  else if (cmd == "M1KI") {
    PIDM1.SetTunings(KcM1,val,KdM1);
  }
  else if (cmd == "M1KD") {
    PIDM1.SetTunings(KcM1,KiM1,val);
  }
  else if (cmd == "FILTRO") {
    if (val > 1){
        val = 1;
    }
    if (val <= 0){
        val = 0.01;
    }
    alpha = val;
  }
  Serial.flush();
  cmd = "";
}
