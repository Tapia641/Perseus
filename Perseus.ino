#include "AFMotor.h"
#define pinTrig A0 //SALIDA DE PULSO ULTRÁSONICO
#define pinEcho A2 //RECEPTOR
#define pinLed A5  //PIN PARA MOSTRAR CAMBIOS
#define coef_aprend 0.5
//ACTIVAMOS EL MOTOR 1 Y 4 POR COMODIDAD A LOS LATERALES.
AF_DCMotor Motor1(1);
AF_DCMotor Motor4(4);

//VARIABLES GLOBALES
long distancia;
long tiempo;
float Distancia[5];

void setup()
{
  /*------------------------------------------------------------------------------*/
  //EN ESTA SECCIÓN DEFINIMOS LOS PINES
  Serial.begin(9600);

  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);
  pinMode(pinLed, OUTPUT);

  Motor1.setSpeed(255);
  Motor4.setSpeed(255);

  /*------------------------------------------------------------------------------*/
  //COMENZAMOS A CAPTURAR LAS DISTANCIAS
  float contador = 0, contador2 = 0;
  String texto = "";
  for (int i = 0; i < 6; i++)
  {
    if (i < 3)
    {
      //SE OBTIENEN LOS TRES DISTANCIAS DESEADAS PARA FRENAR
      texto += "Distancia ";
      texto += String(i + 1);
      texto += " para frenar:";
      Serial.println(texto);
      Distancia[i] = Get_Distancia();
      Serial.print(Distancia[i]);
      Serial.println(" cm");
      //ESPERAMOS 4 SEGUNDOS PARA CAPTURAR LA DISTANCIA SIGUIENTE
      delay(4000);
      Parpadear(1000);
    }
    else
    {
      //SE OBTIENEN LOS TRES DISTANCIAS DESEADOS PARA NO FRENAR
      texto += "Distancia ";
      texto += String(i + 1);
      texto += " para no frenar:";
      Serial.println(texto);
      Distancia[i] = Get_Distancia();
      Serial.print(Distancia[i]);
      Serial.println(" cm");
      //ESPERAMOS 4 SEGUNDOS PARA CAPTURAR LA DISTANCIA SIGUIENTE
      delay(4000);
      Parpadear(1000);
    }
    texto = "";
  }
  /*------------------------------------------------------------------------------*/
  //SECCIÓN DE ENTRENAMIENTO
  float net = 0;
  float Salida = 0;
  float bias[6] = {1, 1, 1, 1, 1, 1};
  float x0_bias = 1, x1_bias = 0;
  float sal_des[6] = {1, 1, 1, 0, 0, 0};
  float w0 = 1, cambio_w0 = 0;
  float w1 = 1, cambio_w1 = 0;
  float y = 0;
  /*
 * Pruebas estáticas
  Distancia[0] = 0.50;
  Distancia[1] = 0.75;
  Distancia[2] = 1.00;
  Distancia[3] = 1.25;
  Distancia[4] = 1.50;
  Distancia[5] = 2.00;
  */

  int mayor = 0;
  //while ((cambio_w0 + cambio_w1) != 0)
  while (true)
  {
    for (int i = 0; i < 6; i++)
    {
      for (int j = 0; j < 6; j++)
      {
        mayor = max(mayor, Distancia[j]);
      }

      x1_bias = Distancia[0] / mayor;
      //RECORREMOS LAS POSICIONES
      float aux = Distancia[0];
      for (int j = 0; j < 6; j++)
      {
        if (j == 6)
        {
          Distancia[j] = aux;
        }
        else
        {
          Distancia[j] = Distancia[j + 1];
        }
      }
      Distancia[5] = aux;
      w0 = w0 + cambio_w0;
      w1 = w1 + cambio_w1;
      net = x0_bias * w0 + x1_bias * w1;
      if (net >= 0)
      {
        y = 1;
      }
      else
      {
        y = 0;
      }

      cambio_w0 = coef_aprend * x0_bias * (sal_des[i] - y);
      cambio_w1 = coef_aprend * x1_bias * (sal_des[i] - y);

      Serial.print("  cambio_w0: ");
      Serial.print(cambio_w0);

      Serial.println("");
      Serial.print("cambio_w1: ");
      Serial.print(cambio_w1);
    }
    if ((cambio_w0 + cambio_w1) == 0.00)
    {
      //NOS ASEGURAMOS DE QUE EL MARGEN DE ERROR NO CAMBIE EN 50 POSICIONES MÁS ADELANTE
      if (contador == 50)
        break;
      else
      {
        contador++;
      }
    }
    else
    {
      contador2++;
      if (contador2 == 50)
        contador = 0;
    }
  }
  digitalWrite(pinLed, HIGH);
}

void loop()
{
  distancia = Get_Distancia();
  if (distancia <= 30)
  {
    //DETENER MOTORES
    //TERCERA CONDICIÓN: DISTANCIAS MENORES O IGUALES A 30 CM 0% PWM
    Motor1.run(RELEASE);
    Motor4.run(RELEASE);
  }
  else if (distancia <= Distancia[0] || distancia <= Distancia[1] || distancia <= Distancia[2])
  {
    //DISMINUIR SU ENERGIA
    //SEGUNDA CONDICIÓN: CONSIDERAR  DISTANCIAS OBTENIDAS PWM 30%
    Motor1.setSpeed(100);
    Motor4.setSpeed(100);
    //AVANZAR
    Motor1.run(FORWARD);
    Motor4.run(FORWARD);
  }
  else
  {
    Motor1.setSpeed(255);
    Motor4.setSpeed(255);
    Motor1.run(FORWARD);
    Motor4.run(FORWARD);
  }
  delay(1000);
}

//FUNCIÓN PARA OBTENER LA DISTANCIA
int Get_Distancia()
{
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(5);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  tiempo = pulseIn(pinEcho, HIGH);
  distancia = int(0.017 * tiempo);
  return distancia;
}

void Parpadear(int tiempo)
{
  digitalWrite(pinLed, HIGH);
  delay(tiempo);
  digitalWrite(pinLed, LOW);
}
/*int max(int x, int y)
{
  return ((x) > (y)) ? (x) : (y);
}*/
