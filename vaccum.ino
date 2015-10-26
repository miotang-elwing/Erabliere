// ------------------------------------------------------------------
// Application pour faire la lecture d'un capteur de vide : MP3V5050V
// ------------------------------------------------------------------
// Dans cette application nous ajoutons une variable avec Spark.variable() dans le cloud afin de pouvoir lire le niveau de vaccum.
// De plus nous allons publier cette variable avec Spark.publish()

int vaccum = A0; // C'est sur cette pin que le capteur de vide est branché.

int power = A5; // C'est `partir de cette pin que le capteur prendra son énergie.

int boardLed = D7;  // C'est la led sur le photon

// La raison que nous branchons le capteur sur A5 au lieu que sur la pin 3.3v, c'est que nous voulons un voltage super stable pour le capteur.
// De cette façon, lorsque nous allons faire la lecture de la valeur sur le capteur, nous pouvons correctement calculer la différence de lecture.


double analogvalue; // Nous déclarons ici la variable double analogvalue,  que nous utiliserons plutard pour sauvarder la valeur du capteur de vide.
int val; // Variable temporaire contenant la lecture de 0 à 4096 du capteur
int val1; // Variable temporaire contenant la lecture divisé par 10
int val2; // Variable temporaire contenant la lecture divisé par 10 prédédente.
float calibration;  //  Variable contenant la valeur de calibration du capteur.

// Maintenant nous faisons la configuration du système

void setup() {

    // Premièrement nous déclarons l'ensemble des pins.
    
    pinMode(vaccum,INPUT);  // Le capteur de vide sera lu sur cette pin
    pinMode(power,OUTPUT); // La pin qui va alimenter le cpateur de vide (sending out consistent power)
    pinMode(boardLed,OUTPUT);   // La pin qui est la led sur le Photon.

    // Maintenant, nous envoyons l'énergie au maximum au capteur.

    digitalWrite(power,HIGH);
    
    //  Nous nous assurons que la led sur le photon est etteinte.
    digitalWrite(boardLed,LOW);

    
    // Nous déclarons la variable Spark.variable() pour qu'elle soit accessible à partir du cloud
    Spark.variable("analogvalue", &analogvalue, DOUBLE);
    
    // Nous faisons une calibration du capteur de vide afin d'avoir une valeur de référence.
    calibration=calibre();
    
    // Nous initialisons les variables.
    val2=0;
    val1=0;
    val=0;
}


// La fonction principale "loop"

void loop() {
    
    // Fait une lecture du capteur
    val = analogRead(vaccum);
    
    val1=(val/10);
    
    //Publication de la valeur (entre 0 et 4096) retourné par le capteur
    Spark.publish("Valeur : ",String(val1),1);
    
    // Ici nous validons s'il y a eu un changement significatif dans la pression depuis la dernière lecture
    //
    // Nous voulons valider une variation d'un dixième de point de la lecture analogique du capteur.
    // Dans la variable val1, il y a la lecture actuelle.  Dans la variable val2, il y a la lecture précédente
    //
    // S'il y a une variation, alors nous faisons une lecture convertie.
    
    if (val1!=val2) {
        
       // Fait clignoter la led sur le Photon pour indiquer qu'il y a une variation dans la pression
       
       digitalWrite(boardLed,HIGH);
       delay(100);
       digitalWrite(boardLed,LOW);
       delay(100);
       digitalWrite(boardLed,HIGH);
       delay(100);
       digitalWrite(boardLed,LOW);
       delay(100);
       
       // Calcul de la pression réel
       analogvalue = raw2kPa(val, calibration);
       
       // Publication sur de la valeur de pression négative (vide)
       Spark.publish("Vaccum : ",String(analogvalue),1);
   }
   val2=val1;
  delay(1000);
}

//Routine de calibration du capteur de vide
double calibre() {
  
  double calibrer = 3850.24 - analogRead(vaccum);
  
  // 3850.24 correspond à la valeur idéal pour une pression de 0 Kpa
  // Il s'obtient ainsi : x = 0.94 * Vmax  (avec Vmax la valeur pour VCC soit 4096)
  
  // Nous faison cinq lectures de suite afin de réchaufer le capteur
  for (int i=0; i <= 5; i++){

  delay(500);

  calibrer = 3850.24 - analogRead(vaccum);

  }

  return calibrer;
}


/* Fonction de conversion valeur numérique <> pression en Kpa */
double raw2kPa(int raw, double calibration) {
  /*
   * D'aprés le datasheet du MP3V5050V
   * Vout = Vs * (P * 0.018 + 0.94)
   * soit : P = ((Vout / Vs) - 0.94) / 0.018
   * avec Vout = Vraw + Vcalibration et Vs = Vmax = 4096
   * pour convertir de kpa to Hg (inch of mercure) il faut multiplier par 0.295301 
   */

  return ((((raw + calibration) / 4096.0) - 0.94) / 0.018)*0.295301;  // multiplie par 0.295301 pour avoir la valeur en Hg
}
