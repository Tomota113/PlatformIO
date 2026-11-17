import cv2
import serial
import time
import sys
import os
import urllib.request

# 1. Vérification et téléchargement automatique du modèle Haar Cascade
XML_FILE = "haarcascade_frontalface_default.xml"
XML_URL = "https://raw.githubusercontent.com/opencv/opencv/master/data/haarcascades/haarcascade_frontalface_default.xml"

if not os.path.exists(XML_FILE):
    print("Téléchargement du fichier de détection de visage...")
    try:
        urllib.request.urlretrieve(XML_URL, XML_FILE)
        print("Téléchargement terminé !")
    except Exception as e:
        print(f"Erreur lors du téléchargement du fichier XML : {e}")
        sys.exit()

face_cascade = cv2.CascadeClassifier(XML_FILE)

# 2. Configuration du port série sous Ubuntu
SERIAL_PORT = '/dev/ttyACM0'  # Adapte en '/dev/ttyUSB0' si nécessaire
BAUD_RATE = 9600

# 3. Connexion à l'Arduino
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    time.sleep(2)
    ser.reset_input_buffer()
    ser.reset_output_buffer()
    print(f"Connecté à l'Arduino sur {SERIAL_PORT}")
except Exception as e:
    print(f"Erreur d'ouverture du port série {SERIAL_PORT} : {e}")
    sys.exit()

# 4. Initialisation de la webcam du PC
cap = cv2.VideoCapture(0)

# Position initiale du servomoteur (au centre)
servo_angle = 90
last_sent_time = time.time()

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        print("Erreur d'accès à la webcam.")
        break

    # Effet miroir horizontal
    frame = cv2.flip(frame, 1)
    height, width, _ = frame.shape
    center_x_screen = width // 2

    # Conversion en niveaux de gris
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    
    # Détection des visages
    faces = face_cascade.detectMultiScale(
        gray,
        scaleFactor=1.1,
        minNeighbors=5,
        minSize=(60, 60)
    )

    if len(faces) > 0:
        # Sélection du premier visage (x, y, w, h)
        (x, y, w, h) = faces[0]

        # Dessin du rectangle et du point central
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        cx = x + w // 2
        cy = y + h // 2
        cv2.circle(frame, (cx, cy), 5, (0, 255, 0), -1)

        # Calcul de l'écart par rapport au centre de l'écran
        error_x = cx - center_x_screen

        # Zone morte de ±30 pixels pour éviter les oscillations du servo
        if abs(error_x) > 30:
            if error_x > 0 and servo_angle > 0:
                servo_angle -= 1
            elif error_x < 0 and servo_angle < 180:
                servo_angle += 1

        # Envoi de la consigne d'angle à l'Arduino (maximum 20 Hz)
        if time.time() - last_sent_time > 0.05:
            ser.write(f"{servo_angle}\n".encode('utf-8'))
            last_sent_time = time.time()

        # Affichage de l'angle sur l'image
        cv2.putText(frame, f"Angle Servo: {servo_angle} deg", (10, 30),
                    cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    # Ligne repère au centre de l'écran (Bleue)
    cv2.line(frame, (center_x_screen, 0), (center_x_screen, height), (255, 0, 0), 1)

    # Affichage du flux vidéo
    cv2.imshow("Suivi de Visage IA - Tourelle Servo", frame)

    # Quitter la boucle avec la touche 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
ser.close()
cv2.destroyAllWindows()